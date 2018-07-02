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

#include "server/http_server.h"
#include "server/session_data_factory.h"

namespace brpc {
    DECLARE_bool(usercode_in_pthread);
}

namespace anyq {

HttpServer::HttpServer() {
}

HttpServer::~HttpServer() {
}

int HttpServer::init(std::string& http_server_conf) {
    if (load_config_from_file(http_server_conf, _server_config) != 0) {
        fprintf(stderr, "load_config_from_file failed\n");
        return -1;
    }
    _idle_timeout_sec = _server_config.idle_timeout_sec();
    _max_concurrency = _server_config.max_concurrency();
    _port = _server_config.port();
    _server_conf_dir = _server_config.server_conf_dir();
    //_log_conf_file = _server_config.log_conf_file();
    _anyq_dict_conf_dir = _server_config.anyq_dict_conf_dir();
    _anyq_conf_dir = _server_config.anyq_conf_dir();
    if (_server_config.has_solr_clear_passwd()) {
        _solr_clear_passwd = _server_config.solr_clear_passwd();
    }

    /*int ret = com_loadlog(_server_conf_dir.c_str(), _log_conf_file.c_str());
    if (ret != 0) {
        fprintf(stderr, "load log conf error\n");
        return -1;
    }*/
    // 启动时，加载词典
    if (_dict_manager.load_dict(_anyq_dict_conf_dir) != 0) {
        FATAL_LOG("load dict error, _anyq_dict_conf_dir=%s", _anyq_dict_conf_dir.c_str());
        return -1;
    }

    return 0;
}

int HttpServer::start() {
    // SessionDataFactory声明, 用于创建sessiondata
    SessionDataFactory session_data_factory(
        &_dict_manager, _anyq_conf_dir, _solr_clear_passwd);

    // server options
    brpc::ServerOptions options;
    options.idle_timeout_sec = _idle_timeout_sec;
    options.max_concurrency = _max_concurrency;
    options.session_local_data_factory = &session_data_factory;

    // Instance of your service.
    HttpServiceImpl http_service_impl;
    if (http_service_impl.init(_server_config) != 0) {
        WARNING_LOG("http_service_impl init failed: req pre/post processor conf issue.");
        return -1;
    }
    // Add the service into server.
    int ret = _server.AddService(&http_service_impl,
                brpc::SERVER_DOESNT_OWN_SERVICE,
                "/anyq => anyq,"
                "/solr/v1/delete => solr_delete,"
                "/solr/v1/insert => solr_insert,"
                "/solr/v1/update => solr_update,"
                "/solr/v1/clear => solr_clear");
    if (ret != 0) {
        FATAL_LOG("Fail to add service to server");
        return -1;
    }
    // Start the server.
    if (_server.Start(_port, &options) != 0) {
        FATAL_LOG("Fail to start HttpServer");
        return -1;
    }

    return 0;
}

int HttpServer::always_run() {
    // 开辟栈空间
    brpc::FLAGS_usercode_in_pthread = true;

    // SessionDataFactory声明, 用于创建sessiondata
    SessionDataFactory session_data_factory(
        &_dict_manager, _anyq_conf_dir, _solr_clear_passwd);

    // server options
    brpc::ServerOptions options;
    options.idle_timeout_sec = _idle_timeout_sec;
    options.max_concurrency = _max_concurrency;
    options.session_local_data_factory = &session_data_factory;

    // Instance of your service.
    HttpServiceImpl http_service_impl;
    if (http_service_impl.init(_server_config) != 0) {
        WARNING_LOG("http_service_impl init failed: req pre/post processor conf issue.");
        return -1;
    }
    // Add the service into server.
    int ret = _server.AddService(&http_service_impl,
                brpc::SERVER_DOESNT_OWN_SERVICE,
                "/anyq => anyq,"
                "/solr/v1/delete => solr_delete,"
                "/solr/v1/insert => solr_insert,"
                "/solr/v1/update => solr_update,"
                "/solr/v1/clear => solr_clear");
    if (ret != 0) {
        FATAL_LOG("Fail to add service to server");
        return -1;
    }
    // Start the server.
    if (_server.Start(_port, &options) != 0) {
        FATAL_LOG("Fail to start HttpServer");
        return -1;
    }
    // Wait until Ctrl-C is pressed, then Stop() and Join() the server.
    _server.RunUntilAskedToQuit();
    return 0;
}

} // namespace anyq
