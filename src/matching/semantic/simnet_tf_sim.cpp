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

#ifdef USE_TENSORFLOW
#include "matching/semantic/simnet_tf_sim.h"

namespace anyq {

TFSimilarity::TFSimilarity(){
    _p_session = NULL;
    _p_tf_pack = NULL;

}

//tensorflow图模型线程资源初始化
int TFSimilarity::init(DualDictWrapper* dict, const MatchingConfig& matching_config) {
    _p_tf_pack = (TFPack*)dict->get_dict();
    _pad_id = matching_config.tfconf().pad_id();
    _sen_len = matching_config.tfconf().sen_len();
    _left_name = matching_config.tfconf().left_input_name();
    _right_name = matching_config.tfconf().right_input_name();
    _output_tensor_name = matching_config.tfconf().output_name();
    tensorflow::Status status = tensorflow::NewSession(tensorflow::SessionOptions(), &_p_session);
    if (!status.ok()) {
        std::string error_string = status.ToString();
        FATAL_LOG("create session error: %s", error_string.c_str());
        return -1;
    }
    tensorflow::Status status_create = _p_session->Create(_p_tf_pack->graphdef);
    if (!status_create.ok()) {
        std::string error_string = status_create.ToString();
        FATAL_LOG("create graph in session error", error_string.c_str());
        return -1;
    }
    return 0;
}

int TFSimilarity::destroy() {
    return 0;
}

int TFSimilarity::compute_similarity(const AnalysisResult& analysis_res, RankResult& candidates) {
    hashmap_str2int& term2id_kv = _p_tf_pack->term2id;
    std::vector<int> query_ids;
    std::vector<int> cand_ids;
    query_ids.reserve(100);
    cand_ids.reserve(100);
    query_ids.clear();
    cand_ids.clear();

    int use_query_index = 0;
    //把输入query的token序列转换成id序列
    trans_term_to_id(term2id_kv, analysis_res.analysis[use_query_index].tokens_basic, query_ids);
    DEBUG_LOG("source query:%s", analysis_res.analysis[use_query_index].query.c_str());

    std::vector<std::pair<std::string, tensorflow::Tensor> > input;
    for (size_t i = 0; i < candidates.size(); i++) {
        // 无效候选，跳过
        if (candidates[i].abandoned) {
            continue;
        }
        // 转换成tensorflow要求的输入
        input.clear();
        trans_to_tf_input(input, _left_name, query_ids);
        cand_ids.clear();
        trans_term_to_id(term2id_kv, candidates[i].match_info.tokens_basic, cand_ids);
        trans_to_tf_input(input, _right_name, cand_ids);
        std::vector<tensorflow::Tensor> outputs;
        // 调用tensorflow图模型计算特征
        tensorflow::Status status = _p_session->Run(input, {_output_tensor_name}, {}, &outputs);
        if (!status.ok()) {
            std::string error_string = status.ToString();
            WARNING_LOG("[ERROR]predictor returned error: %s", error_string.c_str());
            candidates[i].features.push_back(0.0f);
            continue;
        }
        // 获取tensorflow模型的输出结果作为特征值
        tensorflow::Tensor t = outputs[0];
        auto tmap = t.tensor<float, 2>();
        float tf_score = tmap(0, 1);
        candidates[i].features.push_back(tf_score);
        DEBUG_LOG("candidate query:%s, score:%f", candidates[i].match_info.text.c_str(), tf_score);
    }
    return 0;
}
int TFSimilarity::trans_to_tf_input(std::vector<std::pair<std::string, tensorflow::Tensor> > &input, std::string input_name, std::vector<int>& vec) {
    std::vector<int> paded_vec;
    int a_len = vec.size();
    for (int i = 0; i < a_len && i < _sen_len; i++) {
        paded_vec.push_back(vec[i]);
    }
    for (int i = 0; i < _sen_len - a_len; i++) {
        paded_vec.push_back(_pad_id);
    }
    tensorflow::Tensor x(tensorflow::DT_INT32, tensorflow::TensorShape({1, _sen_len}));
    auto x_map = x.tensor<int, 2>();
    for (int j = 0; j < _sen_len; j++) {
        x_map(0, j) = paded_vec[j];
    };
    input.push_back(std::pair<std::string, tensorflow::Tensor>(input_name, x));
    return 0;
}

TFSimilarity::~TFSimilarity(){
    destroy();
}

} // namespace anyq

#endif
