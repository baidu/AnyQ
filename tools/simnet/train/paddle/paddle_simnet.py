#coding=utf-8

# Copyright (c) 2018 Baidu, Inc. All Rights Reserved.
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import numpy as np
import argparse
import logging
import time
import sys
import os

import paddle
import paddle.fluid as fluid
import paddle.fluid.core as core

import layers.paddle_layers as layers
import util.data_reader as data_reader

_WORK_DIR = os.path.split(os.path.realpath(__file__))[0]
sys.path.append(os.path.join(_WORK_DIR, '../../../common'))
import utils
import log


def train(conf_dict):
    """
    train process
    """
    # Get data layer
    data = layers.DataLayer()
    # Load network structure dynamically
    net = utils.import_class(
        "nets", conf_dict["net"]["module_name"], conf_dict["net"]["class_name"])(conf_dict)
    # Load loss function dynamically
    loss = utils.import_class(
        "losses", conf_dict["loss"]["module_name"], conf_dict["loss"]["class_name"])(conf_dict)
    # Load Optimization method
    optimizer = utils.import_class(
        "optimizers", "paddle_optimizers", conf_dict["optimizer"]["class_name"])(conf_dict)
    
    # Get service
    if "use_cuda" in conf_dict and conf_dict["use_cuda"] == 1:
        place = fluid.core.CUDAPlace(0)
    else:
        place = fluid.core.CPUPlace()

    if conf_dict["task_mode"] == "pairwise":
        # Build network
        left = data.ops(name="left", shape=[1], dtype="int64", lod_level=1)
        pos_right = data.ops(name="right", shape=[
                             1], dtype="int64", lod_level=1)
        neg_right = data.ops(name="neg_right", shape=[
                             1], dtype="int64", lod_level=1)
        left_feat, pos_score = net.predict(left, pos_right)
        _, neg_score = net.predict(left, neg_right)
        avg_cost = loss.compute(pos_score, neg_score)
        # Get Feeder and Reader
        feeder = fluid.DataFeeder(place=place, feed_list=[
                                  left.name, pos_right.name, neg_right.name])
        reader = data_reader.get_reader(conf_dict, False, None)
    else:
        # Build network
        left = data.ops(name="left", shape=[1], dtype="int64", lod_level=1)
        right = data.ops(name="right", shape=[1], dtype="int64", lod_level=1)
        label = data.ops(name="label", shape=[1], dtype="int64", lod_level=0)
        left_feat, pred = net.predict(left, right)
        avg_cost = loss.compute(pred, label)
        avg_cost.persistable = True
        # Get Feeder and Reader
        feeder = fluid.DataFeeder(place=place, feed_list=[
                                  left.name, right.name, label.name])
        reader = data_reader.get_reader(conf_dict, False, None)
    # Save Infer model
    infer_program = fluid.default_main_program().clone()
    # operate Optimization
    optimizer.ops(avg_cost)
    # optimize memory 
    fluid.memory_optimize(fluid.default_main_program())
    executor = fluid.Executor(place)
    executor.run(fluid.default_startup_program())
    # Get and run executor
    parallel_executor = fluid.ParallelExecutor(
        use_cuda="use_cuda" in conf_dict and conf_dict["use_cuda"] == 1, 
        loss_name=avg_cost.name,
        main_program=fluid.default_main_program())
    # Get device number
    device_count = parallel_executor.device_count
    logging.info("device count: %d" % device_count)
    # run train
    logging.info("start train process ...")
    for epoch_id in range(conf_dict["epoch_num"]):
        losses = []
        # Get batch data iterator
        batch_data = paddle.batch(reader, conf_dict["batch_size"], drop_last=False)
        start_time = time.time()
        total_loss = 0.0
        for iter, data in enumerate(batch_data()):
            if len(data) < device_count:
                continue
            avg_loss = parallel_executor.run(
                [avg_cost.name], feed=feeder.feed(data))
            total_loss += np.mean(avg_loss[0])
            if (iter + 1) % 100 == 0:
                print("epoch: %d, iter: %d, loss: %f" %
                    (epoch_id, iter, total_loss / 100))
                total_loss = 0.0
            losses.append(np.mean(avg_loss[0]))
        end_time = time.time()
        print("epoch: %d, loss: %f, used time: %d sec" %
              (epoch_id, np.mean(losses), end_time - start_time))
        model_save_dir = conf_dict["model_path"]
        model_path = os.path.join(model_save_dir, str(epoch_id))
        if not os.path.exists(model_save_dir):
            os.makedirs(model_save_dir)
        if conf_dict["task_mode"] == "pairwise":
            feed_var_names = [left.name, pos_right.name]
            target_vars = [left_feat, pos_score]
        else:
            feed_var_names = [left.name, right.name]
            target_vars = [left_feat, pred]
        fluid.io.save_inference_model(
            model_path, feed_var_names, target_vars, executor, infer_program)


def predict(conf_dict):
    """
    run predict
    """
    with open("samples.txt", "w") as samples_file:
        with open("predictions.txt", "w") as predictions_file:
            # Get model path
            model_save_dir = conf_dict["model_path"]
            model_path = os.path.join(model_save_dir, str(conf_dict["use_epoch"]))
            # Get device 
            if "use_cuda" in conf_dict and conf_dict["use_cuda"] == 1:
                place = fluid.core.CUDAPlace(0)
            else:
                place = fluid.core.CPUPlace()
            # Get executor
            executor = fluid.Executor(place=place)
            # Load model
            program, feed_var_names, fetch_targets = fluid.io.load_inference_model(
                model_path, executor)
            if conf_dict["task_mode"] == "pairwise":
                # Get Feeder and Reader
                feeder = fluid.DataFeeder(
                    place=place, feed_list=feed_var_names, program=program)
                reader = data_reader.get_reader(conf_dict, True, samples_file)
            else:
                # Get Feeder and Reader
                feeder = fluid.DataFeeder(
                    place=place, feed_list=feed_var_names, program=program)
                reader = data_reader.get_reader(conf_dict, True, samples_file)
            # Get batch data iterator
            batch_data = paddle.batch(reader, conf_dict["batch_size"], drop_last=False)
            logging.info("start test process ...")
            for iter, data in enumerate(batch_data()):
                output = executor.run(program, feed=feeder.feed(
                    data), fetch_list=fetch_targets)
                if conf_dict["task_mode"] == "pairwise":
                    predictions_file.write(
                        "\n".join(map(lambda item: str(item[0]), output[1])) + "\n")
                else:
                    predictions_file.write(
                        "\n".join(map(lambda item: str(np.argmax(item)), output[1])) + "\n")
    utils.get_result_file(conf_dict, "samples.txt", "predictions.txt")


if __name__ == "__main__":
    log.init_log("./log/paddle")
    parser = argparse.ArgumentParser()
    parser.add_argument("--task_type", default="train",
                        help="task type include train/predict, the default value is train")
    parser.add_argument(
        "--conf_file_path", default="examples/cnn_pointwise.json", help="config file path")
    args = parser.parse_args()
    conf_dict = utils.parse_json(args.conf_file_path)
    print(conf_dict)
    if args.task_type == "train":
        train(conf_dict)
    else:
        predict(conf_dict)
