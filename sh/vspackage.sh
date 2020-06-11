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
PACKAGE_DIR=$ROOT_DIR/packages
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

# Ensure the package output directory exists
mkdir -p ${PACKAGE_DIR}

# Copy the Debian Package files
for DEB_FILE in `ls *.deb`
do
	echo "Copying ${DEB_FILE} to ${PACKAGE_DIR}"
	cp "${DEB_FILE}" "${PACKAGE_DIR}"
done

# Copy the Red Hat RPM Package files
for RPM_FILE in `ls *.rpm`
do
	echo "Copying ${RPM_FILE} to ${PACKAGE_DIR}"
	cp "${RPM_FILE}" "${PACKAGE_DIR}"
done

# Copy the Tarball Package files
for TAR_FILE in `ls *.tar.*`
do
	echo "Copying ${TAR_FILE} to ${PACKAGE_DIR}"
	cp "${TAR_FILE}" "${PACKAGE_DIR}"
done

cd $ROOT_DIR
