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

#ifndef BAIDU_NLP_ANYQ_RANK_STRATEGY_H
#define BAIDU_NLP_ANYQ_RANK_STRATEGY_H

#include <numeric>
#include <algorithm>
#include "anyq.pb.h"
#include "dict/dict_adapter.h"
#include "matching/matching_interface.h"
#include "dict/dict_manager.h"
#include "rank/predictor/predictor_interface.h"
#include "common/common_define.h"
#include "common/utils.h"

namespace anyq {
//rank策略类, rank模块的入口
class RankStrategy {
public:
    RankStrategy();
    ~RankStrategy();
    // 初始化matching feature
    int init_feature(const RankConfig& rank_config);
    // 初始化predictor
    int init_predictor(const RankPredict& predict_config,
            std::vector<std::string>& feature_names,
            RankPredictInterface*& predictor);
    // 初始化线程资源
    int init(DictMap* dict_map, const std::string& rank_conf);
    int destroy();
    // 遍历所有类型特征，选取粗排特征或者精排特征进行计算
    int compute_similarity(const AnalysisResult& analysis_res, RankResult& candidates, bool rough_sort);
    // 将特征转换为LibSVM的格式
    int libsvm_feature_conv(const RankResult& candidates, std::string& feature_out);
    // 过滤低于阈值的候选
    int threshold_filter(RankResult& candidates, float threshold);
    // 计算特征、score，过滤阈值
    int compute_and_filter(const AnalysisResult& analysis_res,
            RankResult& candidates,
            bool rough_sort);
    // 根据score排序，返回从大到小score的index
    std::vector<int> sort_samples(const std::vector<float> &v);
    // answer排序时，删除重复answer
    int rm_duplicate_answer(RankResult& candidates);
    // 返回结果填充
    int collect_result(const RankResult& candidates, ANYQResult& result);
    int collect_result(const RetrievalResult& candidates, ANYQResult& result);
    // debug打印最终结果信息
    int debug_result(const AnalysisResult& analysis_res, ANYQResult& result);
    // 总策略流程
    int run_strategy(const AnalysisResult& analysis_res,
            const RetrievalResult& retrieval_candidates,
            ANYQResult& result);
    // 设置候选的query作为匹配信息
    int matching_fill_query_info(const RetrievalResult& retrieval_candidates,
            RankResult& rank_candidates);
    // 将多答案候选拆分为多个候选
    int trans_answer_info(const RankResult& src_candidates,
            RankResult& dst_candidates);
private:
    std::vector<MatchingInterface*> _feature_calculator;
    DictMap* _dict_map;
    // 粗排选出得分_rough_top_k的候选
    int _rough_top_k;
    // 精排选出得分_top_k的候选
    int _top_k;
    std::vector<std::string> _rough_feature_names;
    std::vector<std::string> _feature_names;
    // 粗排score计算器
    RankPredictInterface* _rough_predictor;
    // 精排score计算器
    RankPredictInterface* _predictor;
    // 粗排阈值
    float _rough_threshold;
    // 精排阈值
    float _threshold;
    // 是否在精排时使用候选的答案与输入query匹配
    bool _deep_rank_with_anwser;
    int _max_ans_num_per_query;
};

} // namespace anyq

#endif  //BAIDU_NLP_ANYQ_RANK_STRATEGY_H
