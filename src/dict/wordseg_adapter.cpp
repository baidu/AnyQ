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
#include "common/utils.h"

namespace anyq {

WordsegAdapter::WordsegAdapter() {
}

WordsegAdapter::~WordsegAdapter() {
}

int WordsegAdapter::load(const std::string& path, const DictConfig& config) {
    WordsegPack *p_wordseg_pack = new WordsegPack();
    p_wordseg_pack->lexer_dict = NULL;
    p_wordseg_pack->lexer_dict = lac_create(path.c_str()); 
    if (p_wordseg_pack->lexer_dict == NULL)
    {
        FATAL_LOG("wordseg dict load error");
        return -1;
    }

    TRACE_LOG("wordseg dict load success.");
    set_dict((void*)p_wordseg_pack);
    return 0;
}

int WordsegAdapter::release() {
    void* dict = get_dict();
    if (dict != NULL) {
        WordsegPack* p_wordseg_pack = static_cast<WordsegPack*>(dict);
        lac_destroy(p_wordseg_pack->lexer_dict);
        p_wordseg_pack->lexer_dict = NULL;
        delete p_wordseg_pack;
        set_dict(NULL);
    }
    return 0;
}

} // namespace anyq
