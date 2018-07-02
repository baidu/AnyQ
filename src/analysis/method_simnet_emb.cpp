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
#include "analysis/method_simnet_emb.h"
#include <exception>

namespace anyq {

AnalysisSimNetEmb::AnalysisSimNetEmb() {
    _p_paddle_pack = NULL;
    _paddle_resource = NULL;
    _query_feed_index = INITIAL_INDEX;
    _cand_feed_index = INITIAL_INDEX;
    _embedding_fetch_index = INITIAL_INDEX;
}

AnalysisSimNetEmb::~AnalysisSimNetEmb() {
    destroy();
}

int AnalysisSimNetEmb::init(DualDictWrapper* dict, const AnalysisMethodConfig& analysis_method) {
    _p_paddle_pack = (PaddlePack*)dict->get_dict();
    set_method_name(analysis_method.name());

    _paddle_resource = new PaddleThreadResource();
    if (_paddle_resource == NULL || _paddle_resource->init(_p_paddle_pack) != 0){
        FATAL_LOG("paddle threadr resource init error");
        delete _paddle_resource;
        return -1;
    }

    const std::vector<std::string>& feed_target_names = _paddle_resource->get_feed_target_names();
    const std::vector<std::string>& fetch_target_names = _paddle_resource->get_fetch_target_names();
    const std::string& query_feed_name = analysis_method.query_feed_name();
    const std::string& cand_feed_name = analysis_method.cand_feed_name();
    const std::string& embedding_fetch_name = analysis_method.embedding_fetch_name();
    _dim = analysis_method.dim();

    for (size_t i = 0; i < feed_target_names.size(); ++i) {
        if (feed_target_names[i] == query_feed_name){
            _query_feed_index = i;
        }else if (feed_target_names[i] == cand_feed_name){
            _cand_feed_index = i;
        }
    }
    for (size_t i = 0; i < fetch_target_names.size(); ++i) {
        if (fetch_target_names[i].find(embedding_fetch_name) == 0){
            _embedding_fetch_index = i;
        }
        DEBUG_LOG("fetch name: %d %s", i, fetch_target_names[i].c_str());
    }

    if (INITIAL_INDEX == _query_feed_index || INITIAL_INDEX == _embedding_fetch_index){
        FATAL_LOG("feed or fetch conf error !");
        return -1;
    }

    DEBUG_LOG("query/cand/emb index: %d %d %d", _query_feed_index, _cand_feed_index, _embedding_fetch_index);

    return 0;
}

int AnalysisSimNetEmb::destroy() {
    if (_paddle_resource != NULL){
        _paddle_resource->destroy();
        delete _paddle_resource;
        _paddle_resource = NULL;
    }
    return 0;
}

int AnalysisSimNetEmb::single_process(AnalysisItem& analysis_item) {
    // 切词term转id
    std::vector<int> query_ids;
    trans_term_to_id(_p_paddle_pack->term2id, analysis_item.tokens_basic, query_ids);

    if (analysis_item.query_emb.size() != _dim){
        analysis_item.query_emb.resize(_dim);
    }

    if (query_ids.size() == 0){
        for (size_t i = 0; i < _dim; ++i){
            analysis_item.query_emb[i] = 0.0; 
        }
        return 0;
    }
    
    _paddle_resource->set_feed(_query_feed_index, query_ids);
    if (INITIAL_INDEX != _cand_feed_index){
        std::vector<int> cand_ids(1, 0);
        _paddle_resource->set_feed(_cand_feed_index, cand_ids);
    }
    if (_paddle_resource->run() != 0){
        FATAL_LOG("AnalysisSimNetEmb run error!");
        return -1;
    }
 
    const float* output_ptr = _paddle_resource->get_fetch(_embedding_fetch_index);
    if (output_ptr == NULL){
        FATAL_LOG("AnalysisSimNetEmb get fetch failed");
        return -1;
    }
    // analysis_item query_emb赋值
    for (size_t i = 0; i < _dim; ++i) {
        analysis_item.query_emb[i] = output_ptr[i];
    }
    return 0;
}

} // namespace anyq

#endif
