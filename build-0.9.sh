#!/bin/sh

ninja -C ./build/linux-pie-disabled-glvnd-debug -j 10 ${1} ; cp ./build/linux-pie-disabled-glvnd-debug/vegastrike-engine bin
