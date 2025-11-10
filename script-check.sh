
In script/bootstrap_local_libs line 155:
    git checkout ${GIT_HASH}
                 ^---------^ SC2086 (info): Double quote to prevent globbing and word splitting.

Did you mean: 
    git checkout "${GIT_HASH}"


In script/bootstrap_local_libs line 156:
    if [ ! -z "${GIT_PATCH_NAME}" ]; then
         ^-- SC2236 (style): Use -n instead of ! -z.


In script/bootstrap_local_libs line 160:
    mkdir -p build-${CMAKE_BUILD_TYPE}
                   ^-----------------^ SC2086 (info): Double quote to prevent globbing and word splitting.

Did you mean: 
    mkdir -p build-"${CMAKE_BUILD_TYPE}"


In script/bootstrap_local_libs line 161:
    cmake -G Ninja -S ./ -B build-${CMAKE_BUILD_TYPE} -DCMAKE_INSTALL_PREFIX:PATH="${INSTALL_DIR}" -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DSDL_EXAMPLES=OFF -DSDL_STATIC=OFF -DSDL_SHARED=ON
                                  ^-----------------^ SC2086 (info): Double quote to prevent globbing and word splitting.
                                                                                                                      ^-----------------^ SC2086 (info): Double quote to prevent globbing and word splitting.

Did you mean: 
    cmake -G Ninja -S ./ -B build-"${CMAKE_BUILD_TYPE}" -DCMAKE_INSTALL_PREFIX:PATH="${INSTALL_DIR}" -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}" -DSDL_EXAMPLES=OFF -DSDL_STATIC=OFF -DSDL_SHARED=ON


In script/bootstrap_local_libs line 162:
    ninja -C build-${CMAKE_BUILD_TYPE}
                   ^-----------------^ SC2086 (info): Double quote to prevent globbing and word splitting.

Did you mean: 
    ninja -C build-"${CMAKE_BUILD_TYPE}"


In script/bootstrap_local_libs line 165:
    ninja -C build-${CMAKE_BUILD_TYPE} install
                   ^-----------------^ SC2086 (info): Double quote to prevent globbing and word splitting.

Did you mean: 
    ninja -C build-"${CMAKE_BUILD_TYPE}" install

For more information:
  https://www.shellcheck.net/wiki/SC2086 -- Double quote to prevent globbing ...
  https://www.shellcheck.net/wiki/SC2236 -- Use -n instead of ! -z.
