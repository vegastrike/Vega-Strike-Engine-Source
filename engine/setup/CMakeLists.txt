##
# CMakeLists.txt
#
# Copyright (C) 2001-2025 Daniel Horn, pyramid3d, Stephen G. Tuggy,
# Benjamen R. Meyer, and other Vega Strike contributors
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
#


CMAKE_MINIMUM_REQUIRED(VERSION 3.21 FATAL_ERROR)

IF (CMAKE_SYSTEM_NAME STREQUAL BEOS OR CMAKE_SYSTEM_NAME STREQUAL Windows)
    MESSAGE ("SKIPPING vegasettings - not supported on this platform at this time")
ELSEIF (CMAKE_SYSTEM_NAME STREQUAL Linux)
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
    TARGET_COMPILE_DEFINITIONS(vegasettings PUBLIC "BOOST_ALL_DYN_LINK")

    #find GTK3
    FIND_PACKAGE(GTK3 3.0 REQUIRED gtk)
    IF(GTK3_FOUND)
        SET(GTK_LIBS ${GTK3_LIBRARIES})
        SET(GTK_CFLAGS ${GTK3_DEFINITIONS} -DGTK)
    ELSE(GTK3_FOUND)
        MESSAGE("Did not find GTK3")
    ENDIF(GTK3_FOUND)

    ADD_DEFINITIONS(${GTK_CFLAGS})

    FIND_PACKAGE(Boost COMPONENTS program_options REQUIRED)

    INCLUDE_DIRECTORIES(
        ${Vega_Strike_SOURCE_DIR}/setup/src/include
        ${Vega_Strike_SOURCE_DIR}/src/common
        ${Vega_Strike_BINARY_DIR}
        ${GTK3_INCLUDE_DIRS}
        ${Boost_INCLUDE_DIRS}
    )
    TARGET_LINK_LIBRARIES(vegasettings ${GTK_LIBS} ${Boost_LIBRARIES})

    INSTALL(TARGETS vegasettings DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT vegasettings)
ELSEIF (CMAKE_SYSTEM_NAME STREQUAL Darwin)
    SET(VEGASETTINGS_SOURCES
            src/c/setup.cpp
            src/include/central.cpp
            src/include/display_gtk.cpp
            src/include/display_dialog.cpp
            src/include/file.cpp
            src/include/general.cpp
            ../src/common/common.cpp
    )

    ADD_EXECUTABLE(vegasettings MACOSX_BUNDLE ${VEGASETTINGS_SOURCES})
    TARGET_COMPILE_DEFINITIONS(vegasettings PUBLIC "BOOST_ALL_DYN_LINK")

    #find GTK3
    FIND_PACKAGE(GTK3 3.0 REQUIRED gtk)
    IF(GTK3_FOUND)
        SET(GTK_LIBS ${GTK3_LIBRARIES})
        SET(GTK_CFLAGS ${GTK3_DEFINITIONS} -DGTK)
    ELSE(GTK3_FOUND)
        MESSAGE("Did not find GTK3")
    ENDIF(GTK3_FOUND)

    ADD_DEFINITIONS(${GTK_CFLAGS})

    FIND_PACKAGE(Boost COMPONENTS program_options REQUIRED)

    INCLUDE_DIRECTORIES(
            ${Vega_Strike_SOURCE_DIR}/setup/src/include
            ${Vega_Strike_SOURCE_DIR}/src/common
            ${Vega_Strike_BINARY_DIR}
            ${GTK3_INCLUDE_DIRS}
            ${Boost_INCLUDE_DIRS}
    )
    TARGET_LINK_LIBRARIES(vegasettings ${GTK_LIBS} ${Boost_LIBRARIES})

    INSTALL(TARGETS vegasettings DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT vegasettings BUNDLE DESTINATION "/Applications/VegaStrike")
ELSE ()
    MESSAGE ("SKIPPING vegasettings - not supported on this platform at this time")
ENDIF ()
