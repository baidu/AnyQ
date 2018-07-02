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

#ifndef BAIDU_NLP_ANYQ_DICT_ADAPTER_H
#define BAIDU_NLP_ANYQ_DICT_ADAPTER_H

#include <fstream>
#include <mutex>
#include <unordered_map>
#include <string>
#include <iostream>

#include "ilac.h"
#ifdef USE_TENSORFLOW
#include "tensorflow/core/public/session.h"
#include "tensorflow/core/platform/env.h"
#else
#include "paddle/fluid/framework/init.h"
#include "paddle/fluid/framework/lod_tensor.h"
#include "paddle/fluid/inference/io.h"
#endif
#include "common/utils.h"
#include "dict/dict_interface.h"

namespace anyq
{

// KV词典模板类
template <class T1, class T2>
class HashAdapter : public DictInterface {
    typedef std::unordered_map<T1, T2> hash_type;
public:
    // KV类型词典支持reload
    HashAdapter() {
        set_support_reload(true);
    }

    virtual ~HashAdapter() override {
        void* dict = get_dict();
        if (dict != NULL) {
            delete static_cast<hash_type*>(dict);
        }
    }

    virtual int load(const std::string& path, const DictConfig& config) override {
        hash_type* tmp_dict = new hash_type();
        if (tmp_dict == NULL) {
            FATAL_LOG("new string2float dict error");
            return -1;
        }
        DEBUG_LOG("%s", path.c_str());
        // KV词典load
        int ret = hash_load(path.c_str(), (*tmp_dict));
        if (ret != 0) {
            FATAL_LOG("load string2float dict %s error", path.c_str());
            return -1;
        }
        set_dict((void*)tmp_dict);
        return 0;
    }

    virtual int release() override {
        void* dict = get_dict();
        if (dict != NULL) {
            hash_type* tmp_dict = static_cast<hash_type*>(dict);
            tmp_dict->clear();
            delete tmp_dict;
            set_dict(NULL);
        }
        return 0;
    }

    int get(const std::string& key, std::string& value) {
        hash_type* tmp_dict = static_cast<hash_type*>(get_dict());
        typename hash_type::const_iterator it = tmp_dict->find(key);
        if (it != tmp_dict->end()) {
            value = it->second;
        } else {
            WARNING_LOG("key[%s] not exist!", key.c_str());
            return -1;
        }
        return 0;
    }

private:
    DISALLOW_COPY_AND_ASSIGN(HashAdapter);
};

typedef HashAdapter<std::string, float> String2FloatAdapter;

typedef HashAdapter<std::string, int> String2IntAdapter;

typedef HashAdapter<std::string, std::string> String2StringAdapter;

// 干预机制，KV形式构建检索Item
class String2RetrievalItemAdapter : public DictInterface {
public:
    String2RetrievalItemAdapter();
    virtual ~String2RetrievalItemAdapter() override;
    virtual int load(const std::string& path, const DictConfig& config) override;
    virtual int release() override;
    // 查找Key对应的Item
    int get(const std::string& key, RetrievalItem& retrieval_item);
private:
    DISALLOW_COPY_AND_ASSIGN(String2RetrievalItemAdapter);
};

struct WordsegPack {
    void* lexer_dict;
};

// 中文分词 词典
class WordsegAdapter : public DictInterface {
public:
    WordsegAdapter();
    virtual ~WordsegAdapter() override;
    virtual int load(const std::string& path, const DictConfig& config) override;
    virtual int release() override;
private:
    DISALLOW_COPY_AND_ASSIGN(WordsegAdapter);
};

#ifndef USE_TENSORFLOW
// Paddle词典
struct PaddlePack {
    paddle::platform::CPUPlace *place;
    paddle::framework::Executor* executor;
    paddle::framework::Scope* scope;
    std::unique_ptr<paddle::framework::ProgramDesc> inference_program;
    hashmap_str2int term2id;
    hashmap_int2str id2term;
    unsigned using_num;
    std::mutex dict_mutex; 
};

class PaddleSimAdapter : public DictInterface {
public:
    PaddleSimAdapter();
    virtual ~PaddleSimAdapter() override;
    virtual int load(const std::string& path, const DictConfig& config) override;
    virtual int release() override;
    static bool _s_initialized;
private:
    DISALLOW_COPY_AND_ASSIGN(PaddleSimAdapter);
};

#else
// Tensorflow 词典
struct TFPack {
    tensorflow::GraphDef graphdef;
    hashmap_str2int term2id;
};

class TFModelAdapter: public DictInterface {
public:
    TFModelAdapter();
    virtual ~TFModelAdapter() override;
    virtual int load(const std::string& path, const DictConfig& config) override;
    virtual int release() override;
private:
    DISALLOW_COPY_AND_ASSIGN(TFModelAdapter);
};

#endif

} //namespace anyq

#endif  //BAIDU_NLP_ANYQ_DICT_ADAPTER_H
