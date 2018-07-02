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

#ifndef USE_TENSORFLOW
#include "matching/semantic/simnet_paddle_sim.h"
#include <exception>

namespace anyq {

PaddleSimilarity::PaddleSimilarity(){
    _p_paddle_pack = NULL;
    _paddle_resource = NULL;
    _query_feed_index = INITIAL_INDEX;
    _cand_feed_index = INITIAL_INDEX;
    _score_fetch_index = INITIAL_INDEX;
}

PaddleSimilarity::~PaddleSimilarity(){
    destroy();
}

int PaddleSimilarity::init(DualDictWrapper* dict, const MatchingConfig& matching_config) {
    _p_paddle_pack = (PaddlePack*)dict->get_dict();
    DEBUG_LOG("_output_num:%d", get_output_num());

    // 初始化paddle的线程资源
    _paddle_resource = new PaddleThreadResource();
    if (_paddle_resource == NULL || _paddle_resource->init(_p_paddle_pack) != 0){
        FATAL_LOG("paddle threadr resource init error");
        delete _paddle_resource;
        return -1;
    }
    // 获取query、候选和得分在feed/fetch中的下标
    const std::vector<std::string>& feed_target_names = _paddle_resource->get_feed_target_names();
    const std::vector<std::string>& fetch_target_names = _paddle_resource->get_fetch_target_names();
    const std::string& query_feed_name = matching_config.query_feed_name();
    const std::string& cand_feed_name = matching_config.cand_feed_name();
    const std::string& score_fetch_name = matching_config.score_fetch_name();

    for (size_t i = 0; i < feed_target_names.size(); ++i) {
        if (feed_target_names[i] == query_feed_name){
            _query_feed_index = i;
        }else if (feed_target_names[i] == cand_feed_name){
            _cand_feed_index = i;
        }
    }
    for (size_t i = 0; i < fetch_target_names.size(); ++i) {
        if (fetch_target_names[i].find(score_fetch_name) == 0){
            _score_fetch_index = i;
        }
    }

    if (INITIAL_INDEX == _query_feed_index || INITIAL_INDEX == _cand_feed_index 
            || INITIAL_INDEX == _score_fetch_index){
        FATAL_LOG("feed or fetch conf error !");
        return -1;
    }

    return 0;
}

int PaddleSimilarity::destroy() {
    if (_paddle_resource != NULL){
        _paddle_resource->destroy();
        delete _paddle_resource;
        _paddle_resource = NULL;
    }
    return 0;
}

int PaddleSimilarity::compute_similarity(const AnalysisResult& analysis_res, RankResult& candidates) {
    hashmap_str2int& term2id_kv = _p_paddle_pack->term2id;
    std::vector<int> query_ids;
    std::vector<int> cand_ids;
    int use_query_index = 0;
    const std::vector<analysis_token_t>& q_token =
            analysis_res.analysis[use_query_index].tokens_basic;
    // term序列转id序列
    trans_term_to_id(term2id_kv, q_token, query_ids);
    DEBUG_LOG("source query:%s", analysis_res.analysis[use_query_index].query.c_str());
    if (0 == query_ids.size()){
        for (size_t i = 0; i < candidates.size(); ++i) {
            candidates[i].features.push_back(0.0);
        }
        return 0;
    }
    // 设置query的feed数据
    _paddle_resource->set_feed(_query_feed_index, query_ids);

    for (size_t i = 0; i < candidates.size(); ++i) {
        // 无效候选，跳过
        if (candidates[i].abandoned) {
            continue;
        }
        const std::vector<analysis_token_t>& c_token = candidates[i].match_info.tokens_basic;
        trans_term_to_id(term2id_kv, c_token, cand_ids);
        if (0 == cand_ids.size()){
            candidates[i].features.push_back(0.0);
            continue;
        }
        // 设置候选的feed数据
        _paddle_resource->set_feed(_cand_feed_index, cand_ids);
        if (_paddle_resource->run() != 0){
            FATAL_LOG("PaddleSimilarity run error!");
            return -1;
        }
        // 从fetch中获取得分
        const float* output_ptr = _paddle_resource->get_fetch(_score_fetch_index);
        if (output_ptr == NULL){
            FATAL_LOG("PaddleSimilarity get fetch failed");
            return -1;
        }
        float dnn_score = output_ptr[0];
        candidates[i].features.push_back(dnn_score);
        DEBUG_LOG("candidate query:%s, socre:%f", candidates[i].match_info.text.c_str(), dnn_score);
    }

    return 0;
}

} // namespace anyq
#endif
