#!/usr/bin/env bash
##
# bootstrap-on-macOS-using-MacPorts.sh
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

port install python312 boost181@1.81.0_12+cmake_scripts+no_single+no_static+python312 glib2 +quartz libepoxy +quartz gtk3 +quartz libsdl2 libGLU freeglut openal-soft cmake expat libjpeg-turbo libpng libvorbis ninja

export CMAKE_PREFIX_PATH="/opt/local/"

echo "CMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH"

# Keep these lines! Otherwise you will likely get PFNGL... not found errors.
ln -s "$CMAKE_PREFIX_PATH/include/GL" "$CMAKE_PREFIX_PATH/include/OpenGL"
ln -s "$CMAKE_PREFIX_PATH/include/GL" "$CMAKE_PREFIX_PATH/include/GLUT"
