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

import string
import random
import math
import sys


def tied_rank(x):
    """
    Computes the tied rank of elements in x.
    This function computes the tied rank of elements in x.
    Args: 
      x : list of numbers, numpy array
    Returns: 
      score : list of numbers
            The tied rank f each element in x
    """
    sorted_x = sorted(zip(x, range(len(x))))
    r = [0 for k in x]
    cur_val = sorted_x[0][0]
    last_rank = 0
    for i in range(len(sorted_x)):
        if cur_val != sorted_x[i][0]:
            cur_val = sorted_x[i][0]
            for j in range(last_rank, i): 
                r[sorted_x[j][1]] = float(last_rank + 1 + i) / 2.0
            last_rank = i
        if i == len(sorted_x)-1:
            for j in range(last_rank, i + 1): 
                r[sorted_x[j][1]] = float(last_rank + i + 2) / 2.0
    return r


def auc(actual, posterior):
    """
    auc
    """
    r = tied_rank(posterior)
    num_positive = len([0 for x in actual if x == 1])
    num_negative = len(actual)-num_positive
    sum_positive = sum([r[i] for i in range(len(r)) if actual[i] == 1])
    auc = ((sum_positive - num_positive * (num_positive + 1) / 2.0) /
           (num_negative * num_positive))
    return auc

if __name__ == '__main__':
    actual = []
    poster = []
    right = 0
    all = 0
    for line in sys.stdin:
        items = [it for it in line.strip().split('\t')]
        label = int(items[1])
        neg_score = float(items[2])
        pos_score = float(items[3])
        actual.append(label)
        poster.append(pos_score)
        if pos_score > neg_score:
            pred = 1
        else:
            pred = 0
        if pred == label:
            right += 1
        all += 1
    print "AUC\t" + str(auc(actual, poster))
    print "acc\t%.4f" % (1.0 * right / all)
    
