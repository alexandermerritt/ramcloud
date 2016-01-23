#! /usr/bin/env bash
set -e
set -u

OBJDIR=../obj.shm
COORD=$OBJDIR/coordinator
SERVER=$OBJDIR/server

# values are log2
THREADS_FROM=0
THREADS_TO=0
# more memory means more hash table buckets in RC
TOTAL_MEMORY=35
KEYS=27
HT_MEMORY=27

# normal values
WRITEPERCENT=0
VALUE_SIZE=100

#PRECMD="sudo chrt --rr 90 numactl --all --interleave=0-15"
PRECMD="sudo chrt --rr 90 numactl --all"
echo "COMMAND PREFIX $PRECMD"

# RC expects memory specified in MiB, so shift
ARGS="-M -t $((1<<(TOTAL_MEMORY-20))) -h $((1<<(HT_MEMORY-20))) -r 0"
echo "ARGS $ARGS"

ULIMIT=unlimited
ulimit -l $ULIMIT
echo "ULIMIT -l $ULIMIT"

keys=$((1<<KEYS))
for ((t=THREADS_FROM;t<=THREADS_TO;t++)); do
    threads=$((1<<t))
    nohup $COORD &
    sleep 2
    $PRECMD $SERVER $ARGS \
        --threadCount $threads \
        --keyCount $keys \
        --sharedKeys 1 \
        --writePercent $WRITEPERCENT \
        --valueSize $VALUE_SIZE
    killall coordinator
    sleep 1
done

