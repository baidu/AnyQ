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

#include "retrieval/semantic/semantic_retrieval.h"

namespace anyq {

// 语义索引插件线程初始化
int SemanticRetrievalPlugin::init(DictMap* dict_map, const RetrievalPluginConfig& plugin_config) {
    init_base(plugin_config.name(), plugin_config.num_result());
    if (!plugin_config.has_vector_size()) {
        FATAL_LOG("RetrievalPluginConfig.%s unset!", "vector_size");
        return -1;
    }
    if (!plugin_config.has_index_path()) {
        FATAL_LOG("RetrievalPluginConfig.%s unset!", "index_path");
        return -1;
    }
    if (!plugin_config.has_using_dict_name()) {
        FATAL_LOG("RetrievalPluginConfig.%s unset!", "using_dict_name");
        return -1;
    }
    if (!plugin_config.has_search_k()) {
        FATAL_LOG("RetrievalPluginConfig.%s unset!", "search_k");
        return -1;
    }
    _index_path = plugin_config.index_path();
    _vector_size = plugin_config.vector_size();
    _search_k = plugin_config.search_k();

    // 构建annoy语义索引树
    _annoy_index = new AnnoyIndex<int, float, Angular, Kiss32Random>(_vector_size);
    if (!_annoy_index->load(_index_path.c_str())) {
        FATAL_LOG("AnnoyIndex load from %s failed!", _index_path.c_str());
        return -1;
    }

    if (_vector_size < 1 || _search_k < 1) {
        FATAL_LOG("vector_size=%d, search_k=%d", _vector_size, _search_k);
        return -1;
    }

    // 设置annoy需要的知识库词典
    if (dict_map == NULL) {
        FATAL_LOG("dict_map is null");
        return -1;
    }
    std::string retrieval_dict_name = plugin_config.using_dict_name();
    if (dict_map->count(retrieval_dict_name) < 1) {
        FATAL_LOG("using dict %s that does not exist", retrieval_dict_name.c_str());
        return -1;
    }
    _knowledge_dict = (String2RetrievalItemAdapter*)((*dict_map)[retrieval_dict_name]->get());

    return 0;
}

int SemanticRetrievalPlugin::destroy() {
    // 销毁annoy语义索引树
    delete _annoy_index;
    return 0;
}

// 召回
int SemanticRetrievalPlugin::retrieval(const AnalysisResult& analysis_result, RetrievalResult& retrieval_res) {
    std::vector<int> closest;
    for (uint32_t i = 0; i < analysis_result.analysis.size(); i++) {
        closest.clear();
        size_t emb_size = analysis_result.analysis[i].query_emb.size();
        if (_vector_size != emb_size) {
            FATAL_LOG("analysis query emb size=%d, _vector_size=%d", emb_size, _vector_size);
            return -1;
        }
        _annoy_index->get_nns_by_vector(&analysis_result.analysis[i].query_emb[0],
                get_num_result(), _search_k, &closest, NULL);
        DEBUG_LOG("Semantic retrieval result len=%d;", closest.size());
        RetrievalItem retrieval_item;
        for (uint32_t i = 0; i < closest.size(); i++) {
            std::stringstream ss;
            ss << closest[i];
            if (_knowledge_dict->get(ss.str(), retrieval_item) != 0) {
                continue;
            }
            retrieval_res.items.push_back(retrieval_item);
            DEBUG_LOG("Semantic retrieval item id=%s, query=%s;", retrieval_item.query.id.c_str(),
                    retrieval_item.query.text.c_str());
        }
    }

    return 0;
}

} // namespace anyq
