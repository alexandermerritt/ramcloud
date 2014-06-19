#! /usr/bin/env bash

# Modify this to your environment.
# Keep this script in same directory as the rc-* scripts.

set -e
set -u

# location of checked-out copy of sources
RAMCLOUD_ROOT=

# persistence replication factor
RAMCLOUD_REPLICATION=0

# if you enable persistence (-r N, N>0) maybe choose a place which isn't slow..
RAMCLOUD_STORAGE_PATH=/scratch/$(whoami)/ramcloud.bak

GIT_BRANCH=hplabs

# port to use; ibv_devinfo
BFC_IB_PORT=2

