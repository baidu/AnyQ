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

#ifndef BAIDU_NLP_ANYQ_RETRIEVAL_INTERFACE_H
#define BAIDU_NLP_ANYQ_RETRIEVAL_INTERFACE_H

#include "anyq.pb.h"
#include "common/utils.h"
#include "dict/dict_interface.h"
#include "dict/dict_manager.h"

namespace anyq {
    
// 检索插件接口
class RetrievalPluginInterface{
public:
    RetrievalPluginInterface() {};
    virtual ~RetrievalPluginInterface() {};
    // 基类初始化
    int init_base(const std::string& plugin_name, const int& num_result){
        _plugin_name = plugin_name;
        _num_result = num_result;
        return 0;
    }

    const std::string& plugin_name(){
        return _plugin_name;
    }

    uint32_t get_num_result(){
        return _num_result;
    }
    // 检索插件线程资源初始化
    virtual int init(DictMap* dict_map, const RetrievalPluginConfig& plugin_config) = 0;
    // 检索插件线程资源销毁
    virtual int destroy() = 0;
    // 根据query的analysis结果进行检索
    virtual int retrieval(const AnalysisResult& analysis_res, RetrievalResult& retrieval_res) = 0;

private:
    std::string _plugin_name;
    // 该插件检索召回的候选个数
    uint32_t _num_result;
    DISALLOW_COPY_AND_ASSIGN(RetrievalPluginInterface);
};

} // namespace anyq

#endif  // BAIDU_NLP_ANYQ_RETRIEVAL_INTERFACE_H
