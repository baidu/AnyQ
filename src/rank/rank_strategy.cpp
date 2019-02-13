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

#include "rank/rank_strategy.h"
#include <sstream>
#include "common/plugin_factory.h"

namespace anyq {

const float MANUAL_CONFIDENCE = 1.0f;

RankStrategy::RankStrategy() {
}

RankStrategy::~RankStrategy() {
    if (destroy() != 0) {
        FATAL_LOG("release rank stategy error");
    }
}

// 获得predictor的type和字典，初始化线程资源
int RankStrategy::init_predictor(const RankPredict& predict_config,
        std::vector<std::string>& feature_names,
        RankPredictInterface*& predictor) {
    std::string predict_type = predict_config.type();
    std::string predict_dict_name("");
    // 是否需要DictManager中的字典
    bool need_dict = false;
    if (predict_config.has_using_dict_name()) {
        predict_dict_name = predict_config.using_dict_name();
        need_dict = true;
    }
    // 获取predictor插件实例
    predictor = static_cast<RankPredictInterface*>(PLUGIN_FACTORY.create_plugin(predict_type));
    if (predictor == NULL) {
        FATAL_LOG("can't find predict_type:%s", predict_type.c_str());
        return -1;
    }
    if (need_dict && _dict_map->count(predict_dict_name) < 1) {
        FATAL_LOG("using dict %s that does not exist", predict_dict_name.c_str());
        return -1;
    }
    DualDictWrapper* dict_ptr = NULL;
    if (need_dict) {
        dict_ptr = (*_dict_map)[predict_dict_name];
    }
    int ret = predictor->init(dict_ptr, feature_names, predict_config);
    if (ret != 0) {
        FATAL_LOG("init predictor error");
        return -1;
    }
    TRACE_LOG("create predictor %s success", predict_type.c_str());

    return 0;
}

// 初始化matching feature
int RankStrategy::init_feature(const RankConfig& rank_config) {
    for (int i = 0; i < rank_config.matching_config_size(); i++) {
        const MatchingConfig& matching_config = rank_config.matching_config(i);
        std::string fea_type = matching_config.type();
        std::string fea_name = matching_config.name();
        int output_num = matching_config.output_num();
        bool rough = matching_config.rough();
        // 获取特征插件实例
        MatchingInterface* tmp_fea = 
                static_cast<MatchingInterface*>(PLUGIN_FACTORY.create_plugin(fea_type));
        if (tmp_fea == NULL) {
            FATAL_LOG("can't find feature_type:%s", fea_type.c_str());
            return -1;
        }
        // 获取全局字典
        DualDictWrapper* dict_ptr = NULL;
        if (matching_config.has_using_dict_name()) {
            std::string matching_dict_name = matching_config.using_dict_name();
            if (_dict_map->count(matching_dict_name) < 1) {
                FATAL_LOG("using dict %s that does not exist", matching_dict_name.c_str());
                return -1;
            }
            dict_ptr = (*_dict_map)[matching_dict_name];
        }
        // 基类成员初始化
        tmp_fea->init_base(fea_name, output_num, rough);
        // 子类成员初始化
        if (tmp_fea->init(dict_ptr, matching_config) != 0) {
            FATAL_LOG("init feature error %s", fea_name.c_str());
            return -1;
        }
        TRACE_LOG("create feature %s success", fea_name.c_str());
        _feature_calculator.push_back(tmp_fea);
        // 如果该特征插件的特征值个数大于1,则特征名用后缀区分开
        if (output_num > 1) {
            for (int i = 0; i < output_num; i++){
                std::stringstream ss;
                ss << fea_name << "-" << i;
                if (matching_config.rough()) {
                    _rough_feature_names.push_back(ss.str());
                } else {
                    _feature_names.push_back(ss.str());
                }
            }
        } else if (output_num > 0) {
            // 区分特征用于粗排还是精排
            if (rough) {
                _rough_feature_names.push_back(fea_name);
            } else {
                _feature_names.push_back(fea_name);
            }
        }
    }
    // 粗排特征放在精排特征之前
    _feature_names.insert(_feature_names.begin(),
            _rough_feature_names.begin(),
            _rough_feature_names.end());

    return 0;
}

int RankStrategy::init(DictMap* dict_map, const std::string& rank_conf) {
    _dict_map = dict_map;
    RankConfig rank_config;
    if (load_config_from_file(rank_conf, rank_config) != 0) {
        return -1;
    }
    _rough_top_k = 0;
    if (rank_config.has_rough_top_result()) {
        _rough_top_k = rank_config.rough_top_result();
    }
    _top_k = rank_config.top_result();
    _rough_threshold = 0.0f;
    if (rank_config.has_rough_threshold()) {
        _rough_threshold = rank_config.rough_threshold();
    }
    _threshold = 0.0f;
    if (rank_config.has_threshold()) {
        _threshold = rank_config.threshold();
    }
    _deep_rank_with_anwser = false;
    if (rank_config.has_deep_rank_with_answer()) {
        _deep_rank_with_anwser = rank_config.deep_rank_with_answer();
    }
    _max_ans_num_per_query = 1000;
    if (rank_config.has_max_ans_num_per_query()) {
        _max_ans_num_per_query = rank_config.max_ans_num_per_query();
    }

    // 初始化 matching feature plugins
    if (init_feature(rank_config) != 0) {
        FATAL_LOG("init matching feature error");
        return -1;
    }
    // 初始化 rough rank redictor
    _rough_predictor = NULL;
    if (rank_config.has_rough_rank_predictor()) {
        if (init_predictor(rank_config.rough_rank_predictor(),
                    _rough_feature_names,
                    _rough_predictor) != 0) {
            return -1;
        }
    }
    // 初始化 rank redictor
    if (!rank_config.has_rank_predictor()) {
        FATAL_LOG("predictor conf not exist");
        return -1;
    }
    if (init_predictor(rank_config.rank_predictor(), _feature_names, _predictor) != 0) {
        FATAL_LOG("init predictor error");
        return -1;
    }

    return 0;
}

int RankStrategy::destroy() {
    std::vector<MatchingInterface*>::iterator it;
    int i = 0;
    for (it = _feature_calculator.begin(); it != _feature_calculator.end(); ++it, ++i) {
        if ((*it)->destroy() != 0) {
            FATAL_LOG("feature %s destroy error", ((*it)->feature_name()).c_str());
            return -1;
        }
        delete *it;
    }
    _feature_calculator.clear();

    if (_predictor != NULL) {
        _predictor->destroy();
        delete _predictor;
        _predictor = NULL;
    }
    if (_rough_predictor != NULL) {
        _rough_predictor->destroy();
        delete _rough_predictor;
        _rough_predictor = NULL;
    }
    return 0;
}

// 遍历所有类型特征，选取粗排特征或者精排特征进行计算
int RankStrategy::compute_similarity(const AnalysisResult& analysis_res,
        RankResult& candidates,
        bool rough_sort) {
    for (size_t i = 0; i < _feature_calculator.size(); i++) {
        if (rough_sort != _feature_calculator[i]->is_rough()) {
            continue;
        }
        if (_feature_calculator[i]->compute_similarity(analysis_res, candidates) != 0) {
            FATAL_LOG("feature %s compute error", (_feature_calculator[i]->feature_name()).c_str());
            return -1;
        }
    }
    return 0;
}

// 将特征转换为LibSVM的格式
int RankStrategy::libsvm_feature_conv(const RankResult& candidates, std::string& feature_out) {
    feature_out = "";
    if (candidates.size() == 0) {
        FATAL_LOG("candidates  is empty");
        return -1;
    }
    for (size_t i = 0; i < candidates[0].features.size(); i++) {
        if (i != 0) {
            feature_out.append(" ");
        }
        feature_out.append(std::to_string(i + 1));
        feature_out.append(":");
        feature_out.append(std::to_string(candidates[0].features[i]));
    }
    return 0;
}

// debug打印最终结果信息
int RankStrategy::debug_result(const AnalysisResult& analysis_res, ANYQResult& result) {
    // get debug query
    std::string query;
    for (size_t i = 0; i < analysis_res.analysis.size(); i++) {
        if (analysis_res.analysis[i].type != 0) {
            continue;
        }
        query = analysis_res.analysis[i].query;
        break;
    }
    DEBUG_LOG("anyq query: %s", query.c_str());
    DEBUG_LOG("output");
    for (size_t j = 0; j < result.items.size(); j++) {
        DEBUG_LOG("\tquery:%s", result.items[j].query.c_str());
        DEBUG_LOG("\tconfidence:%f", result.items[j].confidence);
    }
    return 0;
}

// 过滤低于阈值的元素
int RankStrategy::threshold_filter(RankResult& candidates, float threshold) {
    for (int i = candidates.size() - 1; i >= 0; i--) {
        if (candidates[i].ltr_score < threshold || candidates[i].abandoned) {
            candidates.erase(candidates.begin() + i);
        }
    }
    return 0;
}

// 计算特征、score，过滤阈值
int RankStrategy::compute_and_filter(const AnalysisResult& analysis_res,
        RankResult& candidates,
        bool rough_sort) {
    if (rough_sort) {
        if (_rough_predictor == NULL) {
            FATAL_LOG("_rough_predictor is NULL");
            return -1;
        }
    } else {
        if (_predictor == NULL) {
            FATAL_LOG("_predictor is NULL");
            return -1;
        }
    }
    // 计算特征
    if (compute_similarity(analysis_res, candidates, rough_sort) != 0) {
        return -1;
    }
    // 计算score
    int ret = 0;
    if (rough_sort) {
        ret = _rough_predictor->predict(candidates);
    } else {
        ret = _predictor->predict(candidates);
    }
    if (ret != 0) {
        return -1;
    }
    // 删除低于阈值的元素
    if (rough_sort) {
        ret = threshold_filter(candidates, _rough_threshold);
    } else {
        ret = threshold_filter(candidates, _threshold);
    }
    if (ret != 0) {
        return -1;
    }
    return 0;
}
// 把未被丢弃且得分高的候选排在前面
bool candidate_cmp(const RankItem& item1, const RankItem& item2) {
    if (item1.abandoned < item2.abandoned) {
        return false;
    }
    if (item1.abandoned > item2.abandoned) {
        return true;
    }
    if (item1.ltr_score > item2.ltr_score) {
        return true;
    }
    return false;
}
// // 设置候选的query作为匹配信息
int RankStrategy::matching_fill_query_info(const RetrievalResult& retrieval_candidates,
        RankResult& rank_candidates) {
    size_t cand_cnt = retrieval_candidates.items.size();
    rank_candidates.resize(cand_cnt);
    for (size_t i = 0; i < cand_cnt; i++) {
        // fill info used for matching
        rank_candidates[i].retrieval_item = retrieval_candidates.items[i];
        rank_candidates[i].match_info = retrieval_candidates.items[i].query;
        rank_candidates[i].features.clear();
        rank_candidates[i].abandoned = false;
    }
    return 0;
}

// 将多答案候选拆分为多个候选
int RankStrategy::trans_answer_info(const RankResult& src_candidates, RankResult& dst_candidates) {
    dst_candidates.clear();
    size_t cand_cnt = src_candidates.size();
    std::vector<std::vector<float> > result_features;
    std::vector<float> result_scores;
    for (size_t i = 0; i < cand_cnt; i++) {
        size_t ans_cnt = src_candidates[i].retrieval_item.answer.size();
        for (size_t j = 0; j < ans_cnt && j < _max_ans_num_per_query; j++) {
            if (src_candidates[i].retrieval_item.answer[j].text == "") {
                continue;
            }
            // query and answer info
            RetrievalItem rec_i;
            rec_i.query = src_candidates[i].retrieval_item.query;
            rec_i.answer.push_back(src_candidates[i].retrieval_item.answer[j]);

            RankItem rank_item;
            rank_item.retrieval_item = rec_i;
            rank_item.match_info = src_candidates[i].retrieval_item.answer[j];
            rank_item.features = src_candidates[i].features;
            rank_item.abandoned = src_candidates[i].abandoned;

            dst_candidates.push_back(rank_item);
        }
    }
    return 0;
}

// answer排序时，删除重复answer
int RankStrategy::rm_duplicate_answer(RankResult& candidates) {
    if (!_deep_rank_with_anwser) {
        return 0;
    }
    for (int i = candidates.size() - 1; i >= 0; i--) {
        for (int j = 0; j < i; j++) {
            if (candidates[i].match_info.text == candidates[j].match_info.text) {
                candidates.erase(candidates.begin() + i);
                break;
            }
        }
    }

    return 0;
}
// 将RankResult结构转化为系统的输出结构ANYQResult
int RankStrategy::collect_result(const RankResult& candidates, ANYQResult& result) {
    for (size_t i = 0; i < _top_k && i < candidates.size(); i++) {
        ANYQItem item;
        const RetrievalItem& retrieval_item = candidates[i].retrieval_item;
        item.qa_id = retrieval_item.query.id;
        item.query = retrieval_item.query.text;
        if (retrieval_item.answer.size() > 0) {
            item.answer = retrieval_item.answer[0].text;
            item.answer_info = retrieval_item.answer[0].json_info;
        }
        item.json_info = retrieval_item.query.json_info;
        item.confidence = candidates[i].ltr_score;
        item.matching_features = candidates[i].features;
        item.matching_feature_names = _feature_names;
        result.items.push_back(item);

        result.notice_log.append("anyq_answer=");
        result.notice_log.append(item.answer);
        result.notice_log.append(";");
    }
    return 0;
}
// 将RetrievalResult结构转化为系统的输出结构ANYQResult,用于人工干预的召回
int RankStrategy::collect_result(const RetrievalResult& candidates, ANYQResult& result) {
    for (size_t i = 0; i < _top_k && i < candidates.items.size(); i++) {
        ANYQItem item;
        const RetrievalItem& retrieval_item = candidates.items[i];
        item.qa_id = retrieval_item.query.id;
        item.query = retrieval_item.query.text;
        if (retrieval_item.answer.size() > 0) {
            item.answer = retrieval_item.answer[0].text;
            item.answer_info = retrieval_item.answer[0].json_info;
        }
        item.json_info = retrieval_item.query.json_info;
        item.confidence = MANUAL_CONFIDENCE;
        result.items.push_back(item);

        result.notice_log.append("anyq_answer=");
        result.notice_log.append(item.answer);
        result.notice_log.append(";");
    }
    return 0;
}
// rank策略运行入口
int RankStrategy::run_strategy(const AnalysisResult& analysis_res,
        const RetrievalResult& retrieval_candidates,
        ANYQResult& result) {
    float time_use = 0; // 毫秒
    struct timeval start;
    struct timeval end;
    gettimeofday(&start, NULL);
    if (retrieval_candidates.items.size() == 0) {
        return 0;
    }
    // 如果有人工干预的召回结果，直接返回不走rank策略
    if (retrieval_candidates.anyq_end) {
        collect_result(retrieval_candidates, result);
        return 0;
    }

    RankResult tmp_candidates;
    matching_fill_query_info(retrieval_candidates, tmp_candidates);
    // 粗排特征
    if (_rough_predictor != NULL && compute_and_filter(analysis_res, tmp_candidates, true) != 0) {
        return -1;
    }
    // 粗排top_k筛选
    std::sort(tmp_candidates.begin(), tmp_candidates.end(), candidate_cmp);
    if (_rough_top_k != 0 && tmp_candidates.size() > _rough_top_k) {
        tmp_candidates.erase(tmp_candidates.begin() + _rough_top_k, tmp_candidates.end());
    }

    DEBUG_LOG("after rough rank, remain %d cands", tmp_candidates.size());

    if (tmp_candidates.size() == 0) {
        return 0;
    }

    RankResult trans_candidates = tmp_candidates;
    if (_deep_rank_with_anwser) {
        trans_answer_info(tmp_candidates, trans_candidates);
    }
    // 精排特征
    if (compute_and_filter(analysis_res, trans_candidates, false) != 0) {
        return -1;
    }
    DEBUG_LOG("after rank, remain %d cands", trans_candidates.size());

    // 排序
    std::sort(trans_candidates.begin(), trans_candidates.end(), candidate_cmp);
    // answer排序时，删除重复answer
    rm_duplicate_answer(trans_candidates);

    // 结果填充
    collect_result(trans_candidates, result);
    // debug打印最终结果信息
    debug_result(analysis_res, result);

    gettimeofday(&end, NULL);
    time_use = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000;
    char sub_log[SUB_LOG_LEN];
    snprintf(sub_log, SUB_LOG_LEN, "rank_item_num=%d;rank_time=%.2fms;",
            trans_candidates.size(), time_use);
    result.notice_log.append(sub_log);

    return 0;
}

} // namespace anyq
