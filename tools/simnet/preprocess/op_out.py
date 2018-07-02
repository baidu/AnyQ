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


class OperationOut(object):
    """
    data carrier of ops
    """
    def __init__(self):
        self._data_type = None
        self._qid = None
        self._query = None
        self._title = None
        self._label = None
        self._query_tokens = None
        self._title_tokens = None
        self._sep = None

    def set_pointwise_data(self, **kwargs):
        """
        query/title/label
        """
        self._sep = kwargs['src_sep']
        self._query = kwargs['query']
        self._title = kwargs['title']
        self._label = kwargs['label']
        self._data_type = 'pointwise'

    def set_pairwise_data(self, **kwargs):
        """
        qid/query/title/label
        """
        self._qid = kwargs['qid']
        self._sep = kwargs['src_sep']
        self._query = kwargs['query']
        self._title = kwargs['title']
        self._label = kwargs['label']
        self._data_type = 'pairwise'

    def get_texts(self):
        """
        Return the source query and title
        """
        return self._query, self._title

    def get_tokens(self):
        """
        Return wordseg results
        """
        if self._query_tokens is None:
            self._query_tokens = self._query.split(self._sep)
        if self._title_tokens is None:
            self._title_tokens = self._title.split(self._sep)

        if self._query_tokens is None or self._title_tokens is None:
            raise Exception(
                'There is not any token in workflow, please check your flow or seperator setting.')

        return self._query_tokens, self._title_tokens

    def get_infos(self):
        """
        Return all information
        """
        return {
                'qid': self._qid,
                'query': self._query,
                'title': self._title,
                'label': self._label,
                'query_token': self._query_tokens,
                'title_token': self._title_tokens,
                'data_type': self._data_type
                }

    def get_qid(self):
        """
        Return qid
        """
        return self._qid

    def set_tokens(self, query_token, title_token):
        """
        Store wordseg results
        """
        self._query_tokens = query_token
        self._title_tokens = title_token

# vim: set ts=4 sw=4 sts=4 tw=100:
