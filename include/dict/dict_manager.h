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

#ifndef BAIDU_NLP_ANYQ_DICT_MANAGER_H
#define BAIDU_NLP_ANYQ_DICT_MANAGER_H
#include <string>
#include <unordered_map>
#include <chrono>
#include <thread>

#include "anyq.pb.h"
#include "dict/dict_interface.h"
#include "dict/dual_dict_wrapper.h"
#include "common/utils.h"

namespace anyq 
{
typedef std::string DictName;
typedef std::unordered_map<DictName, DualDictWrapper*> DictMap;

// 词典管理类，负责load/reload词典，获取词典等操作
class DictManager {
public:
    DictManager();
    
    // 根据路径，加载所有词典
    int load_dict(const std::string conf_path);
    
    // 释放词典
    int release_dict();

    // 获取所有词典
    DictMap* get_dict();
    
    ~DictManager();
    // reload函数
    void reload_func();
    
    bool is_dm_released();

private:
    // 词典map
    DictMap* _all_dict;
    
    // reload线程
    std::thread _reload_thread;

    // 是否有需要reload的词典
    bool _need_reload;
    
    // 词典是否已经释放
    bool _dm_released;

    std::mutex _mutex;
    
    DISALLOW_COPY_AND_ASSIGN(DictManager);
};

} // namespace anyq


#endif  //BAIDU_NLP_ANYQ_DICT_MANAGER_H
