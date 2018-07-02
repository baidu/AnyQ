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

#include "dict/dict_manager.h"
#include "analysis/analysis_strategy.h"
#include <sstream>
#include <glog/logging.h>
#include "retrieval/semantic/kissrandom.h"
#include "retrieval/semantic/annoylib.h"
#include "common/plugin_header.h"

int main(int argc, char* argv[]){
    google::InitGoogleLogging(argv[0]);
    FLAGS_stderrthreshold = google::INFO;
    if (argc != 7) {
        FATAL_LOG("Usage: annoy_index_build_tool anyq_dict_dir analysis_conf_path query_file vector_dim "
                "num_trees index_sava_file");
        FATAL_LOG("Example: ./output/bin/annoy_index_build_tool example/conf/ example/conf/analysis.conf "
                "example/conf/annoy_query_to_build_tree.dat 128 200 example/conf/annoy_index.tree");
        return -1;
    }
    anyq::DictManager dm;
    if (dm.load_dict(argv[1]) != 0) {
        FATAL_LOG("load dict error");
        return -1;
    }
    anyq::DictMap* global_dict = dm.get_dict();
    anyq::AnalysisStrategy analysis_strategy;
    analysis_strategy.init(global_dict, argv[2]);

    std::fstream fs(argv[3], std::fstream::in);
    if (!fs.is_open()) {
        FATAL_LOG("open query file error");
        return -1;
    }

    AnnoyIndex<int, float, Angular, Kiss32Random> annoy_index =
            AnnoyIndex<int, float, Angular, Kiss32Random>(atoi(argv[4]));
    std::string line;
    std::vector<std::string> fields;
    while (getline(fs, line)) {
        anyq::AnalysisResult analysis_result;
        fields.clear();
        anyq::split_string(line, fields, "\t");
        analysis_strategy.run_strategy(fields[1], analysis_result);
        annoy_index.add_item(atoi(fields[0].c_str()), &analysis_result.analysis[0].query_emb[0]);
    }
    annoy_index.build(atoi(argv[5]));
    annoy_index.save(argv[6]);

    return 0;
}
