#!/bin/sh
# 
gdb --args ./build/linux-ninja-pie-disabled-glvnd-debug/vegastrike-engine --target "$(pwd)/../Assets-Production"

