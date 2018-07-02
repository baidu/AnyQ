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

#ifndef BAIDU_NLP_ANYQ_SESSION_DATA_FACTORY_H
#define BAIDU_NLP_ANYQ_SESSION_DATA_FACTORY_H

#include <string>
#include "brpc/data_factory.h"
#include "strategy/anyq_strategy.h"
#include "server/solr_accessor.h"

namespace anyq {

// 线程级数据
class SessionData {
public:
    SessionData();
    ~SessionData();
    // 使用全局字典和配置初始化anyq_strategy 
    int init(DictManager* ptr_dict_manager,
             const std::string& anyq_conf_file,
             const std::string& solr_clear_passwd);
    AnyqStrategy* get_anyq() {
        return &_anyq_strategy;
    }
    SolrAccessor* get_solr_accessor() {
        return _use_solr ? &_solr_accessor : NULL;
    }

private:
    AnyqStrategy _anyq_strategy;
    SolrAccessor _solr_accessor;
    bool _use_solr;
    DISALLOW_COPY_AND_ASSIGN(SessionData);
};

class SessionDataFactory : public brpc::DataFactory
{
public:
    SessionDataFactory();
    SessionDataFactory(DictManager* ptr_dict_manager,
        const std::string& anyq_conf_file,
        const std::string& solr_clear_passwd);
    ~SessionDataFactory();

    void* CreateData() const;
    void DestroyData(void* session_data) const;

private:
    // 词典是进程数据，以指针的形式传入，内存中只有一份词典
    DictManager* _dict_manager;
    std::string _anyq_conf_file;
    std::string _solr_clear_passwd;
    DISALLOW_COPY_AND_ASSIGN(SessionDataFactory);
};

} // namespace anyq

#endif  // BAIDU_NLP_ANYQ_SESSION_DATA_FACTORY_H
