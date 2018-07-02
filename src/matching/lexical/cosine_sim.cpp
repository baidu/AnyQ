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

#include "matching/lexical/cosine_sim.h"

namespace anyq {

CosineSimilarity::CosineSimilarity(){
}

CosineSimilarity::~CosineSimilarity(){
    destroy();
}

int CosineSimilarity::init(DualDictWrapper* dict, const MatchingConfig& matching_config) {
    return 0;
}

int CosineSimilarity::destroy() {
    return 0;
}

int CosineSimilarity::compute_similarity(const AnalysisResult& analysis_res, RankResult& candidates) {
    if (analysis_res.analysis.size() < 1) {
        return -1;
    }

    // 传入的参数是const，计算相似度需要排序，所以拷贝一份
    std::vector<analysis_token_t> tmp_analysis_tokens = analysis_res.analysis[0].tokens_basic;
    for (size_t i = 0; i < candidates.size(); i++) {
        // 无效候选，跳过
        if (candidates[i].abandoned) {
            continue;
        }
        std::vector<analysis_token_t> tmp_matching_tokens = candidates[i].match_info.tokens_basic;
        // 余弦相似度
        float cos_sim = cosine_similarity(tmp_analysis_tokens, tmp_matching_tokens);
        DEBUG_LOG("cos_sim %f", cos_sim);
        candidates[i].features.push_back(cos_sim);
    }

    return 0;
}

} // namespace anyq
