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

import itertools
import random
import sys
import os

sys.path.append('../../common')
import utils

def _default_custom_loader(term_dict_in):
    termdict = {}
    with open(term_dict_in) as f:
        for line in f:
            arr = line.strip().split('\t')
            termdict[arr[0]] = int(arr[1])
    return termdict


class BaseOperation(object):
    """
    Operating base unit
    """
    def __init__(self, **kwargs):
        self.task_name = 'Operation'
        self.prev_progressing_val = -1
        self.last_shot_start_pt = 0
        self.last_shot_end_pt = 0


    def operate(self, prev_op_out):
        """
        run
        """
        raise NotImplementedError("operate method must be implemented.")

    def progress(self, total_task, curr_task, limit=100):
        """
    
        """
        if limit != self.last_shot_end_pt:
            self.last_shot_start_pt = self.last_shot_end_pt
            self.last_shot_end_pt = limit
        scale = self.last_shot_end_pt - self.last_shot_start_pt
        # Calculate increment
        increase_amount = (curr_task + 1) * scale / total_task
        curr_prec = self.last_shot_start_pt + increase_amount
        if curr_prec - self.prev_progressing_val < 1:
            return
        utils.print_progress(self.task_name, curr_prec)
        self.prev_progressing_val = curr_prec
        if curr_prec == 100:
            print ''


class OpGenerateDict(BaseOperation):
    """
    Generate dictionary

    """
    def __init__(self, **kwargs):
        BaseOperation.__init__(self, **kwargs)
        self.task_name = 'Generate Dict'
        self.feature_name = kwargs.get('name')
        self.term_dict_in = kwargs.get('term_dict_in')
        self.output_dir = kwargs.get('output_dir')
        self.termdict = None

    def get_termdict(self):
        """
        return dictionary
        """
        return self.termdict

    def custom_loader(self):
        """
        WordTable loading method implemented by users
        """
        raise NotImplementedError("custom loader must be implemented before use.")

    def operate(self, prev_op_out):
        """
        run
        """
        if not self.output_dir or not self.feature_name:
            raise Exception("No output info specified.")

        termdict = {}
        if self.term_dict_in:
            try:
                termdict.update(self.custom_loader())
            except:
                termdict.update(_default_custom_loader(self.term_dict_in))

            sorted_dict = sorted(termdict.items(), key=lambda t:t[1])[:100000]
            self.progress(1, 1, 50)
        else:
            # count word frequency
            len_op = len(prev_op_out)
            for i in range(len_op):
                oo = prev_op_out[i]
                query_tok, title_tok = oo.get_tokens()
                for token in query_tok:
                    if len(token) == 0:
                        continue
                    if token not in termdict:
                        termdict[token] = 0
                    termdict[token] += 1
                for token in title_tok:
                    if len(token) == 0:
                        continue
                    if token not in termdict:
                        termdict[token] = 0
                    termdict[token] += 1
                self.progress(len_op, i, 50)
            termdict['[unk]'] = sys.maxint
            # Sorting by word frequency
            sorted_dict = sorted(termdict.items(), key=lambda t:t[1], reverse=True)[:100000]

        # write dictionary
        self.termdict = {}
        outdir = os.path.join(self.output_dir, self.feature_name)
        outf = open(os.path.join(outdir, 'term2id.dict'), 'w')
        len_dict = len(sorted_dict)
        for i, (word, cnt) in enumerate(sorted_dict):
            outf.write('%s\t%d\n' % (word, i))
            self.termdict[word] = i
            self.progress(len_dict, i)
        return prev_op_out


class OpTerm2ID(BaseOperation):
    """
    text convert to id
    """
    def __init__(self, **kwargs):
        BaseOperation.__init__(self, **kwargs)
        self.task_name = 'Convert Term to ID'
        self.term_dict_in = kwargs.get('term_dict_in')
        self.output_dir = kwargs.get('output_dir')
        self.get_termdict = kwargs.get('get_termdict')
        self.termdict = None

    def operate(self, prev_op_out):
        """
        run
        """
        if prev_op_out is None:
            return prev_op_out
        # If termdict is not generated by the previous step
        self.termdict = self.get_termdict()
        if self.termdict is None:
            # check if the user made the termdict path
            if not self.term_dict_in:
                raise Exception("cannot find term dict")
            # read custom termdict
            try:
                self.termdict = self.custom_loader()
            except NotImplementedError:
                self.termdict = _default_custom_loader(self.term_dict_in)
        op_out = []
        len_op = len(prev_op_out)
        for i in range(len_op):
            oo = prev_op_out[i]
            query_tok, title_tok = oo.get_tokens()
            query_tok = [str(self.termdict.get(t, 0)) for t in query_tok]
            title_tok = [str(self.termdict.get(t, 0)) for t in title_tok]
            oo.set_tokens(query_tok, title_tok)
            op_out.append(oo)
            self.progress(len_op, i)
        return op_out


