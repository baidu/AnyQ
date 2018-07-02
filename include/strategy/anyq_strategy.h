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

#ifndef BAIDU_NLP_ANYQ_ANYQ_STRATEGY_H
#define BAIDU_NLP_ANYQ_ANYQ_STRATEGY_H

#include "dict/dict_manager.h"
#include "analysis/analysis_strategy.h"
#include "retrieval/retrieval_strategy.h"
#include "rank/rank_strategy.h"
#include "common/utils.h"

namespace anyq{
//Anyq总体策略类
class AnyqStrategy {
public:
    AnyqStrategy();
    ~AnyqStrategy();
    // 创建线程资源
    int create_resource(DictManager& dm, const std::string& conf_path);
    /// 释放线程资源
    int release_resource();
    // anyq策略运行接口
    int run_strategy(const std::string& analysis_input, ANYQResult& result);
    // 输出lib-svm格式的特征
    int dump_feature(const std::string& input_file, const std::string& out_file);
private:
    AnalysisStrategy _analysis;
    RetrievalStrategy _retrieval;
    RankStrategy _rank;
    DISALLOW_COPY_AND_ASSIGN(AnyqStrategy);
};

}

#endif  //BAIDU_NLP_ANYQ_ANYQ_STRATEGY_H
