#! /usr/bin/env bash
set -e
set -u
source $(cd "$(dirname $0)" && pwd)/env.sh

MIN_POW2=0
MAX_POW2=23
BIN=$RAMCLOUD_ROOT/obj.$GIT_BRANCH/ClusterPerf

[[ ! -e $BIN ]] && \
    echo "ClusterPerf does not exist. Compile sources." && exit 1

[[ ! -d $SCRIPT_OUTPUT_DIR ]] && mkdir $SCRIPT_OUTPUT_DIR

export RAMCLOUD_IB_PORT=$IB_PORT

for ((pow=MIN_POW2; pow<=MAX_POW2; pow++))
do
    size=$((1 << pow))
    filename="${SCRIPT_OUTPUT_DIR}/${size}B"
    if [[ -e $filename ]]; then
        [[ $(wc -l < $filename) -eq 0 ]] && rm -f $filename
        [[ -e $filename ]] && continue
    fi
    echo -n "$size bytes "
    date
    ( $BIN \
        -C infrc:host=$RAMCLOUD_COORD_DNS,port=11100 \
        --testName readDistRandom \
        --warmup 0 \
        --count 10 \
        --size $size \
        -l ERROR \
        | tr ',' '\n' ) > $filename
    sleep 1
done

