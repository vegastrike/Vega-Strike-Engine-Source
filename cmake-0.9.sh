#!/bin/sh

# CMAKE_MODULE_PATH="${HOME}/.local/lib/"
# echo "CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}"
# cmake -DCMAKE_MODULE_PATH=${CMAKE_MODULE_PATH} -S ./ -B ./build/linux-ninja-pie-disabled-glvnd-debug --preset linux-ninja-pie-disabled-glvnd-debug 
cmake -S engine -B ./build/linux-pie-disabled-glvnd-debug --preset linux-pie-disabled-glvnd-debug 


