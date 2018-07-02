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

#ifdef USE_TENSORFLOW
#include "dict/dict_adapter.h"
#include "common/utils.h"

namespace anyq {

TFModelAdapter::TFModelAdapter() {} 

TFModelAdapter::~TFModelAdapter() {}

int TFModelAdapter::load(const std::string& path, const DictConfig& config) {
    TFPack* p_tf_pack = new TFPack();
    std::string dict_path = path + "/term2id";
    int ret = hash_load(dict_path.c_str(), p_tf_pack->term2id);
    if (ret != 0) {
        FATAL_LOG("lego term2id dict load error");
        return -1;
    }
    // tf训练之后将graph 和weights合并到一个文件
    std::string model_path = path + "/tf.graph";
    tensorflow::Status status_load = tensorflow::ReadBinaryProto(
                                tensorflow::Env::Default(), 
                                model_path, 
                                &p_tf_pack->graphdef);
    if (!status_load.ok()) {
        FATAL_LOG("lego tensorflow model error");
        return -1;
    }
    TRACE_LOG("tf model load success");
    set_dict((void*)p_tf_pack);
    return 0;
}

int TFModelAdapter::release() {
    void* dict = get_dict();
    if (dict != NULL) {
        TFPack* p_tf_pack = static_cast<TFPack*>(dict);
        delete p_tf_pack;
        set_dict(NULL);
    }
    return 0;
}

} // namespace anyq

#endif
