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

#include "rank/predictor/predictor_interface.h"

namespace anyq {

int PredictLinearModel::init(DualDictWrapper* dict,
        const std::vector<std::string>& feature_names,
        const RankPredict& predict_config) {
    _feature_weight = (hashmap_str2float*)dict->get_dict();
    int fea_name_cnt = feature_names.size();
    // 初始化权值向量，如果某特征名不在字典里，则权值为0.0
    _weights.resize(fea_name_cnt, 0.0f);
    for (int i = 0; i < fea_name_cnt; i++) {
        if (_feature_weight->count(feature_names[i]) > 0) {
            _weights[i] = (*_feature_weight)[feature_names[i]];
        } else {
            WARNING_LOG("invalid feature name %s", feature_names[i].c_str());
            _weights[i] = 0.0f;
        }
    }
    return 0;
}

int PredictLinearModel::destroy() {
    return 0;
}

int PredictLinearModel::predict(RankResult& candidates) {
    int weight_cnt = _weights.size();
    for (size_t i = 0; i < candidates.size(); i++) {
        // 无效候选，跳过
        if (candidates[i].abandoned) {
            candidates[i].ltr_score = 0.0f;
            continue;
        }
        float score = 0.0f;
        int feature_size = candidates[i].features.size();
        if (feature_size != weight_cnt) {
            FATAL_LOG("features size=%d; weights size=%d;", feature_size, weight_cnt);
            return -1;
        }
         //线性加权计算得分
        for (int j = 0; j < weight_cnt; j++) {
            score += candidates[i].features[j] * _weights[j];
        }
        candidates[i].ltr_score = score;
    }
    return 0;
}

} // namespace anyq
