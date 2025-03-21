# Copyright (C) 2025 Benjamen Meyer and other Vega Strike contributors.
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.

MESSAGE(STATUS "PROCESSING DIRECTORY: ${CMAKE_CURRENT_SOURCE_DIR}")

#FILE(GLOB VS_LIBRARIES LIST_DIRECTORIES true RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *)
SET(VS_LIBRARIES "collide2" "gfx_generic" "root_generic" "cmd")
FOREACH(VS_LIBDIR IN LISTS VS_LIBRARIES)
    MESSAGE(STATUS "Found Library Directory " ${VS_LIBDIR})
    ADD_SUBDIRECTORY(${VS_LIBDIR})
ENDFOREACH(VS_LIBDIR)
