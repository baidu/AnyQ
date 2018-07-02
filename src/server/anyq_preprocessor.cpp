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

#include "server/anyq_preprocessor.h"
#include "common/utils.h"

namespace anyq {

int AnyqPreprocessor::init(const ReqPreprocPluginConfig& config) {
    if (!config.has_type()) {
        FATAL_LOG("ReqPreprocPluginConfig.type unset!");
        return -1;
    }
    if (!config.has_name()) {
        FATAL_LOG("ReqPreprocPluginConfig.name unset!");
        return -1;
    }
    init_base(config.name());
    return 0;
}

int AnyqPreprocessor::destroy() {
    return 0;
}

int AnyqPreprocessor::process(brpc::Controller* cntl,
        Json::Value& parameters,
        std::string& str_anyq_input) {
    if (!parameters.isMember("question")) {
        FATAL_LOG("Query field is required.");
    }
    Json::Value json_analysis_input;
    Json::Value json_analysis_info;
    Json::Value::Members mem = parameters.getMemberNames();
    for (Json::Value::Members::iterator it = mem.begin(); it != mem.end(); ++it) {
        if (*it == "question") {
            json_analysis_input["question"] = parameters["question"].asString();
            std::string debug_str = parameters["question"].asString();
            Json::Value analysis_item;
            analysis_item["question"] = parameters["question"].asString();
            analysis_item["type"] = 0;
            json_analysis_input["analysis_item"] = analysis_item;
        } else {
            json_analysis_info[*it] = parameters[*it];
        }
    }
    json_analysis_input["info"] = json_analysis_info;
    str_anyq_input = json_dumps(json_analysis_input);
    return 0;
}

} // namespace anyq
