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

#include "common/plugin_factory.h"

namespace anyq{

PluginFactory* PluginFactory::_instance = NULL;

// 注册组件回调函数
int PluginFactory::register_plugin(std::string plugin_type, PluginCreateFunc create_func) {
    _plugin_map[plugin_type] = create_func;
    return 0;
}

// 根据组件类型生成一个组件实例, 自己创建的实例自己销毁，工厂不负责
void* PluginFactory::create_plugin(std::string plugin_type) {
    if (_plugin_map.count(plugin_type) < 1) {
        FATAL_LOG("create plugin[%s] failed.", plugin_type.c_str());
        return NULL;
    }
    return _plugin_map[plugin_type]();
}

PluginFactory& PluginFactory::instance() {
    if(_instance == NULL)
        _instance = new PluginFactory();
        return *_instance;            
}

}
