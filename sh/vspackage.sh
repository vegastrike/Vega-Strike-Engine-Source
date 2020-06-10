#!/bin/bash
#============ DESCRIPTION ===========
# This file generates the packages
# for the build platform.
#
# NOTE: This relies on the build
# happening first.
#====================================

#----------------------------------
# validate parameters
#----------------------------------

ROOT_DIR=$(pwd)
BUILD_DIR=$ROOT_DIR/build
BIN_DIR=$ROOT_DIR/bin
SRC_DIR=$ROOT_DIR/engine
VS_EXECUTABLE=${BUILD_DIR}/vegastrike

echo "---------------------------------"
echo "--- vspackage.sh | 2020-06-09 ---"
echo "---------------------------------"

if [ ! -f ${VS_EXECUTABLE} ]; then
	echo "***************************************"
	echo "*** Please build Vega Strike first. ***"
	echo "***************************************"
	exit 1
fi

cd $BUILD_DIR

make package
make package_source 

cd $ROOT_DIR
