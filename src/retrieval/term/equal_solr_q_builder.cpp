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

#include "retrieval/term/equal_solr_q_builder.h"

namespace anyq {

int EqualSolrQBuilder::init(DualDictWrapper* dict, const SolrQConfig& solr_q_config) {
    init_base(solr_q_config.name(), solr_q_config.solr_field(), solr_q_config.source_name());
    return 0;
}

int EqualSolrQBuilder::make_q(const AnalysisResult& analysis_res,
        int analysis_idx,
        std::string& q) {
    q = "";
    const std::string& source_name = get_source_name();
    if (source_name == "question") {
        q += get_solr_field();
        q.append(":");
        q += escape(analysis_res.analysis[analysis_idx].query);
    } else {
        // 在info map中
        std::map<std::string, std::string>::const_iterator it;
        it = analysis_res.info.find(source_name);
        std::string field_value = "";
        if (it != analysis_res.info.end()) {
            field_value = escape(it->second); 
        } else {
            FATAL_LOG("search filed[%s] not exist in analysis info_map", source_name.c_str());
            return -1;
        }
        q += get_solr_field();
        q.append(":");
        q += escape(field_value);
    }
    DEBUG_LOG("equal solr_fetch_q=%s", q.c_str());

    return 0;
}

} // namespace anyq
