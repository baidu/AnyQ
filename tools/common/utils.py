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
import time
import json
import sys
import re
import os

"""
******functions for file processing******
"""
def load_dic(path): 
    """
    The format of word dictionary : each line is a word.
    Args: 
      Input path
    Returns: 
      load dict {k, v} = {term, id}
    """
    dic = {}
    with open(path) as f: 
        for id, line in enumerate(f): 
            w = line.strip()
            dic[w] = id
    return dic


def get_result_file(conf_dict, samples_file_path, predictions_file_path):
    """
    Get Result File
    Args: 
      conf_dict: Input path config
      samples_file_path: Data path of real training
      predictions_file_path: Prediction results path
    Returns:
      result_file: merge sample and predict result        

    """
    with open(conf_dict["result_file_path"], "w") as result_file:
        with open(samples_file_path) as samples_file:
            with open(predictions_file_path) as predictions_file:
                samples = [line.strip("\n").split("\t")
                           for line in samples_file]
                predictions = [line.strip("\n") for line in predictions_file]
                if len(samples) != len(predictions):
                    logging.error("sample num not equal predict num !")
                    return
                if conf_dict["task_mode"] == "pairwise":
                    for ind, prediction in enumerate(predictions):
                        result_file.write(
                            samples[ind][0] + "\t" + samples[ind][1] + "\t" + prediction + "\n")
                else:
                    for ind, prediction in enumerate(predictions):
                        result_file.write(samples[ind][2] + "\t" + prediction + "\n")
    os.remove(samples_file_path)
    os.remove(predictions_file_path)


"""
******functions for string processing******
"""
def parse_json(json_path):
    """
    parser JSON
    Args: 
      json_path: input json file path
    Returns: 
      json_dict: parser json dict result
    """
    try:
        with open(json_path) as json_file:
            json_dict = json.load(json_file)
    except Exception:
        logging.error("json file load error !")
    else:
        return json_dict


def pattern_match(pattern, line):
    """
    Check whether a string is matched
    Args: 
      pattern: mathing pattern
      line : input string
    Returns: 
      True/False
    """
    if re.match(pattern, line):
        return True
    else:
        return False


"""
******functions for parameter processing******
"""
def print_progress(task_name, percentage, style=0):
    """
    Print progress bar
    Args: 
      task_name: The name of the current task
      percentage: Current progress
      style: Progress bar form
    """
    styles = ['#', '█']
    mark = styles[style] * percentage
    mark += ' ' * (100 - percentage)
    status = '%d%%' % percentage if percentage < 100 else 'Finished'
    sys.stdout.write('%+20s [%s] %s\r' % (task_name, mark, status)) 
    sys.stdout.flush()
    time.sleep(0.002)


def display_args(name, args): 
    """
    Print parameter information
    Args: 
      name: logger instance name
      args: Input parameter dictionary
    """
    logger = logging.getLogger(name)
    logger.info("The arguments passed by command line is :")
    for k, v in sorted(v for v in vars(args).items()):
        logger.info("{}:\t{}".format(k, v))


def import_class(module_path, module_name, class_name):
    """
    Load class dynamically
    Args: 
      module_path: The current path of the module
      module_name: The module name
      class_name: The name of class in the import module
    Return: 
      Return the attribute value of the class object
    """
    if module_path:
        sys.path.append(module_path)
    module = __import__(module_name)
    return getattr(module, class_name)


