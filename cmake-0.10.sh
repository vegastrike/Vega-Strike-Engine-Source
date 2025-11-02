#!/bin/sh

# CMAKE_MODULE_PATH="${HOME}/.local/lib/"
# echo "CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}"

# One directory UP from ${PWD} and down into private_rootdir
CMAKE_MODULE_PATH="${PWD%/*}/private_rootdir" # Doesn't help with the current CMake files, don't know why
cmake --fresh --debug-output --graphviz=deps.dot -DCMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}  -S ./ -B ./build/linux-ninja-pie-disabled-glvnd-debug --preset linux-ninja-pie-disabled-glvnd-debug ${1}
# cmake -S engine -B ./build/linux-pie-disabled-glvnd-debug --preset linux-pie-disabled-glvnd-debug 
dot -Tsvg -o deps.svg deps.dot
rm -f deps.dot.*






