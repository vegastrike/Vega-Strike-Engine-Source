#!/bin/sh

#83;40003;0c Make a Loki Install Binary for Vegastrike
#
#
# Instructions:
#
# Copy the loki_setup/ folder in the same directory as copies of your data dir, music dir, vssetup and vegastrike.
# Note: this script will move the the data and music dirs, not copy...so don't use your cvs masters :-)
# Have the relevant binaries compiled statically.
# 
# Now we have to edit this file, and the files setup.base.xml and setup.music.xml
# 
# To edit this script:
#   + First change OLDVERSIONTEXT to the name of the current home directory, and
#     NEWVERSIONTEXT to the intended name...both sans the '.'
#   + Then change the DATAMODULE and MUSICMODULE names.
#   + If you use SVN, you'll have to edit this script to clean that out.
#   + And you are done, do and edit the xml files.
# 
# The xml files are easy to do:
#   + Change the product name, description, and version attributes in each.
#   + Change the version of the Base component in setup.base.xml
#   + You will then want to change the references to Vegastrike in the text in
#     the Option tag and the Help tag in the Base component.
#   + And then, for the binary bin/vegastrike.sh, you will want to change the
#     symlink and name attributes.
#   + Then you need to change the name of the hidden file
#     from .vegastrike.4.x to whatever your Version.txt will read.
#   + Before adding/removing any files or directories.
# 
# Run this script once with the 'prepare' argument to make sure all the binaries needed are there.
# Run this script with the 'release' argument.
# ie: sh makeloki.sh prepare
#     sh makeloki.sh release
#

# Change these to use for a different mod
# 
# \/ \/ \/ \/ \/ \/ \/ \/ \/

OLDVERSIONTEXT="vegastrike-0.5.0" # The hidden folder in the data dir, with the m3us
NEWVERSIONTEXT="vegastrike-0.5.0" # What you want the hidden folder to be called

DATAMODULE="data4.x" # The folder with the data in it
MUSICMODULE="xmusic" # The folder with the (high quality) music

DESTRUCTIVE=0 # Move music/binaries/data ... or just copy
REPOSITORY=".svn" # Either CVS or .svn
CLEANREPOSITORY=1 # Strip this info from the folders?

# /\ /\ /\ /\ /\ /\ /\ /\ /\
#
# Don't touch anything below here, unless you know what you are doing :-)

ARG=$1
SETUPARG=${ARG:="noarg"}

CURPWD=$PWD

BASEDIR=$CURPWD/../vegastrike-base
MUSICDIR=$CURPWD/../vegastrike-music

# Verify that the required binaries are built or in the relevant place.

if [ $SETUPARG = "prepare" ] || [ $SETUPARG = "release" ]; then
  
  for BINFILE in $CURPWD/../vegastrike/vegastrike $CURPWD/../vegastrike/vegaserver $CURPWD/../vegastrike/soundserver $CURPWD/../vegastrike/vssetup $CURPWD/vsinstall.sh $CURPWD/vegastrike.sh ; do
    {
    if [ \! -s $BINFILE ]; then
      echo Binary $BINFILE not built.
      break
    fi
    } ; done

