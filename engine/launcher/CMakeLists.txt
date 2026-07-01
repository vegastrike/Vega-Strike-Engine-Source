##
# CMakeLists.txt
#
# Vega Strike - Space Simulation, Combat and Trading
# Copyright (C) 2001-2026 The Vega Strike Contributors:
# Project creator: Daniel Horn
# Original development team: As listed in the AUTHORS file
# Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
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

MESSAGE(STATUS "PROCESSING DIRECTORY: ${CMAKE_CURRENT_SOURCE_DIR}")

IF (CMAKE_SYSTEM_NAME STREQUAL BEOS OR CMAKE_SYSTEM_NAME STREQUAL Windows)
    MESSAGE ("SKIPPING vslauncher - not supported on this platform at this time")
ELSEIF (CMAKE_SYSTEM_NAME STREQUAL Linux)
    MESSAGE(STATUS "Configuring vslauncher for Linux")

    SET(LAUNCHER_SOURCES
            saveinterface.cpp
        )

    ADD_EXECUTABLE(vslauncher ${LAUNCHER_SOURCES})

    TARGET_INCLUDE_DIRECTORIES(vslauncher SYSTEM PRIVATE ${VSE_TST_INCLUDES})
    TARGET_INCLUDE_DIRECTORIES(vslauncher PRIVATE
            # VS engine headers
            ${Vega_Strike_SOURCE_DIR}
            ${Vega_Strike_SOURCE_DIR}/engine
            ${Vega_Strike_SOURCE_DIR}/engine/src
            # Library Headers
            ${Vega_Strike_SOURCE_DIR}/libraries
            # CMake Artifacts
            ${Vega_Strike_BINARY_DIR}
            ${Vega_Strike_BINARY_DIR}/src
            ${Vega_Strike_BINARY_DIR}/engine
            ${Vega_Strike_BINARY_DIR}/engine/src
    )

    SET_TARGET_PROPERTIES(vslauncher PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    SET_PROPERTY(TARGET vslauncher PROPERTY CXX_STANDARD_REQUIRED TRUE)
    SET_PROPERTY(TARGET vslauncher PROPERTY CXX_EXTENSIONS ON)
    TARGET_COMPILE_DEFINITIONS(vslauncher PUBLIC "BOOST_ALL_DYN_LINK")

    #find GTK3
    FIND_PACKAGE(GTK3 3.0 REQUIRED gtk)
    IF(GTK3_FOUND)
        SET(GTK_LIBS ${GTK3_LIBRARIES})
        IF(DEFINED GTK3_DEFINITIONS)
            SET(GTK_CFLAGS ${GTK3_DEFINITIONS} -DGTK)
        ELSE ()
            SET(GTK_CFLAGS -DGTK)
        ENDIF ()
    ELSE(GTK3_FOUND)
        MESSAGE("Did not find GTK3")
    ENDIF(GTK3_FOUND)

    ADD_DEFINITIONS(${GTK_CFLAGS})

    INCLUDE_DIRECTORIES(
            ${Vega_Strike_SOURCE_DIR}/common
            ${Vega_Strike_BINARY_DIR}
            ${GTK3_INCLUDE_DIRS}
    )

    TARGET_LINK_LIBRARIES(vslauncher
            ${GTK_LIBS}
            vegastrike_common
    )

    INSTALL(TARGETS vslauncher DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT vslauncher)
ELSEIF (CMAKE_SYSTEM_NAME STREQUAL Darwin)
    MESSAGE(STATUS "Configuring vslauncher for macOS")

    SET(LAUNCHER_SOURCES
            saveinterface.cpp
    )

    ADD_EXECUTABLE(vslauncher MACOSX_BUNDLE ${LAUNCHER_SOURCES})

    TARGET_INCLUDE_DIRECTORIES(vslauncher SYSTEM PRIVATE ${VSE_TST_INCLUDES})
    TARGET_INCLUDE_DIRECTORIES(vslauncher PRIVATE
            # VS engine headers
            ${Vega_Strike_SOURCE_DIR}
            ${Vega_Strike_SOURCE_DIR}/engine
            ${Vega_Strike_SOURCE_DIR}/engine/src
            # Library Headers
            ${Vega_Strike_SOURCE_DIR}/libraries
            # CMake Artifacts
            ${Vega_Strike_BINARY_DIR}
            ${Vega_Strike_BINARY_DIR}/src
            ${Vega_Strike_BINARY_DIR}/engine
            ${Vega_Strike_BINARY_DIR}/engine/src
    )

    SET_TARGET_PROPERTIES(vslauncher PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    SET_PROPERTY(TARGET vslauncher PROPERTY CXX_STANDARD_REQUIRED TRUE)
    SET_PROPERTY(TARGET vslauncher PROPERTY CXX_EXTENSIONS ON)
    TARGET_COMPILE_DEFINITIONS(vslauncher PUBLIC "BOOST_ALL_DYN_LINK")

    #find GTK3
    FIND_PACKAGE(GTK3 3.0 REQUIRED gtk)
    IF(GTK3_FOUND)
        SET(GTK_LIBS ${GTK3_LIBRARIES})
        IF(DEFINED GTK3_DEFINITIONS)
            SET(GTK_CFLAGS ${GTK3_DEFINITIONS} -DGTK)
        ELSE ()
            SET(GTK_CFLAGS -DGTK)
        ENDIF ()
    ELSE(GTK3_FOUND)
        MESSAGE("Did not find GTK3")
    ENDIF(GTK3_FOUND)

    ADD_DEFINITIONS(${GTK_CFLAGS})

    INCLUDE_DIRECTORIES(
            ${Vega_Strike_SOURCE_DIR}/common
            ${Vega_Strike_BINARY_DIR}
            ${GTK3_INCLUDE_DIRS}
            ${GTK3_GLIBCONFIG_INCLUDE_DIR}
            ${CMAKE_PREFIX_PATH}/lib/glib-2.0/include
    )

    TARGET_LINK_LIBRARIES(vslauncher
            ${GTK_LIBS}
            vegastrike_common
    )

    INSTALL(TARGETS vslauncher DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT vslauncher BUNDLE DESTINATION "/Applications/VegaStrike")
ELSE ()
    MESSAGE ("SKIPPING vslauncher - not supported on this platform at this time")
ENDIF ()
