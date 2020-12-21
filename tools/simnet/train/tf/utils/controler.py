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

import sys
import time

import tensorflow as tf


def run_predict(pred, label, config):
    """
    run classification predict function handle
    """
    mean_acc = 0.0
    saver = tf.train.Saver()
    mode = config["training_mode"]
    label_index = tf.argmax(label, 1)
    if mode == "pointwise":
        pred_prob = tf.nn.softmax(pred, -1)
        score = tf.reduce_max(pred_prob, -1)
        pred_index = tf.argmax(pred_prob, 1)
        correct_pred = tf.equal(pred_index, label_index)
        acc = tf.reduce_mean(tf.cast(correct_pred, "float"))
    elif mode == "pairwise":
        score = pred
        pred_index = tf.argmax(pred, 1)
        acc = tf.constant([0.0])
    modelfile = config["test_model_file"]
    result_file = file(config["test_result"], "w")
    step = 0
    init = tf.group(tf.global_variables_initializer(),
                    tf.local_variables_initializer())
    with tf.Session(config=tf.ConfigProto(intra_op_parallelism_threads=1)) \
                    as sess:
        sess.run(init)
        saver.restore(sess, modelfile)
        coord = tf.train.Coordinator()
        read_thread = tf.train.start_queue_runners(sess=sess, coord=coord)
        while not coord.should_stop():
            step += 1
            try:
                ground, pi, a, prob = sess.run([label_index, pred_index, acc, score])
                mean_acc += a
                for i in range(len(prob)):
                    result_file.write("%d\t%d\t%f\n" % (ground[i], pi[i], prob[i]))
            except tf.errors.OutOfRangeError:
                coord.request_stop()
        coord.join(read_thread)
    sess.close()
    result_file.close()
    if mode == "pointwise":
        mean_acc = mean_acc / step*1.0
        print >> sys.stderr, "accuracy: %4.2f" % (mean_acc * 100)


def run_trainer(loss, optimizer, config):
    """
    run classification training function handle
    """
    thread_num = int(config["thread_num"])
    model_path = config["model_path"]
    model_file = config["model_prefix"]
    print_iter = int(config["print_iter"])
    data_size = int(config["data_size"])
    batch_size = int(config["batch_size"])
    epoch_iter = int(data_size / batch_size)
    avg_cost = 0.0
    saver = tf.train.Saver(max_to_keep=None)
    init = tf.group(tf.global_variables_initializer(),
                    tf.local_variables_initializer())
    with tf.Session(config=tf.ConfigProto(intra_op_parallelism_threads=thread_num, 
                                          inter_op_parallelism_threads=thread_num)) \
                    as sess:
        sess.run(init)
        coord = tf.train.Coordinator()
        read_thread = tf.train.start_queue_runners(sess=sess, coord=coord)
        step = 0
        epoch_num = 1
        start_time = time.time()
        while not coord.should_stop():
            try:
                step += 1
                c, _= sess.run([loss, optimizer])
                avg_cost += c

                if step % print_iter == 0:
                    print("loss: %f" % ((avg_cost / print_iter)))
                    avg_cost = 0.0
                if step % epoch_iter == 0:
                    end_time = time.time()
                    print("save model epoch%d, used time: %d" % (epoch_num, 
                          end_time - start_time))
                    save_path = saver.save(sess, 
                            "%s/%s.epoch%d" % (model_path, model_file, epoch_num))
                    epoch_num += 1
                    start_time = time.time()
                    
            except tf.errors.OutOfRangeError:
                save_path = saver.save(sess, "%s/%s.final" % (model_path, model_file))
                coord.request_stop()
        coord.join(read_thread)
    sess.close()


def graph_save(pred, config):
    """
    run classify predict
    """
    graph_path=config["graph_path"]
    graph_name=config["graph_name"]
    mode = config["training_mode"]
    if mode == "pointwise":
        pred_prob = tf.nn.softmax(pred, -1, name="output_prob")
    elif mode == "pairwise":
        pred_prob = tf.identity(pred, name="output_prob")
    saver = tf.train.Saver()
    step = 0
    init = tf.group(tf.global_variables_initializer(),
                    tf.local_variables_initializer())
    with tf.Session(config=tf.ConfigProto(intra_op_parallelism_threads=1)) \
                    as sess:
        sess.run(init)
        tf.train.write_graph(sess.graph_def, graph_path, graph_name, as_text=True)
    sess.close()

