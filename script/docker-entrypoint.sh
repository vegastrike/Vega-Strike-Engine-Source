#!/usr/bin/env bash

set -e

echo "docker-entrypoint.sh: Flags passed in: $@"

# find /usr -iname '*libboost_python*'

if [ $IS_RELEASE -eq 1 ]
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

    pushd build
    ctest -V
    popd
fi
