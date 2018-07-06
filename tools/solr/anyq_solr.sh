#!/bin/bash

if [ $# -lt 1 ];then
    echo "need faq file"
    exit 2
fi
SCRIPT_PATH=$(dirname $0)
FAQ_FILE=$1
# faq 
#   file -> json
#   file -> schema
mkdir -p faq
SCHEMA_FILE=`pwd`/faq/schema_format
FAQ_JSON=`pwd`/faq/faq_json
python ${SCRIPT_PATH}/make_json.py $FAQ_FILE $SCHEMA_FILE $FAQ_JSON
if [ $? -ne 0 ];then
    echo "faq-file trans error"
    exit 1
else
    echo "faq-file trans done"
fi

# set solr dir
SOLR_HOME=./solr-4.10.3-anyq
SOLR_SERVER=${SCRIPT_PATH}/solr_deply.sh
SOLR_API=${SCRIPT_PATH}/solr_api.py
ENGINE_HOST=localhost
ENGINE_NAME=collection1
SOLR_PORT=8900

#set emp schema
SOLR_EMP_CONF=$SOLR_HOME/example/solr_config_set/common
SOLR_CONF=$SOLR_HOME/example/solr/collection1/conf
cp $SOLR_EMP_CONF/* $SOLR_CONF/

#set paddle environment variable
export MKL_NUM_THREADS=1
export OMP_NUM_THREADS=1

#start
/bin/bash ${SOLR_SERVER} start ${SOLR_HOME} ${SOLR_PORT}

# set schema
python ${SOLR_API} set_schema ${ENGINE_HOST} ${ENGINE_NAME} ${SCHEMA_FILE} ${SOLR_PORT}

# clear docs
python ${SOLR_API} clear_doc ${ENGINE_HOST} ${ENGINE_NAME} ${SOLR_PORT}

# upload docs
python ${SOLR_API} upload_doc ${ENGINE_HOST} ${ENGINE_NAME} ${SOLR_PORT} ${FAQ_JSON}

if [ $? -ne 0 ];then
    echo "upload file error"
    exit 1
else
    echo "upload file success"
fi
