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

#ifndef BAIDU_NLP_ANYQ_ANALYSIS_METHOD_INTERFACE_H
#define BAIDU_NLP_ANYQ_ANALYSIS_METHOD_INTERFACE_H

#include <vector>
#include <string>

#include "common/common_define.h"
#include "anyq.pb.h"
#include "dict/dict_adapter.h"
#include "dict/dual_dict_wrapper.h"

namespace anyq {

class AnalysisMethodInterface {
public:
    AnalysisMethodInterface() {
    };
    virtual ~AnalysisMethodInterface() {
    };

    virtual int init(DualDictWrapper* dict, const AnalysisMethodConfig& analysis_method) = 0;
    virtual int destroy() = 0;
    
    // 处理query(可能包含多个，如改写或扩展会增加query)
    virtual int method_process(AnalysisResult& analysis_result);

    // 处理单个query
    virtual int single_process(AnalysisItem& analysis_item);
    
    std::string get_method_name() {
        return _method_name;
    }

protected:
    void set_method_name(const std::string& method_name) {
        _method_name = method_name;
    }

private:
    std::string _method_name;
    DISALLOW_COPY_AND_ASSIGN(AnalysisMethodInterface);
};

} // namespace anyq

#endif //BAIDU_NLP_ANYQ_ANALYSIS_METHOD_INTERFACE_H
