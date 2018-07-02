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

#include "server/http_service_impl.h"
#include "brpc/http_method.h"
#include "server/session_data_factory.h"
#include "common/utils.h"
#include "server/solr_accessor.h"
#include "common/plugin_factory.h"
#include "anyq.pb.h"

namespace anyq {

HttpServiceImpl::HttpServiceImpl() : _preproc_plugin(NULL), _postproc_plugin(NULL) {
}

HttpServiceImpl::~HttpServiceImpl() {
    if (destroy() != 0) {
        FATAL_LOG("release HttpServiceImpl error");
    }
}

int HttpServiceImpl::init(const ServerConfig& server_config) {
    const ReqPreprocPluginConfig& preproc_plugin_config = server_config.preproc_plugin();
    const ReqPostprocPluginConfig& postproc_plugin_config = server_config.postproc_plugin();
    std::string pre_plugin_type = preproc_plugin_config.type();
    std::string pre_plugin_name = preproc_plugin_config.name();
    _preproc_plugin = 
        static_cast<ReqPreprocInterface*>(PLUGIN_FACTORY.create_plugin(pre_plugin_type));
    if (_preproc_plugin == NULL) {
        FATAL_LOG("can't find req preproc plugin_type:%s", pre_plugin_type.c_str());
        return -1;
    }
    if (_preproc_plugin->init(preproc_plugin_config) != 0) {
        FATAL_LOG("init req preproc plugin error %s", pre_plugin_name.c_str());
        return -1;
    }
    TRACE_LOG("create req preproc plugin %s success", pre_plugin_name.c_str());

    std::string post_plugin_type = postproc_plugin_config.type();
    std::string post_plugin_name = postproc_plugin_config.name();
    _postproc_plugin =
        static_cast<ReqPostprocInterface*>(PLUGIN_FACTORY.create_plugin(post_plugin_type));
    if (_postproc_plugin == NULL) {
        FATAL_LOG("can't find req postproc plugin_type:%s", post_plugin_type.c_str());
        return -1;
    }
    if (_postproc_plugin->init(server_config) != 0) {
        FATAL_LOG("init req postproc plugin error %s", post_plugin_name.c_str());
        return -1;
    }
    TRACE_LOG("create req postproc plugin %s success", post_plugin_name.c_str());
    return 0;
}

int HttpServiceImpl::destroy() {
    if (_postproc_plugin != NULL) {
        if (_postproc_plugin->destroy() != 0) {
            FATAL_LOG("req postproc plugin %s destroy error",
                _postproc_plugin->plugin_name().c_str());
            return -1;
        }
        delete _postproc_plugin;
        _postproc_plugin = NULL;
    }
    return 0;
}

int HttpServiceImpl::normalize_input(brpc::Controller* cntl, Json::Value& parameters) {
    brpc::HttpMethod req_method = cntl->http_request().method();
    if (req_method == brpc::HTTP_METHOD_GET) {
        brpc::URI* uri = &cntl->http_request().uri();
        for (brpc::URI::QueryIterator it = uri->QueryBegin(); it != uri->QueryEnd(); ++it) {
            parameters[it->first] = url_decode(it->second);
        }
    } else if (req_method == brpc::HTTP_METHOD_POST) {
        const std::string req_body = cntl->request_attachment().to_string();
        Json::Reader reader;
        if (!reader.parse(req_body, parameters)
            || (parameters.type() != Json::objectValue && parameters.type() != Json::arrayValue)) {
            FATAL_LOG("POST body Parsing Error.");
            return -1;
        }
    } else {
        FATAL_LOG("invalid http request method!");
        return -1;
    }
    return 0;
}

// semantic search
void HttpServiceImpl::anyq(google::protobuf::RpcController* cntl_base,
               const HttpRequest* request,
               HttpResponse*,
               google::protobuf::Closure* done) {
    brpc::ClosureGuard done_guard(done);
    brpc::Controller* cntl = static_cast<brpc::Controller*>(cntl_base);
    cntl->http_response().set_content_type("application/json;charset=utf8");

    // preprocess
    Json::Value parameters;
    if (this->normalize_input(cntl, parameters) != 0
        || parameters.type() != Json::objectValue) {
        FATAL_LOG("normalize input failed!");
        return;
    }

    std::string str_anyq_input;
    if (_preproc_plugin->process(cntl, parameters, str_anyq_input) != 0) {
        FATAL_LOG("make anyq input failed!");
        return;
    }
    // main processing
    SessionData* sd = static_cast<SessionData*>(cntl->session_local_data());
    if (sd == NULL) {
        cntl->SetFailed("Require ServerOptions.session_local_data_factory to be set!");
        FATAL_LOG("%s", cntl->ErrorText().c_str());
        return;
    }
    ANYQResult result;
    if (sd->get_anyq()->run_strategy(str_anyq_input, result) != 0) {
        FATAL_LOG("anyq run_strategy failed!");
        return;
    }
    // postprocess
    std::string output;
    if (_postproc_plugin->process(result, parameters, output) != 0) {
        FATAL_LOG("plugin %s process error", _postproc_plugin->plugin_name().c_str());
        return;
    }

    butil::IOBufBuilder os;
    os << output;
    os.move_to(cntl->response_attachment());
}

void HttpServiceImpl::solr_insert(google::protobuf::RpcController* cntl_base,
               const HttpRequest* request,
               HttpResponse*,
               google::protobuf::Closure* done) {
    // rpc framework env
    brpc::ClosureGuard done_guard(done);
    brpc::Controller* cntl = static_cast<brpc::Controller*>(cntl_base);
    cntl->http_response().set_content_type("application/json;charset=utf8");
    // preprocess
    Json::Value parameters;
    if (this->normalize_input(cntl, parameters) != 0) {
        FATAL_LOG("normalize input failed!");
        return;
    }
    SessionData* sd = static_cast<SessionData*>(cntl->session_local_data());
    if (sd == NULL) {
        cntl->SetFailed("Require ServerOptions.session_local_data_factory to be set!");
        FATAL_LOG("%s", cntl->ErrorText().c_str());
        return;
    }
    // solr
    std::string solr_result;
    SolrAccessor* accessor = sd->get_solr_accessor();
    if (accessor == NULL) {
        solr_result = "SolrAccessor have not be correctly set.";
    }else{
        if (accessor->insert_doc(parameters, solr_result)) {
            WARNING_LOG("Insert doc error, reference info: %s", solr_result.c_str());
        }
    }
    butil::IOBufBuilder os;
    os << solr_result;
    os.move_to(cntl->response_attachment());
}

void HttpServiceImpl::solr_update(google::protobuf::RpcController* cntl_base,
               const HttpRequest* request,
               HttpResponse*,
               google::protobuf::Closure* done) {

    // rpc framework env
    brpc::ClosureGuard done_guard(done);
    brpc::Controller* cntl = static_cast<brpc::Controller*>(cntl_base);
    cntl->http_response().set_content_type("application/json;charset=utf8");
    // preprocess
    Json::Value parameters;
    if (this->normalize_input(cntl, parameters) != 0) {
        FATAL_LOG("normalize input failed!");
        return;
    }
    SessionData* sd = static_cast<SessionData*>(cntl->session_local_data());
    if (sd == NULL) {
        cntl->SetFailed("Require ServerOptions.session_local_data_factory to be set!");
        FATAL_LOG("%s", cntl->ErrorText().c_str());
        return;
    }
    // solr
    std::string solr_result;
    SolrAccessor* accessor = sd->get_solr_accessor();
    if (accessor == NULL) {
        solr_result = "SolrAccessor have not be correctly set.";
    }else{
        if (accessor->update_doc(parameters, solr_result)) {
            WARNING_LOG("Update doc error, reference info: %s", solr_result.c_str());
        }
    }
    butil::IOBufBuilder os;
    os << solr_result;
    os.move_to(cntl->response_attachment());
}

void HttpServiceImpl::solr_delete(google::protobuf::RpcController* cntl_base,
               const HttpRequest* request,
               HttpResponse*,
               google::protobuf::Closure* done) {
    // rpc framework env
    brpc::ClosureGuard done_guard(done);
    brpc::Controller* cntl = static_cast<brpc::Controller*>(cntl_base);
    cntl->http_response().set_content_type("application/json;charset=utf8");
    // preprocess
    Json::Value parameters;
    if (this->normalize_input(cntl, parameters) != 0) {
        FATAL_LOG("normalize input failed!");
        return;
    }
    SessionData* sd = static_cast<SessionData*>(cntl->session_local_data());
    if (sd == NULL) {
        cntl->SetFailed("Require ServerOptions.session_local_data_factory to be set!");
        FATAL_LOG("%s", cntl->ErrorText().c_str());
        return;
    }
    // solr
    std::string solr_result;
    SolrAccessor* accessor = sd->get_solr_accessor();
    if (accessor == NULL) {
        solr_result = "SolrAccessor have not be correctly set.";
    }else{
        if (accessor->delete_doc(parameters, solr_result)) {
            WARNING_LOG("Delete doc error, reference info: %s", solr_result.c_str());
        }
    }
    butil::IOBufBuilder os;
    os << solr_result;
    os.move_to(cntl->response_attachment());
}

void HttpServiceImpl::solr_clear(google::protobuf::RpcController* cntl_base,
               const HttpRequest* request,
               HttpResponse*,
               google::protobuf::Closure* done) {
    // rpc framework env
    brpc::ClosureGuard done_guard(done);
    brpc::Controller* cntl = static_cast<brpc::Controller*>(cntl_base);
    cntl->http_response().set_content_type("application/json;charset=utf8");
    // preprocess
    Json::Value parameters;
    if (this->normalize_input(cntl, parameters) != 0) {
        FATAL_LOG("normalize input failed!");
        return;
    }
    if (!parameters.isMember("passwd")) {
        FATAL_LOG("passwd not set.");
        return;
    }

    SessionData* sd = static_cast<SessionData*>(cntl->session_local_data());
    if (sd == NULL) {
        cntl->SetFailed("Require ServerOptions.session_local_data_factory to be set!");
        FATAL_LOG("%s", cntl->ErrorText().c_str());
        return;
    }
    // solr
    std::string solr_result;
    SolrAccessor* accessor = sd->get_solr_accessor();
    if (accessor == NULL) {
        solr_result = "SolrAccessor have not be correctly set.";
    }else{
        if (accessor->clear_doc(parameters["passwd"].asString(), solr_result)) {
            WARNING_LOG("Clear doc error, reference info: %s", solr_result.c_str());
        }
    }
    butil::IOBufBuilder os;
    os << solr_result;
    os.move_to(cntl->response_attachment());
}

} // namespace anyq
