#! /usr/bin/env bash
# Modify this to your environment.
# Keep this script in same directory as the rc-* scripts.

set -e
set -u

#
# Parameters to the program(s)
#

RAMCLOUD_REPLICATION=0
RAMCLOUD_SEGMENT_FRAMES=64
RAMCLOUD_TOTAL_MEM=2048

# port to use; ibv_devinfo
IB_PORT=1

# zookeeper
ZKSERVER=ifrit

# memory
MASTER_MEM=2048
SEG_FRAMES=64

#
# Parameters scripts sourcing this file will use
#

RAMCLOUD_COORD_DNS=ifrit.cc.gt.atl.ga.us

SCRIPT_OUTPUT_DIR=$(cd "$(dirname $0)" && pwd)/data
[[ ! -d $SCRIPT_OUTPUT_DIR ]] && \
    mkdir -p $SCRIPT_OUTPUT_DIR

# location of checked-out copy of sources
RAMCLOUD_ROOT=/opt/share/users/alex/ramcloud.git

# if you enable persistence (-r N, N>0) maybe choose a place which isn't slow..
# assuming /run is tmpfs-mounted
#RAMCLOUD_STORAGE_PATH=/run/ramcloud/ramcloud.bak
RAMCLOUD_STORAGE_PATH=/tmp/ramcloud.bak

GIT_BRANCH=hugeobj

#
# Environment variable hacks the code will read
#

# timeout in us; the code too eagerly aborts and retries RPCs
export RAMCLOUD_ABORT_TIMEOUT=100000

