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

#include "retrieval/retrieval_strategy.h"
#include "anyq.pb.h"
#include "common/plugin_factory.h"

namespace anyq {

const std::string MANUAL_RETRIEVAL_TYPE = "ManualRetrievalPlugin";

RetrievalStrategy::RetrievalStrategy() {
}

RetrievalStrategy::~RetrievalStrategy() {
}

int RetrievalStrategy::init(DictMap* dict_map, const std::string& retrieval_conf) {
    RetrievalConfig retrieval_config;
    if (load_config_from_file(retrieval_conf, retrieval_config) != 0) {
        return -1;
    }
    // retrieval.conf中至少配置一个检索插件
    if (retrieval_config.retrieval_plugin_size() < 1) {
        FATAL_LOG("retrieval_config.retrieval_plugin_size=%d",
                retrieval_config.retrieval_plugin_size());
        return -1;
    }
    // 初始化retrieval_plugins
    for (int i = 0; i < retrieval_config.retrieval_plugin_size(); i++) {
        RetrievalPluginConfig retrieval_plugin_config = retrieval_config.retrieval_plugin(i);
        std::string plugin_type = retrieval_plugin_config.type();
        std::string plugin_name = retrieval_plugin_config.name();
        // 生产插件类并初始化
        RetrievalPluginInterface* ptr_retrieval_plugin = 
                static_cast<RetrievalPluginInterface*>(PLUGIN_FACTORY.create_plugin(plugin_type));
        if (ptr_retrieval_plugin == NULL) {
            FATAL_LOG("can't find retrieval plugin_type:%s", plugin_type.c_str());
            return -1;
        }
        if (ptr_retrieval_plugin->init(dict_map, retrieval_plugin_config) != 0) {
            FATAL_LOG("init retrieval plugin error %s", plugin_name.c_str());
            return -1;
        }
        TRACE_LOG("create retrieval plugin %s success", plugin_name.c_str());
        if (plugin_type == MANUAL_RETRIEVAL_TYPE) {
            _retrieval_plugins.insert(_retrieval_plugins.begin(), ptr_retrieval_plugin);
        } else {
            _retrieval_plugins.push_back(ptr_retrieval_plugin);
        }
    }

    return 0;
}

int RetrievalStrategy::destroy() {
    std::vector<RetrievalPluginInterface*>::iterator it;
    for (it = _retrieval_plugins.begin(); it != _retrieval_plugins.end(); ++it) {
        if ((*it)->destroy() != 0) {
            FATAL_LOG("retrieval plugin %s destroy error", ((*it)->plugin_name()).c_str());
            return -1;
        }
        delete *it;
    }
    _retrieval_plugins.clear();
    return 0;
}

// 删除检索出的重复query
int RetrievalStrategy::rm_duplicate_query(RetrievalResult& retrieval_result) {
    for (int i = retrieval_result.items.size() - 1; i >= 0; i--) {
        for (int j = 0; j < i; j++) {
            bool answer_same = true;
            if (retrieval_result.items[i].answer.size() !=
                    retrieval_result.items[j].answer.size()) {
                answer_same = false;
                continue;
            }
            for (size_t k = 0; k < retrieval_result.items[i].answer.size(); k++) {
                if (retrieval_result.items[i].answer[k].text !=
                        retrieval_result.items[j].answer[k].text) {
                    answer_same = false;
                }
            }
            if (retrieval_result.items[i].query.id == retrieval_result.items[j].query.id ||
                    (retrieval_result.items[i].query.text ==
                    retrieval_result.items[j].query.text && answer_same)) {
                retrieval_result.items.erase(retrieval_result.items.begin() + i);
                break;
            }
        }
    }

    return 0;
}

// 总的召回策略流程
int RetrievalStrategy::run_strategy(const AnalysisResult& analysis_result,
        RetrievalResult& retrieval_result) {
    float time_use = 0; // 毫秒
    struct timeval start;
    struct timeval end;
    gettimeofday(&start, NULL);

    retrieval_result.items.clear();
    retrieval_result.anyq_end = false;
    // 各分支分别召回
    std::vector<RetrievalPluginInterface*>::iterator it;
    for (it = _retrieval_plugins.begin(); it != _retrieval_plugins.end(); ++it) {
        if ((*it)->retrieval(analysis_result, retrieval_result) != 0) {
            FATAL_LOG("plugin %s retrieval error", ((*it)->plugin_name()).c_str());
            return -1;
        }
        if (retrieval_result.anyq_end) {
            return 0;
        }
    }
    DEBUG_LOG("before rm_dup, retrieval result len=%d;", retrieval_result.items.size());
    // 删除检索出的重复query
    rm_duplicate_query(retrieval_result);
    DEBUG_LOG("after rm_dup retrieval result len=%d;", retrieval_result.items.size());

    gettimeofday(&end, NULL);
    time_use = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000;
    char sub_log[SUB_LOG_LEN];
    snprintf(sub_log, SUB_LOG_LEN, "retrieval_num=%d;retrieval_time=%.2fms;",
            retrieval_result.items.size(), time_use);
    retrieval_result.notice_log.append(sub_log);

    return 0;
}

} // namespace anyq
