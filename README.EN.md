# AnyQ

AnyQ(ANswer Your Questions) is a configurable & pluggable FAQ-based Question Answering framework. SimNet, a Semantic Matching Framework developed by Baidu-NLP, is also conveyed with AnyQ.

In our FAQ-based QA framework, which is designed to be configurable and pluggable, all the processes or functions are plugins. Developers can easily design their own processes and add to our framework, so they can quickly build QA system for their own application.

SimNet, first designed in 2013 by Baidu-NLP, is a flexible semantic matching framework which is widely used in many applications in Baidu. SimNet consists of the neural network structure BOW, CNN, RNN, and MM-DNN. Meanwhile, we have implemented more state-of-the-art structures such as MatchPyramid, MV-LSTM, K-NRM. SimNet has a unified interface, implemented with PaddleFluid and Tensorflow. Models trained using SimNet can be easily added into our AnyQ framework, through which we can augment our semantic matching ability.

This is translated from the [original README in Mandarin](README.md).

## Detailed introduction

**FAQ system framework**

The AnyQ framework is mainly made up of components such as Question Analysis, Retrieval, Matching, Re-Rank. Other functionality is achieved through plugins, such as the Mandarin tokenisation in Analysis, inverted index, semantic search, the Jaccard function in Matching, SimNet semantic matching properties. There are currently over 20 plugins. AnyQ's modular plugin design aids developers in rapid construction, customisation for commercial FAQ question-answering, upgrade, and iteration.

The overall framework of AnyQ is as follows:
<center>
<img src="./docs/images/AnyQ-Framework.png" width="80%" height="80%" />
</center>

#### Configurability
The AnyQ system comprises many search and matching add-ons, and is highly configurable; for instance, the following are examples of plugins for retrieval methods and similarity metrics:

* Retrieval
    * Reverse indexing: based on open-source reverse index Solr, added to Baidu's Open-Source tokenizer
    * Semantic search: based on SimNet semantic representation, using ANNOY to carry out ANN-based search
    * Manual intervention: control output by allowing users to supply the best answers
* Matching metrics
    * Text-matching similarity: to tokenise or process Mandarin questions, compute text-matching features
      * Cosine similarity
      * Jaccard similarity
      * BM25
    * Semantic similarity matching
        * SimNet semantic matching: using a model trained by the SimNet semantic matching framework, you can check the semantic similarity of constructs.

#### Pluggability

Apart from the main framework, most of AnyQ's functionality is provided by plugins. User-defined plugins are easy to integrate into the AnyQ system, as long as the correct interfaces are implemented. For instance, a user-defined dictionary, question analyser, retrieval methods, similarity metrics, sorting methods can be implemented as plugins.


### **SimNet, a semantic matching framework**

SimNet is a semantic matching framework developed by Baidu's NLP department in 2013. It is used in many of Baidu's products, and consists of BOW, CNN, RNN, MM-DNN core network models, as well as collecting many of the popular matching models in academia, such as MatchPyramid, MV-LSTM, K-NRM, etc. SimNet is implemented using PaddleFluid and Tensorflow, to allow for easier extension. Models built with SimNet can be plugged easily into AnyQ systems, adding to the matching capabilities of AnyQ.

There are two main types of network models which the SimNet framework can implement:

* Representation-based Models <br/>
  e.g. BOW, CNN, RNN (LSTM, GRNN) <br/>
  Highlight: a fused similarity score can be computed using input from both ends of the text matching task

* Interaction-based Models <br/>
  e.g.: MatchPyramid, MV-LSTM, K-NRM, MM-DNN <br/>
  Highlight: given word sequences, similarity metrics can be computed quickly for each sequence, fusing the matching scores to provide a final similarity score

For more information on how SimNet uses PaddleFluid and Tensorflow, please read the following links:

* [SimNet PaddleFluid](https://github.com/baidu/AnyQ/blob/master/tools/simnet/train/paddle/README.md)

* [SimNet TensorFlow](https://github.com/baidu/AnyQ/blob/master/tools/simnet/train/tf/README.md)

#### **About big data Search Semantic Models**

As for Baidu's big data search, we trained a SimNet-BOW semantic matching model. This model is able to improve AUC by at least 5% in some real-world FAQ scenarios. To find out more, please see our [Demo](#demo).

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

- **Pre-requisites**

      # obtain and configure anyq-specific solr
      cp ../tools/anyq_deps.sh .
      sh anyq_deps.sh

      # activate solr, assuming you have python-json, jdk>=1.8
      cp ../tools/solr -rp solr_script
      sh solr_script/anyq_solr.sh solr_script/sample_docs

- **HTTP-Server**

    To run the server, just use the binary in `build`:

      build/run_server

- **Sample request**

      http://${host}:${port}/anyq?question=XXX

- **lib**

      build/demo_anyq sample_input_json

## More reading

* [AnyQ configuration](https://github.com/baidu/AnyQ/blob/master/docs/config_tutorial.md)
* [AnyQ plugin creation](https://github.com/baidu/AnyQ/blob/master/docs/plugin_tutorial.md)
* [AnyQ semantic retrieval](https://github.com/baidu/AnyQ/blob/master/docs/semantic_retrieval_tutorial.md)

## How to contribute
* Plugins providing any of the AnyQ framework functionality can be developed with [this tutorial](https://github.com/baidu/AnyQ/blob/master/docs/plugin_tutorial.md)
* If you feel your plugin is usable and readable enough, please feel free to submit a PR


## Copyright and License

AnyQ is provided under the [Apache-2.0 license](https://github.com/baidu/AnyQ/blob/master/LICENSE).
