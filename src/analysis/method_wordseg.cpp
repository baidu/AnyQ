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

#include "analysis/method_wordseg.h"
#include <exception>

namespace anyq{

AnalysisWordseg::AnalysisWordseg(){
}

AnalysisWordseg::~AnalysisWordseg(){
    destroy();
}

int AnalysisWordseg::init(DualDictWrapper* dict, const AnalysisMethodConfig& analysis_method)
{
    _p_wordseg_pack = (WordsegPack*)dict->get_dict();
    _lexer_buff = NULL;
    _lexer_buff = lac_buff_create(_p_wordseg_pack->lexer_dict);

    if (_lexer_buff == NULL) {
        FATAL_LOG("error init lexer_buff = thread");
        return -1;
    }

    set_method_name(analysis_method.name());
    TRACE_LOG("init wordseg success");
    return 0;
}

int AnalysisWordseg::destroy(){
    if (_lexer_buff != NULL) {
        lac_buff_destroy(_p_wordseg_pack->lexer_dict, _lexer_buff);
        _lexer_buff = NULL;
    }
    return 0;
};

int AnalysisWordseg::single_process(AnalysisItem& analysis_item) {
    const char* c_query = analysis_item.query.c_str();
    int basic_tk_num = -1;
    try {
        basic_tk_num = lac_tagging(_p_wordseg_pack->lexer_dict,
                _lexer_buff,
                c_query,
                _basic_tokens,
                MAX_TERM_COUNT);
    } catch (std::exception& e) {
        FATAL_LOG("wordseg segment error.");
        return -1;
    }

    if (basic_tk_num < 0) {
        FATAL_LOG("wordseg segment error.");
        return -1;
    }

    int ret = array_tokens_conduct(_basic_tokens, 
            basic_tk_num, 
            analysis_item.tokens_basic,
            analysis_item.query);
    if (ret != 0) {
        FATAL_LOG("analysis segment token convert error.");
        return -1;
    }

    return 0;
};

} // namespace anyq
