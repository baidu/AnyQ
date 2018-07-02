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
import time
import sys
import os

import tensorflow as tf
from tensorflow.contrib import learn

from utils.utility import get_all_files


def load_batch_ops(example, batch_size, shuffle):
    """
    load batch ops
    """
    if not shuffle:
        return tf.train.batch([example], 
                              batch_size = batch_size,
                              num_threads = 1,
                              capacity = 10000 + 2 * batch_size)
    else:
        return tf.train.shuffle_batch([example],
                                      batch_size = batch_size,
                                      num_threads = 1,
                                      capacity = 10000 + 2 * batch_size,
                                      min_after_dequeue = 10000)


class TFPairwisePaddingData(object):
    """
    for pairwise padding data
    """
    def __init__(self, config):
        self.filelist = get_all_files(config["train_file"])
        self.batch_size = int(config["batch_size"])
        self.epochs = int(config["num_epochs"])
        if int(config["shuffle"]) == 0:
            shuffle = False
        else:
            shuffle = True
        self.shuffle = shuffle
        self.reader = None
        self.file_queue = None
        self.left_slots = dict(config["left_slots"])
        self.right_slots = dict(config["right_slots"])
        
    def ops(self):
        """
        produce data
        """
        self.file_queue = tf.train.string_input_producer(self.filelist,
                                                         num_epochs=self.epochs)
        self.reader = tf.TFRecordReader()
        _, example = self.reader.read(self.file_queue)
        batch_examples = load_batch_ops(example, self.batch_size, self.shuffle)
        features_types = {}
        [features_types.update({u: tf.FixedLenFeature([v], tf.int64)}) 
                            for (u, v) in self.left_slots.iteritems()]
        [features_types.update({"pos_" + u: tf.FixedLenFeature([v], tf.int64)}) 
                            for (u, v) in self.right_slots.iteritems()]
        [features_types.update({"neg_" + u: tf.FixedLenFeature([v], tf.int64)}) 
                            for (u, v) in self.right_slots.iteritems()]
        features = tf.parse_example(batch_examples, features = features_types)
        return dict([(k, features[k]) for k in self.left_slots.keys()]),\
                dict([(k, features["pos_" + k]) for k in self.right_slots.keys()]),\
                    dict([(k, features["neg_" + k]) for k in self.right_slots.keys()])


class TFPointwisePaddingData(object):
    """
    for pointwise padding data
    """
    def __init__(self, config):
        self.filelist = get_all_files(config["train_file"])
        self.batch_size = int(config["batch_size"])
        self.epochs = int(config["num_epochs"])
        if int(config["shuffle"]) == 0:
            shuffle = False
        else:
            shuffle = True
        self.shuffle = shuffle
        self.reader = None
        self.file_queue = None
        self.left_slots = dict(config["left_slots"])
        self.right_slots = dict(config["right_slots"])
    
    def ops(self):
        """
        gen data
        """
        self.file_queue = tf.train.string_input_producer(self.filelist, 
                                                         num_epochs=self.epochs)
        self.reader = tf.TFRecordReader()
        _, example = self.reader.read(self.file_queue)
        batch_examples = load_batch_ops(example, self.batch_size, self.shuffle)
        features_types = {"label": tf.FixedLenFeature([2], tf.int64)}
        [features_types.update({u: tf.FixedLenFeature([v], tf.int64)}) 
                            for (u, v) in self.left_slots.iteritems()]
        [features_types.update({u: tf.FixedLenFeature([v], tf.int64)}) 
                            for (u, v) in self.right_slots.iteritems()]
        features = tf.parse_example(batch_examples, features = features_types)
        return dict([(k, features[k]) for k in self.left_slots.keys()]),\
                dict([(k, features[k]) for k in self.right_slots.keys()]),\
                    features["label"]

