anyq/tools/simnet

应用介绍: 
1) 提供语义相似度模型的训练和预测流程;
2) 支持paddle和tensorflow两种框架进行模型训练; 
3) 提供pointwise和pairwise等方法的神经网络配置;

使用说明: 
1) 试用: 用户指定运行平台, 如下：
   * paddle框架训练: sh train.sh paddle
   * paddle框架预测: sh predict.sh paddle
   * tensorflow框架训练: sh train.sh tensorflow
   * tensorflow框架预测: sh predict.sh tensorflow

2) 用户自定义训练: 
   * paddle训练目录: anyq/tools/simnet/train/paddle
      * run_train.sh内为训练流程:
       指定训练参数in_conf_file_path, 训练参数参见anyq/tools/simnet/train/paddle/examples/cnn_pointwise.json
      * run_infer.sh内为预测流程

   * tensorflow训练目录: anyq/tools/simnet/train/tf
      * run_train.sh内为训练流程:
       指定训练参数in_task_conf, 训练参数参见anyq/tools/simnet/train/tf/examples/cnn-pointwise.json
      * run_infer.sh内为预测流程
   
注意事项: 
用户在本地安装paddle和tensorflow包后即可使用;

问题咨询: 
欢迎用户提交任何问题和Bug Report                                                                                                                                                                                                                               
