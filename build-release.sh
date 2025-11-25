#!/bin/sh

ninja -C ./build/linux-ninja-pie-disabled-glvnd-release -j 10 ${1} ; cp ./build/linux-ninja-pie-disabled-glvnd-debug/vegastrike-engine bin
