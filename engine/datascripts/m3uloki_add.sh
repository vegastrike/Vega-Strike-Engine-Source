#!/bin/sh

DATAPATH=$SETUP_INSTALLPATH
CURPWD=$PWD

cd $DATAPATH/bin

sh fixmusic.sh oggify
