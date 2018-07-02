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

#ifndef BAIDU_NLP_ANYQ_MATCHING_INTERFACE_H
#define BAIDU_NLP_ANYQ_MATCHING_INTERFACE_H

#include <vector>
#include <string>
#include "anyq.pb.h"
#include "common/common_define.h"
#include "dict/dict_adapter.h"
#include "dict/dual_dict_wrapper.h"
#include "common/utils.h"

namespace anyq {

class MatchingInterface {
//mathcing特征插件接口,继承类必须实现init,destroy和compute_similarity三个函数
public:
    MatchingInterface() {};
    virtual ~MatchingInterface() {};
    // 线程资源初始化
    virtual int init(DualDictWrapper* dict, const MatchingConfig& matching_config) = 0;
    // 释放线程资源
    virtual int destroy() = 0;
    virtual int compute_similarity(const AnalysisResult& analysis_res, RankResult& candidates) = 0;
    // 基类初始化
    int init_base(const std::string& feature_name, int output_num, bool rough){
        _feature_name = feature_name;
        _output_num = output_num;
        _rough = rough;
        return 0;
    }

    std::string feature_name(){
        return _feature_name;
    }

    int get_output_num(){
        return _output_num;
    }
    bool is_rough(){
        return _rough;
    }

protected:
    void set_output_num(int output_num){
        _output_num = output_num;
    }

private:
    std::string _feature_name;
    // 特征值个数。一个matching插件可以有多个特征值; 
    // 当output_num=0时,该插件不输出特征值,只对候选query进行处理,如分词、过滤等。
    int _output_num;
    // 该特征是否用于粗排
    bool _rough;
    DISALLOW_COPY_AND_ASSIGN(MatchingInterface);
};

} // namespace anyq

#endif  //BAIDU_NLP_ANYQ_MATCHING_INTERFACE_H
