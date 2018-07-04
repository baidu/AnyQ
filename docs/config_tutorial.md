# AnyQ系统配置教程

## 简介
在AnyQ系统中，词典和策略都以插件的形式封装。开发者可以根据实际的业务需要配置Analysis、Retrieval、Matching和Ranking策略及相关的词典。Anyq的配置文件需按照Protobuf格式。

## load词典

在策略插件中使用的词典，需在dict.conf中配置。当前系统的词典插件主要包括以下几种：   
* 哈希词典：HashAdapter<TYPE1, TYPE2>   
* 干预词典：String2RetrievalItemAdapter   
* 切词词典: WordsegAdapter   
* Paddle SimNet匹配模型词典: PaddleSimAdapter 
* Tensorflow模型词典: TFModelAdapter   

配置的格式, 举例如下

```
dict_config {
    name: "fluid_simnet"
    type: "PaddleSimAdapter"
    path: "./simnet"
}
```

策略类型的插件如果要使用词典，需给策略插件的using_dict_name与词典的name配置相同的值。

## Analysis

在analysis.conf中可以增添query分析策略插件。当前系统的analysis插件主要包括以下几种:   
* 切词：AnalysisWordseg   
* query语义向量表示：AnalysisSimNetEmb   
* query替换: AnalysisQueryIntervene   

配置的格式如下

```
analysis_method {
    name: "method_simnet_emb"
    type: "AnalysisSimNetEmb"
    using_dict_name: "fluid_simnet"
    dim: 128
    query_feed_name: "left" 
    cand_feed_name: "right" 
    embedding_fetch_name: "tanh.tmp"
}
```

## Retrieval配置
在retrieval.conf中可以增添检索策略插件。当前系统的retrieval插件主要包括以下几种:   
* Term检索：TermRetrievalPlugin   
* 语义检索：SemanticRetrievalPlugin   
* 人工干预：ManualRetrievalPlugin   

配置的格式如下

```
retrieval_plugin {
    name : "term_recall_1"
    type : "TermRetrievalPlugin"
    search_host : "127.0.0.1"
    search_port : 8900
    engine_name : "collection1"
    solr_result_fl : "id,question,answer"
    solr_q : {
        type : "EqualSolrQBuilder"
        name : "equal_solr_q_1"
        solr_field : "question"
        source_name : "question"
    }
    num_result : 15
}
```

- **Solr查询插件**

    其中"solr_q"是solr查询的插件，当前可配置的solr查询插件包括:   
    * EqualSolrQBuilder：字段等于   
    * BoostSolrQBuilder：字段term加权   
    * ContainSolrQBuilder：字段包含关键词   
    * SynonymSolrQBuilder：字段term同义词   
    * DateCompareSolrQBuilder：日期字段比较   

## Matching
在rank.conf中可以增添匹配策略插件。当前系统的matching插件主要包括以下几种：   
* 编辑距离相似度：EditDistanceSimilarity   
* Cosine相似度：CosineSimilarity   
* Jaccard相似度: JaccardSimilarity   
* BM25相似度: BM25Similarity   
* Paddle SimNet匹配模型相似度: PaddleSimilarity   
* Tensorflow匹配模型相似度： TFSimilarity   
* 对候选切词：WordsegProcessor   

配置的格式如下

```
matching_config {
    name: "fluid_simnet_similarity"
    type: "PaddleSimilarity"
    using_dict_name: "fluid_simnet"
    output_num : 1
    rough : false 
    query_feed_name: "left" 
    cand_feed_name: "right" 
    score_fetch_name: "cos_sim.tmp"
}
```

其中output_num表示该插件输出的相似度值个数，对于不输出相似度的插件(如WordsegProcessor)，output_num应配置为0; rough配置为true表示该相似度用于粗排，反之则用于精排。   
计算相似度需要先对候选切词，因此WordsegProcessor插件应该配置在其他matching_config之前。

## Ranking
在rank.conf中配置predictorc插件，用于根据多个相似度对候选计算得分。当前系统的rank插件主要包括以下几种：    
* 线性预测模型： PredictLinearModel    
* XGBoost预测模型： PredictXGBoostModel    
* 特征选择预测模型：PredictSelectModel    

配置的格式如下

```
rank_predictor {
    type: "PredictLinearModel"
    using_dict_name: "rank_weights"
}
```

