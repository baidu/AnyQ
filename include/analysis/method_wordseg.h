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

#ifndef BAIDU_NLP_ANYQ_METHOD_WORDSEG_H
#define BAIDU_NLP_ANYQ_METHOD_WORDSEG_H

#include "analysis/method_interface.h"

namespace anyq {

class AnalysisWordseg: public AnalysisMethodInterface {
public:
    AnalysisWordseg();
    virtual ~AnalysisWordseg() override;
    virtual int init(DualDictWrapper* dict, const AnalysisMethodConfig& analysis_method) override;
    virtual int destroy() override;
    virtual int single_process(AnalysisItem& analysis_item) override;

private:
    WordsegPack* _p_wordseg_pack;
    void* _lexer_buff;
    tag_t _basic_tokens[MAX_TERM_COUNT];
    DISALLOW_COPY_AND_ASSIGN(AnalysisWordseg);
};

} // namespace anyq

#endif  // BAIDU_NLP_ANYQ_METHOD_WORDSEG_H
