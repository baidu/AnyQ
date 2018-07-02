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

#ifndef BAIDU_NLP_ANYQ_DICT_INTERFACE_H
#define BAIDU_NLP_ANYQ_DICT_INTERFACE_H

#include <string>

#include "anyq.pb.h"
#include "common/common_define.h"

namespace anyq
{

// 字典接口
class DictInterface {
public:
    // 默认不支持reload
    DictInterface() {
        _support_reload = false;
        _dict = NULL;
    }

    // 根据路径和配置加载词典
    virtual int load(const std::string& path, const DictConfig& config) = 0;

    virtual int release() = 0;
    
    // 获取词典
    void* get_dict() {
        return _dict;
    }

    // 是否支持reload
    bool support_reload() {
        return _support_reload;
    }

    virtual ~DictInterface() {
    };

protected:
    // 设置是否支持reload
    void set_support_reload(const bool& support_reload){
        _support_reload = support_reload;
    }

    void set_dict(void* dict){
        _dict = dict;
    }

private:
    bool _support_reload;
    void* _dict;
    DISALLOW_COPY_AND_ASSIGN(DictInterface);
};

} // namespace anyq
#endif  //BAIDU_NLP_ANYQ_DICT_INTERFACE_H
