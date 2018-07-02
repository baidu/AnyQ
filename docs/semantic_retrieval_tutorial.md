# 添加语义索引

## 语义索引库构建
(1) 将灌库文件faq_file(utf8编码)转换成Json格式：

```
cp -rp ../tool/solr ./solr_script
mkdir -p faq
python solr_script/make_json.py solr_script/sample_docs faq/schema_format faq/faq_json
```

(2) 对Json文本添加索引id：

```
awk -F "\t" '{print ++ind"\t"$0}' faq/faq_json > faq/faq_json.index
```

(3) 在anyq词典配置dict.conf增加语义表示模型的插件：

```
dict_config{
    name: "fluid_simnet"
    type: "PaddleSimAdapter"
    path: "./simnet"
}
```

(4) 在analysis.conf中增加query语义表示的插件：

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

(5) 生成语义索引库：

```
./annoy_index_build_tool example/conf/ example/conf/analysis.conf faq/faq_json.index 128 10 semantic.annoy 1>std 2>err
```

## 语义索引库使用

(1) 把带索引id的faq库文件和语义索引库放到anyq配置目录下

```
cp faq/faq_json.index semantic.annoy example/conf
```

(2) 在dict.conf中配置faq库文件的读取

```
dict_config {
    name: "annoy_knowledge_dict"
    type: "String2RecallItemAdapter"
    path: "./faq_json.index"
}
```
    
(3) 在retrieval.conf中配置语义检索插件

```
retrieval_plugin {
    name : "semantic_recall"
    type : "SemanticRetrievalPlugin"
    vector_size : 128
    search_k : 10000 
    index_path : "./example/conf/semantic.annoy"
    using_dict_name: "annoy_knowledge_dict"
    num_result : 10
}
```
