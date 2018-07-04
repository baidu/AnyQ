# AnyQ系统如何添加插件

## 已有插件

- **字典插件**

	此类插件继承DictInterface接口    
	* 哈希词典：HashAdapter<TYPE1, TYPE2>    
	* 干预词典：String2RetrievalItemAdapter    
	* 切词词典: WordsegAdapter    
	* Paddle匹配模型词典: PaddleSimAdapter    
	* Tensorflow模型词典: TFModelAdapter    

- **Analysis插件**

	此类插件继承AnalysisMethodInterface接口    
	* 切词：AnalysisWordseg    
	* query语义向量表示：AnalysisSimNetEmb    
	* query替换: AnalysisQueryIntervene    

- **Retrieval插件**

	此类插件继承RetrievalPluginInterface接口    
	* Term检索：TermRetrievalPlugin    
	* 语义检索：SemanticRetrievalPlugin    
	* 人工干预：ManualRetrievalPlugin    

- **Solr查询插件**

	此类插件继承SolrQInterface接口    
	* EqualSolrQBuilder：字段等于    
	* BoostSolrQBuilder：字段term加权    
	* ContainSolrQBuilder：字段包含关键词    
	* SynonymSolrQBuilder：字段term同义词    
	* DateCompareSolrQBuilder：日期字段比较    

- **Matching插件**

	此类插件继承MatchingInterface接口    
	* 编辑距离相似度：EditDistanceSimilarity    
	* Cosine相似度：CosineSimilarity    
	* Jaccard相似度: JaccardSimilarity    
	* BM25相似度: BM25Similarity    
	* Paddle匹配模型相似度: PaddleSimilarity    
	* Tensorflow匹配模型相似度：TFSimilarity    
	* 对候选切词：WordsegProcessor    

- **Rank插件**

	此类插件继承RankPredictInterface接口    
	* 线性预测模型： PredictLinearModel    
	* XGBoost预测模型： PredictXGBoostModel    
	* 特征选择预测模型：PredictSelectModel    

- **请求处理插件**

	* 请求解包插件(继承ReqPreprocInterface接口)：AnyqPreprocessor    
	* 请求包装插件(继承ReqPostprocInterface接口)：AnyqPostprocessor    


## 添加插件

以BM25Similarity插件为实例，说明如何添加用户自定义插件。

### 头文件定义

1. BM25Similarity是Matching插件，需继承MatchingInterface接口，并重写相关的虚函数.

	include/matching/lexical/bm25_sim.h
	```
	...

	class BM25Similarity : public MatchingInterface {
	public:
		BM25Similarity();
		virtual ~BM25Similarity() override;
		virtual int init(DualDictWrapper* dict, const MatchingConfig& matching_config) override;
		virtual int destroy() override;
		virtual int compute_similarity(const AnalysisResult& analysis_res,
			 RankResult& candidates) override;
	...

	};
	```

2. 注册BM25Similarity插件.

	include/common/plugin_header.h
	```
	...

	#include "matching/lexical/bm25_sim.h"
	...

	REGISTER_PLUGIN(BM25Similarity);
	...

	```

### 功能实现

实现构造、析构、init、destroy、compute_similarity等函数。

src/matching/lexical/bm25_sim.cpp
```
...

BM25Similarity::BM25Similarity(){
...

}
...

BM25Similarity::~BM25Similarity(){
...

}
...

int BM25Similarity::init(DualDictWrapper* dict, const MatchingConfig& matching_config) {
...

}
...

int BM25Similarity::destroy() {
...

}
...

```

### 如何编译

```
mkdir build && cd build && cmake .. && make
```
