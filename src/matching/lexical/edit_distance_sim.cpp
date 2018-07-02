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

#include "matching/lexical/edit_distance_sim.h"
#include <algorithm>

namespace anyq {

EditDistanceSimilarity::EditDistanceSimilarity(){
}

EditDistanceSimilarity::~EditDistanceSimilarity(){
    destroy();
}

int EditDistanceSimilarity::init(DualDictWrapper* dict, const MatchingConfig& matching_config) {
    return 0;
}

int EditDistanceSimilarity::destroy() {
    return 0;
}

int EditDistanceSimilarity::compute_edit_distance(const std::vector<analysis_token_t>& seq1,
        const std::vector<analysis_token_t>& seq2){
    size_t len1 = seq1.size();
    size_t len2 = seq2.size();
    if (len1 == 0) {
        return len2;
    }
    if (len2 == 0) {
        return len1;
    }
    std::vector<std::vector<int> > matrix(len1 + 1, std::vector<int>(len2 + 1));
    for (size_t i = 0; i <= len1; ++i) {
        matrix[i][0] = i;
    }
    for (size_t j = 0; j <= len2; ++j) {
        matrix[0][j] = j;
    }
    // 动态规划
    for (size_t i = 1; i <= len1; ++i) {
        for (size_t j = 1; j <= len2; ++j) {
            int cost = 0;
            if (seq1[i - 1].buffer != seq2[j - 1].buffer) {
                cost = 1;
            }
            matrix[i][j] = std::min(std::min(matrix[i -1][j] + 1, matrix[i][j - 1] + 1),
                    matrix[i -1][j - 1] + cost);
        }
    }
    return matrix[len1][len2];    
}

int EditDistanceSimilarity::compute_similarity(const AnalysisResult& analysis_res, RankResult& candidates) {
    if (analysis_res.analysis.size() < 1) {
        return -1;
    }

    for (size_t i = 0; i < candidates.size(); i++) {
        // 无效候选，跳过
        if (candidates[i].abandoned) {
            continue;
        }
        int max_len = std::max(analysis_res.analysis[0].tokens_basic.size(),
                candidates[i].match_info.tokens_basic.size()); 
        int edit_distance = compute_edit_distance(analysis_res.analysis[0].tokens_basic,
                candidates[i].match_info.tokens_basic);
        float sim = 0.0;
        if (max_len > 0) {
            sim = 1 - edit_distance * 1.0 / max_len;
        }
        DEBUG_LOG("edit_distance_sim %f", sim);
        candidates[i].features.push_back(sim);
    }

    return 0;
}

} // namespace anyq
