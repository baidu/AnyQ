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

#include <glog/logging.h>
#include "server/http_server.h"
#include "common/utils.h"
#include "common/plugin_header.h"

int main(int argc, char* argv[]) {
    google::InitGoogleLogging(argv[0]);
    FLAGS_stderrthreshold = google::INFO;
    anyq::HttpServer server;
    std::string anyq_brpc_conf = "./example/conf/anyq_brpc.conf";
    if (server.init(anyq_brpc_conf) != 0) {
        FATAL_LOG("server init failed");
        return -1;
    }

    if (server.always_run() != 0) {
        FATAL_LOG("server run failed");
        return -1;
    }
    return 0;
}
