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

#include "common/utils.h"
#include <algorithm>
#include <fstream>
#include <math.h>
#include <iconv.h>
#include <cassert>
#include <stdexcept>
#include <pthread.h>
#include "json/json.h"
#include "boost/algorithm/string.hpp"

namespace anyq {

int hash_load(const char* dict_file, std::unordered_map<std::string, std::string>& dict_map){
    std::fstream dict_fs(dict_file, std::fstream::in);
    if (!dict_fs.is_open()) {
        FATAL_LOG("open dict_file error");
        return -1;
    } else {
        std::string line;
        std::vector<std::string> fields;
        while (getline(dict_fs, line)) {
            split_string(line, fields, "\t");
            if (fields.size() != 2) {
                WARNING_LOG("field_size, %d", fields.size());
                WARNING_LOG("dict[%s] invalid line, %s", dict_file, line.c_str());
                continue;
            }
            dict_map[fields[0]] = fields[1];
        }
    }
    TRACE_LOG("hash load %s done", dict_file);
    return 0;
}

int hash_load(const char* dict_file, std::unordered_map<std::string, float>& dict_map){
    std::fstream dict_fs(dict_file, std::fstream::in);
    if (!dict_fs.is_open()) {
        FATAL_LOG("open dict_file error");
        return -1;
    } else {
        std::string line;
        std::vector<std::string> fields;
        while (getline(dict_fs, line)) {
            split_string(line, fields, "\t");
            if (fields.size() != 2) {
                WARNING_LOG("dict[%s] invalid line, %s", dict_file, line.c_str());
                continue;
            }
            dict_map[fields[0]] = atof(fields[1].c_str());
        }
    }
    TRACE_LOG("hash load %s done", dict_file);
    return 0;
}

int hash_load(const char* dict_file, std::unordered_map<std::string, int>& dict_map){
    std::fstream dict_fs(dict_file, std::fstream::in);
    if (!dict_fs.is_open()) {
        FATAL_LOG("open dict_file error");
        return -1;
    } else {
        std::string line;
        std::vector<std::string> fields;
        while (getline(dict_fs, line)) {
            split_string(line, fields, "\t");
            if (fields.size() != 2) {
                WARNING_LOG("dict[%s] invalid line, %s", dict_file, line.c_str());
                continue;
            }
            dict_map[fields[0]] = atoi(fields[1].c_str());
        }
    }
    TRACE_LOG("hash load %s done", dict_file);
    return 0;
}

int hash_load(const char* dict_file, std::unordered_map<int, std::string>& dict_map){
    std::fstream dict_fs(dict_file, std::fstream::in);
    if (!dict_fs.is_open()) {
        FATAL_LOG("open dict_file error");
        return -1;
    } else {
        std::string line;
        std::vector<std::string> fields;
        while (getline(dict_fs, line)) {
            split_string(line, fields, "\t");
            if (fields.size() != 2) {
                WARNING_LOG("dict[%s] invalid line, %s", dict_file, line.c_str());
                continue;
            }
            dict_map[atoi(fields[0].c_str())] = fields[1];
        }
    }
    TRACE_LOG("hash load %s done", dict_file);
    return 0;
}

int get_file_last_modified_time(const std::string& file_path, std::string& mtime_str) {
    struct stat f_stat;
    if (stat(file_path.c_str(), &f_stat) != 0) {
        WARNING_LOG("get file state error: %s", file_path.c_str()); 
        return -1;
    }
    mtime_str = ctime(&f_stat.st_mtime);
    return 0;
}

void split_string(const std::string& input,
                  std::vector<std::string>& result,
                  const std::string& delimeter) {
    std::string::size_type pos1;
    std::string::size_type pos2;
    result.clear();
    pos2 = input.find(delimeter);
    pos1 = 0;

    while (std::string::npos != pos2) {
        result.push_back(input.substr(pos1, pos2 - pos1));
        pos1 = pos2 + delimeter.size();
        pos2 = input.find(delimeter, pos1);
    }
    result.push_back(input.substr(pos1, input.length()));
}

int utf8_to_char(const std::string& sent, std::vector<std::string>& chars) {
    size_t len = sent.size();
    chars.clear();
    for (size_t i = 0; i < len;) {
        size_t beg = i;
        unsigned char p = (unsigned char) sent[i];
        if (p < 0x80) {
            if (p == ' ') {
                ++i;
            }else {
                while (i < len && p < 0x80 && p != ' ') {
                    p = (unsigned char) sent[++i];
                }
            }
        }else if (p < 0xC0) {
            return -1;
        }else if (p < 0xE0) {
            i += 2;
        }else if (p < 0xF0) {
            i += 3;
        }else if (p < 0xF8) {
            i += 4;
        }else if (p < 0xFC) {
            i += 5;
        }else if (p < 0xFE) {
            i += 6;
        }else {
            return -1;
        }
        if (i > len) {
            return -1;
        }
        chars.push_back(sent.substr(beg, i - beg));
    }
    return 0;
}

// 计算两个字符按传的jaccard相似度
float jaccard_similarity(const std::string& sent1, const std::string& sent2) {
    std::vector<std::string> chars_a;
    int ret = utf8_to_char(sent1, chars_a);
    if (ret != 0 || chars_a.size() == 0) {
        return 0.0;
    }
    std::vector<std::string> chars_b;
    ret = utf8_to_char(sent2, chars_b);
    if (ret != 0 || chars_b.size() == 0) {
        return 0.0;
    }

    std::sort(chars_a.begin(), chars_a.end());
    std::sort(chars_b.begin(), chars_b.end());

    // 交集
    std::vector<std::string> words_intersect;
    std::set_intersection(chars_a.begin(), chars_a.end(), chars_b.begin(), chars_b.end(),
            std::back_inserter(words_intersect));

    // 并集
    std::vector<std::string> words_union;
    std::set_union(chars_a.begin(), chars_a.end(), chars_b.begin(), chars_b.end(),
            std::back_inserter(words_union));

    // 交集长度 /并集长度
    float sim = float(words_intersect.size()) / float(words_union.size());

    return sim;
}

// 将json对象转为化retrieval_item结构体
int json2retrieval_item(Json::Value& json_item, RetrievalItem& retrieval_item) {
    retrieval_item.query.id = "";
    retrieval_item.query.text = "";
    retrieval_item.answer.clear();
    retrieval_item.query.json_info = "";
    retrieval_item.query.tokens_basic.clear();

    if (!json_item.isMember("id") || json_item["id"].isNull()) {
        WARNING_LOG("json[%s], id not exist;", json_item.toStyledString().c_str());
        return -1;
    }
    retrieval_item.query.id = json_item["id"].asString();
    json_item.removeMember("id");

    if (!json_item.isMember("question") || json_item["question"].isNull()) {
        WARNING_LOG("json[%s], qeury not exist;", json_item.toStyledString().c_str());
        return -1;
    }
    retrieval_item.query.text = json_item["question"].asString();
    json_item.removeMember("question");

    if (json_item.isMember("answer")) {
        Json::Value& answer_json = json_item["answer"];
        retrieval_item.answer.clear();
        if (answer_json.type() == Json::arrayValue) {
            for (uint32_t i = 0; i < answer_json.size(); i++) {
                TextInfo ans;
                ans.text = answer_json[i].asString();
                ans.tokens_basic.clear();
                retrieval_item.answer.push_back(ans);
            }
        } else if (answer_json.type() == Json::stringValue) {
            TextInfo ans;
            ans.text = answer_json.asString();
            ans.tokens_basic.clear();
            retrieval_item.answer.push_back(ans);
        }
        json_item.removeMember("answer");
    }
    retrieval_item.query.tokens_basic.clear();
    if (json_item.isMember("basic_token")) {
        Json::Value basic_token = json_item["basic_token"];
        for (uint32_t j = 0; j < basic_token.size(); j++) {
            analysis_token_t token_t;
            token_t.buffer = basic_token[j].asString();
            retrieval_item.query.tokens_basic.push_back(token_t);
        }
        json_item.removeMember("basic_token");
    }
    if (json_item.isMember("basic_weight")) {
        Json::Value basic_weight = json_item["basic_weight"];
        if (retrieval_item.query.tokens_basic.size() == basic_weight.size()) {
            for (uint32_t j = 0; j < basic_weight.size(); j++) {
                retrieval_item.query.tokens_basic[j].weight = basic_weight[j].asFloat();
            }
        }
        json_item.removeMember("basic_weight");
    }

    Json::FastWriter writer;
    // 保存检索的其他字段
    retrieval_item.query.json_info = writer.write(json_item);

    return 0;
}

int str2retrieval_item_load(const char* dict_file, hashmap_str2retrieval_item& dict_map) {
    std::fstream dict_fs(dict_file, std::fstream::in);
    if (!dict_fs.is_open()) {
        FATAL_LOG("open dict_file error");
        return -1;
    } else {
        Json::Reader reader;
        std::string line;
        std::vector<std::string> fields;
        while (getline(dict_fs, line)) {
            fields.clear();
            split_string(line, fields, "\t");
            if (fields.size() != 2) {
                WARNING_LOG("dict[%s] invalid line, %s", dict_file, line.c_str());
                continue;
            }
            Json::Value json_item;
            if (!reader.parse(fields[1], json_item) || json_item.type() != Json::objectValue) {
                WARNING_LOG("annoy forword index parse error, id=%s, value=%s;",
                        fields[0].c_str(), fields[1].c_str());
                continue;
            }
            RetrievalItem retrieval_item;
            if (json2retrieval_item(json_item, retrieval_item) != 0) {
                continue;
            }
            dict_map[fields[0]] = retrieval_item;
        }
    }
    return 0;
}

// analysis切词结果结构体按照buffer字符串进行比较
bool token_str_cmp(const analysis_token_t& a, const analysis_token_t& b) {
    return a.buffer < b.buffer;
}

// analysis切词结果结构体按照weight进行wordrank权重比较
bool token_weight_cmp(const analysis_token_t& a, const analysis_token_t& b) {
    return a.analysis_term_weight < b.analysis_term_weight;
}

// 计算输入query与召回query之间的余弦相似度,借助wordrank结果
float cosine_similarity(std::vector<analysis_token_t>& src_tokens, std::vector<analysis_token_t>& trg_tokens) {
    std::sort(src_tokens.begin(), src_tokens.end(), token_str_cmp);
    std::sort(trg_tokens.begin(), trg_tokens.end(), token_str_cmp);

    std::vector<analysis_token_t>::iterator analysis_it = src_tokens.begin();
    std::vector<analysis_token_t>::iterator retrieval_it = trg_tokens.begin();

    float product = 0.0;
    float norm_analysis = 0.0000001;
    float norm_retrieval = 0.0000001;

    while (analysis_it != src_tokens.end() && retrieval_it != trg_tokens.end()) {
        if (analysis_it->buffer == retrieval_it->buffer) {
            product = analysis_it->analysis_term_weight * retrieval_it->analysis_term_weight;
            norm_analysis += analysis_it->analysis_term_weight * analysis_it->analysis_term_weight;
            norm_retrieval += retrieval_it->analysis_term_weight * retrieval_it->analysis_term_weight;
            ++analysis_it;
            ++retrieval_it;
        } else if (analysis_it->buffer < retrieval_it->buffer) {
            norm_analysis += analysis_it->analysis_term_weight * analysis_it->analysis_term_weight;
            ++analysis_it;
        } else {
            norm_retrieval += retrieval_it->analysis_term_weight * retrieval_it->analysis_term_weight;
            ++retrieval_it;
        }
    }
    while (analysis_it != src_tokens.end()) {
        norm_analysis += analysis_it->analysis_term_weight * analysis_it->analysis_term_weight;
        ++analysis_it;
    }
    while (retrieval_it != trg_tokens.end()) {
        norm_retrieval += retrieval_it->analysis_term_weight * retrieval_it->analysis_term_weight;
        ++retrieval_it;
    }

    float cos_sim = product / (sqrt(norm_analysis) * sqrt(norm_retrieval));
    return cos_sim;
}

// 特殊字符转义,比如 , 转义为 \,
std::string escape(const std::string& str) {
    std::string escape_str;
    for (size_t i = 0; i < str.size(); i++) {
        if (str[i] & 0x80) {
            escape_str += str[i];
            if (i == (str.size() - 1)) {
                break;
            }
            i++;
            escape_str += str[i];
        } else {
            if (ESCAPE_CHAR.find(str[i]) != ESCAPE_CHAR.end()) {
                escape_str += '\\';
                escape_str += str[i];
            } else {
                escape_str += str[i];
            }
        }
    }
    return escape_str;
}

std::string url_encode(const std::string& str_source)
{
    char const *in_str = str_source.c_str();
    int in_str_len = strlen(in_str);
    unsigned char *to = (unsigned char *)malloc(3 * in_str_len + 1);
    unsigned char *start = to;
    unsigned char const *from = (unsigned char *)in_str;
    unsigned char const *end = (unsigned char *)in_str + in_str_len;
    unsigned char hexchars[] = "0123456789ABCDEF";

    while (from < end) {
        register unsigned char c = *from++;

        if (c == ' ') {
            *to++ = '+';
        }
        else if ((c < '0' && c != '-' && c != '.') ||
            (c < 'A' && c > '9') ||
            (c > 'Z' && c < 'a' && c != '_') ||
            (c > 'z')) {
            to[0] = '%';
            to[1] = hexchars[c >> 4];
            to[2] = hexchars[c & 15];
            to += 3;
        }
        else {
            *to++ = c;
        }
    }
    *to = 0;

    int out_str_len = to - start;
    std::string out_str = (char *)start;
    free(start);
    return out_str;
}

std::string url_decode(std::string text) {
    char h = 0;
    std::ostringstream escaped;
    escaped.fill('0');
    for (auto i = text.begin(), n = text.end(); i != n; ++i) {
        std::string::value_type c = (*i);
        if (c == '%') {
            if (i[1] && i[2]) {
                h = from_hex(i[1]) << 4 | from_hex(i[2]);
                escaped << h;
                i += 2;
            }
        } else if (c == '+') {
            escaped << ' ';
        } else {
            escaped << c;
        }
    }
    return escaped.str();
}

std::string utf8_to_gbk(const std::string& utf8_str) {
    if (utf8_str.size() == 0) {
        return "";
    }
    std::vector<char> gbk_buf(utf8_str.size() * 2);
    char* in_buf = const_cast<char*>(utf8_str.c_str());
    size_t in_size = utf8_str.size();
    char* out_buf = &gbk_buf[0];
    size_t out_size = gbk_buf.size();
    iconv_t cd = iconv_open("GBK", "UTF-8");
    size_t res = iconv(cd, &in_buf, &in_size, &out_buf, &out_size);
    iconv_close(cd);
    if (res == size_t(-1)) {
        return "";
    }
    std::string gbk_str(&gbk_buf[0], (gbk_buf.size() - out_size));
    return gbk_str;
}

std::string gbk_to_utf8(const std::string& gbk_str) {
    std::vector<char> utf8_buf(gbk_str.size() * 3 / 2 + 1);
    char* in_buf = const_cast<char*>(gbk_str.c_str());
    size_t in_size = gbk_str.size();
    char* out_buf = &utf8_buf[0];
    size_t out_size = utf8_buf.size();
    iconv_t cd = iconv_open("UTF-8", "GBK");
    size_t res = iconv(cd, &in_buf, &in_size, &out_buf, &out_size);
    iconv_close(cd);
    if (res == size_t(-1)) {
        return "";
    }
    std::string utf8_str(&utf8_buf[0], (utf8_buf.size() - out_size));
    return utf8_str;
}

void trans_term_to_id(const hashmap_str2int &term2id_kv,
        const std::vector<analysis_token_t> &tokens,
        std::vector<int> &result_ids){
    int length = tokens.size();
    hashmap_str2int::const_iterator got;
    for (int i = 0; i < length; i++) {
        got = term2id_kv.find(tokens[i].buffer);
        if (got != term2id_kv.end()){
            result_ids.push_back(got->second);
        }
    }
}

void trans_id_to_term(const hashmap_int2str &id2term_kv,
        std::vector<int> &result_ids,
        std::vector<std::string> &tokens){
    int length = result_ids.size();
    hashmap_int2str::const_iterator got;
    for (int i = 0; i < length; i++) {
        got = id2term_kv.find(result_ids[i]);
        if (got != id2term_kv.end()){
            tokens.push_back(got->second);
        }
    }
}

int string_to_id_buffer(float* out_buffer,
        const int capacity,
        const std::vector<float>& id_strs) {
    if (id_strs.size() > capacity){
        WARNING_LOG("input length larger");
        return -1;
    }
    for (size_t i = 0; i < id_strs.size(); i++) {
        out_buffer[i] = id_strs[i];
    }
    return id_strs.size();
}

// 十六进制
char from_hex(char ch) {
    return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
}

std::string json_dumps(const Json::Value& value){
    std::string result;
    Json::FastWriter writer;
    result = writer.write(value);
    boost::trim(result);
    return result;
}

int convert_analysis_token_t(const tag_t& token, const std::string& query, analysis_token_t& at) {
    if (token.offset + token.length > query.size()) {
        return -1;
    }
    at.length = token.length;
    at.offset = token.offset;
    at.type = token.type;
    at.buffer = query.substr(at.offset, at.length);
    at.analysis_term_weight = 0.0;
    return 0;
}

int array_tokens_conduct(tag_t src_tokens[],
        int token_num,
        std::vector<analysis_token_t>& dst_tokens,
        const std::string& query) {
    dst_tokens.resize(token_num);
    for (int i = 0; i < token_num; ++i){
        if (convert_analysis_token_t(src_tokens[i], query, dst_tokens[i]) != 0) {
            return -1;
        }
        dst_tokens[i].analysis_term_weight = 1.0 / token_num;
    }
    return 0;
}

int json_to_map(const Json::Value& v, std::map<std::string, std::string>& info_map) {
    Json::Value::Members mem = v.getMemberNames();
    for (auto iter = mem.begin(); iter != mem.end(); ++iter)
    {
        if (v[*iter].type() == Json::stringValue)
        {
            info_map[*iter] = v[*iter].asString();
        }
    }
    return 0;
}

int json_to_token_t(const Json::Value& v, std::vector<analysis_token_t>& vdt){
    const Json::Value& tb = v;
    if (v.type() != Json::arrayValue)
    {
        DEBUG_LOG("json_to_token_t err");
        return -1;
    }
    DEBUG_LOG("token_t length: %d", (int)tb.size());
    for (int i = 0; i < (int)tb.size(); i++)
    {
        analysis_token_t dt;
        if (!tb[i]["length"].isNull()) dt.length = tb[i]["length"].asUInt();
        if (!tb[i]["offset"].isNull()) dt.offset = tb[i]["offset"].asUInt();
        if (!tb[i]["type"].isNull()) dt.type = tb[i]["type"].asUInt();
        if (!tb[i]["weight"].isNull()) dt.weight = tb[i]["weight"].asUInt();
        if (!tb[i]["buffer"].isNull()) dt.buffer = tb[i]["buffer"].asString();
        if (!tb[i]["analysis_term_weight"].isNull())
            dt.analysis_term_weight = tb[i]["analysis_term_weight"].asDouble();
        vdt.push_back(dt);
    }
    return 0;
}

int json_to_analysis_item(const Json::Value& v, AnalysisItem& analysis_item){
    if (!v["question"].isNull()){
        analysis_item.query = v["question"].asString();
        DEBUG_LOG("in json_to_analysis_item, query:%s", analysis_item.query.c_str());
    }
    if (!v["type"].isNull()) {
        analysis_item.type = v["type"].asInt();
        DEBUG_LOG("in json_to_analysis_item, type:%s", analysis_item.type);
    }
    if (!v["tokens_basic"].isNull()) {
        DEBUG_LOG("json parser tokens_basic");
        Json::Value tb = v["tokens_basic"];
        json_to_token_t(tb, analysis_item.tokens_basic);
    }
    return 0;
}

void print_analysis_result(AnalysisResult& analysis_result){
    std::vector<AnalysisItem>& analysis_items = analysis_result.analysis;
    std::map<std::string, std::string>& info = analysis_result.info;
    std::map<std::string, std::string>::iterator it;
    for (int i = 0; i<analysis_items.size(); i++){
        DEBUG_LOG("query:%s", analysis_items[i].query.c_str());
        std::vector<analysis_token_t>& bdt = analysis_items[i].tokens_basic;
        DEBUG_LOG("tokens_basic size is %d", bdt.size());
        for (int j = 0; j < bdt.size(); j++) {
            DEBUG_LOG("tokens_basic\tbuffer:%s\tlength:%d\toffset:%d\tanalysis_term_weight:%f",
                      bdt[j].buffer.c_str(), bdt[j].length, bdt[j].offset, bdt[j].analysis_term_weight);
        }
    }
    for (it = info.begin(); it != info.end(); ++it){
        DEBUG_LOG("info\tkey:%s\tvalue:%s", it->first.c_str(), it->second.c_str());
    }
}

} // namespace anyq