class OpPartition(BaseOperation):
    """
    Sample slicing operation
    """
    def __init__(self, passive=False, **kwargs):
        BaseOperation.__init__(self, **kwargs)
        self.task_name = 'Sample Partition'
        self.model_type = kwargs['model_type']
        self.partition_ratio = kwargs.get('partition_ratio', '9:1')
        if passive:
            self.partition_ratio = '10:0'

    def _organize_samples(self, prev_op_out):
        """
        Merge data into rows
        """
        if self.model_type == 'pointwise':
            self.progress(1, 1, 50)
            return prev_op_out
        elif self.model_type == 'pairwise':
            pairs = {}
            len_op = len(prev_op_out)
            for i in range(len_op):
                oo = prev_op_out[i]
                qid = oo.get_qid()
                if qid not in pairs:
                    pairs[qid] = []
                pairs[qid].append(oo)
                self.progress(len_op, i, 50)
            return pairs.items()

    def operate(self, prev_op_out):
        """
        run
        """
        if prev_op_out is None:
            return prev_op_out

        samples = self._organize_samples(prev_op_out)
        [train, test] = self.partition_ratio.split(':')
        trainset, testset = [], []
        for i in range(10):
            trainset.append(list())
            testset.append(list())
        len_samples = len(samples)
        for i in range(len_samples):
            sample = samples[i]
            part = random.randint(0, 9)
            if random.randint(1, int(train) + int(test)) > int(test):
                trainset[part].append(sample)
            else:
                testset[part].append(sample)
            self.progress(len_samples, i)
        return (trainset, testset)


class OpWriteData(BaseOperation):
    """
    write data operation
    """
    def __init__(self, **kwargs):
        BaseOperation.__init__(self, **kwargs)
        self.task_name = 'Write Data'
        self.output_dir = kwargs['output_dir']
        self.feature_name = kwargs['name']
        self.model_type = kwargs['model_type']

    def _get_pairwise_lines(self, dataset, setname):
        """
        pairwise data generator
        """
        for (qid, oo_list) in dataset: 
            if setname == 'train': 
                pos_titles = []
                neg_titles = []
                for oo in oo_list:
                    infos = oo.get_infos()
                    if int(infos['label']) > 0:
                        pos_titles.append(' '.join(infos['title_token']))
                    else:
                        neg_titles.append(' '.join(infos['title_token']))
                if len(pos_titles) * len(neg_titles) <= 0:
                    continue
                query = ''
                if len(oo_list) > 0:
                    infos = oo_list[0].get_infos()
                    query = ' '.join(infos['query_token'])
                pos_neg_pair = itertools.product(pos_titles, neg_titles)
                for pair in pos_neg_pair: 
                    line = '%s\t%s\t%s\n' % (query, pair[0], pair[1])
                    yield line
            elif setname == 'test': 
                for oo in oo_list: 
                    infos = oo.get_infos()
                    line = "%s\t%s\t%s\t%s\n" % (infos['qid'], infos['label'], \
                            ' '.join(infos['query_token']), ' '.join(infos['title_token']))
                    yield line

    def _get_pointwise_lines(self, dataset):
        """
        pointwise data generator
        """
        for oo in dataset:
            infos = oo.get_infos()
            query_tok = infos['query_token']
            title_tok = infos['title_token']
            label = infos['label']
            line = '%s\t%s\t%s\n' % \
                (' '.join(query_tok), ' '.join(title_tok), label)
            yield line

    def write_data(self, dataset, outdir, setname, pro):
        """
        write data
        """
        outlist = open(os.path.join(outdir, '%s.list' % setname), 'w')
        len_set = len(dataset)
        for i, subset in enumerate(dataset):
            outf = open(os.path.join(outdir, '%s/part-000%02d' % (setname, i)), 'w')
            if self.model_type == 'pairwise':
                for line in self._get_pairwise_lines(subset, setname): 
                    out_file = os.path.join(outdir, '%s/part-000%02d' % (setname, i))
                    outf.write(line)
            elif self.model_type == "pointwise":
                for line in self._get_pointwise_lines(subset):
                    outf.write(line)

            abs_path = os.path.abspath(outdir)
            outlist.write('%s/%s/part-000%02d\n' % (abs_path, setname, i))
            outf.close()
            self.progress(len_set, i, pro)
        outlist.close()

    def operate(self, prev_op_out):
        """
        run
        """
        if prev_op_out is None:
            return prev_op_out
        (trainset, testset) = prev_op_out
        outdir = os.path.join(self.output_dir, self.feature_name)
        if not os.path.exists(os.path.join(outdir, 'train')):
            os.makedirs(os.path.join(outdir, 'train'))
        if not os.path.exists(os.path.join(outdir, 'test')):
            os.makedirs(os.path.join(outdir, 'test'))
        self.write_data(trainset, outdir, 'train', 50)
        self.write_data(testset, outdir, 'test', 100)
        return True

# vim: set ts=4 sw=4 sts=4 tw=100:
