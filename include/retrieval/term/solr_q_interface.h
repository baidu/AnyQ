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

#ifndef BAIDU_NLP_ANYQ_SOLR_Q_INTERFACE_H
#define BAIDU_NLP_ANYQ_SOLR_Q_INTERFACE_H

#include "anyq.pb.h"
#include "common/utils.h"
#include "dict/dict_interface.h"
#include "dict/dual_dict_wrapper.h"

namespace anyq {
// solr索引表达式构建插件
class SolrQInterface{
public:
    SolrQInterface() {};
    virtual ~SolrQInterface() {};
    int init_base(const std::string& plugin_name,
            const std::string& solr_field, 
            const std::string& source_name){
        _plugin_name = plugin_name;
        _solr_field = solr_field;
        _source_name = source_name;
        return 0;
    }
    // 线程初始化
    virtual int init(DualDictWrapper* dict, const SolrQConfig& solr_q_config) = 0;
    // 构造solr索引表达式
    virtual int make_q(const AnalysisResult& analysis_res, int analysis_idx, std::string& q) = 0;

    const std::string& plugin_name(){
        return _plugin_name;
    }
    const std::string& get_solr_field(){
        return _solr_field;
    }
    const std::string& get_source_name(){
        return _source_name;
    }

private:
    std::string _plugin_name;
    // solr检索字段
    std::string _solr_field;
    // 指定analysis结果中用于检索的内容
    std::string _source_name;
    DISALLOW_COPY_AND_ASSIGN(SolrQInterface);
};

} // namespace anyq

#endif  // BAIDU_NLP_ANYQ_SOLR_Q_INTERFACE_H
