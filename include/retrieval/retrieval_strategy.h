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

#ifndef BAIDU_NLP_ANYQ_RETRIEVAL_STRATEGY_H
#define BAIDU_NLP_ANYQ_RETRIEVAL_STRATEGY_H

#include "retrieval/retrieval_interface.h"
#include "dict/dict_manager.h"

namespace anyq {
// 检索策略类,检索模块的入口 
class RetrievalStrategy{
public:
    RetrievalStrategy();
    ~RetrievalStrategy();
    int init(DictMap* dict_map, const std::string& retrieval_conf);
    int destroy();
    // 删除检索出的重复query
    int rm_duplicate_query(RetrievalResult& retrieval_result);
    // 总的召回策略流程
    int run_strategy(const AnalysisResult& analysis_result, RetrievalResult& retrieval_result);

private:
    // 存放检索插件
    std::vector<RetrievalPluginInterface*> _retrieval_plugins;
    DISALLOW_COPY_AND_ASSIGN(RetrievalStrategy);
};

} // namespace anyq

#endif  // BAIDU_NLP_ANYQ_RETRIEVAL_STRATEGY_H
