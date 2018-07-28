# AnyQ

AnyQ(ANswer Your Questions) is a configurable & plugable FAQ-based Question Answering framework. SimNet, a Semantic Matching Framework developed by Baidu-NLP, is also conveyed with AnyQ. 

In our FAQ-based QA framework, which is designed to be configurable and plugable, all the processes or functions are plugins. Developers can easily designed their own processes and add to our framework, so they can quickly build QA system for their own application.  

SimNet, first designed in 2013 by Baidu-NLP, is a flexiable semantic matching framework which is widely used in many applications in Baidu. SimNet consists of the neural network structure BOW、CNN、RNN and MM-DNN. Meanwhile, we have implemented more state-of-the-art structures such as MatchPyramid、MV-LSTM、K-NRM. SimNet has a unified interface, implemented with PaddleFluid and Tensorflow. Models trained using SimNet can be easily added into our AnyQ framework, through which we can augment our semantic matching ability.

The overall framework of AnyQ is as follows:
<center>
<img src="./docs/images/AnyQ-Framework.png" width="80%" height="80%" />
</center>
