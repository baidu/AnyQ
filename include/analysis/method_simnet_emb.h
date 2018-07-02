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

#ifndef BAIDU_NLP_ANYQ_METHOD_SIMNET_EMB_H
#define BAIDU_NLP_ANYQ_METHOD_SIMNET_EMB_H

#ifndef USE_TENSORFLOW
#include "analysis/method_interface.h"
#include "common/paddle_thread_resource.h"
#include "anyq.pb.h"

namespace anyq {
// 获取SimNet网络的Query表示向量
class AnalysisSimNetEmb : public AnalysisMethodInterface {
public:
    AnalysisSimNetEmb();
    virtual ~AnalysisSimNetEmb() override;
    virtual int init(DualDictWrapper* dict, const AnalysisMethodConfig& analysis_method) override;
    virtual int destroy() override;
    // 复用Interface的method_process, 实现自己的single_process
    virtual int single_process(AnalysisItem& analysis_item) override;

private:
    PaddlePack* _p_paddle_pack;
    PaddleThreadResource* _paddle_resource;
    size_t _query_feed_index;
    size_t _cand_feed_index; 
    size_t _embedding_fetch_index;
    size_t _dim;
    DISALLOW_COPY_AND_ASSIGN(AnalysisSimNetEmb);
};

} //namespace anyq
#endif

#endif  // BAIDU_NLP_ANYQ_METHOD_SIMNET_EMB_H
