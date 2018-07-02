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

#ifndef BAIDU_NLP_ANYQ_CONTAIN_SIM_H
#define BAIDU_NLP_ANYQ_CONTAIN_SIM_H

#include "matching/matching_interface.h"
#include "common/utils.h"

namespace anyq {

//该特征表示query与候选之间是否存在包含关系
class ContainSimilarity : public MatchingInterface {
public:
    ContainSimilarity();
    virtual ~ContainSimilarity() override;
    virtual int init(DualDictWrapper* dict, const MatchingConfig& matching_config) override;
    virtual int destroy();
    // 判断analysis中query是否与召回query存在包含的关系
    virtual bool contain(const AnalysisItem& analysis_item, const RankItem& rank_item);
    // 特征计算
    virtual int compute_similarity(const AnalysisResult& analysis_res,
            RankResult& candidates) override;
private:
    DISALLOW_COPY_AND_ASSIGN(ContainSimilarity);
};

} // namespace anyq

#endif  // BAIDU_NLP_ANYQ_CONTAIN_SIMILARITY_H
