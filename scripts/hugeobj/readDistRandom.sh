#! /usr/bin/env bash
set -e
set -u
source $(cd "$(dirname $0)" && pwd)/env.sh

START_SIZE=$((1<<20))
STOP_SIZE=$((1<<21))
INCR=$((1<<17))
BIN=$RAMCLOUD_ROOT/obj.$GIT_BRANCH/ClusterPerf

[[ ! -e $BIN ]] && \
    echo "ClusterPerf does not exist. Compile sources." && exit 1

[[ ! -d $SCRIPT_OUTPUT_DIR ]] && mkdir $SCRIPT_OUTPUT_DIR

export RAMCLOUD_IB_PORT=$IB_PORT

for ((size=$START_SIZE; size<=$STOP_SIZE; size=size+$INCR))
do
    filename="${SCRIPT_OUTPUT_DIR}/${size}B"
    if [[ -e $filename ]]; then
        [[ $(wc -l < $filename) -eq 0 ]] && rm -f $filename
        [[ -e $filename ]] && continue
    fi
    echo -n "$size bytes "
    date
    cmd="$BIN \
        -C infrc:host=$RAMCLOUD_COORD_DNS,port=11100 \
        --testName readDistRandom \
        --warmup 0 \
        --count 1000 \
        --size $size \
        -l WARNING"
    echo $cmd
    ( $cmd | tr ',' '\n' ) > $filename
    sleep 1
done

