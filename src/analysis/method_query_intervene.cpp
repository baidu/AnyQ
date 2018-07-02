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

#include "analysis/method_query_intervene.h"
#include "common/utils.h"

namespace anyq{

AnalysisQueryIntervene::AnalysisQueryIntervene(){
}

AnalysisQueryIntervene::~AnalysisQueryIntervene(){

}
int AnalysisQueryIntervene::init(DualDictWrapper* dict, const AnalysisMethodConfig& analysis_method){
    _dict = dict;
    TRACE_LOG("analysis_method_query_intervene init");
    set_method_name(analysis_method.name());
    return 0;

};

int AnalysisQueryIntervene::destroy(){
    TRACE_LOG("destroy analysis_query_intervene");
    return 0;

};

int AnalysisQueryIntervene::single_process(AnalysisItem& analysis_item) {
    TRACE_LOG("method_process analysis_method_query_intervene, query is: %s", analysis_item.query.c_str());
    // 每次调用，获取最新词典
    hashmap_str2str* tmp_dict = (hashmap_str2str*)(_dict->get_dict());
    if (tmp_dict->count(analysis_item.query) >= 1) {
        analysis_item.query = (*tmp_dict)[analysis_item.query];
    }
    TRACE_LOG("method_process analysis_method_query_intervene, query is: %s", analysis_item.query.c_str());
    return 0;

};

} // namespace anyq
