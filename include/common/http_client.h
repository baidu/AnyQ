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

#ifndef BAIDU_NLP_ANYQ_HTTP_CLIENT_H
#define BAIDU_NLP_ANYQ_HTTP_CLIENT_H

#include <string.h>
#include <curl/curl.h>
#include <map>
#include "common/common_define.h"

namespace anyq {

class HttpClient {
public:
    HttpClient();
    ~HttpClient();
    // get请求
    int curl_get(const char* url, std::string* buffer);
    // post请求
    int curl_post(const char* url, const std::map<std::string, std::string>& para_map,
            std::string* buffer);
    int curl_post(const char* url, const std::string& para_str, std::string* buffer,
            const std::string& header_str);
    // 请求结束后，读取返回数据回调函数
    static int str_write_callback(char* data, size_t size, size_t nmemb, std::string* buffer);

private:
    // curl指针
    CURL* _p_curl;
    DISALLOW_COPY_AND_ASSIGN(HttpClient);
};
} // namespace anyq

#endif  // BAIDU_NLP_ANYQ_HTTP_CLIENT_H
