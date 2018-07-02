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

#ifndef BAIDU_NLP_ANYQ_CONTAIN_SOLR_Q_BUILDER_H
#define BAIDU_NLP_ANYQ_CONTAIN_SOLR_Q_BUILDER_H

#include "retrieval/term/solr_q_interface.h"

namespace anyq
{
// 黑、白名单term的solr表达式构造插件
class ContainSolrQBuilder : public SolrQInterface
{
public:
    ContainSolrQBuilder() {};
    virtual ~ContainSolrQBuilder() override {};
    virtual int init(DualDictWrapper* dict, const SolrQConfig& solr_q_config) override;
    int term_contain(const std::string& term,
            std::string& contain_terms,
            std::string& exclude_terms);
    virtual int make_q(const AnalysisResult& analysis_res,
            int analysis_idx,
            std::string& q) override;

private:
    DualDictWrapper *_p_dual_dict_wrapper;
    DISALLOW_COPY_AND_ASSIGN(ContainSolrQBuilder);
};

} // namespace anyq
#endif  // BAIDU_NLP_ANYQ_CONTAIN_SOLR_Q_BUILDER_H
