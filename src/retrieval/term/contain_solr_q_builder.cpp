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

#include "retrieval/term/contain_solr_q_builder.h"

namespace anyq {

int ContainSolrQBuilder::init(DualDictWrapper* dict, const SolrQConfig& solr_q_config) {
    init_base(solr_q_config.name(), solr_q_config.solr_field(), solr_q_config.source_name());
    _p_dual_dict_wrapper = dict;
    return 0;
}

int ContainSolrQBuilder::term_contain(const std::string& term,
        std::string& contain_terms,
        std::string& exclude_terms) {
    float weight = 1.0;
    hashmap_str2float* black_white_list = (hashmap_str2float*)_p_dual_dict_wrapper->get_dict();
    if (black_white_list->count(term) > 0) {
        weight = (*black_white_list)[term];
    } else {
        return 0;
    }
    if (weight < 0) {
        // term命中黑名单
        exclude_terms += escape(term);
        exclude_terms.append(" ");
    } else {
        // term命中白名单
        contain_terms += escape(term);
        contain_terms.append(" ");
    }

    return 0;
}

int ContainSolrQBuilder::make_q(const AnalysisResult& analysis_res, int analysis_idx, std::string& q) {
    q = "";
    std::string contain_terms = "";
    std::string exclude_terms = "";
    const std::string& source_name = get_source_name();
    if (source_name == "basic_token") {
        for (uint32_t j = 0; j < analysis_res.analysis[analysis_idx].tokens_basic.size(); j++) {
            term_contain(analysis_res.analysis[analysis_idx].tokens_basic[j].buffer,
                    contain_terms, exclude_terms);
        }
    } 

    if (contain_terms != "") {
        q.append("+");
        q += source_name;
        q.append(":(");
        q += contain_terms;
        q.append(")");
    }
    if (exclude_terms != "") {
        q.append("-");
        q += source_name;
        q.append(":(");
        q += exclude_terms;
        q.append(")");
    }
    return 0;
}

} // namespace anyq
