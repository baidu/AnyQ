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

#include "server/anyq_postprocessor.h"
#include "common/utils.h"

namespace anyq {

int AnyqPostprocessor::init(const ServerConfig& config) {
    if (!config.postproc_plugin().has_type()) {
        FATAL_LOG("ReqPostprocPluginConfig.type unset!");
        return -1;
    }
    if (!config.postproc_plugin().has_name()) {
        FATAL_LOG("ReqPostprocPluginConfig.name unset!");
        return -1;
    }
    init_base(config.postproc_plugin().name());
    return 0;
}

int AnyqPostprocessor::destroy() {
    return 0;
}

int AnyqPostprocessor::process(ANYQResult& any_result,
        Json::Value& parameters,
        std::string& output) {
    // do nothing
    Json::Value json_anyq_res = Json::Value(Json::arrayValue);
    for (size_t i = 0; i < any_result.items.size(); i++) {
        Json::Value result_item;
        result_item["question"] = any_result.items[i].query;
        result_item["answer"] = any_result.items[i].answer;
        result_item["confidence"] = any_result.items[i].confidence;
        result_item["qa_id"] = any_result.items[i].qa_id;
        result_item["json_info"] = any_result.items[i].json_info;
        json_anyq_res.append(result_item);
    }
    output = json_dumps(json_anyq_res);
    return 0;
}

} // namespace anyq
