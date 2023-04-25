##
# CMakeLists.txt
#
# Copyright (c) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
# Benjamen Meyer, and other Vega Strike contributors
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


CMAKE_MINIMUM_REQUIRED(VERSION 3.16 FATAL_ERROR)

IF ((NOT BEOS) AND (NOT WIN32))
	#SET_PROPERTY(DIRECTORY PROPERTY INCLUDE_DIRECTORIES "")
	SET(VEGASETTINGS_SOURCES
		src/c/setup.cpp
		src/include/central.cpp
		src/include/display_gtk.cpp
		src/include/display_dialog.cpp
		src/include/file.cpp
		src/include/general.cpp
		../src/common/common.cpp
		)

	ADD_EXECUTABLE(vegasettings ${VEGASETTINGS_SOURCES})

	INSTALL(TARGETS vegasettings DESTINATION bin)

	#find GTK3
	FIND_PACKAGE(GTK3 REQUIRED)
	IF(GTK3_FOUND)
		SET(GTK_LIBS ${GTK3_LIBRARIES})
		SET(GTK_CFLAGS ${GTK3_DEFINITIONS} -DGTK)
	ELSE(GTK3_FOUND)
		MESSAGE("Did not find GTK3")
	ENDIF(GTK3_FOUND)

	ADD_DEFINITIONS(${GTK_CFLAGS})
	INCLUDE_DIRECTORIES(
        ${Vega_Strike_SOURCE_DIR}/setup/src/include
        ${Vega_Strike_SOURCE_DIR}/src/common
        ${Vega_Strike_BINARY_DIR}
        ${GTK3_INCLUDE_DIRS}
    )
	TARGET_LINK_LIBRARIES(vegasettings ${GTK_LIBS})
ELSE ((NOT BEOS) AND (NOT WIN32))
	MESSAGE ("SKIPPING vegasettings - not supported on this platform at this time")
ENDIF ((NOT BEOS) AND (NOT WIN32))
