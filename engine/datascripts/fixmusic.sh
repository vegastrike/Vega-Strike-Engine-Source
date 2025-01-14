#!/bin/sh
##
# fixmusic.sh
#
# Vega Strike - Space Simulation, Combat and Trading
# Copyright (C) 2001-2025 The Vega Strike Contributors:
# Project creator: Daniel Horn
# Original development team: As listed in the AUTHORS file
# Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
#
#
# https://github.com/vegastrike/Vega-Strike-Engine-Source
#
# This file is part of Vega Strike.
#
# Vega Strike is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Vega Strike is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
#

DOTVS=".vegastrike"
if test -e ../Version.txt; then
DOTVS=`cat ../Version.txt`
fi
find ../$DOTVS -name "*.m3u" -exec sh -c 'sed -e s/.ogg/.mid/g {} > {}.bak'  \;
find ../$DOTVS -name "*.m3u" -exec sh -c 'sed -e s/.mid/.ogg/g {} > {}.fore'  \;
find ../$DOTVS -name "*.m3u" -exec mv {}.bak {} \;
for i in $1; do
    find ../$DOTVS -name "*.m3u" -exec mv {}.fore {} \;
done
rm -f ../$DOTVS/*.fore
