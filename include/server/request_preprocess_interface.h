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

#ifndef BAIDU_NLP_ANYQ_REQUEST_PREPROCESS_INTERFACE_H
#define BAIDU_NLP_ANYQ_REQUEST_PREPROCESS_INTERFACE_H

#include "json/json.h"
#include "brpc/server.h"
#include "common/utils.h"
#include "anyq.pb.h"

namespace anyq{

class ReqPreprocInterface{
public:
    ReqPreprocInterface(){};
    virtual ~ReqPreprocInterface() {};
    int init_base(const std::string& plugin_name){
        _plugin_name = plugin_name;
        return 0;
    }
    virtual int init(const ReqPreprocPluginConfig& config) = 0;
    virtual int destroy() = 0;
    const std::string& plugin_name() {
        return _plugin_name;
    }
    virtual int process(brpc::Controller* cntl,
            Json::Value& parameters,
            std::string& str_input) = 0;

private:
    std::string _plugin_name;
    DISALLOW_COPY_AND_ASSIGN(ReqPreprocInterface);
};

}

#endif  // BAIDU_NLP_ANYQ_REQUEST_PREPROCESS_INTERFACE_H
