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

#include "analysis/method_interface.h"

namespace anyq {

int AnalysisMethodInterface::single_process(AnalysisItem& analysis_item) {
    return 0;
}

int AnalysisMethodInterface::method_process(AnalysisResult& analysis_result) {
    std::vector<AnalysisItem>& analysis = analysis_result.analysis;
    for (size_t j = 0; j < analysis.size(); j++) {
        AnalysisItem& analysis_item = analysis[j];
        if (single_process(analysis_item) != 0) {
            FATAL_LOG("single_process err");
            return -1;
        }
    }
    return 0;
}

} // namespace anyq
