#!/bin/bash

#set -e # set -o errexit
set -u # set -o nounset
set -o pipefail

readonly SOLR_STOP_KEY="secret_key"
readonly START="start"
readonly STOP="stop"

function help() {
    echo "usage: sh solr_deply.sh start solr_home solr_port"
    echo "       sh solr_deply.sh stop solr_home solr_port"
}

# start solr service
function solr_start() {
    if [[ $# -ne 2 ]]; then
        echo "usage: solr_start solr_home solr_port"
        return 1
    fi
    local solr_home=$1
    local solr_port=$2
    # start solr
    nohup java \
        -DSTOP.PORT=$((solr_port+1)) \
        -DSTOP.KEY=$SOLR_STOP_KEY \
        -Djetty.port=$solr_port \
        -Dsolr.solr.home=${solr_home}/example/solr/ \
        -Djetty.home=${solr_home}/example/ \
        -jar ${solr_home}/example/start.jar &

    # check solr service
    sleep 20s #time needed to start solr, maybe longer
    curl "http://localhost:$solr_port/solr"
    if [[ $? -ne 0 ]]; then
        echo "solr[$solr_port] start failed!"
        res=`solr_stop $solr_home $solr_port`
        return 1
    fi
    echo "solr[$solr_port] start success!"
    return 0
}

# stop solr service
function solr_stop() {
    if [[ $# -ne 2 ]]; then
        echo "usage: solr_stop solr_home sole_port"
        return 1
    fi
    local solr_home=$1
    local solr_port=$2
    java \
        -DSTOP.PORT=$((solr_port+1)) \
        -DSTOP.KEY=$SOLR_STOP_KEY \
        -jar ${solr_home}/example/start.jar --stop
    if [[ $? -ne 0 ]]; then
        echo "solr[$solr_port] stop fail!"
        return 1
    fi
    echo "solr[$solr_port] stop success!"

    return 0
}

# main
function main() {
    if [[ $# -ne 3 ]]; then
        help
        return 1
    fi

    if [[ $1 == ${START} ]]; then
        solr_start $2 $3
    elif [[ $1 == ${STOP} ]]; then
        solr_stop $2 $3
    else
        help
        return 1
    fi
    return 0
}

main "$@"
