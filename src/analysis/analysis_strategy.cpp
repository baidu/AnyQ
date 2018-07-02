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

#include "analysis/analysis_strategy.h"
#include "google/protobuf/text_format.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "dict/dict_manager.h"
#include "dict/dict_adapter.h"
#include "common/utils.h"
#include "common/plugin_factory.h"

namespace anyq {

AnalysisStrategy::AnalysisStrategy() {
}

AnalysisStrategy::~AnalysisStrategy() {
    if (destroy() != 0) {
        FATAL_LOG("release analysis stategy error");
    }
}

int AnalysisStrategy::init(DictMap* dict_map, const std::string& analysis_conf){
    _dict_map = dict_map;
    AnalysisConfig analysis_config;
    if (load_config_from_file(analysis_conf, analysis_config) != 0) {
        return -1;
    }
    for (int i = 0; i < analysis_config.analysis_method_size(); i++) {
        AnalysisMethodConfig analysis_method_config = analysis_config.analysis_method(i);
        const std::string& method_type = analysis_method_config.type();
        std::string dict_name("");
        bool need_dict = false;
        if (analysis_method_config.has_using_dict_name()) {
            dict_name = analysis_method_config.using_dict_name();
            need_dict = true;
        }

        // 创建Analysis方法
        AnalysisMethodInterface* tmp_method = 
                static_cast<AnalysisMethodInterface*>(PLUGIN_FACTORY.create_plugin(method_type));
        if (tmp_method == NULL) {
            FATAL_LOG("can't find method_name:%s", method_type.c_str());
            return -1;
        }

        if (need_dict && _dict_map->count(dict_name) < 1) {
            FATAL_LOG("using dict %s that does not exist", dict_name.c_str());
            return -1;
        }
        int ret = 0;
        // 根据配置的词典，进行初始化，未配置词典则使用NULL
        if (need_dict) {
            ret = tmp_method->init((*_dict_map)[dict_name], analysis_method_config);
        } else {
            ret = tmp_method->init(NULL, analysis_method_config); 
        }

        if (ret != 0) {
            FATAL_LOG("init method error %s", dict_name.c_str());
            return -1;
        }
        TRACE_LOG("create method %s success", method_type.c_str());
        _method_list.push_back(tmp_method);
    }
    return 0;
}

// 从json串，构建出analysis的结果形式
int AnalysisStrategy::json_parser(const std::string& analysis_input_str,
        AnalysisResult& analysis_result){
    Json::Reader reader;
    Json::Value analysis_input;
    if (!reader.parse(analysis_input_str, analysis_input))
    {
        FATAL_LOG("analysis_input_str is not json style");
        return -1;
    }
    if (analysis_input.type() != Json::objectValue)
    {
        TRACE_LOG("analysis_input data is not json style");
        return -1;
    }

    if (!analysis_input.isMember("question") || analysis_input["question"].isNull())
    {
        FATAL_LOG("question is not analysis_input's memeber");
        return -1;
    }
    AnalysisItem analysis_item;
    if (analysis_input.isMember("analysis_item") && !analysis_input["analysis_item"].isNull())
    {
        int ret = json_to_analysis_item(analysis_input["analysis_item"], analysis_item);
        if (ret != 0)
        {
            FATAL_LOG("json_to_analysis_item failed!");
            return -1;
        }
    } else {
        analysis_item.type = 0;
        analysis_item.query = analysis_input["question"].asString();
    }
    analysis_result.analysis.push_back(analysis_item);
    TRACE_LOG("analysis_analysis size: %d", analysis_result.analysis.size());
    analysis_result.notice_log.append("input_query=");
    analysis_result.notice_log.append(analysis_item.query);

    std::map<std::string, std::string> info_map;
    if (analysis_input.isMember("info") && !analysis_input["info"].isNull())
    {
        int ret = json_to_map(analysis_input["info"], info_map);
        if (ret != 0)
        {
            FATAL_LOG("json_to_map failed!");
            return -1;
        }
    }
    analysis_result.info = std::move(info_map);
    DEBUG_LOG("before use analysis strategy's analysis_result");
    print_analysis_result(analysis_result);
    return 0;
}

int AnalysisStrategy::run_strategy(const std::string& analysis_input_str,
        AnalysisResult& analysis_result){
    float time_use = 0; // 毫秒
    struct timeval start;
    struct timeval end;
    gettimeofday(&start, NULL);
    if (json_parser(analysis_input_str, analysis_result) != 0) {
        FATAL_LOG("analysis_input json parser failed!");
        return -1;
    }

    std::vector<AnalysisMethodInterface*>::iterator it;
    for (it = _method_list.begin(); it != _method_list.end(); ++it) {
        TRACE_LOG("method_process %s start", ((*it)->get_method_name()).c_str());
        if ((*it)->method_process(analysis_result) != 0) {
            FATAL_LOG("method_process %s error", ((*it)->get_method_name()).c_str());
            return -1;
        }
        else {
            TRACE_LOG("method_process %s sucess", ((*it)->get_method_name()).c_str());
        }
    }
    gettimeofday(&end, NULL);
    time_use = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000;
    char sub_log[SUB_LOG_LEN];
    snprintf(sub_log, SUB_LOG_LEN, ";analysis_time=%.2fms;", time_use);
    analysis_result.notice_log.append(sub_log);

    DEBUG_LOG("after use analysis strategy's analysis_result");
    print_analysis_result(analysis_result);
    return 0;
}

int AnalysisStrategy::destroy()
{
    std::vector<AnalysisMethodInterface*>::iterator it;
    int i = 0;
    for (it = _method_list.begin(); it != _method_list.end(); ++it, ++i) {
        if ((*it)->destroy() != 0) {
            FATAL_LOG("analysis_method %s destroy error", ((*it)->get_method_name()).c_str());
            return -1;
        }
        delete *it;
    }
    _method_list.clear();
    return 0;
}

} // namespace anyq
