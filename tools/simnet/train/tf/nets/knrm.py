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

import layers.tf_layers as layers


class KNRM(object):
    """
    k-nrm init funtion
    """
    def __init__(self, config):
        self.vocab_size = int(config['vocabulary_size'])
        self.emb_size = int(config['embedding_dim'])
        self.kernel_num = int(config['kernel_num'])
        self.left_name, self.seq_len1 = config['left_slots'][0]
        self.right_name, self.seq_len2 = config['right_slots'][0]
        self.lamb = float(config['lamb'])
        self.task_mode = config['training_mode']
        self.emb_layer = layers.EmbeddingLayer(self.vocab_size, self.emb_size)
        self.sim_mat_layer = layers.SimilarityMatrixLayer()
        self.kernel_pool_layer = layers.KernelPoolingLayer(self.kernel_num, self.lamb)
        self.tanh_layer = layers.TanhLayer()
        if self.task_mode == "pointwise":
            self.n_class = int(config['n_class'])
            self.fc_layer = layers.FCLayer(self.kernel_num, self.n_class)
        elif self.task_mode == "pairwise":
            self.fc_layer = layers.FCLayer(self.kernel_num, 1)
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
        sim_mat = self.sim_mat_layer.ops(left_emb, right_emb)
        feats = self.kernel_pool_layer.ops(sim_mat)
        pred = self.fc_layer.ops(feats)
        return pred
