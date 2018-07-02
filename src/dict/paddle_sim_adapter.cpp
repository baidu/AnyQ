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
#include "dict/dict_adapter.h"

#include <stdlib.h>
#include <exception>
#include <stdio.h>
#include "common/utils.h"

namespace anyq {

bool PaddleSimAdapter::_s_initialized = false;

PaddleSimAdapter::PaddleSimAdapter() {
}

PaddleSimAdapter::~PaddleSimAdapter() {
}

int PaddleSimAdapter::load(const std::string& path, const DictConfig& config) {
    int ret = -1;
    std::string name = config.name();

    PaddlePack* p_paddle_pack = NULL;
    p_paddle_pack = new PaddlePack();
    if (NULL == p_paddle_pack){
        FATAL_LOG("PaddlePack malloc error");
        return -1;
    }

    std::string dict_path = path + "/term2id.dict"; 
    ret = hash_load(dict_path.c_str(), p_paddle_pack->term2id);    
    if (ret != 0) {
        FATAL_LOG("paddle term2id dict load error");
        return -1;
    }

    p_paddle_pack->using_num = 0;
    try{
        paddle::framework::InitDevices(false);
        p_paddle_pack->place = new paddle::platform::CPUPlace();
        paddle::framework::InitDevices(false);
        p_paddle_pack->executor = new paddle::framework::Executor(*(p_paddle_pack->place));
        p_paddle_pack->scope = new paddle::framework::Scope();
        p_paddle_pack->inference_program = paddle::inference::Load(p_paddle_pack->executor,
                p_paddle_pack->scope,
                path + "/model");
    }catch(std::exception& e){
        FATAL_LOG("paddle dict load error: %s", e.what());
        delete p_paddle_pack;
        return -1;
    }
    TRACE_LOG("paddle model %s load success", name.c_str());
    set_dict((void*)p_paddle_pack);
    return 0;
}

int PaddleSimAdapter::release() {
    void* dict = get_dict();
    if (dict != NULL) {
        PaddlePack* p_paddle_pack = static_cast<PaddlePack*>(dict);
        delete p_paddle_pack->scope;
        delete p_paddle_pack->executor;
        delete p_paddle_pack->place;
        delete p_paddle_pack;
        set_dict(NULL);
    }
    return 0;
}

} // namespace anyq
#endif
