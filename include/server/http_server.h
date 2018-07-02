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

#ifndef BAIDU_NLP_ANYQ_HTTP_SERVER_H
#define BAIDU_NLP_ANYQ_HTTP_SERVER_H

#include <string>
#include "anyq.pb.h"
#include "dict/dict_manager.h"
#include "server/http_service_impl.h"

namespace anyq {

class HttpServer {
public:
    HttpServer();
    ~HttpServer();

    int init(std::string& http_server_conf);
    // 启动服务器,启动后会退出,抽离出来方便单测
    int start();
    // 启动服务器,启动后会一直运行，直至ctrl-c
    int always_run();

private:
    int32_t _idle_timeout_sec;
    int32_t _max_concurrency;
    int32_t _port;
    // server配置
    std::string _server_conf_dir;
    std::string _log_conf_file;
    std::string _anyq_dict_conf_dir;
    std::string _anyq_conf_dir;
    ServerConfig _server_config;
    // solr 清空验证passwd
    std::string _solr_clear_passwd;
    // 进程级词典
    DictManager _dict_manager;
    brpc::Server _server;
    DISALLOW_COPY_AND_ASSIGN(HttpServer);
};

} // namespace anyq

#endif  // BAIDU_NLP_ANYQ_HTTP_SERVER_H
