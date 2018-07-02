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

from collections import Counter
import logging
import numpy
import json
import time
import sys
import os

import tensorflow as tf

_WORK_DIR = os.path.split(os.path.realpath(__file__))[0]
_UPPER_DIR = os.path.split(_WORK_DIR)[0]
sys.path.append(_UPPER_DIR)
from utils import datafeeds


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
    logging.debug("\n".join(["%s=%s" % (u, conf_dict[u]) for u in conf_dict]))
    return conf_dict


def read_tfrecords_pointwise(config):
    """
    read tf records
    """
    datafeed = datafeeds.TFPointwisePaddingData(config)
    input_l, input_r, label_y = datafeed.ops()
    init_op = tf.group(tf.global_variables_initializer(),
                       tf.local_variables_initializer())
    start_time = time.time()
    sess = tf.InteractiveSession()
    sess.run(init_op)
    coord = tf.train.Coordinator()
    threads = tf.train.start_queue_runners(sess=sess, coord=coord)
    step = 0
    while not coord.should_stop():
        step += 1
        try:
            left_, right_, label_ = sess.run([input_l, input_r, label_y])
            print "pointwise data read is good"
        except tf.errors.OutOfRangeError:
            print("read %d steps" % step)
            coord.request_stop()
    coord.join(threads)
    duration = time.time() - start_time
    print("duration: %ds, step: %d" % (duration, step))
    sess.close()


def read_tfrecords_pairwise(config):
    """
    read tf records
    """
    datafeed = datafeeds.TFPairwisePaddingData(config)
    query, pos, neg = datafeed.ops()
    init_op = tf.group(tf.global_variables_initializer(),
                       tf.local_variables_initializer())
    start_time = time.time()
    sess = tf.InteractiveSession()
    sess.run(init_op)
    coord = tf.train.Coordinator()
    threads = tf.train.start_queue_runners(sess=sess, coord=coord)
    step = 0
    while not coord.should_stop():
        step += 1
        try:
            query_, pos_, neg_ = sess.run([query, pos, neg])
            print "pairwise data read is good"
        except tf.errors.OutOfRangeError:
            print("read %d steps" % step)
            coord.request_stop()
    coord.join(threads)
    duration = time.time() - start_time
    print("duration: %ds, step: %d" % (duration, step))
    sess.close()


def usage():
    """
    usage
    """
    print sys.argv[0], "options"
    print "options"
    print "\tconfig_path: configure file path"


if __name__ == "__main__":
    if len(sys.argv) != 2:
        usage()
        sys.exit(1)
    config_path = sys.argv[1]
    config = load_config(config_path)
    data_format_func = {"pointwise": read_tfrecords_pointwise,
                        "pairwise": read_tfrecords_pairwise}
    if config["training_mode"] in data_format_func:
        using_func = data_format_func[config["training_mode"]]
    else:
        logging.error("data_format not supported")
        sys.exit(1)
    using_func(config)
