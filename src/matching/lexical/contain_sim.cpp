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

#include "matching/lexical/contain_sim.h"

namespace anyq {

ContainSimilarity::ContainSimilarity() {
}

ContainSimilarity::~ContainSimilarity() {
    destroy();
}
 
int ContainSimilarity::init(DualDictWrapper* dict, const MatchingConfig& matching_config) {
    return 0;
}

int ContainSimilarity::destroy() {
    return 0;
}

// 判断analysis中query是否与召回query存在包含的关系
bool ContainSimilarity::contain(const AnalysisItem& analysis_item, const RankItem& rank_item) {
    if (analysis_item.query.length() < 4) {
        return false;
    }
    if (rank_item.match_info.text.length() < 4) {
        return false;
    }
    if (analysis_item.query.find(rank_item.match_info.text) != std::string::npos) {
        return true;
    }
    if (rank_item.match_info.text.find(analysis_item.query) != std::string::npos) {
        return true;
    }
    return false;
}

int ContainSimilarity::compute_similarity(const AnalysisResult& analysis_res, RankResult& candidates) {
    if (analysis_res.analysis.size() < 1) {
        return -1;
    }
    for (int i = 0; i < candidates.size(); i++) {
        // 无效候选，跳过
        if (candidates[i].abandoned) {
            continue;
        }
        bool fea_value = contain(analysis_res.analysis[0], candidates[i]);
        DEBUG_LOG("contain %d", fea_value);
        // 如果存在包含关系，特征值为1,否则为0
        candidates[i].features.push_back(fea_value);
    }

    return 0;
}

} // namespace anyq
