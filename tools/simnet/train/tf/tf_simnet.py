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

import argparse
import logging
import json
import sys
import os

import tensorflow as tf
from tensorflow.python.framework import graph_util

from utils import datafeeds
from utils import controler
from utils import utility
from utils import converter

_WORK_DIR = os.path.split(os.path.realpath(__file__))[0]
sys.path.append(os.path.join(_WORK_DIR, '../../../common'))
import log 


def load_config(config_file):
    """
    load config
    """
    with open(config_file, "r") as f:
        try:
            conf = json.load(f)
        except Exception:
            logging.error("load json file %s error" % config_file)
    conf_dict = {}
    unused = [conf_dict.update(conf[k]) for k in conf]
    logging.debug("\n".join(
        ["%s=%s" % (u, conf_dict[u]) for u in conf_dict]))
    return conf_dict


def train(conf_dict):
    """
    train
    """
    training_mode = conf_dict["training_mode"]
    net = utility.import_object(
        conf_dict["net_py"], conf_dict["net_class"])(conf_dict)
    if training_mode == "pointwise":
        datafeed = datafeeds.TFPointwisePaddingData(conf_dict)
        input_l, input_r, label_y = datafeed.ops()
        pred = net.predict(input_l, input_r)
        output_prob = tf.nn.softmax(pred, -1, name="output_prob")
        loss_layer = utility.import_object(
            conf_dict["loss_py"], conf_dict["loss_class"])()
        loss = loss_layer.ops(pred, label_y)
    elif training_mode == "pairwise":
        datafeed = datafeeds.TFPairwisePaddingData(conf_dict)
        input_l, input_r, neg_input = datafeed.ops()
        pos_score = net.predict(input_l, input_r)
        output_prob = tf.identity(pos_score, name="output_prob")
        neg_score = net.predict(input_l, neg_input)
        loss_layer = utility.import_object(
            conf_dict["loss_py"], conf_dict["loss_class"])(conf_dict)
        loss = loss_layer.ops(pos_score, neg_score)
    else:
        print >> sys.stderr, "training mode not supported"
        sys.exit(1)
    # define optimizer
    lr = float(conf_dict["learning_rate"])
    optimizer = tf.train.AdamOptimizer(learning_rate=lr).minimize(loss)

    # run_trainer
    controler.run_trainer(loss, optimizer, conf_dict)


def predict(conf_dict):
    """
    predict
    """
    net = utility.import_object(
        conf_dict["net_py"], conf_dict["net_class"])(conf_dict)
    conf_dict.update({"num_epochs": "1", "batch_size": "1",
                      "shuffle": "0", "train_file": conf_dict["test_file"]})
    test_datafeed = datafeeds.TFPointwisePaddingData(conf_dict)
    test_l, test_r, test_y = test_datafeed.ops()
    # test network
    pred = net.predict(test_l, test_r)
    controler.run_predict(pred, test_y, conf_dict)


def freeze(conf_dict):
    """
    freeze net for c api predict
    """
    model_path = conf_dict["save_path"]
    freeze_path = conf_dict["freeze_path"]
    training_mode = conf_dict["training_mode"]
    
    graph = tf.Graph()
    with graph.as_default():
        net = utility.import_object(
                conf_dict["net_py"], conf_dict["net_class"])(conf_dict)
        test_l = dict([(u, tf.placeholder(tf.int32, [None, v], name=u))
            for (u, v) in dict(conf_dict["left_slots"]).iteritems()])
        test_r = dict([(u, tf.placeholder(tf.int32, [None, v], name=u))
            for (u, v) in dict(conf_dict["right_slots"]).iteritems()])
        pred = net.predict(test_l, test_r)
        if training_mode == "pointwise":
            output_prob = tf.nn.softmax(pred, -1, name="output_prob")
        elif training_mode == "pairwise":
            output_prob = tf.identity(pred, name="output_prob")
 
        restore_saver = tf.train.Saver()
    with tf.Session(graph=graph) as sess:
        sess.run(tf.global_variables_initializer())
        restore_saver.restore(sess, model_path)
        output_graph_def = tf.graph_util.\
            convert_variables_to_constants(sess, sess.graph_def, ["output_prob"])
        tf.train.write_graph(output_graph_def, '.', freeze_path, as_text=False)

def convert(conf_dict):
    """
    convert
    """
    converter.run_convert(conf_dict)


if __name__ == "__main__": 
    log.init_log("./log/tensorflow")
    parser = argparse.ArgumentParser()
    parser.add_argument('--task', default='train',
                        help='task: train/predict/freeze/convert, the default value is train.')
    parser.add_argument('--task_conf', default='./examples/cnn-pointwise.json',
                        help='task_conf: config file for this task')
    args = parser.parse_args()
    task_conf = args.task_conf
    config = load_config(task_conf)
    task = args.task
    if args.task == 'train':
        train(config)
    elif args.task == 'predict':
        predict(config)
    elif args.task == 'freeze':
        freeze(config)
    elif args.task == 'convert':
        convert(config)
    else:
        print >> sys.stderr, 'task type error.'
