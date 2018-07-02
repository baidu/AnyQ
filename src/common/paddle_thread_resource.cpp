// Copyright (c) 2018 Baidu, Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef USE_TENSORFLOW
#include "common/paddle_thread_resource.h"
#include <exception>

namespace anyq{

int PaddleThreadResource::init(PaddlePack* p_paddle_pack){
    _p_paddle_pack = p_paddle_pack;
    unsigned& using_num = _p_paddle_pack->using_num;
    std::mutex& dict_mutex = _p_paddle_pack->dict_mutex;
    try{
        {
            _feed_target_names = _p_paddle_pack->inference_program->GetFeedTargetNames();
            _fetch_target_names = _p_paddle_pack->inference_program->GetFetchTargetNames();
            std::lock_guard<std::mutex> lock(dict_mutex);
            _copy_program = std::unique_ptr<paddle::framework::ProgramDesc>(
                    new paddle::framework::ProgramDesc(*(_p_paddle_pack->inference_program)));
            int thread_id = using_num++;
            _feed_holder_name = "feed_" + paddle::string::to_string(thread_id);
            _fetch_holder_name = "fetch_" + paddle::string::to_string(thread_id);
            _copy_program->SetFeedHolderName(_feed_holder_name);
            _copy_program->SetFetchHolderName(_fetch_holder_name);
        }
    }catch(std::exception& e){
        FATAL_LOG("paddle thread init error: %s", e.what());
        return -1;
    }

    _feeds.resize(_feed_target_names.size());
    for (size_t i = 0; i < _feed_target_names.size(); ++i) {
        _feed_targets[_feed_target_names[i]] = &_feeds[i];
    }
    _fetchs.resize(_fetch_target_names.size());
    for (size_t i = 0; i < _fetch_target_names.size(); ++i) {
        _fetch_targets[_fetch_target_names[i]] = &_fetchs[i];
    }
    return 0;
}


int PaddleThreadResource::set_feed(const size_t& index,  const std::vector<int>& ids){
    paddle::framework::LoDTensor* input = NULL;
    if (_feeds.size() <= index){
        FATAL_LOG("set paddle feed index error: %d >= %d", index, _feeds.size());
        return -1;
    }
    input = &_feeds[index];
    paddle::framework::LoD q_lod{{0, ids.size()}};
    input->set_lod(q_lod);
    int64_t* id_ptr = input->mutable_data<int64_t>({ids.size(), 1},
            paddle::platform::CPUPlace());
    for (size_t i = 0; i < ids.size(); ++i){
        id_ptr[i] = static_cast<int64_t>(ids[i]);
    }
    return 0;
}

int PaddleThreadResource::run(){
    try{
        _p_paddle_pack->executor->Run(*_copy_program,
                &*(_p_paddle_pack->scope),
                &_feed_targets,
                &_fetch_targets,
                true,
                true,
                _feed_holder_name,
                _fetch_holder_name);
    }catch(std::exception& e){
        FATAL_LOG("paddle run error: %s", e.what());
        return -1;
    }
    return 0;
}

const float* PaddleThreadResource::get_fetch(const size_t& index){
    if (_fetchs.size() <= index){
        FATAL_LOG("get paddle fetch index error: %d >= %d", index, _fetchs.size());
        return NULL;
    }
    paddle::framework::LoDTensor* out = &_fetchs[index];
    float* output_data = out->data<float>();
    if (output_data == NULL){
        FATAL_LOG("paddle get fetch failed");
        return NULL;
    }
    return output_data;
}

}
#endif
