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

#ifndef BAIDU_NLP_ANYQ_UTILS_H
#define BAIDU_NLP_ANYQ_UTILS_H
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <set>
#include <sstream>
#include <fstream>
#include <unordered_map>
#include <algorithm>
#include <list>
#include "google/protobuf/text_format.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "glog/logging.h"
#include "glog/raw_logging.h"
#include "json/json.h"
#include "ilac.h"
#include "common/common_define.h"

#define INFO_LOG(...) RAW_LOG_INFO(__VA_ARGS__)
#define NOTICE_LOG(...) RAW_LOG_INFO(__VA_ARGS__)
#define TRACE_LOG(...) RAW_LOG_INFO(__VA_ARGS__)
#define DEBUG_LOG(...) RAW_LOG_INFO(__VA_ARGS__)
#define WARNING_LOG(...) RAW_LOG_WARNING(__VA_ARGS__)
#define FATAL_LOG(...) RAW_LOG_ERROR(__VA_ARGS__)

namespace anyq
{

// 按照分隔符切分字符串
void split_string(const std::string& input,
        std::vector<std::string>& result,
        const std::string& delimeter);

// 以 str \t int的格式读取字典文件
typedef std::unordered_map<std::string, int> hashmap_str2int;

// 以 str \t int的格式读取字典文件
typedef std::unordered_map<int, std::string> hashmap_int2str;

// 以 str \t float的格式读取字典文件
typedef std::unordered_map<std::string, float> hashmap_str2float;

// 以 str \t str的格式读取字典文件
typedef std::unordered_map<std::string, std::string> hashmap_str2str;

// KV, string -> string
int hash_load(const char* dict_file, std::unordered_map<std::string, std::string>& dict_map);

// KV, string -> float
int hash_load(const char* dict_file, std::unordered_map<std::string, float>& dict_map);

// KV, string -> int
int hash_load(const char* dict_file, std::unordered_map<std::string, int>& dict_map);

// KV, int -> string
int hash_load(const char* dict_file, std::unordered_map<int, std::string>& dict_map);

// 获取文件/目录的最后修改时间，作为标示
int get_file_last_modified_time(const std::string& file_path, std::string& mtime_str);

// 将json对象转为化retrieval_item结构体
int json2retrieval_item(Json::Value& json_item, RetrievalItem& retrieval_item);

// 以 str \t str的格式读取字典文件
typedef std::unordered_map<std::string, RetrievalItem> hashmap_str2retrieval_item;
int str2retrieval_item_load(const char* dict_file, hashmap_str2retrieval_item& dict_map);

// 读取配置文件，模板函数形式，哥策略模块均可使用
template<typename T> int load_config_from_file(const std::string& filename, T& param){
    int file_descriptor = open(filename.c_str(), O_RDONLY);
    if (file_descriptor == -1){
        FATAL_LOG("load_config_from_file failed, cant open %s", filename.c_str());
        return -1;
    }
    google::protobuf::io::FileInputStream fileInput(file_descriptor);
    // Parse return boo, success is true, fail is false
    if (!google::protobuf::TextFormat::Parse(&fileInput, &param)) {
        close(file_descriptor);
        FATAL_LOG("protobuf parse from file[%s] failed!", filename.c_str());
        return -1;
    }
    close(file_descriptor);
    return 0;
}

// analysis切词结果结构体按照buffer字符串进行比较
bool token_str_cmp(const analysis_token_t& a, const analysis_token_t& b);

// analysis切词结果结构体按照weight进行wordrank权重比较
bool token_weight_cmp(const analysis_token_t& a, const analysis_token_t& b);

// 计算两个字符按传的jaccard相似度
float jaccard_similarity(const std::string& sent1, const std::string& sent2);

// 计算输入query与召回query之间的余弦相似度,借助wordrank结果
float cosine_similarity(std::vector<analysis_token_t>& src_tokens,
        std::vector<analysis_token_t>& trg_tokens);

const char SPECIAL_CHAR[15] =
        {'+', '&', '(', ')', '{', '}', '[', ']', '^', '~', '*', '?', ':', '"', ' '};

const std::set<char> ESCAPE_CHAR(SPECIAL_CHAR, SPECIAL_CHAR + 15);

// 特殊字符转义,比如 , 转义为 \,
std::string escape(const std::string& str);

// url编码
std::string url_encode(const std::string& str_source);
std::string url_decode(std::string text);

// 编码转换
std::string utf8_to_gbk(const std::string& utf8_str);
std::string gbk_to_utf8(const std::string& gbk_str);

void trans_term_to_id(const hashmap_str2int &term2id_kv,
        const std::vector<analysis_token_t> &tokens,
        std::vector<int> &result_ids);

void trans_id_to_term(const hashmap_int2str &id2term_kv,
        std::vector<int> &result_ids,
        std::vector<std::string> &tokens);

int string_to_id_buffer(float* out_buffer,
        const int capacity,
        const std::vector<float>& id_strs);

// 十六进制
char from_hex(char ch);

int utf8_to_char(const std::string& sent, std::vector<std::string>& chars);

// JSON数据转为字符串
std::string json_dumps(const Json::Value& value);

int convert_analysis_token_t(const tag_t& token, const std::string& query, analysis_token_t& at);

// array格式的tokens转化为vector格式
int array_tokens_conduct(tag_t src_tokens[],
        int token_num,
        std::vector<analysis_token_t>& dst_tokens,
        const std::string& query);

int json_to_analysis_item(const Json::Value& v, AnalysisItem& analysis_item);
int json_to_map(const Json::Value& v, std::map<std::string, std::string>& info_map);
int json_to_token_t(const Json::Value& v, std::vector<analysis_token_t>& vdt);
void print_analysis_result(AnalysisResult& analysis_result);

} // namespace anyq

#endif  //BAIDU_NLP_ANYQ_UTILS_H
