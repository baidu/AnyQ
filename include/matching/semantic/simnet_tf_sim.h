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

#ifndef BAIDU_NLP_ANYQ_SIMNET_TF_SIM_H
#define BAIDU_NLP_ANYQ_SIMNET_TF_SIM_H

#ifdef USE_TENSORFLOW
#include "matching/matching_interface.h"

namespace anyq {
// 基于tensorflow的语义相似特征
class TFSimilarity : public MatchingInterface {
public:
    TFSimilarity();
    virtual ~TFSimilarity() override;
    virtual int init(DualDictWrapper* dict, const MatchingConfig& matching_config) override;
    virtual int destroy() override;
    virtual int compute_similarity(const AnalysisResult& analysis_res,
            RankResult& candidates) override;
    // 转化成tensorflow的输入
    int trans_to_tf_input(
            std::vector<std::pair<std::string, tensorflow::Tensor> > &input, 
            std::string input_name, 
            std::vector<int>& vec);
private:
    tensorflow::Session* _p_session;
    TFPack* _p_tf_pack;
    std::string _left_name;
    std::string _right_name;
    std::string _output_tensor_name;
    int _pad_id;
    int _sen_len;
    DISALLOW_COPY_AND_ASSIGN(TFSimilarity);
};

} // namespace anyq
#endif

#endif  //BAIDU_NLP_ANYQ_TF_SIM_H
