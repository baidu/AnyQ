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

#include "retrieval/term/date_compare_solr_q_builder.h"

namespace anyq {

int DateCompareSolrQBuilder::init(DualDictWrapper* dict, const SolrQConfig& solr_q_config) {
    init_base(solr_q_config.name(), solr_q_config.solr_field(), solr_q_config.source_name());
    _compare_type = solr_q_config.compare_type();
    return 0;
}

int DateCompareSolrQBuilder::make_q(const AnalysisResult& analysis_res, int analysis_idx, std::string& q) {
    q = "";
    std::map<std::string, std::string>::const_iterator it;
    const std::string& source_name = get_source_name();
    it = analysis_res.info.find(source_name);
    // 构造时间比较字段
    std::string field_value = "";
    if (it != analysis_res.info.end()) {
        if (_compare_type == "before") {
            field_value.append("[");
            field_value.append(it->second);
            field_value.append(" TO *]");
        }else if (_compare_type == "after") {
            field_value.append("[* TO ");
            field_value.append(it->second);
            field_value.append("]");
        }else{
            FATAL_LOG("compare type[%s] is invalid", _compare_type.c_str());
        }

    } else {
        FATAL_LOG("search filed[%s] not exist in analysis info_map", source_name.c_str());
        return -1;
    }
    q += get_solr_field();
    q.append(":");
    q += field_value;

    DEBUG_LOG("date compare solr_fetch_q=%s", q.c_str());
    return 0;
}

} // namespace anyq
