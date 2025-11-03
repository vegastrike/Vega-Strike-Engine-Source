#!/bin/sh
# export CPLUS_INCLUDE_PATH="/home/pmx/Documents/Dev/VegaStrike/private_rootdir"
ninja -C ./build/linux-ninja-pie-disabled-glvnd-debug -j 10 ${1} ; cp ./build/linux-ninja-pie-disabled-glvnd-debug/vegastrike-engine bin
