##
# vssetup-gtk+osx.sh
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

g++ -DGTK -DHAVE_PYTHON=1  -pipe  -O2  -D_GNU_SOURCE=1 -D_THREAD_SAFE -D_REENTRANT -arch i386 -arch ppc -o vegasettings setup/src/c/setup.cpp setup/src/include/central.cpp setup/src/include/display_gtk.cpp setup/src/include/file.cpp setup/src/include/general.cpp src/common/common.cpp ~/prog/gtk-osx-0.7/glib-1.2.10/mac-project-builder/build/glib.build/Deployment/GLib\ Framework.build/Objects-normal/i386/* ~/prog/gtk-osx-0.7/gdk/build/gdk.build/Deployment/gdk\ \(Carbon\ Events\ Framework\).build/Objects-normal/i386/* ~/prog/gtk-osx-0.7/gtk/build/gtk.build/Deployment/gtk\ \(Carbon\ Events\ Framework\).build/Objects-normal/i386/* -framework Carbon -mmacosx-version-min=10.4
