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

#include "common/http_client.h"
#include "common/common_define.h"
#include "common/utils.h"

namespace anyq {

HttpClient::HttpClient(){
    _p_curl = curl_easy_init();
}

HttpClient::~HttpClient(){
    curl_easy_cleanup(_p_curl);
}

int HttpClient::str_write_callback(char* data, size_t size, size_t nmemb, std::string* buffer) {
    unsigned long sizes = size * nmemb;
    if (buffer == NULL) {
        return 0;
    }
    buffer->append(data, sizes);
    return sizes;
}

// get请求
int HttpClient::curl_get(const char* url, std::string* buffer)
{
    if (url == NULL || buffer == NULL) {
        WARNING_LOG("url or buffer is NULL;");
        return -1;
    }

    curl_easy_setopt(_p_curl, CURLOPT_URL, url);
    curl_easy_setopt(_p_curl, CURLOPT_WRITEDATA, buffer);
    curl_easy_setopt(_p_curl, CURLOPT_WRITEFUNCTION, HttpClient::str_write_callback);

    /* get it! */
    CURLcode res = curl_easy_perform(_p_curl);
    if (res != CURLE_OK) {
        WARNING_LOG("curl_easy_perform Failed[%s];", curl_easy_strerror(res));
        return -1;
    }
    return 0;
}

// post请求
int HttpClient::curl_post(const char* url,
        const std::map<std::string, std::string>& para_map, std::string* buffer)
{
    if (url == NULL || buffer == NULL) {
        WARNING_LOG("url or buffer is NULL;");
        return -1;
    }

    std::string para_url = "";
    std::map<std::string, std::string>::const_iterator para_iterator;
    bool is_first = true;
    for (para_iterator = para_map.begin(); para_iterator != para_map.end(); ++para_iterator) {
        if (is_first) {
            is_first = false;
        } else {
            para_url.append("&");
        }
        std::string key = para_iterator->first;
        std::string value = para_iterator->second;
        para_url.append(key);
        para_url.append("=");
        para_url.append(value);
    }

    DEBUG_LOG("para_url: %s", para_url.c_str());

    curl_easy_setopt(_p_curl, CURLOPT_URL, url);
    curl_easy_setopt(_p_curl, CURLOPT_POSTFIELDS, para_url.c_str());
    curl_easy_setopt(_p_curl, CURLOPT_WRITEDATA, buffer);
    curl_easy_setopt(_p_curl, CURLOPT_WRITEFUNCTION, HttpClient::str_write_callback);

    /* get it! */
    CURLcode res = curl_easy_perform(_p_curl);
    if (res != CURLE_OK) {
        WARNING_LOG("curl_easy_perform Failed[%s];", curl_easy_strerror(res));
        return -1;
    }

    return 0;
}

int HttpClient::curl_post(const char* url, const std::string& para_str, std::string* buffer,
            const std::string& header_str) {
    if (url == NULL || buffer == NULL) {
        WARNING_LOG("url or buffer is NULL;");
        return -1;
    }

    curl_easy_setopt(_p_curl, CURLOPT_URL, url);
    curl_easy_setopt(_p_curl, CURLOPT_POSTFIELDS, para_str.c_str());
    curl_easy_setopt(_p_curl, CURLOPT_WRITEDATA, buffer);
    curl_easy_setopt(_p_curl, CURLOPT_WRITEFUNCTION, HttpClient::str_write_callback);

    if (!header_str.empty()) {
        curl_slist *plist = curl_slist_append(NULL, header_str.c_str());
        curl_easy_setopt(_p_curl, CURLOPT_HTTPHEADER, plist);
    }

    /* get it! */
    CURLcode res = curl_easy_perform(_p_curl);
    if (res != CURLE_OK) {
        WARNING_LOG("curl_easy_perform Failed[%s];", curl_easy_strerror(res));
        return -1;
    }

    return 0;

}

}
