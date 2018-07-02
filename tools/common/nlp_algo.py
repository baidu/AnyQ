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

def forward_maximum_matching(
                            wordlist,
                            candidates,
                            wd_len=10,
                            loc=False,
                            ignore_unknown_word=True):
    """
    Maximum Forward Matching Algorithm
    Args: 
      wordlist: List of words or lists of chars
      candidates: Maximum forward matching candidate entity
      wd_len: Matching maximum window
      loc: Location information, True/False, if True return location
      ignore_unknown_word: Whether to ignore unknown words
    Returns:
      if loc=False, return [cand,]
      else return [(cand, startidx, endidx),]
    """
    idx = 0
    results = []
    while idx < len(wordlist):
        matched = False
        for i in xrange(min(wd_len, len(wordlist) - idx), 0, -1):
            cand = "".join(wordlist[idx: idx + i])
            if cand in candidates:
                if loc:
                    pair = (cand, idx, idx + i - 1)
                else:
                    pair = cand
                results.append(pair)
                matched = True
                idx += i
                break
        if not matched:
            if not ignore_unknown_word:
                cand = wordlist[idx]
                if loc:
                    pair = (cand, idx, idx)
                else:
                    pair = cand
                results.append(pair)
            idx += 1
    return results


def jaccard_similarity(str_a, str_b):
    """
    Calculate jaccard similarity
    Returns: jaccard score
    """
    set1 = set(str_a.decode('utf8'))
    set2 = set(str_b.decode('utf8'))
    if len(set1) == 0 or len(set2) == 0:
        return 0
    return len(set1 & set2) / float(len(set1 | set2))


def cosine_similarity(str_a, str_b):
    """
    Calculate cosine similarity
    Returns: cosine score
    """
    set1 = set(str_a.decode('utf8'))
    set2 = set(str_b.decode('utf8'))
    norm1 = len(set1) ** (1. / 2)
    norm2 = len(set2) ** (1. / 2)
    return len(set1 & set2) / (norm1 * norm2)


def word_weight_cos_sim(word_weight_a, word_weight_b):
    """
    Calculate cosine similarity with term weight
    Returns: cosine score
    """
    word_weight_dict_a = {}
    word_weight_dict_b = {}
    for word, weight in word_weight_a:
        if word not in word_weight_dict_a:
            word_weight_dict_a[word] = 0.0
        word_weight_dict_a[word] += weight

    for word, weight in word_weight_b:
        if word not in word_weight_dict_b:
            word_weight_dict_b[word] = 0.0
        word_weight_dict_b[word] += weight

    norm_a = 0.0001
    norm_b = 0.0001

    for weight in word_weight_dict_a.values():
        norm_a += weight ** 2
    norm_a = norm_a ** (1. / 2)

    for weight in word_weight_dict_b.values():
        norm_b += weight ** 2
    norm_b = norm_b ** (1. / 2)

    product = 0.0
    for k in set(word_weight_dict_a.keys()) & set(word_weight_dict_b.keys()):
        product += word_weight_dict_a[k] * word_weight_dict_b[k]
    return product / (norm_a * norm_b)


def character_similarity(str_a, str_b):
    """
    Calculate character similarity
    Returns: character similarity score
    """
    set1 = set(str_a.decode('utf8'))
    set2 = set(str_b.decode('utf8'))
    matching_degree = float(len(set1 & set2))
    return min(matching_degree / len(set1), matching_degree / len(set2))

