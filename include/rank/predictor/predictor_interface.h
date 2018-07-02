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

#ifndef BAIDU_NLP_ANYQ_PREDICTOR_INTERFACE_H
#define BAIDU_NLP_ANYQ_PREDICTOR_INTERFACE_H

#include <vector>
#include <string>
#include "xgboost/data.h"
#include "xgboost/learner.h"
#include "xgboost/c_api.h"
#include "common/common_define.h"
#include "dict/dict_adapter.h"
#include "common/utils.h"
#include "anyq.pb.h"
#include "dict/dual_dict_wrapper.h"

namespace anyq {
//rank得分预测器,根据匹配特征为候选计算得分
class RankPredictInterface {
public:
    RankPredictInterface() {};
    virtual ~RankPredictInterface() {};
    // 线程资源初始化
    virtual int init(DualDictWrapper* dict,
            const std::vector<std::string>& feature_names,
            const RankPredict& predict_config) = 0;
    // 线程资源销毁
    virtual int destroy() = 0;
    //预测候选得分
    virtual int predict(RankResult& candidates) = 0;

private:
    DISALLOW_COPY_AND_ASSIGN(RankPredictInterface);
};

//线性预测模型
class PredictLinearModel: public RankPredictInterface {
public:
    PredictLinearModel() {};
    virtual ~PredictLinearModel() override{};
    virtual int init(DualDictWrapper* dict,
            const std::vector<std::string>& feature_names,
            const RankPredict& predict_config) override;
    virtual int destroy() override;
    virtual int predict(RankResult& candidates) override;
public:
    // 特征名到权值的映射
    hashmap_str2float* _feature_weight;
    // 权值向量
    std::vector<float> _weights;
    DISALLOW_COPY_AND_ASSIGN(PredictLinearModel);
};

//xgboost预测模型
class PredictXGBoostModel: public RankPredictInterface {
public:
    PredictXGBoostModel() {};
    virtual ~PredictXGBoostModel() override {};
    virtual int init(DualDictWrapper* dict,
            const std::vector<std::string>& feature_names,
            const RankPredict& predict_config) override;
    virtual int destroy() override;
    virtual int predict(RankResult& candidates) override;
private:
    std::string _model_path;
    BoosterHandle _booster;
    hashmap_str2int* _sel_dict;
    std::vector<bool> _weights;
    DISALLOW_COPY_AND_ASSIGN(PredictXGBoostModel);
};

// 该预测插件根据选择某一特征作为候选得分
class PredictSelectModel: public RankPredictInterface {
public:
    PredictSelectModel() {};
    virtual ~PredictSelectModel() override {};
    virtual int init(DualDictWrapper* dict,
            const std::vector<std::string>& feature_names,
            const RankPredict& predict_config) override;
    virtual int destroy() override;
    virtual int predict(RankResult& candidates) override;
public:
    // 选择的特征下标
    int _select_index;
    DISALLOW_COPY_AND_ASSIGN(PredictSelectModel);
};

} // namespace anyq
#endif  //BAIDU_NLP_ANYQ_PREDICTOR_INTERFACE_H
