#!/bin/bash

set -e

echo "docker-entrypoint.sh: Flags passed in: $@"

# find /usr -iname '*libboost_python*'

if [ $IS_RELEASE ]
then
    script/build.sh -DCMAKE_BUILD_TYPE=RelWithDebInfo $@
    script/package
else
    if [ "$CC" == "clang" ]
    then
        script/build.sh -DCMAKE_BUILD_TYPE=RelWithDebInfo $@
    else
        script/build.sh -DCMAKE_BUILD_TYPE=Debug $@
    fi
fi
