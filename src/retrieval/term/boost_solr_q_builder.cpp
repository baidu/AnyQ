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

#include "retrieval/term/boost_solr_q_builder.h"

namespace anyq {

int BoostSolrQBuilder::init(DualDictWrapper* dict, const SolrQConfig& solr_q_config) {
    init_base(solr_q_config.name(), solr_q_config.solr_field(), solr_q_config.source_name());
    _p_dual_dict_wrapper = dict;
    return 0;
}

int BoostSolrQBuilder::term_weight(const std::string& term,
        std::string& high_freq_token_q,
        std::string& low_freq_token_q_with_stopword,
        std::string& low_freq_token_q_without_stopword) {
    hashmap_str2float* term_weight_map = (hashmap_str2float*)_p_dual_dict_wrapper->get_dict();
    float weight = 1.0;
    if (term_weight_map->count(term) > 0) {
        weight = (*term_weight_map)[term];
    }
    // 停用词
    if (weight <= -1) {
        solr_wrapper(term, low_freq_token_q_with_stopword, 1.0);
        return 0;
    // 超高频词
    } else if (weight <= 0) {
        solr_wrapper(term, high_freq_token_q, 1.0);
        return 0;
    // 低频次
    } else {
        solr_wrapper(term, low_freq_token_q_with_stopword, weight);
        solr_wrapper(term, low_freq_token_q_without_stopword, weight);
    }

    return 0;
}

void BoostSolrQBuilder::solr_wrapper(const std::string& term, std::string& solr_q, float weight) {
    solr_q += escape(term);
    solr_q.append("^");
    solr_q += std::to_string(weight);
    solr_q.append(" ");
} 

int BoostSolrQBuilder::make_q(const AnalysisResult& analysis_res, int analysis_idx, std::string& q) {
    std::string high_freq_token_q = "";
    std::string low_freq_token_q_with_stopword = "";
    std::string low_freq_token_q_without_stopword = "";
    const std::string& source_name = get_source_name();
    if (source_name == "basic_token") {
        for (uint32_t j = 0; j < analysis_res.analysis[analysis_idx].tokens_basic.size(); j++) {
            term_weight(analysis_res.analysis[analysis_idx].tokens_basic[j].buffer, 
                    high_freq_token_q,
                    low_freq_token_q_with_stopword,
                    low_freq_token_q_without_stopword);
        }
    } 

    q = "";
    if (high_freq_token_q != "") {
        if (low_freq_token_q_without_stopword != "") {
            q.append("(");
            q += source_name;
            q.append(":(");
            q += low_freq_token_q_without_stopword;
            q.append(") AND ");
            q += source_name;
            q.append(":(");
            q += high_freq_token_q;
            q.append(")) OR ");
            q += source_name;
            q.append(":(");
            q += low_freq_token_q_without_stopword;
            q.append(")");
        } else if (low_freq_token_q_with_stopword != "") {
            q.append("(");
            q += source_name;
            q.append(":(");
            q += low_freq_token_q_with_stopword;
            q.append(") AND ");
            q += source_name;
            q.append(":(");
            q += high_freq_token_q;
            q.append(")) OR ");
            q += source_name;
            q.append(":(");
            q += low_freq_token_q_with_stopword;
            q.append(")");
        } else {
            q += source_name;
            q.append(":(");
            q += high_freq_token_q;
            q.append(")");
        }
    } else {
        if (low_freq_token_q_without_stopword != "") {
            q += source_name;
            q.append(":(");
            q += low_freq_token_q_without_stopword;
            q.append(")");
        } else if (low_freq_token_q_with_stopword != "") {
            q += source_name;
            q.append(":(");
            q += low_freq_token_q_with_stopword;
            q.append(")");
        }
    }

    return 0;
}

}
