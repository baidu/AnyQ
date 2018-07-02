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

#ifndef BAIDU_NLP_ANYQ_ANYQ_POSTPROCESSOR_H
#define BAIDU_NLP_ANYQ_ANYQ_POSTPROCESSOR_H

#include "server/request_postprocess_interface.h"

namespace anyq {

class AnyqPostprocessor : public ReqPostprocInterface {
public:
    AnyqPostprocessor() {};
    virtual ~AnyqPostprocessor() override {};
    virtual int init(const ServerConfig& config) override;
    virtual int destroy() override;
    virtual int process(ANYQResult& anyq_result,
            Json::Value& parameters, 
            std::string& output) override;

private:
    DISALLOW_COPY_AND_ASSIGN(AnyqPostprocessor);
};
} // namespace anyq

#endif  // BAIDU_NLP_ANYQ_ANYQ_POSTPROCESSOR_H
