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

#ifndef BAIDU_NLP_ANYQ_ANALYSIS_STRATEGY_H
#define BAIDU_NLP_ANYQ_ANALYSIS_STRATEGY_H

#include "analysis/method_interface.h"
#include "anyq.pb.h"
#include "dict/dict_manager.h"

namespace anyq {
// 对输入query进行分析，
class AnalysisStrategy {
public:
    AnalysisStrategy();
    ~AnalysisStrategy();
    
    // 使用词典和配置初始化
    int init(DictMap* dict_map, const std::string& analysis_conf);
    // 运行Analysis的插件
    int run_strategy(const std::string& analysis_input_str, AnalysisResult& analysis_result);
    
    int destroy();
private:
    // 输入格式转换
    int json_parser(const std::string& analysis_input_str, AnalysisResult& analysis_result);
    
    // 插件列表
    std::vector<AnalysisMethodInterface*> _method_list;
    
    DictMap* _dict_map;
    DISALLOW_COPY_AND_ASSIGN(AnalysisStrategy);
};

} // namespace anyq

#endif //BAIDU_NLP_ANYQ_ANALYSIS_STRATEGY_H
