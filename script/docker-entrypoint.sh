#!/bin/bash

set -e

echo "docker-entrypoint.sh: Flags passed in: $@"

# find /usr -iname '*libboost_python*'

if [ -z "$IS_RELEASE" ]
then
    if [ "$CC" == "clang" ]
    then
        script/build.sh -DCMAKE_BUILD_TYPE=RelWithDebInfo $@
    else
        script/build.sh -DCMAKE_BUILD_TYPE=Debug $@
    fi
else
    script/build.sh -DCMAKE_BUILD_TYPE=RelWithDebInfo $@
    script/package
fi
