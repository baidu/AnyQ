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

import glob
import os

import operation_unit
import op_out

class WorkFlow(object):
    """
    work flow
    """
    def __init__(self, **configs):
        """
        Initialize instance
        """
        self.feature_name = configs.get('name')
        self.flow = configs.get('flow')
        self.model_type = configs.get('model_type')
        self.src_data = configs.get('data')
        self.platform = configs.get('platform').split(',')
        self.output_dir = configs.get('output_dir')
        self.src_sep = configs.get('src_data_seg_sep')
        self.ops = None

    def __rglob(self, path, filelist):
        """
        Recursively search files, support matching
        """
        for fn in glob.glob(path):
            if os.path.isdir(fn):
                for f in os.listdir(fn):
                    self.__rglob(os.path.join(fn, f), filelist)
            else:
                filelist.append(fn)

    def read(self):
        """
        Load data
        """
        filelist = []
        self.__rglob(self.src_data, filelist)
        if self.model_type == 'pointwise':
            dataset = self.read_pointwise(filelist)
        else:
            dataset = self.read_pairwise(filelist)
        return dataset

    def read_pointwise(self, filelist):
        """
        Load pointwise data
        """
        dataset = []
        for file in filelist:
            with open(file) as f:
                for line in f:
                    tpl = line.rstrip('\n').split('\t')
                    if 0 in map(lambda t:len(t), tpl):
                        continue
                    oo = op_out.OperationOut()
                    oo.set_pointwise_data(label=tpl[0], query=tpl[1], \
                            title=tpl[2], src_sep=self.src_sep)
                    dataset.append(oo)
        return dataset

    def read_pairwise(self, filelist):
        """
        Load pairwise data
        """
        inputdata = []
        for file in filelist:
            with open(file) as f:
                for line in f:
                    tpl = line.rstrip('\n').split('\t')
                    if 0 in map(lambda t:len(t), tpl):
                        continue
                    inputdata.append(tpl)
        dataset = []
        qid = 0
        prev_query = ''
        sorted_data = sorted(inputdata, key=lambda t:t[1])
        for (label, query, title) in sorted_data:
            if query != prev_query:
                qid += 1
                prev_query = query
            oo = op_out.OperationOut()
            oo.set_pairwise_data(qid=qid, label=label, query=query, \
                    title=title, src_sep=self.src_sep)
            dataset.append(oo)
        return dataset

    def check_topological(self):
        """
        Check the workflow topology
        """
        standard = [
            operation_unit.OpGenerateDict,
            operation_unit.OpTerm2ID,
            operation_unit.OpPartition,
            operation_unit.OpWriteData
            ]
        control = [type(op) for op in self.ops]
        intersec = list(set(standard) & set(control))
        intersec.sort(key=standard.index)
        for i, op in enumerate(self.ops):
            if not isinstance(op, intersec[i]):
                raise Exception("Wrong workflow topology, please check.")

    def initialize(self, **configs):
        """
        Initialize work flow
        """
        self.ops = []
        get_termdict_method = None
        for op in self.flow.split(','):
            if op == 'gendict':
                op_generate_dict = operation_unit.OpGenerateDict(**configs)
                self.ops.append(op_generate_dict)
                get_termdict_method = op_generate_dict.get_termdict
            elif op == 'convertid':
                conf = dict(**configs)
                conf['get_termdict'] = get_termdict_method
                op_term2id = operation_unit.OpTerm2ID(**conf)
                self.ops.append(op_term2id)
            elif op == 'partition':
                op_partition = operation_unit.OpPartition(**configs)
                self.ops.append(op_partition)
            elif op == 'write':
                if operation_unit.OpPartition not in map(type, self.ops):
                    op_partition = \
                    operation_unit.OpPartition(passive=True, **configs)
                    self.ops.append(op_partition)
                op_write = operation_unit.OpWriteData(**configs)
                self.ops.append(op_write)

        self.check_topological()

        if 'write' in self.flow or 'gendict' in self.flow:
            outdir = os.path.join(self.output_dir, self.feature_name)
            if not os.path.exists(outdir):
                os.makedirs(outdir)

    def start_flow(self):
        """
        start workflow
        """
        prev_op_out = self.read()
        for op in self.ops:
            prev_op_out = op.operate(prev_op_out)
        return prev_op_out

# vim: set ts=4 sw=4 sts=4 tw=100:
