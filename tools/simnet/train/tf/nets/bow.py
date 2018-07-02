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

import logging

import tensorflow as tf

import layers.tf_layers as layers


class BOW(object):
    """
    bow init function
    """
    def __init__(self, config):
        self.vocab_size = int(config['vocabulary_size'])
        self.emb_size = int(config['embedding_dim'])
        self.bow_size = int(config["bow_size"])
        self.hidden_size = int(config['hidden_size'])
        self.left_name, self.seq_len = config["left_slots"][0]
        self.right_name, self.seq_len = config["right_slots"][0]
        self.task_mode = config['training_mode']
        self.emb_layer = layers.EmbeddingLayer(self.vocab_size, self.emb_size)
        self.seq_pool_layer = layers.SequencePoolingLayer()
        self.softsign_layer = layers.SoftsignLayer()
        self.bow_layer = layers.FCLayer(self.emb_size, self.bow_size)
        if self.task_mode == "pointwise":
            self.n_class = int(config['n_class'])
            self.fc_layer = layers.FCLayer(2 * self.hidden_size, self.n_class)
        elif self.task_mode == "pairwise":
            self.cos_layer = layers.CosineLayer()
        else:
            logging.error("training mode not supported")
     
    def predict(self, left_slots, right_slots):
        """
        predict graph of this net
        """
        left = left_slots[self.left_name]
        right = right_slots[self.right_name]
        left_emb = self.emb_layer.ops(left)
        right_emb = self.emb_layer.ops(right)
        left_pool = self.seq_pool_layer.ops(left_emb)
        right_pool = self.seq_pool_layer.ops(right_emb)
        left_soft = self.softsign_layer.ops(left_pool)
        right_soft = self.softsign_layer.ops(right_pool)
        left_bow = self.bow_layer.ops(left_soft)
        right_bow = self.bow_layer.ops(right_soft)
        if self.task_mode == "pointwise":
            concat = tf.concat([left_bow, right_bow], -1)
            pred = self.fc_layer.ops(concat)
        else:
            pred = self.cos_layer.ops(left_bow, right_bow)
        return pred



# vim: set expandtab ts=4 sw=4 sts=4 tw=100:
