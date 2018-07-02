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

int PredictSelectModel::init(DualDictWrapper* dict,
        const std::vector<std::string>& feature_names,
        const RankPredict& predict_config) {
    // 获取选择特征的下标
    _select_index = predict_config.select_config().select_index();
    return 0;
}

int PredictSelectModel::destroy() {
    return 0;
}

int PredictSelectModel::predict(RankResult& candidates) {
    for (size_t i = 0; i < candidates.size(); i++) {
        // 无效候选，跳过
        if (candidates[i].abandoned) {
            candidates[i].ltr_score = 0.0f;
            continue;
        }
        float score = 0.0f;
        // 用下标为_select_index的特征值作为得分
        if (candidates[i].features.size() > _select_index) {
            score = candidates[i].features[_select_index];
        }
        candidates[i].ltr_score = score;
    }
    return 0;
}

} // namespace anyq
