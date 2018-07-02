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

#include "retrieval/manual/manual_retrieval.h"

namespace anyq {

int ManualRetrievalPlugin::init(DictMap* dict_map, const RetrievalPluginConfig& plugin_config) {
    // 基类初始化
    init_base(plugin_config.name(), plugin_config.num_result());
    // 设置人工干预字典
    if (dict_map == NULL) {
        FATAL_LOG("dict_map is null");
        return -1;
    }
    if (!plugin_config.has_using_dict_name()) {
        FATAL_LOG("RetrievalPluginConfig.%s unset!", "using_dict_name");
        return -1;
    }
    std::string q2a_dict_name = plugin_config.using_dict_name();
    if (dict_map->count(q2a_dict_name) < 1) {
        FATAL_LOG("using dict %s that does not exist", q2a_dict_name.c_str());
        return -1;
    }
    _p_dual_dict_wrapper = (*dict_map)[q2a_dict_name];
    return 0;
}

int ManualRetrievalPlugin::destroy() {
    return 0;
}

// 召回
int ManualRetrievalPlugin::retrieval(const AnalysisResult& analysis_result, RetrievalResult& retrieval_res) {
    // 干预词典支持reload，检索时动态获取词典
    hashmap_str2str* q2a_dict = (hashmap_str2str*)(_p_dual_dict_wrapper->get_dict());
    for (uint32_t i = 0; i < analysis_result.analysis.size(); i++) {
        if (q2a_dict->count(analysis_result.analysis[i].query) == 0) {
            continue;
        }
        TextInfo t_answer;
        t_answer.text  =  (*q2a_dict)[analysis_result.analysis[i].query];
        // 人工干预的检索结果优先级最高,query如果命中干预字典,将anyq_end设置为true,跳过其他的检索和rank
        retrieval_res.anyq_end = true;
        RetrievalItem retrieval_item;
        retrieval_item.query.text = analysis_result.analysis[i].query;
        retrieval_item.answer.push_back(t_answer);
        retrieval_res.items.push_back(retrieval_item);
    }

    return 0;
}

} // namespace anyq
