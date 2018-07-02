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

#ifndef BAIDU_NLP_ANYQ_SIMNET_PADDLE_SIM_H
#define BAIDU_NLP_ANYQ_SIMNET_PADDLE_SIM_H

#ifndef USE_TENSORFLOW
#include "matching/matching_interface.h"
#include "common/paddle_thread_resource.h"

namespace anyq {
//基于paddle的语义相似度特征
class PaddleSimilarity : public MatchingInterface {
public:
    PaddleSimilarity();
    virtual ~PaddleSimilarity() override;
    virtual int init(DualDictWrapper* dict, const MatchingConfig& matching_config) override;
    virtual int destroy() override;
    virtual int compute_similarity(const AnalysisResult& analysis_res,
            RankResult& candidates) override;

private:
    // paddle dict 指针
    PaddlePack* _p_paddle_pack;
    PaddleThreadResource* _paddle_resource;
    // paddle线程资源
    size_t _query_feed_index;
    // "query" 在feed中的下标
    size_t _cand_feed_index;
    // "score"在fetch中的下标
    size_t _score_fetch_index;
    DISALLOW_COPY_AND_ASSIGN(PaddleSimilarity);
};

} // namespace anyq
#endif

#endif  // BAIDU_NLP_ANYQ_PADDLE_SIM_FEATURE_H

