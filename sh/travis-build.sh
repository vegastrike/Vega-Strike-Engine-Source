#!/bin/bash
set -ev

if [ -z "$TRAVIS_TAG" ]
then
    ./sh/vsbuild.sh -DCMAKE_BUILD_TYPE=Debug $@
else
    ./sh/vsbuild.sh -DCMAKE_BUILD_TYPE=RelWithDebInfo $@
fi
