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

int PredictXGBoostModel::init(DualDictWrapper* dict,
        const std::vector<std::string>& feature_names,
        const RankPredict& predict_config) {
    // xgboost 线程资源初始化
    if (!predict_config.has_booster_model_path()) {
        FATAL_LOG("RankPluginConfig.search_host unset!");
        return -1;
    }
    _model_path = predict_config.booster_model_path();

    XGBoosterCreate(NULL, 0, &_booster);
    int ret = XGBoosterLoadModel(_booster, _model_path.c_str()); 
    if (ret != 0) {
        FATAL_LOG("XGBoosterLoadModel error, model path is: %s",  _model_path.c_str());
        return -1;
    }

    // 确定xgboost的输入特征
    _sel_dict = (hashmap_str2int*)dict->get_dict();
    int fea_name_cnt = feature_names.size();
    _weights.resize(fea_name_cnt, false);
    for (int i = 0; i < fea_name_cnt; i++) {
        if (_sel_dict->count(feature_names[i]) > 0 && (*_sel_dict)[feature_names[i]] > 0) {
            _weights[i] = true;
        }
    }
    return 0;
}

int PredictXGBoostModel::destroy() {
    int ret = XGBoosterFree(_booster);
    if (ret != 0) {
        FATAL_LOG("XGBoosterFree valid");
        return -1;
    }
    return 0;
}

int PredictXGBoostModel::predict(RankResult& candidates) {
    int weight_cnt = _weights.size();
    for (size_t i = 0; i < candidates.size(); i++) {
        // 无效候选，跳过
        if (candidates[i].abandoned) {
            candidates[i].ltr_score = 0.0f;
            continue;
        }
        int feature_size = candidates[i].features.size();
        if (feature_size != weight_cnt) {
            FATAL_LOG("features size=%d; weights size=%d;", feature_size, weight_cnt);
            return -1;
        }

        //获取输入特征向量
        std::vector<float> tmp_vector;
        tmp_vector.push_back(0.0);
        for (size_t j = 0; j < feature_size; j++) {
            if (_weights[j]) {
                tmp_vector.push_back(candidates[i].features[j]);
            }
        }
        //xgboost计算得分
        DMatrixHandle h_predict;
        if (XGDMatrixCreateFromMat(&tmp_vector[0], 1, tmp_vector.size(), 0.0, &h_predict) != 0) {
            FATAL_LOG("XGDMatrixCreateFromMat error");
            return -1;
        }
        bst_ulong out_len;
        const float *res;
        if (XGBoosterPredict(_booster, h_predict, 0, 0, &out_len, &res) != 0) {
            FATAL_LOG("xgb predict error");
            return -1;
        }
        XGDMatrixFree(h_predict);
        candidates[i].ltr_score = res[0];
    }
    return 0;
}

} // namespace anyq
