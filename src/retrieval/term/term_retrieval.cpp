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

#include "retrieval/term/term_retrieval.h"
#include <json/json.h>
#include <sstream>
#include <iostream>
#include "common/plugin_factory.h"

namespace anyq {
//term检索插件初始化
int TermRetrievalPlugin::init(DictMap* dict_map, const RetrievalPluginConfig& plugin_config) {
    if (!plugin_config.has_search_host()) {
        FATAL_LOG("RetrievalPluginConfig.search_host unset!");
        return -1;
    }
    if (!plugin_config.has_search_port()) {
        FATAL_LOG("RetrievalPluginConfig.search_port unset!");
        return -1;
    }
    if (!plugin_config.has_engine_name()) {
        FATAL_LOG("RetrievalPluginConfig.engine_name unset!");
        return -1;
    }
    if (!plugin_config.has_solr_result_fl()) {
        FATAL_LOG("RetrievalPluginConfig.solr_result_fl unset!");
        return -1;
    }
    if (plugin_config.solr_q_size() < 1) {
        FATAL_LOG("RetrievalPluginConfig.solr_q unset!");
        return -1;
    }
    _search_host = plugin_config.search_host();
    _search_port = plugin_config.search_port();
    _engine_name = plugin_config.engine_name();
    _solr_result_fl = plugin_config.solr_result_fl();
    // 基类初始化
    init_base(plugin_config.name(), plugin_config.num_result());

    for (int i = 0; i < plugin_config.solr_q_size(); i++) {
        // 获取并初始化term索引所需要的solr参数构造插件
        SolrQConfig solr_q_config = plugin_config.solr_q(i);
        std::string solr_q_type = solr_q_config.type();
        std::string solr_q_name = solr_q_config.name();

        DualDictWrapper* dict_ptr = NULL;
        if (solr_q_config.has_weight_dict()) {
            std::string weight_dict_name = solr_q_config.weight_dict();
            if (dict_map->count(weight_dict_name) < 1) {
                FATAL_LOG("using dict %s that does not exist", weight_dict_name.c_str());
                return -1;
            }
            dict_ptr = (*dict_map)[weight_dict_name];
        }

        SolrQInterface* ptr_solr_q_builder = 
                static_cast<SolrQInterface*>(PLUGIN_FACTORY.create_plugin(solr_q_type));
        if (ptr_solr_q_builder == NULL) {
            FATAL_LOG("can't find builder solr_q_type:%s", solr_q_type.c_str());
            return -1;
        }
        if (ptr_solr_q_builder->init(dict_ptr, solr_q_config) != 0) {
            FATAL_LOG("init solr q builder error %s", solr_q_name.c_str());
            return -1;
        }
        TRACE_LOG("create solr q builder %s success", solr_q_name.c_str());
        _solr_q_builder.push_back(ptr_solr_q_builder);
    }

    return 0;
}

int TermRetrievalPlugin::destroy() {
    return 0;
}

// 构造检索solr所需的q参数
int TermRetrievalPlugin::make_fetch_q(const AnalysisResult& analysis_result, int analysis_idx, std::string& q) {
    // 各分支分别召回
    std::vector<SolrQInterface*>::iterator it;
    for (it = _solr_q_builder.begin(); it != _solr_q_builder.end(); ++it) {
        std::string sub_q = "";
        if ((*it)->make_q(analysis_result, analysis_idx, sub_q) != 0) {
            FATAL_LOG("plugin %s retrieval error", ((*it)->plugin_name()).c_str());
            return -1;
        }
        if (sub_q == "") {
            continue;
        }
        if (q == "") {
            q += sub_q;
        } else {
            q.append(" AND ");
            q += sub_q;
        }
    }

    DEBUG_LOG("solr_fetch_q=%s", q.c_str());
    return 0;
}

// 检索solr
int TermRetrievalPlugin::solr_request(const char* q, std::string* buffer)
{
    char url[URL_LENGTH];
    snprintf(url, URL_LENGTH, "http://%s:%d/solr/%s/select", _search_host.c_str(),
            _search_port, _engine_name.c_str());
    DEBUG_LOG("url = %s", url);

    std::map<std::string, std::string> para_map;
    para_map["wt"] = "json";
    std::stringstream ss;
    ss << get_num_result();
    para_map["rows"] = ss.str();
    para_map["fl"] = _solr_result_fl;
    para_map["q"] = url_encode(q);

    if (_http_client.curl_post(url, para_map, buffer) != 0) {
        return -1;
    }

    return 0;
}

// 对solr返回结构进行解析
int TermRetrievalPlugin::solr_result_parse(const char* solr_result, RetrievalResult& retrieval_res) {
    if (solr_result == NULL) {
        WARNING_LOG("solr_result is NULL");
        return -1;
    }

    // solr result json
    Json::Reader reader;
    Json::Value json_result;
    if (!reader.parse(solr_result, json_result)) {
        WARNING_LOG("solr_result[%s] json parse error;", solr_result);
        return -1;
    }
    if (json_result.type() != Json::objectValue) {
        WARNING_LOG("solr_result[%s] json parse error;", solr_result);
        return -1;
    }

    // response
    if (!json_result.isMember("response") || json_result["response"].isNull()) {
        WARNING_LOG("solr_result[%s], response not exist;", solr_result);
        return -1;
    }
    Json::Value response = json_result["response"];
    if (!response.isObject()) {
        WARNING_LOG("solr_result[%s], response is not object;", solr_result);
        return -1;
    }

    // response->docs
    if (!response.isMember("docs") || response["docs"].isNull()) {
        WARNING_LOG("solr_result[%s], docs not exist;", solr_result);
        return -1;
    }
    Json::Value docs = response["docs"];
    if (!docs.isArray()) {
        WARNING_LOG("solr_result[%s], docs is not array;", solr_result);
        return -1;
    }
    
    // 遍历docs，读取信息
    for (uint32_t i = 0; i < docs.size(); i++) {
        RetrievalItem retrieval_item;
        if (json2retrieval_item(docs[i], retrieval_item) != 0) {
            return -1;
        }
        retrieval_res.items.push_back(retrieval_item);
        DEBUG_LOG("Term retrieval item id=%s, query=%s;", retrieval_item.query.id.c_str(),
                retrieval_item.query.text.c_str());
    }
    DEBUG_LOG("keyword retrieval result len=%d", retrieval_res.items.size());

    return 0;
}

int TermRetrievalPlugin::retrieval(const AnalysisResult& analysis_result, RetrievalResult& retrieval_res) {
    RetrievalResult tmp;
    for (uint32_t i = 0; i < analysis_result.analysis.size(); i++) {
        std::string solr_fetch_q = "";
        // 获取solr检索请求
        if (make_fetch_q(analysis_result, i, solr_fetch_q) != 0) {
            WARNING_LOG("make_fetch_q failed.");
            return -1;
        }

        std::string solr_result_buffer = "";
        // 向solr检索服务发请求
        if (solr_request(solr_fetch_q.c_str(), &solr_result_buffer) != 0) {
            WARNING_LOG("solr_request failed.");
            return -1;
        }

        tmp.items.clear();
        // 解析solr服务返回的检索结果
        if (solr_result_parse(solr_result_buffer.c_str(), tmp) != 0) {
            WARNING_LOG("solr_result_parse failed.");
            return -1;
        }

        retrieval_res.items.insert(retrieval_res.items.end(), tmp.items.begin(), tmp.items.end());
    }

    return 0;
}

} // namespace anyq
