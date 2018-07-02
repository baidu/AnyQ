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

#ifndef BAIDU_NLP_ANYQ_EDIT_DISTANCE_SIM_H
#define BAIDU_NLP_ANYQ_EDIT_DISTANCE_SIM_H

#include "matching/matching_interface.h"

namespace anyq {
//该特征表示query与候选编辑距离相似度
class EditDistanceSimilarity : public MatchingInterface {
public:
    EditDistanceSimilarity();
    virtual ~EditDistanceSimilarity() override;
    virtual int init(DualDictWrapper* dict, const MatchingConfig& matching_config) override;
    virtual int destroy() override;
    virtual int compute_similarity(const AnalysisResult& analysis_res,
            RankResult& candidates) override;
    int compute_edit_distance(const std::vector<analysis_token_t>& seq1,
            const std::vector<analysis_token_t>& seq2);
private:
    DISALLOW_COPY_AND_ASSIGN(EditDistanceSimilarity);
};

} // namespace anyq

#endif  // BAIDU_NLP_ANYQ_EDIT_DISTANCE_SIM_H
