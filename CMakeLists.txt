#====================================
# @file   : CMakeLists.txt
# @brief  : cmake configuration file for VegaStrike
#====================================
# create a build directory somewhere outside of this source tree
# > cd .. && mkdir build && cd build
# run > cmake ..
# run > make
# to change build target (in Release, RelWithDebInfo, Debug, Profiler)
# > cmake -DCMAKE_BUILD_TYPE=Debug <path to source tree>
#====================================
# Copyright (C) 2001-2022 safemode, Anth0rx, pyramid, Roy Falk,
# Nachum Barcohen, Rune Morling, Stephen G. Tuggy, Benjamen Meyer, s0600204,
# Evert Vorster, and other Vega Strike contributors.
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

CMAKE_MINIMUM_REQUIRED(VERSION 3.16 FATAL_ERROR)

# There are a number of custom CMake packages
# Tell CMake where to find them
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

SET(VEGASTRIKE_VERSION_MAJOR "0")
SET(VEGASTRIKE_VERSION_MINOR "9")
SET(VEGASTRIKE_VERSION_PATCH "0")

IF (DEFINED ENV{SHORT_SHA} AND NOT "$ENV{SHORT_SHA}" STREQUAL "")
    SET(VEGASTRIKE_VERSION_TWEAK "$ENV{SHORT_SHA}")
ELSE ()
    SET (GIT_ROOT_DIR "${CMAKE_SOURCE_DIR}")
    MESSAGE("-- CMAKE_SOURCE_DIR = ${CMAKE_SOURCE_DIR}")
    MESSAGE("-- GIT_ROOT_DIR = ${GIT_ROOT_DIR}")
    EXECUTE_PROCESS(
            COMMAND git rev-parse --short HEAD
            WORKING_DIRECTORY ${GIT_ROOT_DIR}
            OUTPUT_VARIABLE VEGASTRIKE_VERSION_TWEAK
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
ENDIF ()

SET(VEGASTRIKE_VERSION_LONG_STR "${VEGASTRIKE_VERSION_MAJOR}.${VEGASTRIKE_VERSION_MINOR}.${VEGASTRIKE_VERSION_PATCH}-${VEGASTRIKE_VERSION_TWEAK}")
SET(VEGASTRIKE_VERSION_SHORT_STR "${VEGASTRIKE_VERSION_MAJOR}.${VEGASTRIKE_VERSION_MINOR}.${VEGASTRIKE_VERSION_PATCH}")
SET(VEGASTRIKE_PKG_VERSION_STR "${VEGASTRIKE_VERSION_MAJOR}.${VEGASTRIKE_VERSION_MINOR}.${VEGASTRIKE_VERSION_PATCH}")

IF (DEFINED ENV{TAG_NAME} AND NOT "$ENV{TAG_NAME}" STREQUAL "")
    STRING(REGEX REPLACE "^v([0-9]+)\\..*" "\\1" TAG_VERSION_MAJOR "$ENV{TAG_NAME}")
    STRING(REGEX REPLACE "^v[0-9]+\\.([0-9]+).*" "\\1" TAG_VERSION_MINOR "$ENV{TAG_NAME}")
    STRING(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" TAG_VERSION_PATCH "$ENV{TAG_NAME}")
    STRING(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.[0-9]+(.*)" "\\1" TAG_VERSION_TWEAK "$ENV{TAG_NAME}")

    SET(TAG_VERSION_SHORT_STR "${TAG_VERSION_MAJOR}.${TAG_VERSION_MINOR}.${TAG_VERSION_PATCH}")

    IF (NOT "${VEGASTRIKE_VERSION_SHORT_STR}" VERSION_EQUAL "${TAG_VERSION_SHORT_STR}")
        MESSAGE(FATAL_ERROR "!! Project version spelled out in CMake file does not match project version from TAG_NAME environment variable")
    ENDIF ()
    IF (NOT "${TAG_VERSION_TWEAK}" STREQUAL "")
        SET(VEGASTRIKE_VERSION_LONG_STR "${VEGASTRIKE_VERSION_MAJOR}.${VEGASTRIKE_VERSION_MINOR}.${VEGASTRIKE_VERSION_PATCH}${TAG_VERSION_TWEAK}-${VEGASTRIKE_VERSION_TWEAK}")
        SET(VEGASTRIKE_PKG_VERSION_STR "${VEGASTRIKE_VERSION_MAJOR}.${VEGASTRIKE_VERSION_MINOR}.${VEGASTRIKE_VERSION_PATCH}${TAG_VERSION_TWEAK}")
    ENDIF ()
ENDIF ()

# API Version for Game Assets
# This is an incrementing number similar to the Google Android API Version
# allowing us to differentiate our Assets API across multiple versions.
# If a release is missing this value, then version `1` can be assumed.
SET(VEGASTRIKE_ASSETS_API_VERSION "2")

PROJECT(Vega_Strike
    VERSION
        "${VEGASTRIKE_VERSION_MAJOR}.${VEGASTRIKE_VERSION_MINOR}.${VEGASTRIKE_VERSION_PATCH}"   #.${VEGASTRIKE_VERSION_TWEAK}   # CMake only allows numeric version components, unfortunately.
    LANGUAGES
        CXX C
)

MESSAGE("== Vega Strike Version: ${VEGASTRIKE_VERSION_LONG_STR}")

SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_STANDARD_REQUIRED TRUE)
SET(CMAKE_CXX_EXTENSIONS ON)
SET(CMAKE_C_STANDARD 11)
SET(CMAKE_C_STANDARD_REQUIRED TRUE)
SET(CMAKE_C_EXTENSIONS ON)

SET(VS_SUBDIRECTORIES "engine")
FOREACH(VS_SUBDIR IN LISTS VS_SUBDIRECTORIES)
    ADD_SUBDIRECTORY(${VS_SUBDIR})
ENDFOREACH(VS_SUBDIR)
