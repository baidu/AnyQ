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

#include "server/solr_accessor.h"
#include "common/common_define.h"
#include "common/utils.h"

namespace anyq {

SolrAccessor::SolrAccessor() {
}

SolrAccessor::~SolrAccessor() {
}

int SolrAccessor::init(const std::string& conf_path, const std::string& solr_clear_passwd) {
    AnyqStrategyConfig anyq_config;
    std::string anyq_conf_file = conf_path + std::string("/anyq.conf");
    if (load_config_from_file(anyq_conf_file, anyq_config) != 0) {
        WARNING_LOG("Wrong anyq config path!");
    }

    RetrievalConfig retrieval_config;
    std::string retrieval_conf = conf_path + anyq_config.retrieval_config();
    if (load_config_from_file(retrieval_conf, retrieval_config) != 0) {
        return -1;
    }
    bool url_set = false;
    for (int i = 0; i < retrieval_config.retrieval_plugin_size(); i++) {
        RetrievalPluginConfig plugin_config = retrieval_config.retrieval_plugin(i);
        if (plugin_config.type() != std::string("TermRetrievalPlugin")) {
            continue;
        }
        url_set = true;
        if (!plugin_config.has_search_host()) {
            WARNING_LOG("RetrievalPluginConfig.search_host unset!");
            return -1;
        }
        if (!plugin_config.has_search_port()) {
            WARNING_LOG("RetrievalPluginConfig.search_port unset!");
            return -1;
        }
        if (!plugin_config.has_engine_name()) {
            WARNING_LOG("RetrievalPluginConfig.engine_name unset!");
            return -1;
        }
        snprintf(_solr_url,
            URL_LENGTH,
            "http://%s:%d/solr/%s/update",
            plugin_config.search_host().c_str(),
            plugin_config.search_port(),
            plugin_config.engine_name().c_str());
        break;
    }
    if (!url_set) {
        WARNING_LOG("solr url has not been set!");
        return -1;
    }
    _solr_clear_passwd = solr_clear_passwd;
    return 0;
}

int SolrAccessor::parse_request_result(const std::string& buffer, std::string& result) {
    // parsing result error
    Json::Reader reader;
    Json::Value value;
    if (!reader.parse(buffer, value) || value.type() != Json::objectValue) {
        result = pack_error_msg("Solr Result Parsing Error.");
        return -1;
    }
    // if solr return error, pass it to user
    if (value.isMember("error")) {
        Json::Value json_item;
        json_item = value["error"];
        result = json_dumps(json_item);
        return -1;
    }
    Json::Value json_item;
    json_item["code"] = 0;
    json_item["msg"] = "Success";
    result = json_dumps(json_item);
    return 0;
}

const std::string SolrAccessor::pack_error_msg(const std::string& msg) {
    Json::Value json_item;
    json_item["code"] = -1;
    json_item["msg"] = msg;
    return json_dumps(json_item);
}

const std::string SolrAccessor::pack_str(std::string param_str) {
    std::string doc_string = "";
    doc_string.append("[");
    doc_string.append(param_str);
    doc_string.append("]");
    return doc_string;
}

int SolrAccessor::single_insert_param(const Json::Value& param,
                                      std::string& param_str,
                                      std::string& result) {
    if (!param.isMember("id")) {
        result = pack_error_msg("id Field Must Be Indicated.");
        return -1;
    }
    Json::Value document;
    Json::Value::Members mem = param.getMemberNames();
    for (Json::Value::Members::iterator it = mem.begin(); it != mem.end(); ++it) {
        document[*it] = param[*it];
    }
    param_str = json_dumps(document);
    return 0;
}

int SolrAccessor::single_update_param(const Json::Value& param,
                                      std::string& param_str,
                                      std::string& result) {
    if (!param.isMember("id")) {
        result = pack_error_msg("id Field Must Be Indicated.");
        return -1;
    }
    Json::Value document;
    Json::Value::Members mem = param.getMemberNames();
    for (Json::Value::Members::iterator it = mem.begin(); it != mem.end(); ++it) {
        if (*it == "id") {
            document[*it] = param[*it];
        } else {
            Json::Value set_field;
            set_field["set"] = param[*it];
            document[*it] = set_field;
        }
    }
    param_str = json_dumps(document);
    return 0;
}

int SolrAccessor::single_delete_param(const Json::Value& param,
                                      std::string& param_str,
                                      std::string& result) {
    if (!param.isMember("id")) {
        result = pack_error_msg("id Field Must Be Indicated.");
        return -1;
    }
    Json::Value::Members mem = param.getMemberNames();
    std::string condition = "";
    bool is_first = true;
    for (Json::Value::Members::iterator it = mem.begin(); it != mem.end(); ++it) {
        if (is_first) {
            is_first = false;
        }else{
            condition.append(" AND ");
        }
        condition.append(*it);
        condition.append(":");
        condition.append(param[*it].asString());
    }
    param_str = "(" + condition + ")";
    return 0;
}

int SolrAccessor::batch_param(int (*single_pf)(const Json::Value&, std::string&, std::string&),
                              const Json::Value& param,
                              std::string& batch_param_str,
                              const std::string& sep,
                              std::string& result) {

    if (param.type() == Json::objectValue) { // batch_size=1
        if (single_pf(param, batch_param_str, result) != 0) {
            return -1;
        }
    }else{ // batch insert
        bool is_first = true;
        batch_param_str = "";
        for (int i = 0; i < param.size(); ++i) {
            std::string param_str;
            if (single_pf(param[i], param_str, result) != 0) {
                return -1;
            }
            if (!is_first) {
                batch_param_str.append(sep);
            }else{
                is_first = false;
            }
            batch_param_str.append(param_str);
        }
    }
    return 0;
}

int SolrAccessor::insert_doc(const Json::Value& param, std::string& result) {
    const std::string sep = ",";
    int (*single_param_func)(const Json::Value&, std::string&, std::string&)
        = SolrAccessor::single_insert_param;
    std::string batch_param_str;
    if (this->batch_param(single_param_func, param, batch_param_str, sep, result) != 0) {
        return -1;
    }
    _buffer = "";
    if (_http_client.curl_post(_solr_url, pack_str(batch_param_str), &_buffer, HEADER_STR) != 0) {
        result = SolrAccessor::pack_error_msg("Curl Post Error");
        return -1;
    }
    return parse_request_result(_buffer, result);
}

int SolrAccessor::update_doc(const Json::Value& param, std::string& result) {
    const std::string sep = ",";
    int (*single_param_func)(const Json::Value&, std::string&, std::string&)
        = SolrAccessor::single_update_param;
    std::string batch_param_str;
    if (this->batch_param(single_param_func, param, batch_param_str, sep, result) != 0) {
        return -1;
    }
    _buffer = "";
    if (_http_client.curl_post(_solr_url, pack_str(batch_param_str), &_buffer, HEADER_STR) != 0) {
        result = SolrAccessor::pack_error_msg("Curl Post Error");
        return -1;
    }
    return parse_request_result(_buffer, result);
}

int SolrAccessor::delete_doc(const Json::Value& param, std::string& result) {
    const std::string sep = " OR ";
    int (*single_param_func)(const Json::Value&, std::string&, std::string&)
        = SolrAccessor::single_delete_param;
    std::string batch_param_str;
    if (this->batch_param(single_param_func, param, batch_param_str, sep, result) != 0) {
        return -1;
    }
    Json::Value document;
    Json::Value query;
    query["question"] = batch_param_str;
    document["delete"] = query;

    _buffer = "";
    if (_http_client.curl_post(_solr_url, json_dumps(document), &_buffer, HEADER_STR) != 0) {
        result = SolrAccessor::pack_error_msg("Curl Post Error");
        return -1;
    }
    return parse_request_result(_buffer, result);
}

int SolrAccessor::clear_doc(const std::string& passwd, std::string& result) {
    if (_solr_clear_passwd.empty()) {
        result = pack_error_msg("Password not set.");
        return -1;
    }else if (passwd != _solr_clear_passwd) {
        result = pack_error_msg("Password not correct.");
        return -1;
    }
    Json::Value document;
    Json::Value query;
    query["question"] = std::string("*:*");
    document["delete"] = query;

    _buffer = "";
    if (_http_client.curl_post(_solr_url, json_dumps(document), &_buffer, HEADER_STR) != 0) {
        result = SolrAccessor::pack_error_msg("Curl Post Error");
        return -1;
    }
    return parse_request_result(_buffer, result);
}

}
