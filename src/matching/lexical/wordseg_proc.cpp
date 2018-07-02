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

#include "matching/lexical/wordseg_proc.h"
#include <exception>
#include "common/utils.h"

namespace anyq {

WordsegProcessor::WordsegProcessor(){
}
// 分词线程资源初始化
int WordsegProcessor::init(DualDictWrapper* dict, const MatchingConfig& matching_config) {
    _p_wordseg_pack = (WordsegPack*)dict->get_dict();
    _lexer_buff = NULL;
    _lexer_buff = lac_buff_create(_p_wordseg_pack->lexer_dict);
    if (_lexer_buff == NULL) {
        FATAL_LOG("error init lexer_buff thread");
        return -1;
    }
    set_output_num(0);
    TRACE_LOG("init wordseg_feature success");
    return 0;
}
// 销毁分词线程资源
int WordsegProcessor::destroy() {
    if (_lexer_buff != NULL) {
        lac_buff_destroy(_p_wordseg_pack->lexer_dict, _lexer_buff);
        _lexer_buff = NULL;
    }
    return 0;
}

int WordsegProcessor::compute_similarity(const AnalysisResult& analysis_res, RankResult& candidates) {
    for (size_t i = 0; i < candidates.size(); i++) {
        // 无效候选，跳过
        if (candidates[i].abandoned) {
            continue;
        }

        const char* c_query = candidates[i].match_info.text.c_str();
        // 调用分词接口
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
        // 将tag_t类型的token_t序列转化成analysis_token_t类型序列
        int ret = array_tokens_conduct(_basic_tokens,
                basic_tk_num, 
                candidates[i].match_info.tokens_basic,
                candidates[i].match_info.text);
        if (ret != 0) { 
            FATAL_LOG("matching segment token convert error.");
            return -1;
        }
    }
    return 0;
}

WordsegProcessor::~WordsegProcessor() {
    destroy();
}

} // namespace anyq
