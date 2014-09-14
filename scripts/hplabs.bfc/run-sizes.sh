#! /usr/bin/env bash
set -e
set -u
source $(cd "$(dirname $0)" && pwd)/bfc-env.sh

MIN_POW2=17
MAX_POW2=21
BIN=$RAMCLOUD_ROOT/obj.$GIT_BRANCH/ClusterPerf

[[ ! -e $BIN ]] && \
    echo "ClusterPerf does not exist. Compile sources." && exit 1

export RAMCLOUD_IB_PORT=$BFC_IB_PORT

for ((pow=MIN_POW2; pow<=MAX_POW2; pow++))
do
    size=$((1 << pow))
    filename="${size}B"
    if [[ -e $filename ]]; then
        [[ $(wc -l < $filename) -eq 0 ]] && rm -f $filename
        [[ -e $filename ]] && continue
    fi
    echo $size bytes
    ( $BIN \
        -C infrc:host=221-1,port=11100 \
        --testName readDist \
        --warmup 0 \
        --count 1000 \
        --size $size \
        -l ERROR \
        | tr ',' '\n' ) > $filename
    sleep 1
done

