
# Paddle版SimNet
Paddle版本Simnet提供了BOW，CNN，LSTM及GRU四种网络实现，您可以通过配置文件的形式灵活选择您需要的网络，损失函数，训练方式。

## 内容
- [代码结构](#1)
- [安装](#2)
- [数据准备](#3)
- [模型训练](#4)
- [模型推断](#5)

## <span id="5">代码结构</span>
```
simnet
    |- paddle
        |- date //示例数据
        |- examples //示例配置文件
        |- layers //网络中使用操作层的实现
        |- losses //损失函数实现
        |- nets //网络结构实现
        |- optimizers //优化方法实现
        |- util //工具类
```

## <span id="1">安装</span>

Paddle版SimNet需要PaddlePaddle Fluid v1.0.2，详细安装方式见[PaddlePaddle官网](http://www.paddlepaddle.org/)。

## <span id="2">数据准备</span>

SimNet可以使用Pointwise与Pairwise两种类型的数据进行训练

**Pointwise训练及测试数据格式** 

* *训练数据格式*：训练数据包含三列，依次为Query1的ID序列（ID间使用空格分割），Query2的ID序列（ID间使用空格分割），Label，每列间使用TAB分割，例如；
```
1 1 1 1 1   2 2 2 2 2   0
1 1 1 1 1   1 1 1 1 1   1
...
```
* *测试数据格式*：Pointwise测试数据格式与训练数据格式相同。

**Pairwise训练及测试数据格式** 

* *训练数据格式*：训练数据包含三列，依次为Query1的ID序列（ID间使用空格分割），Positive Query2的ID序列（ID间使用空格分割），Negative Query3的ID序列（ID间使用空格分割），每列间使用TAB分割，例如；
```
1 1 1 1 1   1 1 1 1 1   2 2 2 2 2   
1 1 1 1 1   1 1 1 1 1   3 3 3 3 3
...
```
* *测试数据格式*：测试数据格式包含四列，依次为QID（用于标记唯一Query1）， Label，Query1的ID序列（ID间使用空格分割），Query2的ID序列（ID间使用空格分割），每列间使用TAB分割，例如；
```
1   1   1 1 1 1 1   1 1 1 1 1
1   0   1 1 1 1 1   2 2 2 2 2
2   1   3 3 3 3 3   3 3 3 3 3
...
```

## <span id="3">模型训练</span>

数据准备完毕后，可以通过执行脚本run_train.sh启动示例训练，也可以通过如下方式启动自定义训练：
```
python paddle_simnet.py
        --task_type train
        --conf_file_path examples/cnn_pointwise.json
```
**参数说明：**
* **task_type**: 任务类型 ，可选择train或predict。
* **conf_file_path**: 使用配置文件地址

**配置文件说明：** 通过配置文件您可以灵活的选择网络类型，数据类型，损失函数以及其他超参数，例如：
```
{
    //网络类型
    "net": {
        "module_name": "cnn", // 网络对应模块名
        "class_name": "CNN", // 网络对应类名
        "emb_dim": 128, //Embedding Dimension
        "filter_size": 3, //卷机核大小
        "num_filters": 256, //卷机核数量
        "hidden_dim": 128 //隐藏层维度
    },
    //损失函数
    "loss": {
        "module_name": "softmax_cross_entropy_loss", //损失函数对应模块名
        "class_name": "SoftmaxCrossEntropyLoss" //损失函数对应类名
    },
    //优化方法
    "optimizer": {
        "class_name": "SGDOptimizer", //优化方法对应类名
        "learning_rate" : 0.0001 //学习率
    },
    "dict_size": 10, //词典大小
    "task_mode": "pointwise", //数据类型
    "train_file_path": "data/train_pointwise_data", //训练数据文件路径
    "test_file_path": "data/test_pointwise_data", //测试数据文件路径
    "result_file_path": "result", //存储测试结果文件路径
    "epoch_num": 10, //Number of Epoch
    "model_path": "models/cnn_pointwise", //模型保存目录
    "use_epoch": 0, //使用哪个epoch产生的模型测试，其值范围为[0, epoch_num)的整数
    "batch_size": 64, //Batch Size
    "num_threads": 4 //训练线程数
}
```

## <span id="4">模型推断</span>
模型推断可以得到模型预测结果或得分，您可以通过执行脚本run_infer.sh启动示例训练，也可以通过如下方式启动自定义训练：
```
python paddle_simnet.py
        --task_type predict
        --conf_file_path examples/cnn_pointwise.json
```
**参数说明：**
同模型训练部分

**配置文件说明：**
同模型训练部分


