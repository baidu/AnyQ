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

#include "strategy/anyq_strategy.h"
#include "common/utils.h"

namespace anyq{

AnyqStrategy::AnyqStrategy() {}

AnyqStrategy::~AnyqStrategy() {
    release_resource();
}

int AnyqStrategy::create_resource(DictManager& dm, const std::string& conf_path) {
    DictMap* dict_map = dm.get_dict();
    if (dict_map == NULL) {
        FATAL_LOG("get_dict error");
        return -1;
    }
    AnyqStrategyConfig anyq_config;
    load_config_from_file(conf_path + std::string("/anyq.conf"), anyq_config);

    std::string analysis_conf = conf_path + anyq_config.analysis_config();
    if (_analysis.init(dict_map, analysis_conf) != 0) {
        FATAL_LOG("analysis module init error");
        return -1;
    }
    std::string retrieval_conf = conf_path + anyq_config.retrieval_config();
    if (_retrieval.init(dict_map, retrieval_conf) != 0) {
        FATAL_LOG("retrieval module init error");
        return -1;
    }
    std::string rank_conf = conf_path + anyq_config.rank_config();
    if (_rank.init(dict_map, rank_conf) != 0) {
        FATAL_LOG("rank module init error");
        return -1;
    }
    TRACE_LOG("anyq init success");
    return 0;
}

int AnyqStrategy::release_resource() {
    _analysis.destroy();
    _retrieval.destroy();
    _rank.destroy();
    return 0;
}

int AnyqStrategy::run_strategy(const std::string& analysis_input, ANYQResult& result) {
    AnalysisResult analysis_res;
    RetrievalResult candidates;
    if (_analysis.run_strategy(analysis_input, analysis_res) != 0) {
        FATAL_LOG("analysis module process error");
        return -1;
    }
    if (_retrieval.run_strategy(analysis_res, candidates) != 0) {
        FATAL_LOG("retrieval module process error");
        return -1;
    }
    if (_rank.run_strategy(analysis_res, candidates, result) != 0) {
        FATAL_LOG("rank module process error");
        return -1;
    }
    NOTICE_LOG("%s%s%s", analysis_res.notice_log.c_str(), candidates.notice_log.c_str(),
            result.notice_log.c_str());
    return 0; 
}

// 将特征以libsvm的格式输出到文件, 需要配置analysis里的切词、rank里的本地切词
int AnyqStrategy::dump_feature(const std::string& input_file, const std::string& out_file) {
    // 输入文件
    std::fstream query_file(input_file, std::fstream::in);
    if (!query_file.is_open()) {
        FATAL_LOG("open query file error");
        return -1;
    }
    // 输出文件
    std::ofstream feature_file(out_file, std::ofstream::out);
    if (!feature_file.is_open()) {
        FATAL_LOG("open feature file[%s] error", out_file.c_str());
        return -1;
    }
    // 计算特征
    std::string line;
    std::vector<std::string> fields;
    while (getline(query_file, line)) {
        fields.clear();
        split_string(line, fields, "\t");
        if (fields.size() != 3) {
            WARNING_LOG("invalid line: %s", line.c_str());
            continue;
        }
        //input1 -> analysis format
        AnalysisResult analysis_res;
        if (_analysis.run_strategy(fields[1], analysis_res) != 0) {
            return -1;
        }
        //input2 -> retrieval format
        RetrievalResult retrieval_candidates;
        RetrievalItem retrieval_item;
        retrieval_item.query.text = fields[2];
        retrieval_candidates.items.push_back(retrieval_item);
        RankResult rank_candidates;
        _rank.matching_fill_query_info(retrieval_candidates, rank_candidates);
        // 计算特征
        if (_rank.compute_similarity(analysis_res, rank_candidates, false) != 0) {
            WARNING_LOG("invalid line: %s", line.c_str());
            continue;
        }
        std::string feature = "";
        if (_rank.libsvm_feature_conv(rank_candidates, feature) != 0) {
            WARNING_LOG("invalid line: %s", line.c_str());
            continue;
        }

        std::string out_line = fields[0];
        out_line.append(" ");
        out_line.append(feature);
        out_line.append("\n");

        feature_file.write(out_line.c_str(), out_line.size());
    }
    query_file.close();
    feature_file.close();

    return 0;
}

}
