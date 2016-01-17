#! /usr/bin/env bash
set -e
set -u

OBJDIR=../obj.shm
COORD=$OBJDIR/coordinator
SERVER=$OBJDIR/server


THREADS_FROM=1
THREADS_TO=16

# log base 2
KEYS_FROM=16
KEYS_TO=20

WRITEPERCENT=0
VALUESIZE=100

#PRECMD="numactl -C 10-23"
PRECMD="sudo chrt --rr 90"
ARGS="-M -t 5% -r 0"

for ((s=0;s<=1;s++)); do
    for ((k=KEYS_FROM;k<=KEYS_TO;k++)); do
        keys=$((1<<k))
        for ((t=THREADS_FROM;t<=THREADS_TO;t++)); do
            for ((iter=0;iter<3;iter++)); do
                ($COORD 2>&1) >/dev/null &
                sleep 2
                $PRECMD $SERVER $ARGS \
                    --threadCount $t \
                    --keyCount $keys \
                    --sharedKeys $s \
                    --writePercent $WRITEPERCENT \
                    --valueSize $VALUESIZE
                killall coordinator
                sleep 1
            done
        done
    done
done


