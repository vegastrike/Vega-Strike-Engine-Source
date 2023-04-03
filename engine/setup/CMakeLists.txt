##
# CMakeLists.txt
#
# Copyright (C) 2001-2002 Daniel Horn
# Copyright (C) 2002-2019 pyramid3d and other Vega Strike Contributors
# Copyright (C) 2019-2022 Stephen G. Tuggy and other Vega Strike Contributors
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

    UNSET(Boost_FOUND)
    UNSET(Boost_DIR)

    IF (USE_SYSTEM_BOOST)
        MESSAGE("++ Using System Boost")
        FIND_PACKAGE(Boost)
        IF (Boost_FOUND)
            FIND_PACKAGE(Boost COMPONENTS program_options REQUIRED)
        ELSE (Boost_FOUND)
            SET(USE_SYSTEM_BOOST OFF)
        ENDIF (Boost_FOUND)
    ENDIF (USE_SYSTEM_BOOST)

    IF (NOT USE_SYSTEM_BOOST)
        MESSAGE("++ Using Internal Boost")
        # This is just to cleanup advanced variable viewing, since they're not used anyway
        UNSET(Boost_INCLUDE_DIRS)
        UNSET(Boost_LIBRARIES)
        UNSET(Boost_INCLUDE_DIR)
        UNSET(Boost_LIBRARY_DIRS)
        SET(Boost_DIR ../ext/)
        ADD_SUBDIRECTORY(${Boost_DIR} build)
        MESSAGE("++ boost dir: ${Boost_DIR}")
    ENDIF (NOT USE_SYSTEM_BOOST)

    INCLUDE_DIRECTORIES(${Boost_DIR})

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
        ${Boost_INCLUDE_DIRS}
    )
	TARGET_LINK_LIBRARIES(vegasettings ${GTK_LIBS} ${Boost_LIBRARIES})
ELSE ((NOT BEOS) AND (NOT WIN32))
	MESSAGE ("SKIPPING vegasettings - not supported on this platform at this time")
ENDIF ((NOT BEOS) AND (NOT WIN32))
