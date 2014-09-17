#! /usr/bin/env bash
set -e
set -u
source $(cd "$(dirname $0)" && pwd)/env.sh

BIN=$RAMCLOUD_ROOT/obj.$GIT_BRANCH/ClusterPerf

[[ ! -e $BIN ]] && \
    echo "ClusterPerf does not exist. Compile sources." && exit 1

[[ $# -ne 2 ]] && \
    echo "Usage: $0 numClients objSizeB" && exit 1

export RAMCLOUD_IB_PORT=$BFC_IB_PORT

numClients=$1
objSize=$2

runMaster()
{
    echo Starting master
    $BIN \
        -C infrc:host=221-1,port=11100 \
        --testName readLoaded \
        --numClients $numClients \
        --clientIndex 0 \
        --warmup 0 \
        --count 1 \
        --size $objSize \
        -l NOTICE
}

__runClient()
{
    idx=$1
    echo Starting client $idx
    $BIN \
        -C infrc:host=221-1,port=11100 \
        --testName readLoaded \
        --clientIndex $idx \
        --warmup 0 \
        --count 1 \
        --size $objSize \
        -l NOTICE
}

runClients()
{
    for ((idx=1; idx<=numClients; idx++)); do
        (__runClient $idx) &
    done
    wait
}

(runClients) &
sleep 1
(runMaster) &
wait

