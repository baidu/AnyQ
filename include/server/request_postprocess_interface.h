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

#ifndef BAIDU_NLP_ANYQ_REQUEST_POSTPROCESS_INTERFACE_H
#define BAIDU_NLP_ANYQ_REQUEST_POSTPROCESS_INTERFACE_H

#include "json/json.h"
#include "common/utils.h"
#include "anyq.pb.h"

namespace anyq {

class ReqPostprocInterface
{
public:
    ReqPostprocInterface() {};
    virtual ~ReqPostprocInterface() {};
    int init_base(const std::string& plugin_name){
        _plugin_name = plugin_name;
        return 0;
    }; 
    virtual int init(const ServerConfig&) = 0;
    virtual int destroy() = 0;
    const std::string& plugin_name(){
        return _plugin_name;
    }
    virtual int process(ANYQResult& anyq_result,
            Json::Value& parameters, 
            std::string& output) = 0;

private:
    std::string _plugin_name;
    DISALLOW_COPY_AND_ASSIGN(ReqPostprocInterface);
};

} // namespace anyq

#endif  // BAIDU_NLP_ANYQ_REQUEST_PROCESSOR_INTERFACE_H
