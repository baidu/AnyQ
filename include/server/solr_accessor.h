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

#ifndef BAIDU_NLP_ANYQ_SOLR_ACCESSOR_H
#define BAIDU_NLP_ANYQ_SOLR_ACCESSOR_H

#include <brpc/uri.h>
#include "common/common_define.h"
#include "common/http_client.h"
#include "anyq.pb.h"

namespace anyq {

// 通过anyq操纵solr 数据接口，通过该接口，anyq可以提供统一对外服务的接口，包括问答检索、FAQ库的增删改等操作
class SolrAccessor {
public:
    SolrAccessor();
    ~SolrAccessor();

    int init(const std::string& conf_path, const std::string& solr_clear_passwd);
    int insert_doc(const Json::Value& param, std::string& result);
    int update_doc(const Json::Value& param, std::string& result);
    int delete_doc(const Json::Value& param, std::string& result);
    int clear_doc(const std::string& passwd, std::string& result);

private:
    // solr配置相关 
    HttpClient _http_client;
    char _solr_url[URL_LENGTH];
    std::string _buffer;
    std::string _solr_clear_passwd;

    static const std::string pack_error_msg(const std::string& msg);
    const std::string pack_str(std::string param_str);
    static int parse_request_result(const std::string& buffer, std::string& result);

    static int single_insert_param(const Json::Value&, std::string&, std::string&);
    static int single_update_param(const Json::Value&, std::string&, std::string&);
    static int single_delete_param(const Json::Value&, std::string&, std::string&);
    // 支持batch级 solr库操作
    int batch_param(int (*single_pf)(const Json::Value&, std::string&, std::string&),
            const Json::Value& param,
            std::string& batch_param_str,
            const std::string& sep,
            std::string& result);

    DISALLOW_COPY_AND_ASSIGN(SolrAccessor);
};

} // namespace anyq

#endif  // BAIDU_NLP_ANYQ_SOLR_ACCESSOR_H
