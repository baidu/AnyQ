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
import time
import sys
import os

import tensorflow as tf


def int_feature(v):
    """
    int feature
    """
    return tf.train.Feature(int64_list=tf.train.Int64List(value=v))


def write_data_to_tf(filename, func, func_args, writer):
    """
    writes tf records, write data
    """
    with open(filename) as fin_data:
        for line in fin_data:
            example = func(line, func_args)
            if not example:
                continue
            writer.write(example.SerializeToString())


def parse_text_match_pointwise_pad_data(line, func_args):
    """
    pointwise parse
    """
    seq_len = func_args[0]
    pad_id = func_args[1]
    # left_ids \t right_ids \t label
    group = line.strip().split("\t")
    if len(group) != 3:
        logging.warning(
            "the line not conform to format(left_ids, right_ids, label)")
        return
    label = [0, 0]
    all_ids = []
    for i in [0, 1]:
        tmp_ids = [int(t) for t in group[i].strip().split(" ")]
        if len(tmp_ids) < seq_len:
            pad_len = seq_len - len(tmp_ids)
            tmp_ids = tmp_ids + [pad_id] * pad_len
        all_ids.append(tmp_ids[:seq_len])
    label[int(group[2])] = 1
    example = tf.train.Example(features=tf.train.Features(
        feature={"label": int_feature(label),
                 "left": int_feature(all_ids[0]),
                 "right": int_feature(all_ids[1])}))
    return example


def parse_text_match_pairwise_pad_data(line, func_args):
    """
    pairwise parse
    """
    seq_len = func_args[0]
    pad_id = func_args[1]
    # query_terms\t postitle_terms\t negtitle_terms
    group = line.strip().split("\t")
    if len(group) != 3:
        logging.warning(
            "the line not conform to format(query_terms, postitle_terms, negtitle_terms)")
        return
    all_ids = []
    for i in [0, 1, 2]:
        tmp_ids = [int(t) for t in group[i].strip().split(" ")]
        if len(tmp_ids) < seq_len:
            pad_len = seq_len - len(tmp_ids)
            tmp_ids = tmp_ids + [pad_id] * pad_len
        all_ids.append(tmp_ids[:seq_len])
    example = tf.train.Example(features=tf.train.Features(
        feature={"left": int_feature(all_ids[0]),
                 "pos_right": int_feature(all_ids[1]),
                 "neg_right": int_feature(all_ids[2])}))
    return example


def usage():
    """
    usage
    """
    print sys.argv[0], "options"
    print "options"
    print "\ttype: data type include pointwise or pairwise"
    print "\tinputfile: input file path"
    print "\trecordfile: output recorf file"
    print "\tpad_id: pad id"
    print "\tmax_len: sequence max length"


if __name__ == "__main__":
    if len(sys.argv) != 6:
        usage()
        sys.exit(-1)
    input_data_format = sys.argv[1]
    filename = sys.argv[2]
    tfrecord_name = sys.argv[3]
    pad_id = int(sys.argv[4])
    max_len = int(sys.argv[5])
    data_format_func = {"pointwise": parse_text_match_pointwise_pad_data,
                        "pairwise": parse_text_match_pairwise_pad_data}
    if input_data_format in data_format_func:
        using_func = data_format_func[input_data_format]
    else:
        logging.error("data_format not supported")
        sys.exit(1)
    local_writer = tf.python_io.TFRecordWriter(tfrecord_name)
    write_data_to_tf(filename, using_func, [max_len, pad_id], local_writer)
    local_writer.close()
