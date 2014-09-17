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
BFC_IB_PORT=2

#
# Parameters scripts sourcing this file will use
#

# location of checked-out copy of sources
RAMCLOUD_ROOT=/home/merrital/src/ramcloud.git

# if you enable persistence (-r N, N>0) maybe choose a place which isn't slow..
RAMCLOUD_STORAGE_PATH=/scratch/$(whoami)/ramcloud.bak

GIT_BRANCH=hplabs

#
# Environment variable hacks the code will read
#

# timeout in us; the code too eagerly aborts and retries RPCs
export RAMCLOUD_ABORT_TIMEOUT=100000

