##
# FindOGRE.cmake
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

# - Try to find Ogre
# Once done this will define
#
#  HAVE_OGRE - system has Ogre
#  OGRE_LIBRARIES - Linker flags
#  OGRE_DEFINITIONS - Compiler switches required for using Ogre
#
#  Copyright (c) 2006 Andreas Schneider <mail@cynapses.org>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

if (OGRE_LIBRARIES AND OGRE_DEFINITIONS AND OGRE_INCLUDE_DIRS)
  # in cache already
  set(HAVE_OGRE TRUE)
else (OGRE_LIBRARIES AND OGRE_DEFINITIONS AND OGRE_INCLUDE_DIRS)
  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  include(FindPkgConfig)
  
  pkg_check_modules(OGRE OGRE>=1.2.0)
  
  if (OGRE_FOUND)
      SET(OGRE_DEFINITIONS ${OGRE_CFLAGS})
      SET(OGRE_LIBRARIES ${OGRE_LIBRARIES})
      SET(OGRE_INCLUDE_DIRS ${OGRE_INCLUDE_DIRS})
      message(STATUS "Found Ogre: ${OGRE_VERSION}")
  else (OGRE_FOUND)
      SET(HAVE_OGRE FALSE)
      message(STATUS "Ogre not found")
  endif (OGRE_FOUND)
  
  # show the FFMPEG_INCLUDE_DIRS and FFMPEG_LIBRARIES variables only in the advanced view
  mark_as_advanced(OGRE_LIBRARIES OGRE_DEFINITIONS OGRE_INCLUDE_DIRS)

endif (OGRE_LIBRARIES AND OGRE_DEFINITIONS AND OGRE_INCLUDE_DIRS)
