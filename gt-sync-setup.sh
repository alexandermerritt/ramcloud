#! /usr/bin/env bash
# Create a new remote that connects to stanford
set -u

# remote stanford exists?
git remote | grep -q stanford
[[ $? -eq 1 ]] && \
    git remote add stanford git://fiz.stanford.edu/git/ramcloud.git

git fetch --all

echo ""
echo " Overview of how to maintain the branches:"
echo ""
echo "    stanford/master -> master    Mirror of Stanford sources"
echo "        master -> gtmaster       Patched to run on shiva/ifrit/++"
echo "            gtmaster -> hugeobj  Alex's code"
echo "            gtmaster -> nvcloud  Jian's code"
echo ""
