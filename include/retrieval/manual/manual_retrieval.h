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

#ifndef BAIDU_NLP_ANYQ_MANUAL_RETRIEVAL_H
#define BAIDU_NLP_ANYQ_MANUAL_RETRIEVAL_H

#include "retrieval/retrieval_interface.h"

namespace anyq {
//人工干预检索插件
class ManualRetrievalPlugin : public RetrievalPluginInterface{
public:
    ManualRetrievalPlugin(){};
    virtual ~ManualRetrievalPlugin() override {};
    virtual int init(DictMap* dict_map, const RetrievalPluginConfig& plugin_config) override;
    virtual int destroy() override;
    virtual int retrieval(const AnalysisResult& analysis_res,
            RetrievalResult& retrieval_res) override;
private:
    DualDictWrapper *_p_dual_dict_wrapper;
    DISALLOW_COPY_AND_ASSIGN(ManualRetrievalPlugin);
};

} // namespace anyq
#endif  // BAIDU_NLP_ANYQ_MANUAL_RETRIEVAL_H
