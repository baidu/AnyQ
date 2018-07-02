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

#ifndef BAIDU_NLP_ANYQ_HTTP_SERVICE_IMPL_H
#define BAIDU_NLP_ANYQ_HTTP_SERVICE_IMPL_H

#include "brpc/uri.h"
#include "brpc/server.h"
#include "http_service.pb.h"
#include "anyq.pb.h"
#include "common/common_define.h"
#include "server/request_preprocess_interface.h"
#include "server/request_postprocess_interface.h"

namespace anyq {

class HttpServiceImpl : public anyq::HttpService {
public:
    HttpServiceImpl();
    ~HttpServiceImpl();
    int init(const ServerConfig& server_config);
    int destroy();
    int normalize_input(brpc::Controller* cntl, Json::Value& parameters);
    // 问答语义检索
    void anyq(google::protobuf::RpcController* cntl_base,
            const HttpRequest*,
            HttpResponse*,
            google::protobuf::Closure* done);
    
    // solr 数据操纵接口--增加数据
    void solr_insert(google::protobuf::RpcController* cntl_base,
            const HttpRequest*,
            HttpResponse*,
            google::protobuf::Closure* done);

    // solr 数据操纵接口--更新数据
    void solr_update(google::protobuf::RpcController* cntl_base,
            const HttpRequest*,
            HttpResponse*,
            google::protobuf::Closure* done);

    // solr 数据操纵接口--删除数据
    void solr_delete(google::protobuf::RpcController* cntl_base,
            const HttpRequest*,
            HttpResponse*,
            google::protobuf::Closure* done);

    // solr 数据操纵接口--清空索引库, 需要密码验证
    void solr_clear(google::protobuf::RpcController* cntl_base,
            const HttpRequest*,
            HttpResponse*,
            google::protobuf::Closure* done);

private:
    // 前处理，将server接收到的数据(get/post)处理成anyq的输入格式
    ReqPreprocInterface* _preproc_plugin;
    // 后处理，将anyq的输出结果定制输出
    ReqPostprocInterface* _postproc_plugin;
    DISALLOW_COPY_AND_ASSIGN(HttpServiceImpl);
};

} // namespace anyq
#endif  // BAIDU_NLP_ANYQ_HTTP_SERVICE_IMPL_H
