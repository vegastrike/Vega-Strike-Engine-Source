#!/bin/bash
#====================================
# @file   : build.sh
# @brief  : build VegaStrike
# @usage  : ./script/build.sh
# @param  : none
#====================================

#============ DESCRIPTION ===========
# This script is making a clean build of VegaStrike.
# After this, it copies the relevant files to the 'bin' directory.
# The steps for creating this script were taken from the projects wiki:
# https://github.com/Taose/Vegastrike-taose/wiki/How-to-Build
#====================================


set -e

echo "-----------------------------"
echo "--- build.sh | 2021-11-26 ---"
echo "-----------------------------"

#----------------------------------
# validate parameters
#----------------------------------

ROOT_DIR=$(pwd)
BUILD_DIR="${ROOT_DIR}/build"
BIN_DIR="${ROOT_DIR}/bin"
SRC_DIR="${ROOT_DIR}/engine"
COMMAND=""

echo "ROOT_DIR: ${ROOT_DIR}"
echo "BUILD_DIR: ${BUILD_DIR}"

# -p creates if the target doesn't exist, noop otherwise
mkdir -pv "${BUILD_DIR}" && cd "${BUILD_DIR}"

# configure libraries and prepare for the Debug build having -Werror set,
# thus gating VS commits on being warning-free at some point in the near
# future -- see https://github.com/vegastrike/Vega-Strike-Engine-Source/issues/50
cmake $@ "${SRC_DIR}"

# for a clean build only
# mut we can do it manually
#make clean

# compile now using all cpus and show compilation commands
# MAX_THREADS_TO_BUILD_WITH=8
NUM_THREADS_TO_BUILD_WITH=$(getconf _NPROCESSORS_ONLN)
# if [ ${NUM_THREADS_TO_BUILD_WITH} -gt ${MAX_THREADS_TO_BUILD_WITH} ]
# then
#     NUM_THREADS_TO_BUILD_WITH=${MAX_THREADS_TO_BUILD_WITH}
# fi

VERBOSE=1 cmake --build "${BUILD_DIR}" -j $NUM_THREADS_TO_BUILD_WITH

cd "${ROOT_DIR}"

mkdir -pv "${BIN_DIR}"

cp -v "${BUILD_DIR}"/{vegastrike-engine,setup/vegasettings,objconv/vega-meshtool} "${BIN_DIR}"
