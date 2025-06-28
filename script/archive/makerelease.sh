#! /usr/bin/env bash

##
# makerelease.sh
#
# Copyright (c) 2001-2002 Daniel Horn
# Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
# Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
#
# https://github.com/vegastrike/Vega-Strike-Engine-Source
#
# This file is part of Vega Strike.
#
# Vega Strike is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Vega Strike is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
#

#
# THIS SCRIPT MUST BE RUN AS ROOT
# This script *should* build, from the seperate vegastrike modules, a full RPM (and SRPM) based
# release for your automagic pleasure :-)
#
# First, make sure that you have all the required modules present in this relative position to this script:
#       ../data
#       ../vegastrike        NO OTHER DIRECTORIES SHOULD BE PRESENT IN THIS PARENT DIRECTORY
#       ../music
# You should also have placed, in the data directory, the vslauncher binary, and in the vegastrike dir,
# the vegasettings binary.
# Now, make sure that the vegastrike source is prepared for compile, including the removal of all non-essential
# directories (such as objconv, launcher and the extra boost dir -- depending on your srpm options).  And make
# sure the data dir is also ready, by removing all non-linux files from all modules -- some of this is accounted
# for in this script, some is not (see Removing Windows/Mac junk).  This will minimise srpm and rpm file size.
#
# Now, before you launch this script, make sure that the .spec files are up to date with version, release and builder,
# and that the included documentation (man, info etc) are up to date for this version.
#
# Finally, make sure that you know the SRCPATH for your system.  This path should contain the directories:
# BUILD  RPMS  SOURCES  SPECS  SRPMS
# In RedHat for example, it is /usr/src/redhat

SRCPATHDEFAULT="/usr/src"

# And make sure you know the program that can build RPMs -- mainly either /usr/bin/rpm or /usr/bin/rpmbuild

RPMBUILDERDEFAULT="/usr/bin/rpm"

# Now, simply run this script! :-)
#
#

echo "Welcome to the Vegastrike Auto-Release System.  If you have prepared the directories
as instructed in the top part of this file, then simply follow the on-screen prompts
to create a new release for vegastrike.  If you have not prepared the directories as
instructed, then please exit this script by pressing ctrl-c NOW.
 "

echo "Now Setting up the scripts Variables."

read -p "Enter the src path for your system.  This path should contain the directories:
                       BUILD  RPMS  SOURCES  SPECS  SRPMS
In RedHat for example, it is /usr/src/redhat.

Press ENTER to accept the default [$SRCPATHDEFAULT]
Your path is: " SRCPATHPROMPT

SRCPATH=${SRCPATHPROMPT:-${SRCPATHDEFAULT}}

echo "SRCPATH set at $SRCPATH"

read -p "Enter the program used to build RPM files.  In most distributions it is
/usr/bin/rpm  However, since RedHat 8.0, RedHat's program has been /usr/bin/rpmbuild.

Press ENTER to accept the default [$RPMBUILDERDEFAULT]
Your program is: " RPMBUILDERPROMPT

RPMBUILDER=${RPMBUILDERPROMPT:-${RPMBUILDERDEFAULT}}

echo "RPMBUILDER set at $RPMBUILDER"



# Now that our variables are set, we can go on.
# Here we prepare the directories for making good RPMS

echo "Preparing Directories -- Removing CVS junk"
cd ..
find . -type d -name CVS -exec /bin/rm -r {} \;

echo "Preparing Directories -- Removing Windows/Mac junk"

rm -f data/*.exe
rm -f data/*.dll

# Extra lines should be added here as the list of non-linux stuff grows ;-) or as wanted.

echo "Preparing Directories -- isolating .spec files"

mv data/vegastrike-data.spec ./
mv vegastrike/vegastrike.spec ./
mv music/vegastrike-music.spec ./

echo "Making RPMS -- (1 of 3 -- Data)"

tar -cvzf vegastrike-data.tar.gz data
mv vegastrike-data.tar.gz $SRCPATH/SOURCES
$RPMBUILDER -bb vegastrike-data.spec

# Remove these next two lines to prevent the vegastrike-data tarball being removed by me.
echo "Removing Temporary .tar.gz file [$SRCPATH/SOURCES/vegastrike-data.tar.gz]"
rm -f $SRCPATH/SOURCES/vegastrike-data.tar.gz

echo "Data RPM Complete...you can start uploading now"

echo "Making RPMS -- (2 of 3 -- Music)"

tar -cvzf vegastrike-music.tar.gz music
mv vegastrike-music.tar.gz $SRCPATH/SOURCES
$RPMBUILDER -bb vegastrike-music.spec

# Remove these next two lines to prevent the vegastrike-music tarball being removed by me.
echo "Removing Temporary .tar.gz file [$SRCPATH/SOURCES/vegastrike-music.tar.gz]"
rm -f $SRCPATH/SOURCES/vegastrike-music.tar.gz

echo "Music RPM Complete...you can start uploading now"

echo "Making RPMS -- (3 of 3 -- Vegastrike)"

tar -cvzf vegastrike.tar.gz vegastrike
mv vegastrike.tar.gz $SRCPATH/SOURCES
$RPMBUILDER -ba vegastrike.spec

# Remove these next two lines to prevent the vegastrike tarball being removed by me.
echo "Removing Temporary .tar.gz file [$SRCPATH/SOURCES/vegastrike.tar.gz]"
rm -f $SRCPATH/SOURCES/vegastrike.tar.gz

echo "Vegastrike RPM and SRPM Complete...you can start uploading now"

# echo "Congratulations on a sucessfull build.  I will *NOT* clean up the tarballs now, in
# case you wish to use them again and I get in trouble.  They are located in $SRCPATH/SOURCES
# The src.rpm is located in $SRCPATH/SRPMS The RPMS are located in $SRCPATH/RPMS/i386

echo "Congratulations on a sucessfull build.  I have already cleaned up the temporary tarballs
created by this script from $SRCPATH/SOURCES
The final src.rpm and RPMs are located in $SRCPATH/SRPMS and $SRCPATH/RPMS/i386 respectively.

Happy releasing, may the gods of the GPL shine upon you :-)"

# Okay, RPMS built, do as you wish :-)
# Written by Daniel Aleksandrow <dandandamdan@users.sourceforge.net>
