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
#include "common/plugin_header.h"
#include "dict/dict_manager.h"
#include "strategy/anyq_strategy.h"

int main(int argc, char* argv[]){
    google::InitGoogleLogging(argv[0]);
    FLAGS_stderrthreshold = google::INFO;
    if (argc != 5) {
        FATAL_LOG("Usage: ./output/bin/feature_dump_tool anyq_dict_dir "
                "anyq_conf_dir query_file feature_file");
        return -1;
    }

    anyq::DictManager dm;
    if (dm.load_dict(argv[1]) != 0) {
        FATAL_LOG("load dict error");
        return -1;
    }
    anyq::AnyqStrategy anyq_strategy;
    if (anyq_strategy.create_resource(dm, argv[2]) != 0) {
        FATAL_LOG("create resource error");
        return -1;
    }
    if (anyq_strategy.dump_feature(argv[3], argv[4]) != 0) {
        FATAL_LOG("feature dump failed!");
        return -1;
    }

    return 0;
}
