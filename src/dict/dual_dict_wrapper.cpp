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

#include "dict/dual_dict_wrapper.h"
#include "common/plugin_factory.h"

namespace anyq {

DualDictWrapper::DualDictWrapper(const std::string& conf_path, const DictConfig& config) {
    // 创建2个词典对象，通过当前使用词典的index切换
    _config = config;
    std::string config_name = config.name();
    std::string config_type = config.type();
    std::string config_path = config.path();
    TRACE_LOG("config_name: %s", config_name.c_str());
    TRACE_LOG("config_type: %s", config_type.c_str());
    TRACE_LOG("config_path: %s", config_path.c_str());
    _dict_name = config_name;
    _dict_path = conf_path + config_path;
    _dual_dict[0] = NULL;
    _dual_dict[1] = NULL;
    _dual_dict[0] =
        static_cast<DictInterface*>(PLUGIN_FACTORY.create_plugin(config_type));
    _dual_dict[1] = 
        static_cast<DictInterface*>(PLUGIN_FACTORY.create_plugin(config_type));
    if (_dual_dict[0] == NULL || _dual_dict[1] == NULL){
        FATAL_LOG("can't find dict_type:%s", config_type.c_str());
        return;
    }
    _reload_able = false;
    if (_dual_dict[0]->support_reload()) {
        if (config.has_is_reload_able() && config.is_reload_able()) {
            _reload_able = true;
        }
    }
    _last_identifier = "";
    _cur_dict = 0;
    TRACE_LOG("dual dict init: %s, reload:%s", 
            _dict_name.c_str(), _reload_able ? "true" : "false");
}

int DualDictWrapper::reload() {
    std::string last_mtime_str("");
    // 获取词典表示，判断是否发生变化
    int ret = get_file_last_modified_time(_dict_path, last_mtime_str);
    if (ret != 0) {
        WARNING_LOG("get file stat error");
        return -1;
    }
    if (last_mtime_str == _last_identifier) {
        return 0;
    } else {
        TRACE_LOG("load dict file:%s", _dict_path.c_str());
        TRACE_LOG("dict file state: %s", last_mtime_str.c_str());
        uint8_t new_dict = (_cur_dict + 1) % 2;
        DictInterface* tmp_dict = _dual_dict[new_dict];
        if (tmp_dict->release() != 0) {
            FATAL_LOG("release before load %s error", _dict_path.c_str());
            return -1;
        }
        int ret = tmp_dict->load(_dict_path, _config);
        if (ret != 0) {
            FATAL_LOG("load dict %s error", _dict_path.c_str());
            return -1;
        } else {
            NOTICE_LOG("load dict %s success", _dict_path.c_str());
        }
        // 切换当前使用词典的index
        std::lock_guard<std::mutex> lock(_mutex);
        _cur_dict = new_dict;
        _last_identifier = last_mtime_str;
    }
    return 0;
}

int DualDictWrapper::release() {
    if (_dual_dict[0] != NULL) {
        if (_dual_dict[0]->release() != 0) {
            FATAL_LOG("release dict error:%s", _dict_name.c_str());
            return -1;
        }
        delete _dual_dict[0];
        _dual_dict[0] = NULL;
    }
    if (_dual_dict[1] != NULL) {
        if (_dual_dict[1]->release() != 0) {
            FATAL_LOG("release dict error:%s", _dict_name.c_str());
            return -1;
        }
        delete _dual_dict[1];
        _dual_dict[1] = NULL;
    }
    return 0;
}

DictInterface* DualDictWrapper::get() {
    std::lock_guard<std::mutex> lock(_mutex);
    return _dual_dict[_cur_dict];
}

void* DualDictWrapper::get_dict() {
    std::lock_guard<std::mutex> lock(_mutex);
    return _dual_dict[_cur_dict]->get_dict();
}

DualDictWrapper::~DualDictWrapper() {
    if (release() != 0) {
        FATAL_LOG("release dict error");
    }
}
} // namespace anyq
