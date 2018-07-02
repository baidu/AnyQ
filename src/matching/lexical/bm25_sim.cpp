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

#include "matching/lexical/bm25_sim.h"
#include <cmath>

namespace anyq {

const float BM25Similarity::_s_bm25_k1 = 2.0;
const float BM25Similarity::_s_bm25_k2 = 1.0;
const float BM25Similarity::_s_bm25_b = 0.75;

BM25Similarity::BM25Similarity(){
}

BM25Similarity::~BM25Similarity(){
    destroy();
}

int BM25Similarity::init(DualDictWrapper* dict, const MatchingConfig& matching_config) {
    _p_dual_dict_wrapper = dict;
    return 0;
}

int BM25Similarity::destroy() {
    return 0;
}

int BM25Similarity::compute_similarity(const AnalysisResult& analysis_res,
        RankResult& candidates) {
    if (analysis_res.analysis.size() < 1) {
        return -1;
    }
    if (candidates.size() == 0) {
        return 0;
    }

    hashmap_str2float* term_weights = (hashmap_str2float*)(_p_dual_dict_wrapper->get_dict());
    if (!term_weights) {
        FATAL_LOG("BM25 term weight dict is null");
        return -1;
    }

    const std::vector<analysis_token_t>& query_terms =  analysis_res.analysis[0].tokens_basic;
    size_t query_len = query_terms.size();
    hashmap_str2float query_tfidf;
    for (size_t i = 0; i < query_len; ++i) {
        if (0 == query_tfidf.count(query_terms[i].buffer)) {
            query_tfidf[query_terms[i].buffer] = 1.0; 
        }else {
            query_tfidf[query_terms[i].buffer] += 1.0;
        }
    }
    for (auto iter = query_tfidf.begin(); iter != query_tfidf.end(); ++iter) {
        float weight = 0.0;
        if (term_weights->count(iter->first) != 0) {
            weight = (*term_weights)[iter->first];
        }
        float tf = iter->second;
        iter->second = weight * tf * (_s_bm25_k2 + 1.0) / (tf + _s_bm25_k2);
    }

    size_t cand_size = candidates.size();
    float avg_cand_len = 0.0;
    for (size_t i = 0; i < cand_size; ++i) {
        avg_cand_len += candidates[i].match_info.tokens_basic.size();
    }
    avg_cand_len /= cand_size;

    for (size_t i = 0; i < cand_size; ++i) {
        if (candidates[i].abandoned) {
            continue;
        }
        float bm25 = 0.0;
        const std::vector<analysis_token_t>& cand_terms = candidates[i].match_info.tokens_basic;
        size_t cand_len = cand_terms.size();
        hashmap_str2float cand_count;
        for (size_t j = 0; j < cand_len; ++j) {
            if (0 == query_tfidf.count(cand_terms[j].buffer)) {
                continue;
            }
            if (0 == cand_count.count(cand_terms[j].buffer)) {
                cand_count[cand_terms[j].buffer] = 1.0;
            }else {
                cand_count[cand_terms[j].buffer] += 1.0;
            }
        } 
        for (auto iter = cand_count.begin(); iter != cand_count.end(); ++iter) {
            float tf = iter->second;
            float k = _s_bm25_k1 * (1 + (cand_len / avg_cand_len - 1.0) * _s_bm25_b);
            bm25 += query_tfidf[iter->first] * tf * (_s_bm25_k1 + 1.0) / (tf + k);
        }
        // 归一化
        float sim = 1.0 / (1 + exp(-bm25));
        DEBUG_LOG("bm25_sim %f", sim);
        candidates[i].features.push_back(sim);
    }
    return 0;
}

} // namespace anyq
