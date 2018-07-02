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

#ifndef BAIDU_NLP_ANYQ_PLUGIN_HEADER_H
#define BAIDU_NLP_ANYQ_PLUGIN_HEADER_H

#include "common/plugin_factory.h"
// DICT
#include "dict/dict_adapter.h"
// Analysis
#include "analysis/method_wordseg.h"
#include "analysis/method_query_intervene.h"
#include "analysis/method_simnet_emb.h"
// SOLR_Q
#include "retrieval/term/equal_solr_q_builder.h"
#include "retrieval/term/boost_solr_q_builder.h"
#include "retrieval/term/contain_solr_q_builder.h"
#include "retrieval/term/synonym_solr_q_builder.h"
#include "retrieval/term/date_compare_solr_q_builder.h"
// RETRIEVAL
#include "retrieval/manual/manual_retrieval.h"
#include "retrieval/term/term_retrieval.h"
#include "retrieval/semantic/semantic_retrieval.h"
// MATCHING
#include "matching/lexical/wordseg_proc.h"
#include "matching/lexical/contain_sim.h"
#include "matching/lexical/cosine_sim.h"
#include "matching/lexical/jaccard_sim.h"
#include "matching/lexical/edit_distance_sim.h"
#include "matching/lexical/bm25_sim.h"
#include "matching/semantic/simnet_paddle_sim.h"
#include "matching/semantic/simnet_tf_sim.h"
// MATCHING PREDICTOR
#include "rank/predictor/predictor_interface.h"
// REQUEST POSTPROCESS
#include "server/anyq_preprocessor.h"
#include "server/anyq_postprocessor.h"

// DICT
REGISTER_PLUGIN(String2FloatAdapter);
REGISTER_PLUGIN(String2IntAdapter);
REGISTER_PLUGIN(String2StringAdapter);
REGISTER_PLUGIN(String2RetrievalItemAdapter);
REGISTER_PLUGIN(WordsegAdapter);
#ifdef USE_TENSORFLOW
REGISTER_PLUGIN(TFModelAdapter);
#else
REGISTER_PLUGIN(PaddleSimAdapter);
#endif
// ANALYSIS
REGISTER_PLUGIN(AnalysisWordseg);
REGISTER_PLUGIN(AnalysisQueryIntervene);
#ifndef USE_TENSORFLOW
REGISTER_PLUGIN(AnalysisSimNetEmb);
#endif
// SOLR_Q
REGISTER_PLUGIN(EqualSolrQBuilder);
REGISTER_PLUGIN(BoostSolrQBuilder);
REGISTER_PLUGIN(ContainSolrQBuilder);
REGISTER_PLUGIN(SynonymSolrQBuilder);
REGISTER_PLUGIN(DateCompareSolrQBuilder);
// RETRIEVAL
REGISTER_PLUGIN(ManualRetrievalPlugin);
REGISTER_PLUGIN(TermRetrievalPlugin);
REGISTER_PLUGIN(SemanticRetrievalPlugin);
// MATCHING
REGISTER_PLUGIN(WordsegProcessor);
REGISTER_PLUGIN(ContainSimilarity);
REGISTER_PLUGIN(CosineSimilarity);
REGISTER_PLUGIN(JaccardSimilarity);
REGISTER_PLUGIN(EditDistanceSimilarity);
REGISTER_PLUGIN(BM25Similarity);
#ifdef USE_TENSORFLOW
REGISTER_PLUGIN(TFSimilarity);
#else
REGISTER_PLUGIN(PaddleSimilarity);
#endif
// MATCHING PREDICTOR
REGISTER_PLUGIN(PredictLinearModel);
REGISTER_PLUGIN(PredictXGBoostModel);
REGISTER_PLUGIN(PredictSelectModel);
// REQUEST POSTPROCESS
REGISTER_PLUGIN(AnyqPreprocessor);
REGISTER_PLUGIN(AnyqPostprocessor);

#endif  // BAIDU_NLP_ANYQ_PLUGIN_HEADER_H
