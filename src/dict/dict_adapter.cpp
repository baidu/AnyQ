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

#include "dict/dict_adapter.h"
#include <fstream>

namespace anyq {

String2RetrievalItemAdapter::String2RetrievalItemAdapter(){
    hashmap_str2retrieval_item* tmp_dict = new hashmap_str2retrieval_item();
    if (tmp_dict == NULL) {
        FATAL_LOG("new str2retrieval_item dict error");
    }
    DEBUG_LOG("new str2retrieval_item dict suecess");
    set_dict((void*)tmp_dict);
}

String2RetrievalItemAdapter::~String2RetrievalItemAdapter(){
    void* dict = get_dict();
    if (dict != NULL) {
        delete static_cast<hashmap_str2retrieval_item*>(dict);
    }
}

int String2RetrievalItemAdapter::load(const std::string& path, const DictConfig& config) {
    DEBUG_LOG("%s", path.c_str());
    hashmap_str2retrieval_item* tmp_dict = static_cast<hashmap_str2retrieval_item*>(get_dict());
    int ret = str2retrieval_item_load(path.c_str(), (*tmp_dict));
    if (ret != 0) {
        FATAL_LOG("load string2retrieval_item dict %s error", path.c_str());
        return -1;
    }
    return 0;
}

// 查找
int String2RetrievalItemAdapter::get(const std::string& key, RetrievalItem& retrieval_item) {
    hashmap_str2retrieval_item* tmp_dict = static_cast<hashmap_str2retrieval_item*>(get_dict());
    hashmap_str2retrieval_item::const_iterator it = tmp_dict->find(key);
    if (it != tmp_dict->end()) {
        retrieval_item = it->second;
    } else {
        WARNING_LOG("String2RetrievalItemAdapter key[%s] not exist!", key.c_str());
        return -1;
    }
    return 0;
}

int String2RetrievalItemAdapter::release() {
    void* dict = get_dict();
    if (dict != NULL) {
        hashmap_str2retrieval_item* tmp_dict = static_cast<hashmap_str2retrieval_item*>(dict);
        tmp_dict->clear();
    }
    return 0;
}

}
