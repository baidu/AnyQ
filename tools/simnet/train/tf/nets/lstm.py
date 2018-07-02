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
from utils.utility import seq_length


class LSTM(object):
    """
    mlp cnn init function
    """
    def __init__(self, config):
        self.vocab_size = int(config['vocabulary_size'])
        self.emb_size = int(config['embedding_dim'])
        self.rnn_hidden_size = int(config['rnn_hidden_size'])
        self.hidden_size = int(config['hidden_size'])
        self.left_name, self.seq_len1 = config['left_slots'][0]
        self.right_name, self.seq_len2 = config['right_slots'][0]
        self.task_mode = config['training_mode']
        self.emb_layer = layers.EmbeddingEnhancedLayer(self.vocab_size, 
                                self.emb_size, zero_pad=True, scale=False)
        self.rnn = layers.LSTMLayer(self.rnn_hidden_size)
        self.extract = layers.ExtractLastLayer()
        if self.task_mode == "pointwise":
            self.n_class = int(config['n_class'])
            self.fc1_layer = layers.FCLayer(self.rnn_hidden_size * 2, self.hidden_size)
            self.fc2_layer = layers.FCLayer(self.hidden_size, self.n_class)
        elif self.task_mode == "pairwise":
            self.fc1_layer = layers.FCLayer(self.rnn_hidden_size * 1, self.hidden_size)
            self.cos_layer = layers.CosineLayer()
        else:
            logging.error("training mode not supported")
        
    def predict(self, left_slots, right_slots):
        """
        predict graph of this net
        """
        left = left_slots[self.left_name]
        right = right_slots[self.right_name]
        left_emb = self.emb_layer.ops(left) # (N, len, D)
        right_emb = self.emb_layer.ops(right) # (N, len, D)
        ## left
        left_length = seq_length(left)
        left_encoder = self.rnn.ops(left_emb, left_length)
        left_rep = self.extract.ops(left_encoder, left_length)
        right_length = seq_length(right)
        right_encoder = self.rnn.ops(right_emb, right_length)
        right_rep = self.extract.ops(right_encoder, right_length)
        if self.task_mode == "pointwise":
            rep_concat = tf.concat([left_rep, right_rep], -1)
            hidden1 = self.fc1_layer.ops(rep_concat)
            pred = self.fc2_layer.ops(hidden1)
        elif self.task_mode == "pairwise":
            left_hidden1 = self.fc1_layer.ops(left_rep)
            right_hidden1 = self.fc1_layer.ops(right_rep)
            pred = self.cos_layer.ops(left_hidden1, right_hidden1)
        return pred

