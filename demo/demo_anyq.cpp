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
#include "dict/dict_manager.h"
#include "strategy/anyq_strategy.h"
#include <sstream>
#include "common/plugin_header.h"

int main(int argc, char* argv[]){
    google::InitGoogleLogging(argv[0]);
    FLAGS_stderrthreshold = google::INFO;
    anyq::DictManager dict_manager;
    if (dict_manager.load_dict("example/conf/") != 0) {
        FATAL_LOG("load dict error");
        return -1;
    }
    anyq::AnyqStrategy anyq_strategy;
    if (anyq_strategy.create_resource(dict_manager, std::string("example/conf/")) != 0) {
        FATAL_LOG("create resource error");
        return -1;
    }

    std::fstream query_file(argv[1], std::fstream::in);
    if (!query_file.is_open()) {
        FATAL_LOG("open query file error");
        return -1;
    }
    std::string line;
    std::vector<std::string> fields;
    std::stringstream ss;

    while (getline(query_file, line)) {
        ss.str("");
        fields.clear();
        anyq::split_string(line, fields, "\t");
        std::string query;
        query = fields[0];
        anyq::ANYQResult result;
        int ret = anyq_strategy.run_strategy(fields[1], result);
        if (ret != 0) {
            WARNING_LOG("running anyq error\n%s", line.c_str());
            std::cout << "anyq_result\t" << query << std::endl;
            return -1;
        }
        ss << "anyq_result\t" << query;
        INFO_LOG("[final_anyq]\ts_query:%s", query.c_str());
        for (int i = 0; i < result.items.size(); i++) {
            ss << "\t" << result.items[i].query << "||" 
                << result.items[i].confidence << "||" << result.items[i].answer;
            INFO_LOG("[final_anyq]\tt_query:%s\tt_answer:%s\tt_score:%f", 
                    result.items[i].query.c_str(), result.items[i].answer.c_str(),
                    result.items[i].confidence);
        }
        std::cout << ss.str() << std::endl;
    }
    
    query_file.close();
    return 0;
}
