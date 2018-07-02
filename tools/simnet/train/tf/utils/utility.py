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

import traceback
import sys
import os

import tensorflow as tf


def get_all_files(train_data_file):
    """
    get all files
    """
    train_file = []
    train_path = train_data_file
    if os.path.isdir(train_path):
        data_parts = os.listdir(train_path)
        for part in data_parts:
            train_file.append(os.path.join(train_path, part))
    else:
        train_file.append(train_path)
    return train_file


def merge_config(config, *argv):
    """
    merge multiple configs
    """
    cf = {}
    cf.update(config)
    for d in argv:
        cf.update(d)
    return cf


def import_object(module_py, class_str):
    """
    string to class
    """
    mpath, mfile = os.path.split(module_py)
    sys.path.append(mpath)
    module=__import__(mfile)
    try:
        return getattr(module, class_str)
    except AttributeError:
        raise ImportError('Class %s cannot be found (%s)' %
                (class_str, traceback.format_exception(*sys.exc_info())))


def seq_length(sequence):
    """
    get sequence length
    for id-sequence, (N, S)
        or vector-sequence  (N, S, D)
    """
    if len(sequence.get_shape().as_list()) == 2:
        used = tf.sign(tf.abs(sequence))
    else:
        used = tf.sign(tf.reduce_max(tf.abs(sequence), 2))
    length = tf.reduce_sum(used, 1)
    length = tf.cast(length, tf.int32)
    return length


def get_cross_mask(seq1, seq2):
    """
    get matching matrix mask, for two sequences( id-sequences or vector-sequences)
    """
    length1 = seq_length(seq1)
    length2 = seq_length(seq2)
    max_len1 = tf.shape(seq1)[1]
    max_len2 = tf.shape(seq2)[1]
    ##for padding left
    mask1 = tf.sequence_mask(length1, max_len1, dtype=tf.int32)
    mask2 = tf.sequence_mask(length2, max_len2, dtype=tf.int32)
    cross_mask = tf.einsum('ij,ik->ijk', mask1, mask2)
    return cross_mask
