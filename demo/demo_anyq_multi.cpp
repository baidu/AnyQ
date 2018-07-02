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
#include <pthread.h>
#include <sstream>
#include "common/plugin_header.h"

anyq::DictManager g_dict_manager;
unsigned int g_thread_number = 1;
std::string g_file_input;
std::string g_file_output;

void* thread_run(void* arg) {
    unsigned int thread_index = *((unsigned int *)arg);
    anyq::AnyqStrategy anyq_strategy;
    if (anyq_strategy.create_resource(g_dict_manager, std::string("example/conf/")) != 0) {
        FATAL_LOG("create resource error");
        return NULL;
    }

    std::fstream query_file(g_file_input.c_str(), std::fstream::in);
    std::stringstream ss_file;
    ss_file << g_file_output << "-thread" << thread_index;
    std::ofstream fout(ss_file.str().c_str());
    if (!query_file.is_open()) {
        FATAL_LOG("open query file error");
        return NULL;
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
            fout << "anyq_result\t" << query << std::endl;
            continue;
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
        fout << ss.str() << std::endl;
    }
    query_file.close();
    fout.close();
} 

int main(int argc, char* argv[]){
    google::InitGoogleLogging(argv[0]);
    FLAGS_stderrthreshold = google::INFO;
    FLAGS_logbufsecs = 0;
    if (argc < 4) {
        FATAL_LOG("usage ./%s input output_prefix thread_num", argv[0]);
        return 1;
    }
    if (g_dict_manager.load_dict("example/conf/") != 0) {
        FATAL_LOG("load dict error");
        return 1;
    }
    g_file_input = argv[1];
    g_file_output = argv[2];
    g_thread_number = atoi(argv[3]);
    unsigned int thread_indexes[g_thread_number];
    for (unsigned int i = 0; i < g_thread_number; i++) {
        thread_indexes[i] = i;
    }

    pthread_t tids[g_thread_number];

    // create each thread and start thread
    for (unsigned int i = 0; i < g_thread_number; i++){
        int tmpret = pthread_create(
                            &tids[i], NULL, 
                            thread_run,
                            (void*)(&thread_indexes[i]));
        if (tmpret != 0) {
            fprintf(stderr, "pthread_create error, error_code = %d\n", tmpret);
            return 1;
        }
        fprintf(stderr, "created %d thread\n", (int)i);
    }
    for (unsigned int i = 0; i < g_thread_number; ++i){
        pthread_join(tids[i], NULL);
    }
    g_dict_manager.release_dict();
    return 0;
}