# Copy the required setup files.  
  mkdir -p $BASEDIR
  mkdir -p $MUSICDIR
  cp -r $CURPWD/../loki_setup/image/* $BASEDIR/
  cp -r $CURPWD/../loki_setup/image/* $MUSICDIR/

fi

# Or copy the binaries and data to the package dir.

if [ $SETUPARG = "organise" ] || [ $SETUPARG = "release" ]; then

  if [ $CLEANREPOSITORY = 1 -a $DESTRUCTIVE = 1 ] ; then
# First remove unwanted/conflicting files
    echo "Cleaning Data"
    find $CURPWD/../$DATAMODULE -type d -name $REPOSITORY -exec /bin/rm -rf {} \; #Removes repository dirs
    echo "Cleaning Music"
    find $CURPWD/../$MUSICMODULE -type d -name $REPOSITORY -exec /bin/rm -rf {} \; #Removes repository dirs
#    find $CURPWD/../$DATAMODULE -name *.xmesh -exec /bin/rm -rf {} \; #Removes xmesh files
  fi

  echo "Cleaning Base Package"
  find $BASEDIR -type d -name .svn -exec /bin/rm -r {} \; #Removes CVS dirs
  echo "Cleaning Music Package"
  find $MUSICDIR -type d -name .svn -exec /bin/rm -r {} \; #Removes CVS dirs

  mkdir -p $BASEDIR/.$NEWVERSIONTEXT

  if [ $DESTRUCTIVE = 1 ] ; then
# Move the vegastrike data, but not this script!
#    mv $CURPWD/../$DATAMODULE/m3uloki_add.sh $MUSICDIR/setup.data/
#    mv $CURPWD/../$DATAMODULE/m3uloki_remove.sh $MUSICDIR/setup.data/
    mv $CURPWD/../$DATAMODULE/* $BASEDIR/
    mv $BASEDIR/makeloki.sh $CURPWD/
    mv $CURPWD/../$DATAMODULE/.$OLDVERSIONTEXT/*.m3u $BASEDIR/.$NEWVERSIONTEXT/
  else
# Copy the vegastrike data
#    cp $CURPWD/../$DATAMODULE/m3uloki_add.sh $MUSICDIR/setup.data/
#    cp $CURPWD/../$DATAMODULE/m3uloki_remove.sh $MUSICDIR/setup.data/
    cp -r $CURPWD/../$DATAMODULE/* $BASEDIR/
    cp $CURPWD/../$DATAMODULE/.$OLDVERSIONTEXT/*.m3u $BASEDIR/.$NEWVERSIONTEXT/
  fi

# Update Version.txt
  echo '.'$NEWVERSIONTEXT > $BASEDIR/Version.txt

# Get rid of windows binaries -- not optional!
  rm -rf $CURPWD/../$DATAMODULE/bin/* #Removes windows binaries
  rmdir $CURPWD/../$DATAMODULE/bin

  if [ $DESTRUCTIVE = 1 ] ; then
# Move the vegastrike music
    mv $CURPWD/../$MUSICMODULE $MUSICDIR/
  else
    mkdir -p $MUSICDIR/music
    cp -r $CURPWD/../$MUSICMODULE/* $CURPWD/../$MUSICMODULE/.[^.]* $MUSICDIR/music/
  fi

# If there is a hidden file with new m3us in it, move it up a level
  if [ -d $MUSICDIR/music/.$OLDVERSIONTEXT ]; then
    mv $MUSICDIR/music/.$OLDVERSIONTEXT $MUSICDIR/.$NEWVERSIONTEXT
  elif [ -d $MUSICDIR/music/.$NEWVERSIONTEXT ]; then
    mv $MUSICDIR/music/.$NEWVERSIONTEXT $MUSICDIR/
  fi

# If there are sounds, ie music becomes a speech and music pack
  if [ -d $MUSICDIR/music/sounds ]; then
    mv $MUSICDIR/music/sounds $MUSICDIR/
  fi

# Copy the vegastrike binaries
  mkdir -p $BASEDIR/bin
  
  for BINFILE in $CURPWD/../vegastrike/vegastrike $CURPWD/../vegastrike/vegaserver $CURPWD/../vegastrike/soundserver $CURPWD/../vegastrike/vssetup $CURPWD/vsinstall.sh $CURPWD/vegastrike.sh ; do
    if [ $DESTRUCTIVE = 1 ] ; then
      mv $BINFILE $BASEDIR/bin/
    else
      cp $BINFILE $BASEDIR/bin
    fi ; done
mv $BASEDIR/bin/vssetup $BASEDIR/bin/setup
#cant call it that in source directory cus there is a dir by that name


# Move the required files to their expected positions for Base
  mv $BASEDIR/setup.base.xml $BASEDIR/setup.data/setup.xml
  mv $BASEDIR/vslogo.xpm $BASEDIR/setup.data/splash.xpm
  cp $CURPWD/../vegastrike/COPYING $BASEDIR/

# Move the required files to their expected positions for Music
  mv $BASEDIR/setup.music.xml $MUSICDIR/setup.data/setup.xml
  cp $BASEDIR/setup.data/splash.xpm $MUSICDIR/setup.data/splash.xpm

  if [ $CLEANREPOSITORY = 1 -a $DESTRUCTIVE = 0 ] ; then
    echo "Cleaning Base Package"
    find $BASEDIR -type d -name $REPOSITORY -exec /bin/rm -rf {} \; #Removes repository dirs
    echo "Cleaning Music Package"
    find $MUSICDIR -type d -name $REPOSITORY -exec /bin/rm -rf {} \; #Removes repository dirs
  fi

fi

if [ $SETUPARG = "package" ] || [ $SETUPARG = "release" ]; then
# Create the packages
  cd $CURPWD/../loki_setup/makeself/
  echo "Making the Base Installer"
  sh makeself.sh --bzip2 $BASEDIR $CURPWD/../vegastrike-0.5.0-base.run.bz2.sh "Vegastrike Space Simulator 0.5.0 - Base" sh setup.sh
  echo "Making the Music Installer"
  sh makeself.sh --bzip2 $MUSICDIR $CURPWD/../vegastrike-0.5.0-music.run.bz2.sh "Vegastrike Space Simulator 0.5.0 - Music" sh setup.sh
  cd $CURPWD
fi
