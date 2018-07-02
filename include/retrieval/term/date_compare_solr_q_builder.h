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

#ifndef BAIDU_NLP_ANYQ_DATE_COMPARE_SOLR_Q_BUILDER_H
#define BAIDU_NLP_ANYQ_DATE_COMPARE_SOLR_Q_BUILDER_H

#include "retrieval/term/solr_q_interface.h"

namespace anyq {
// 时间限定的solr索引表达式构造插件
class DateCompareSolrQBuilder : public SolrQInterface{
public:
    DateCompareSolrQBuilder() {};
    virtual ~DateCompareSolrQBuilder() override {};
    virtual int init(DualDictWrapper* dict, const SolrQConfig& solr_q_config) override;
    virtual int make_q(const AnalysisResult& analysis_res,
            int analysis_idx, 
            std::string& q) override;

private:
    std::string _compare_type;
    DISALLOW_COPY_AND_ASSIGN(DateCompareSolrQBuilder);
};

} // namespace anyq

#endif  // BAIDU_NLP_ANYQ_DATE_COMPARE_SOLR_Q_BUILDER_H
