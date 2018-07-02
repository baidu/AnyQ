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

#include "retrieval/term/synonym_solr_q_builder.h"

namespace anyq {

int SynonymSolrQBuilder::init(DualDictWrapper* dict, const SolrQConfig& solr_q_config) {
    init_base(solr_q_config.name(), solr_q_config.solr_field(), solr_q_config.source_name());
    _p_dual_dict_wrapper = dict;
    return 0;
}

int SynonymSolrQBuilder::term_synonym(const std::string& term,
        std::string& synonym_terms) {
    hashmap_str2str* black_white_list = (hashmap_str2str*)_p_dual_dict_wrapper->get_dict();
    if (black_white_list->count(term) > 0) {
        std::string synonym_str = (*black_white_list)[term];
        std::vector<std::string> synonym_list;
        split_string(synonym_str, synonym_list, "|");
        for (size_t i = 0; i < synonym_list.size(); ++i) {
            synonym_terms += escape(synonym_list[i]);
            synonym_terms.append(" ");
        }
    }
    return 0;
}

int SynonymSolrQBuilder::make_q(const AnalysisResult& analysis_res, int analysis_idx, std::string& q) {
    q = "";
    std::string synonym_terms = "";
    const std::string& source_name = get_source_name();
    if (source_name == "basic_token") {
        for (uint32_t j = 0; j < analysis_res.analysis[analysis_idx].tokens_basic.size(); j++) {
            term_synonym(analysis_res.analysis[analysis_idx].tokens_basic[j].buffer,
                    synonym_terms);
        }
    } 
    if (synonym_terms != "") {
        q.append("+");
        q += source_name;
        q.append(":(");
        q += synonym_terms;
        q.append(")");
    }

    return 0;
}

} // namespace anyq
