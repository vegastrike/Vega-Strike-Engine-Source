##
# CMakeLists.txt
#
# Vega Strike - Space Simulation, Combat and Trading
# Copyright (C) 2001-2025 The Vega Strike Contributors:
# Creator: Daniel Horn
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


CMAKE_MINIMUM_REQUIRED(VERSION 3.21 FATAL_ERROR)

IF (UNIX)
	MESSAGE("Always using preferred method now to install man pages")
	INSTALL(FILES vegastrike-engine.1 TYPE MAN COMPONENT man)
	INSTALL(FILES vegastrike-engine.1 TYPE MAN RENAME vegastrike.1 COMPONENT man)
	INSTALL(FILES vsinstall.1 TYPE MAN COMPONENT man)
	INSTALL(FILES vslauncher.1 TYPE MAN COMPONENT man)
	INSTALL(FILES vegasettings.1 TYPE MAN COMPONENT man)
ENDIF (UNIX)
