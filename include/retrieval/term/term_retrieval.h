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

#ifndef BAIDU_NLP_ANYQ_TERM_RETRIEVAL_H
#define BAIDU_NLP_ANYQ_TERM_RETRIEVAL_H

#include "retrieval/retrieval_interface.h"
#include "common/http_client.h"
#include "retrieval/term/solr_q_interface.h"

namespace anyq {
// 基于solr的term检索插件
class TermRetrievalPlugin : public RetrievalPluginInterface
{
public:
    TermRetrievalPlugin() {};
    virtual ~TermRetrievalPlugin() override {};
    virtual int init(DictMap* dict_map, const RetrievalPluginConfig& plugin_config) override;
    virtual int destroy() override;
    // 构造检索solr所需的q参数
    int make_fetch_q(const AnalysisResult& analysis_result, int analysis_idx, std::string& q);
    // 对solr返回结构进行解析
    int solr_result_parse(const char* solr_result, RetrievalResult& retrieval_result);
    // 请求solr
    int solr_request(const char* q, std::string* buffer);
    virtual int retrieval(const AnalysisResult& analysis_res,
        RetrievalResult& retrieval_res) override;

private:
    // http请求客户端
    HttpClient _http_client;
    // solr服务器ip
    std::string _search_host;
    // solr服务器端口
    int32_t _search_port;
    // collention name
    std::string _engine_name;
    // 请求solr的字段
    std::string _solr_request_fl;
    // solr_q builder
    std::vector<SolrQInterface*> _solr_q_builder;
    // solr返回的字段
    std::string _solr_result_fl;
    DISALLOW_COPY_AND_ASSIGN(TermRetrievalPlugin);
};

} // namespace anyq
#endif  // BAIDU_NLP_ANYQ_TERM_RETRIEVAL_H
