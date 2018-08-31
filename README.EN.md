# AnyQ

AnyQ(ANswer Your Questions) is a configurable & pluggable FAQ-based Question Answering framework. SimNet, a Semantic Matching Framework developed by Baidu-NLP, is also conveyed with AnyQ.

In our FAQ-based QA framework, which is designed to be configurable and pluggable, all the processes or functions are plugins. Developers can easily design their own processes and add to our framework, so they can quickly build QA system for their own application.

SimNet, first designed in 2013 by Baidu-NLP, is a flexible semantic matching framework which is widely used in many applications in Baidu. SimNet consists of the neural network structure BOW, CNN, RNN, and MM-DNN. Meanwhile, we have implemented more state-of-the-art structures such as MatchPyramid, MV-LSTM, K-NRM. SimNet has a unified interface, implemented with PaddleFluid and Tensorflow. Models trained using SimNet can be easily added into our AnyQ framework, through which we can augment our semantic matching ability.

This is translated from the [original README in Mandarin](README.md).

The overall framework of AnyQ is as follows:
<center>
<img src="./docs/images/AnyQ-Framework.png" width="80%" height="80%" />
</center>

## Detailed introduction

**FAQ system framework**

The AnyQ framework is mainly made up of components such as Question Analysis, Retrieval, Matching, Re-Rank. Other functionality is achieved through plug-ins, such as the Mandarin tokenisation in Analysis, the reverse search in Retrieval (倒排索引), semantic search, the Jaccard function in Matching, SimNet semantic matching (语义匹配特征). There are currently over 20 plug-ins. AnyQ's modular plug-in design aids developers in rapid (构建), rapid customisation for commercial FAQ question-answering (定制适用于特定业务场景的FAQ问答系统), selective upgrade (加速迭代和升级).
The AnyQ framework is structured as shown above.

#### Configurability
The AnyQ system comprises many search and matching add-ons, and is highly configurable; for instance, the following are examples of plug-ins for retrieval methods and similarity metrics:

* Retrieval
    * Reverse indexing: (基于) open-source reverse index Solr, added to Baidu's Open-Source tokenizer
    * Semantic search: (基于) SimNet semantic representation, using ANNOY to carry out ANN-based search
    * Manual intervention: control output by allowing users to supply the best answers
* Matching metrics
    * Text-matching similarity: to tokenise or process Mandarin questions, compute text-match metrics (字面匹配特征)
      * Cosine similarity
      * Jaccard similarity
      * BM25
    * Semantic similarity matching
        * SimNet semantic matching: using a model trained by the SimNet semantic matching framework, you can check the semantic similarity (构建问题在语义层面的相似度)

#### Pluggability

Apart from the main framework, most of AnyQ's functionality is provided by plug-ins. User-defined plug-ins are easy to integrate into the AnyQ system, as long as the correct interfaces are implemented. For instance, a user-defined dictionary, question analyser, retrieval methods, similarity metrics, sorting methods can be implemented as plug-ins.


### **SimNet, a semantic matching framework**

SimNet is a semantic matching framework developed by Baidu's NLP department in 2013. It is used in many of Baidu's products, and consists of BOW, CNN, RNN, MM-DNN neural networks. (核心网络结构形式), as well as collecting many of the popular matching models in academia, such as MatchPyramid, MV-LSTM, K-NRM, etc. SimNet is implemented using PaddleFluid and Tensorflow, to allow for easier extension. Models built with SimNet can be plugged easily into AnyQ systems, adding to the matching capabilities of AnyQ.

There are two main types of network models which the SimNet framework can implement:

* Representation-based Models <br/>
  e.g. BOW, CNN, RNN (LSTM, GRNN) <br/>
  Highlight: both ends of the matching can be annotated and compared (文本匹配任务的两端输入，分别进行表示，之后将表示进行融合计算相似度)

* Interaction-based Models <br/>
  e.g.: MatchPyramid, MV-LSTM, K-NRM, MM-DNN <br/>
  Highlight: once the text has been annotated for word-level order (word级别的序列表示), a similarity metric can be computed quickly by the ((两个序列表示)), fusing the matching scores and their similarity (融合每个位置上的匹配信息给出最终相似度打分)

For more information on how SimNet uses PaddleFluid and Tensorflow, please read the following links:

* [SimNet PaddleFluid](https://github.com/baidu/AnyQ/blob/master/tools/simnet/train/paddle/README.md)

* [SimNet TensorFlow](https://github.com/baidu/AnyQ/blob/master/tools/simnet/train/tf/README.md)

#### **About big data Search Semantic Models**

As for Baidu's big data search (百度海量搜索数据), we trained a SimNet-BOW semantic matching model. This model is able to improve AUC by at least 5% in some real-world FAQ scenarios. To find out more, please see our [Demo](#demo).

## Compilation

### Linux

- cmake 3.0 or higher (3.2.2 is recommended)
- g++ 4.8.2 or higher
- bison 3.0 or higher

    mkdir build && cd build && cmake .. && make

### Others

For MacOS, Windows, and other environments, please use docker images

    # paddle official repository
    docker pull paddlepaddle/paddle:latest-dev
    # paddle China repository
    docker pull docker.paddlepaddlehub.com/paddle:latest-dev

## Demo

Pre-requisites

    # obtain anyq-specific solr，anyq (示例配置)
    cp ../tools/anyq_deps.sh .
    sh anyq_deps.sh

    # activate solr, assuming you have python-json, jdk>=1.8
    cp ../tools/solr -rp solr_script
    sh solr_script/anyq_solr.sh solr_script/sample_docs

- **HTTP-Server**

To run the server, just use the binary in `build`:

    build/run_server

# Sample request

    http://${host}:${port}/anyq?question=XXX

- **lib**

    ./demo_anyq sample_input_json

## More reading

* [AnyQ configuration](https://github.com/baidu/AnyQ/blob/master/docs/config_tutorial.md)
* [AnyQ plugin creation](https://github.com/baidu/AnyQ/blob/master/docs/plugin_tutorial.md)
* [AnyQ semantic retrieval](https://github.com/baidu/AnyQ/blob/master/docs/semantic_retrieval_tutorial.md)

## How to contribute
* 可以在AnyQ框架下定制特定功能的插件，教程参考[AnyQ如何添加插件](https://github.com/baidu/AnyQ/blob/master/docs/plugin_tutorial.md)
* 如果觉得自己定制的插件功能足够通用&漂亮，欢迎给我们提交PR


## Copyright and License

AnyQ is provided under the [Apache-2.0 license](https://github.com/baidu/AnyQ/blob/master/LICENSE).
