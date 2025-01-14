#!/usr/bin/env bash
##
# bootstrap-on-macos.sh
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

set -e

declare -a packages_to_install=("openssl@3" "python@3.11" "python@3.12" "boost" "boost-python3" "gtk+3" "gtkglext" "sdl12-compat" "sdl2" "mesa" "mesa-glu" "freeglut" "openal-soft" "cmake" "expat" "jpeg" "libpng" "libvorbis")
declare -a links_to_delete=("2to3" "idle3" "pydoc3" "python3" "python3-config" "2to3-3.11" "idle3.11" "pydoc3.11" "python3.11" "python3.11-config" "2to3-3.12" "idle3.12" "pydoc3.12" "python3.12" "python3.12-config")

for i in "${links_to_delete[@]}"
do
    link_path="/usr/local/bin/$i"
    if [ -f "$link_path" ]
    then
        rm "/usr/local/bin/$i"
    fi
done

brew update

for j in "${packages_to_install[@]}"
do
    brew list "$j" &>/dev/null || brew install "$j"
    PACKAGE_INSTALLED_PREFIX=$(brew --prefix "$j")
    PACKAGE_INSTALLED_BIN="$PACKAGE_INSTALLED_PREFIX/bin"
    export PATH="$PACKAGE_INSTALLED_BIN:$PATH"
    export CMAKE_PREFIX_PATH="$PACKAGE_INSTALLED_PREFIX:$CMAKE_PREFIX_PATH"
done

# Keep these lines! Otherwise you will likely get PFNGL... not found errors.
ln -s "$(brew --prefix)/include/GL" "$(brew --prefix)/include/OpenGL"
ln -s "$(brew --prefix)/include/GL" "$(brew --prefix)/include/GLUT"

OPENALDIR="$(brew --prefix openal-soft)"
export OPENALDIR

echo "PATH=$PATH"
echo "CMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH"
echo "OPENALDIR=$OPENALDIR"
