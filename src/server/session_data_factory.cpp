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

#include "server/session_data_factory.h"

namespace anyq {

SessionData::SessionData() : _use_solr(true) {
}

SessionData::~SessionData() {
}

int SessionData::init(DictManager* ptr_dict_manager,
                      const std::string& anyq_conf_file,
                      const std::string& solr_clear_passwd) {
    if (ptr_dict_manager == NULL) {
        FATAL_LOG("ptr_dict_manager is NULL");
        return -1;
    }
    if (_anyq_strategy.create_resource(*ptr_dict_manager, anyq_conf_file) != 0) {
        FATAL_LOG("anyq create resource failed, anyq_conf_file=%s", anyq_conf_file.c_str());
        return -1;
    }
    if (_solr_accessor.init(anyq_conf_file, solr_clear_passwd) != 0) {
        WARNING_LOG("solr accessor init not success!");
        _use_solr = false;
    }
    return 0;
}

SessionDataFactory::SessionDataFactory() {
}

SessionDataFactory::~SessionDataFactory() {
}

SessionDataFactory::SessionDataFactory(DictManager* ptr_dict_manager,
        const std::string& anyq_conf_file,
        const std::string& solr_clear_passwd) {
    _dict_manager = ptr_dict_manager;
    _anyq_conf_file = anyq_conf_file;
    _solr_clear_passwd = solr_clear_passwd;
}

void* SessionDataFactory::CreateData() const {
    if (_dict_manager == NULL) {
        FATAL_LOG("_dict_manager is NULL");
        return NULL;
    }
    SessionData* sd = new SessionData();
    if (sd->init(_dict_manager, _anyq_conf_file, _solr_clear_passwd) != 0) {
        FATAL_LOG("session data init failed.");
        return NULL;
    }
    DEBUG_LOG("session data init success!!!");

    return static_cast<void*>(sd);
}

void SessionDataFactory::DestroyData(void* session_data) const {
    delete static_cast<SessionData*>(session_data);
}

}
