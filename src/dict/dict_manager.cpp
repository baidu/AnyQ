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

#include "dict/dict_manager.h"
#include "common/plugin_factory.h"

namespace anyq {

const int DICT_RELOAD_INTERVAL = 1000;

DictManager::DictManager(){
    _all_dict = NULL;
    _all_dict = new DictMap();
    if (_all_dict == NULL) {
        FATAL_LOG("new dict map error");
    }
    _need_reload = false;
    _dm_released = false;
}

DictManager::~DictManager(){
    release_dict();
    if (_need_reload) {
        _reload_thread.join();
    }
}

int DictManager::load_dict(const std::string conf_path) {
    DictManagerConfig dict_manager_param; 
    int ret = load_config_from_file(conf_path + "/dict.conf", dict_manager_param);
    if (ret != 0) {
        FATAL_LOG("load dict.conf from %s error", conf_path.c_str());
        return -1;
    }

    for (int i = 0; i < dict_manager_param.dict_config_size(); i++) {
        DictConfig config = dict_manager_param.dict_config(i);
        DualDictWrapper* p_dual_dict_wrapper = new DualDictWrapper(conf_path, config);
        std::string dict_name = p_dual_dict_wrapper->get_dict_name();
        if (p_dual_dict_wrapper->reload() != 0){
            FATAL_LOG("dict load failed: %s", dict_name.c_str());
            return -1;
        }
        // 如果有需要reload的词典存在
        if (p_dual_dict_wrapper->is_reload_able()) {
            _need_reload = true;
        }
        TRACE_LOG("dict load success: %s", dict_name.c_str());
        (*_all_dict)[dict_name] = p_dual_dict_wrapper;
    }
    
    // 有reload的词典，则reload线程开启
    if (_need_reload) {
        _reload_thread = std::thread(&DictManager::reload_func, this);
    }
    return 0;
}

void DictManager::reload_func() {
    while (!_dm_released) {
        // 互斥锁的作用域
        {
        // 词典是否已经被释放
            std::lock_guard<std::mutex> lock(_mutex);
            if (_all_dict != NULL) {
                for (DictMap::iterator it = _all_dict->begin(); it != _all_dict->end(); ++it) {
                    DualDictWrapper* p_dual_dict_wrapper = (DualDictWrapper*)it->second;
                    if (p_dual_dict_wrapper != NULL && p_dual_dict_wrapper->is_reload_able()) {
                        if (p_dual_dict_wrapper->reload() != 0) {
                            WARNING_LOG("dict reload error:%s", 
                                    p_dual_dict_wrapper->get_dict_name().c_str());
                        }
                    }
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(DICT_RELOAD_INTERVAL));
    }
}

int DictManager::release_dict() {
    // 互斥锁，判断词典是否在reload过程中
    std::lock_guard<std::mutex> lock(_mutex);
    if (_all_dict != NULL) {
        for (DictMap::iterator it = _all_dict->begin(); it != _all_dict->end(); ++it) {
            DualDictWrapper* p_dual_dict_wrapper = it->second;
            if (p_dual_dict_wrapper == NULL) {
                continue;
            }
            if (p_dual_dict_wrapper->release() != 0) {
                FATAL_LOG("release dict error:%s", (it->first).c_str());
                return -1;
            }
            TRACE_LOG("dict release success:%s", (it->first).c_str()); 
            delete p_dual_dict_wrapper;
            it->second = NULL;
        }
        _all_dict->clear();
        delete _all_dict;
        _all_dict = NULL;
    }
    _dm_released = true;
    return 0;
}

DictMap* DictManager::get_dict() {
    return _all_dict;
}
}
