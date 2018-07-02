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

#ifndef BAIDU_NLP_ANYQ_COMMON_DEFINE_H
#define BAIDU_NLP_ANYQ_COMMON_DEFINE_H
        
#include <vector>
#include <string>
#include "json/json.h"

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
        TypeName(const TypeName&); \
        TypeName(TypeName&&); \
        TypeName& operator=(const TypeName&); \
        TypeName& operator=(TypeName&&)

namespace anyq {

struct analysis_token_t{
    unsigned int length : 8;    // length of term, not include the '\0'
    unsigned int offset : 24;   // offset of term in input text
    std::string type;    // type info, e.g. POS tag
    unsigned int weight : 24;   // weight of term, NOT used
    struct{
        unsigned int m_lprop;   // property: high 32 bit
        unsigned int m_hprop;   // property: low 32 bit
    } prop;
    long index; // dict index: point to the address of dict item, used by wordseg
    std::string buffer; // change nerl_plus::token_t.buffer from char * to sring
    float analysis_term_weight = 0.0; // term weight
};
// 分析结果，分词，表示
struct AnalysisItem{
    int type;
    std::string query;
    std::vector<analysis_token_t> tokens_basic; // basic wordseg
    std::vector<float> query_emb;
};

struct AnalysisResult {
    std::vector<AnalysisItem> analysis;
    // 传入的额外信息, KV形式保存
    std::map<std::string, std::string> info;
    // analysis 执行信息
    std::string notice_log;
};

struct TextInfo {
    std::string id;
    std::string text;
    std::vector<analysis_token_t> tokens_basic;
    std::string json_info;
};

struct RetrievalItem {
    TextInfo query;
    // 考虑单query 多个答案情况，可根据answer进行排序
    std::vector<TextInfo> answer;
};

struct RetrievalResult {
    std::vector<RetrievalItem> items;
    // 设置标志位，若命中人工干预的结构，可略去之后的流程
    bool anyq_end;
    std::string notice_log;
};

struct RankItem {
    RetrievalItem retrieval_item;
    // 用于匹配的信息，可配置使用query或者answer
    TextInfo match_info;
    // 匹配的特征
    std::vector<float> features;
    // 最终的rank打分
    float ltr_score;
    // 是否保留当前候选
    bool abandoned;
};

typedef std::vector<RankItem> RankResult;

// AnyQ系统最终输出
struct ANYQItem {
    std::string query;
    std::string answer;
    float confidence;
    std::string qa_id;
    std::string json_info;
    std::string answer_info;
    std::vector<float> matching_features;
    std::vector<std::string> matching_feature_names;
};

struct ANYQResult {
    std::vector<ANYQItem> items;
    std::string notice_log;
};

const size_t URL_LENGTH = 500;
const unsigned MAX_TERM_COUNT = 1024;
const unsigned SUB_LOG_LEN = 1024;
const unsigned INITIAL_INDEX = 0xffff; 
// 操作solr http-api的header内容
const std::string HEADER_STR = "Content-Type:application/json;charset=UTF-8";

} //namespace anyq

#endif  // BAIDU_NLP_ANYQ_COMMON_DEFINE_H
