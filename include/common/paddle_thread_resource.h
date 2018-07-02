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

#ifndef BAIDU_NLP_ANYQ_PADDLE_THREAD_RESOURCE_H
#define BAIDU_NLP_ANYQ_PADDLE_THREAD_RESOURCE_H

#ifndef USE_TENSORFLOW
#include "dict/dict_adapter.h"

namespace anyq {

class PaddleThreadResource{
public:
    PaddleThreadResource(){
        _p_paddle_pack = NULL;
    }
    ~PaddleThreadResource() {}
    int init(PaddlePack* p_paddle_pack);
    int destroy(){
        return 0;
    }
    int run();
    int set_feed(const size_t& index,  const std::vector<int>& ids);
    const float* get_fetch(const size_t& index);

    const std::vector<std::string>& get_feed_target_names(){
        return _feed_target_names;
    }

    const std::vector<std::string>& get_fetch_target_names(){
        return _fetch_target_names;
    }

private:
    PaddlePack* _p_paddle_pack;
    std::unique_ptr<paddle::framework::ProgramDesc> _copy_program;
    std::vector<std::string> _feed_target_names;
    std::vector<std::string> _fetch_target_names;
    std::string _feed_holder_name;
    std::string _fetch_holder_name;
    std::vector<paddle::framework::LoDTensor> _feeds;
    std::vector<paddle::framework::LoDTensor> _fetchs;
    std::map<std::string, const paddle::framework::LoDTensor*> _feed_targets;
    std::map<std::string, paddle::framework::LoDTensor*> _fetch_targets;
    DISALLOW_COPY_AND_ASSIGN(PaddleThreadResource);
};

} // namespace anyq
#endif

#endif
