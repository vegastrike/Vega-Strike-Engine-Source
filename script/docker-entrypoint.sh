#!/bin/bash

set -e

echo "docker-entrypoint.sh: Flags passed in: $FLAGS"

# find /usr -iname '*libboost_python*'

if [ $IS_RELEASE -eq 1 ]
then
    script/build.sh -DCMAKE_BUILD_TYPE=RelWithDebInfo $FLAGS
    script/package
else
    if [ "$CC" == "clang" ]
    then
        script/build.sh -DCMAKE_BUILD_TYPE=RelWithDebInfo $FLAGS
    else
        script/build.sh -DCMAKE_BUILD_TYPE=Debug $FLAGS
    fi
fi
