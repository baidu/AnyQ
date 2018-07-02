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

#ifndef BAIDU_NLP_ANYQ_SEMANTIC_RETRIEVAL_H
#define BAIDU_NLP_ANYQ_SEMANTIC_RETRIEVAL_H

#include "annoylib.h"
#include "kissrandom.h"
#include "retrieval/retrieval_interface.h"
#include "dict/dict_adapter.h"

namespace anyq {
// 语义索引插件
class SemanticRetrievalPlugin : public RetrievalPluginInterface{
public:
    SemanticRetrievalPlugin() {};
    virtual ~SemanticRetrievalPlugin() override {};
    virtual int init(DictMap* dict_map, const RetrievalPluginConfig& plugin_config) override;
    virtual int destroy() override;
    virtual int retrieval(const AnalysisResult& analysis_res,
            RetrievalResult& retrieval_res) override;

private:
    // annoy索引字典路径
    std::string _index_path;
    // query语义向量维度
    uint32_t _vector_size;
    uint32_t _search_k;
    // annoy语义索引
    AnnoyIndex<int, float, Angular, Kiss32Random>* _annoy_index;
    // id到候选的映射知识库
    String2RetrievalItemAdapter* _knowledge_dict;
    DISALLOW_COPY_AND_ASSIGN(SemanticRetrievalPlugin);
};

} // namespace anyq
#endif  // BAIDU_NLP_ANYQ_SEMANTIC_RETRIEVAL_H
