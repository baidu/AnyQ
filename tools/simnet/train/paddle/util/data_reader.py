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
import sys
import os

_WORK_DIR = os.path.split(os.path.realpath(__file__))[0]
sys.path.append(os.path.join(_WORK_DIR, '../../../../common'))
import utils


def get_reader(conf_dict, is_infer, samples_file):
    """
    Get Reader
    """
    def reader_with_pairwise():
        """
            Reader with Pairwise
        """
        if is_infer:
            with open(conf_dict["test_file_path"]) as file:
                for line in file:
                    if not utils.pattern_match(r"(\d+)\t(\d+)\t((\d+ )*\d+)\t((\d+ )*\d+)\n", line):
                        logging.warning("line not match format in test file")
                        continue
                    items = line.strip("\n").split("\t")
                    query = [int(id) for id in items[2].split(" ")]
                    title = [int(id) for id in items[3].split(" ")]
                    if samples_file: 
                        samples_file.write(line)
                    yield [query, title]
        else:
            with open(conf_dict["train_file_path"]) as file:
                for line in file:
                    if not utils.pattern_match(r"((\d+ )*\d+)\t((\d+ )*\d+)\t((\d+ )*\d+)\n", line):
                        logging.warning("line not match format in train file")
                        continue
                    items = line.strip("\n").split("\t")
                    query = [int(id) for id in items[0].split(" ")]
                    pos_title = [int(id) for id in items[1].split(" ")]
                    neg_title = [int(id) for id in items[2].split(" ")]
                    if samples_file: 
                        samples_file.write(line)
                    yield [query, pos_title, neg_title]

    def reader_with_pointwise():
        """
        Reader with Pointwise
        """
        if is_infer:
            with open(conf_dict["test_file_path"]) as file:
                for line in file:
                    if not utils.pattern_match(r"((\d+ )*\d+)\t((\d+ )*\d+)\t(\d+)\n", line):
                        logging.warning("line not match format in test file")
                        continue
                    items = line.strip("\n").split("\t")
                    query = [int(id) for id in items[0].split(" ")]
                    title = [int(id) for id in items[1].split(" ")]
                    if samples_file:
                        samples_file.write(line)
                    yield [query, title]
        else:
            with open(conf_dict["train_file_path"]) as file:
                for line in file: 
                    if not utils.pattern_match(r"((\d+ )*\d+)\t((\d+ )*\d+)\t(\d+)\n", line):
                        logging.warning("line not match format in train file: %s" % line)
                        continue
                    items = line.strip("\n").split("\t")
                    query = [int(id) for id in items[0].split(" ")]
                    title = [int(id) for id in items[1].split(" ")]
                    label = int(items[2])
                    if samples_file:
                        samples_file.write(line)
                    yield [query, title, label]

    if conf_dict["task_mode"] == "pairwise":
        return reader_with_pairwise
    else:
        return reader_with_pointwise
