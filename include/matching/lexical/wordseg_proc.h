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

#ifndef BAIDU_NLP_ANYQ_WORDSEG_PROC_H
#define BAIDU_NLP_ANYQ_WORDSEG_PROC_H

#include "matching/matching_interface.h"

namespace anyq {
// 该特征插件仅对候选进行分词输出处理,不输出特征值,output_num应设置为0
class WordsegProcessor : public MatchingInterface {
public:
    WordsegProcessor();
    virtual ~WordsegProcessor() override;
    virtual int init(DualDictWrapper* dict, const MatchingConfig& matching_config) override;
    virtual int destroy() override;
    virtual int compute_similarity(const AnalysisResult& analysis_res,
            RankResult& candidates) override;
private:
    // 分词全局字典指针
    WordsegPack* _p_wordseg_pack;
    // 分词线程资源
    void* _lexer_buff;
    tag_t _basic_tokens[MAX_TERM_COUNT];
    DISALLOW_COPY_AND_ASSIGN(WordsegProcessor);
};

} // namespace anyq
#endif  // BAIDU_NLP_ANYQ_WORDSEG_PROC_H
