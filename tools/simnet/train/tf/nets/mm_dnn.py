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

from utils import utility
import layers.tf_layers as layers


class MMDNN(object):
    """
    mlp cnn init function
    """
    def __init__(self, config):
        self.vocab_size = int(config['vocabulary_size'])
        self.emb_size = int(config['embedding_dim'])
        self.lstm_dim = int(config['lstm_dim'])
        self.kernel_size = int(config['num_filters'])
        self.win_size1 = int(config['window_size_left'])
        self.win_size2 = int(config['window_size_right'])
        self.dpool_size1 = int(config['dpool_size_left'])
        self.dpool_size2 = int(config['dpool_size_right'])
        self.hidden_size = int(config['hidden_size'])
        self.left_name, self.seq_len1 = config['left_slots'][0]
        self.right_name, self.seq_len2 = config['right_slots'][0]
        self.task_mode = config['training_mode']
        self.emb_layer = layers.EmbeddingEnhancedLayer(self.vocab_size, 
                            self.emb_size, zero_pad=True, scale=False)
        self.fw_cell = tf.nn.rnn_cell.LSTMCell(num_units=self.lstm_dim, state_is_tuple=True)
        self.bw_cell = tf.nn.rnn_cell.LSTMCell(num_units=self.lstm_dim, state_is_tuple=True)
        self.cnn_layer = layers.CNNDynamicPoolingLayer(self.seq_len1, self.seq_len2, 
                                                self.win_size1, self.win_size2, 
                                                self.dpool_size1, self.dpool_size2,
                                                self.kernel_size)
        self.relu_layer = layers.ReluLayer()
        self.tanh_layer = layers.TanhLayer()
        if 'match_mask' in config and config['match_mask'] != 0:
            self.match_mask = True
        else:
            self.match_mask = False
        self.fc1_layer = layers.FCLayer(self.kernel_size * self.dpool_size1 * self.dpool_size2, 
                                    self.hidden_size)
        if self.task_mode == "pointwise":
            self.n_class = int(config['n_class'])
            self.fc2_layer = layers.FCLayer(self.hidden_size, self.n_class)
        elif self.task_mode == "pairwise":
            self.fc2_layer = layers.FCLayer(self.hidden_size, 1)
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
        bi_left_outputs, _ = tf.nn.bidirectional_dynamic_rnn(self.fw_cell, self.bw_cell, 
                left_emb, sequence_length = utility.seq_length(left), dtype=tf.float32)
        left_seq_encoder = tf.concat(bi_left_outputs, -1)
        ## right
        bi_right_outputs, _ = tf.nn.bidirectional_dynamic_rnn(self.fw_cell, self.bw_cell, 
                right_emb, sequence_length = utility.seq_length(right), dtype=tf.float32) 
        right_seq_encoder = tf.concat(bi_right_outputs, -1)

        cross = tf.matmul(left_seq_encoder, tf.transpose(right_seq_encoder, [0, 2, 1])) # (N, len, len)
        if self.match_mask:
            cross_mask = utility.get_cross_mask(left, right)
        else:
            cross_mask = None
        conv_pool = self.cnn_layer.ops(cross, mask=cross_mask)
        pool_relu = self.relu_layer.ops(conv_pool)
        relu_hid1 = self.fc1_layer.ops(pool_relu)
        hid1_tanh = self.tanh_layer.ops(relu_hid1)
        pred = self.fc2_layer.ops(hid1_tanh)
        return pred

