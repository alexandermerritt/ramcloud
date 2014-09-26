#! /usr/bin/env bash
set -e
set -u
[[ $(whoami) != "root" ]] && \
    echo "Run as root" && exit 1
make -C ../.. stopZoo
sleep 1
rm -rf /var/lib/zookeeper/version-2
make -C ../.. startZoo

