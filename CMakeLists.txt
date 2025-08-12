#====================================
# @file   : CMakeLists.txt
# @brief  : cmake configuration file for VegaStrike
#====================================
# create a build directory somewhere outside of this source tree
# > cd .. && mkdir build && cd build
# run > cmake <path to source tree>
# run > make
# to change build target (in Release, RelWithDebInfo, Debug, Profiler)
# > cmake -DCMAKE_BUILD_TYPE=Debug <path to source tree>
#====================================
# Copyright (C) 2001-2025 safemode, Anth0rx, pyramid, Roy Falk,
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.


CMAKE_MINIMUM_REQUIRED(VERSION 3.21 FATAL_ERROR)

IF (POLICY CMP0087)
    CMAKE_POLICY(SET CMP0087 NEW)
ENDIF ()

# TODO: this suppresses warnings about CMP0102 caching.
# We should do something about it instead.
IF (POLICY CMP0102)
    CMAKE_POLICY(SET CMP0102 OLD)
ENDIF ()

IF (POLICY CMP0167)
    CMAKE_POLICY (SET CMP0167 NEW)
ENDIF ()

# There are a number of custom CMake packages
# Tell CMake where to find them
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

IF (WIN32)
    SET(VCPKG_MANIFEST_MODE ON)
    SET(VCPKG_MANIFEST_DIR "${CMAKE_SOURCE_DIR}")
    SET(VCPKG_MANIFEST_INSTALL ON)
ENDIF (WIN32)

SET(VEGASTRIKE_VERSION_MAJOR "0")
SET(VEGASTRIKE_VERSION_MINOR "10")
SET(VEGASTRIKE_VERSION_PATCH "0")

IF (DEFINED ENV{SHORT_SHA} AND NOT "$ENV{SHORT_SHA}" STREQUAL "")
    SET(VEGASTRIKE_VERSION_TWEAK "$ENV{SHORT_SHA}")
ELSE ()
    SET (GIT_ROOT_DIR "${CMAKE_SOURCE_DIR}")
    MESSAGE(STATUS "CMAKE_SOURCE_DIR = ${CMAKE_SOURCE_DIR}")
    MESSAGE(STATUS "GIT_ROOT_DIR = ${GIT_ROOT_DIR}")
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
SET(VEGASTRIKE_ASSETS_API_VERSION "3")

PROJECT(Vega_Strike
    VERSION
    "${VEGASTRIKE_VERSION_MAJOR}.${VEGASTRIKE_VERSION_MINOR}.${VEGASTRIKE_VERSION_PATCH}"   #.${VEGASTRIKE_VERSION_TWEAK}   # CMake only allows numeric version components, unfortunately.
    LANGUAGES
    CXX C
)

MESSAGE("== Vega Strike Version: ${VEGASTRIKE_VERSION_LONG_STR}")

SET(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_STANDARD_REQUIRED TRUE)
SET(CMAKE_CXX_EXTENSIONS ON)
SET(CMAKE_C_STANDARD 11)
SET(CMAKE_C_STANDARD_REQUIRED TRUE)
SET(CMAKE_C_EXTENSIONS ON)

ADD_COMPILE_DEFINITIONS("NV_CUBE_MAP"
        "BOOST_PYTHON_NO_PY_SIGNATURES"
        "BOOST_ALL_DYN_LINK"
        "$<$<CONFIG:Debug>:BOOST_DEBUG_PYTHON>"
        "$<$<CXX_COMPILER_ID:MSVC>:__x86_64__>"
        "$<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>"
        "$<$<CXX_COMPILER_ID:MSVC>:_CRT_NONSTDC_NO_DEPRECATE>"
)
ADD_COMPILE_OPTIONS("$<$<CXX_COMPILER_ID:MSVC>:/FI${Vega_Strike_BINARY_DIR}/engine/config.h>"
        "$<$<CXX_COMPILER_ID:MSVC>:/wd4244>"
        "$<$<CXX_COMPILER_ID:MSVC>:/wd4267>"
        "$<$<CXX_COMPILER_ID:MSVC>:/wd4305>"
        "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Debug>>:/Z7>"
)
# for DEBUG remove all optimizations
ADD_LINK_OPTIONS("$<$<CXX_COMPILER_ID:MSVC>:/DEBUG>")
ADD_COMPILE_OPTIONS("$<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-pipe>"
        "$<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-Wall>"
        "$<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-fvisibility=hidden>"
        "$<$<AND:$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>,$<CONFIG:Debug>>:-fstandalone-debug>"
        "$<$<AND:$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>,$<CONFIG:Debug>>:-O0>"
        "$<$<AND:$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>,$<OR:$<CONFIG:Profiler>,$<CONFIG:RelWithDebInfo>>>:-Og>"
        "$<$<AND:$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>,$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>>:-g3>"
        "$<$<AND:$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>,$<CONFIG:Profiler>>:-pg>"
        "$<$<AND:$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>,$<CONFIG:Profiler>>:-g2>"
        "$<$<AND:$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>,$<CONFIG:Release>>:-O3>"
)

# On some Ubuntu versions and derivatives, a bug exists whereby enabling
# PIE compilation (Position Independent Executables) results in the
# `file` utility incorrectly recognising the compiled vegastrike-engine binary
# as a shared library instead of a position independent shared executable
# object.
#
# The effect of the bug is that vegastrike-engine can still be started from the
# command line but that it will not be recognised as an executable by GUI
# file managers such as Nautilus and Dolphin.
#
# To avoid this scenario, turn off this flag by default and let packagers
# on other distributions turn this on if their OS is able to correctly deal
# with Position Independent Executables.

# For more info, see:
# - https://bugs.launchpad.net/ubuntu/+source/file/+bug/1747711
# - https://github.com/vegastrike/Vega-Strike-Engine-Source/issues/94
#
OPTION(ENABLE_PIE "Enable Position Independent Executables/Shared Libraries (NOT RECOMMENDED on Ubuntu/Mint)" OFF)
MESSAGE(STATUS "Always using preferred PIE logic now")

INCLUDE(CheckPIESupported)
check_pie_supported()
UNSET(CMAKE_POSITION_INDEPENDENT_CODE)
IF (ENABLE_PIE)
    MESSAGE(WARNING "Enabling Position Independent Executables/Shared Libraries (NOT RECOMMENDED on Ubuntu/Mint) !!")
    SET(CMAKE_POSITION_INDEPENDENT_CODE ON)
ELSE (ENABLE_PIE)
    MESSAGE(STATUS "Disabling Position Independent Executables/Shared Libraries (Recommended on Ubuntu/Mint)")
    SET(CMAKE_POSITION_INDEPENDENT_CODE OFF)
ENDIF (ENABLE_PIE)

INCLUDE(CheckIncludeFile)
INCLUDE(CheckIncludeFileCXX)
INCLUDE(CheckTypeSize)
INCLUDE(CMakePushCheckState)
SET(SIZEOF_VOID_P CMAKE_SIZEOF_VOID_P)

SET(CMAKE_FIND_FRAMEWORK "LAST")

#
# Dependency Searching
#
SET(TST_INCLUDES "")
SET(TST_LIBS "")

#find Zlib
FIND_PACKAGE(ZLIB REQUIRED)
IF (ZLIB_FOUND)
    LIST(APPEND TST_INCLUDES ${ZLIB_INCLUDE_DIR})
    LIST(APPEND TST_LIBS ${ZLIB_LIBRARIES})
ELSE (ZLIB_FOUND)
    MESSAGE(FATAL_ERROR "Failed to find required library zlib")
ENDIF (ZLIB_FOUND)

#find Expat
FIND_PACKAGE(EXPAT REQUIRED)
IF (EXPAT_FOUND)
    LIST(APPEND TST_INCLUDES ${EXPAT_INCLUDE_DIRS})
    LIST(APPEND TST_LIBS ${EXPAT_LIBRARIES})
ELSE (EXPAT_FOUND)
    MESSAGE(FATAL_ERROR "Failed to find required library expat for XML parsing")
ENDIF (EXPAT_FOUND)

#Find FFMpeg
OPTION(DISABLE_FFMPEG "Disable ffmpeg codecs" ON)
IF (NOT DISABLE_FFMPEG)
    FIND_PACKAGE(FFMPEG)
    IF (FFMPEG_FOUND)
        LIST(APPEND TST_INCLUDES ${FFMPEG_INCLUDE_DIRS})
        LIST(APPEND TST_LIBS ${FFMPEG_LIBRARIES})
        ADD_DEFINITIONS(${FFMPEG_DEFINITIONS})
        SET(HAVE_FFMPEG 1 )
        MESSAGE(STATUS "FFmpeg found.")
        IF (swscale1_FOUND)
            MESSAGE(STATUS "FFmpeg's libswscale found.")
        ELSE (swscale1_FOUND)
            MESSAGE(STATUS "FFMpeg's libswscale not found... depending on your ffmpeg version, VS might not build.")
        ENDIF (swscale1_FOUND)
    ELSE (FFMPEG_FOUND)
        MESSAGE(STATUS "FFMPEG Not Found")
    ENDIF (FFMPEG_FOUND)
ELSE (NOT DISABLE_FFMPEG)
    MESSAGE(STATUS "FFMPEG disabled")
ENDIF (NOT DISABLE_FFMPEG)

#Find Ogre
# current OGRE 1.12 is not compatible
OPTION(DISABLE_OGRE "Disable Ogre-mesh support in mesh tool" ON)
IF (NOT DISABLE_OGRE)
    FIND_PACKAGE(OGRE)
    IF (OGRE_FOUND)
        LIST(APPEND TST_INCLUDES ${OGRE_INCLUDE_DIR})
        LIST(APPEND TST_LIBS ${OGRE_LIBRARY})
        ADD_DEFINITIONS(${OGRE_DEFINITIONS})
        SET(HAVE_OGRE 1)
        MESSAGE(STATUS "Found Ogre: ${OGRE_VERSION}")
    ELSE (OGRE_FOUND)
        MESSAGE(STATUS "Ogre Not Found: compiling without")
    ENDIF (OGRE_FOUND)
ELSE (NOT DISABLE_OGRE)
    MESSAGE(STATUS "Ogre disabled")
ENDIF (NOT DISABLE_OGRE)

#Find GL
SET(CMAKE_FIND_FRAMEWORK "FIRST")
FIND_PACKAGE(OpenGL REQUIRED)
IF (OPENGL_FOUND AND OPENGL_GLU_FOUND)
    MESSAGE(STATUS "OpenGL found")
    SET(CMAKE_FIND_FRAMEWORK "LAST")
    FIND_FILE(VEGA_GLEXT_H_PATH glext.h PATH_SUFFIXES GL/ OpenGL/)
    get_filename_component(VEGA_GLEXT_H_DIRECTORY ${VEGA_GLEXT_H_PATH} DIRECTORY)
    MESSAGE(STATUS "Path to glext.h: ${VEGA_GLEXT_H_PATH}")
    FIND_FILE(VEGA_GL_H_PATH gl.h PATH_SUFFIXES GL/ OpenGL/)
    get_filename_component(VEGA_GL_H_DIRECTORY ${VEGA_GL_H_PATH} DIRECTORY)
    MESSAGE(STATUS "Path to gl.h: ${VEGA_GL_H_PATH}")
    FIND_FILE(VEGA_GLU_H_PATH glu.h PATH_SUFFIXES GL/ OpenGL/)
    get_filename_component(VEGA_GLU_H_DIRECTORY ${VEGA_GLU_H_PATH} DIRECTORY)
    MESSAGE(STATUS "Path to glu.h: ${VEGA_GLU_H_PATH}")
    LIST(APPEND TST_INCLUDES ${VEGA_GLEXT_H_DIRECTORY} ${VEGA_GL_H_DIRECTORY} ${VEGA_GLU_H_DIRECTORY})
ELSE (OPENGL_FOUND AND OPENGL_GLU_FOUND)
    MESSAGE(FATAL_ERROR "Why you no have GL?")
ENDIF (OPENGL_FOUND AND OPENGL_GLU_FOUND)

# Workaround two oversights in FindGLUT (when trying to use freeglut on MacOS):
#
# 1. Use of OPENGL_LIBRARY_DIR
#    This was added to FindOpenGL in June 2002, then removed 3 months later. In that time it had
#    made its way into use within FindGLUT, where (oddly) it was used as a possible
#    location of GLUT's headers but not its libraries. From here, it was never removed.
#
# 2. Missing path component
#    FindGLUT looks for glut.h in various locations when (on MacOS) it should be looking for
#    GLUT/glut.h. We use the latter in our headers when building on MacOS, so the include path
#    FindGLUT returns isn't actually of any use, but if FindGLUT can't find glut.h the build
#    ends up using Apple's deprecated Framework (which defeats the purpose of using freeglut).
#
# If we're not on MacOS and using freeglut this line is not required, but as OPENGL_LIBRARY_DIR
# is not used by anything else the following does no harm either.
SET(OPENGL_LIBRARY_DIR "${OPENGL_INCLUDE_DIR}/GLUT")

#Find GLUT
SET(CMAKE_FIND_FRAMEWORK "FIRST")
FIND_PACKAGE(GLUT REQUIRED)
IF (GLUT_FOUND)
    MESSAGE(STATUS "GLUT found : ${GLUT_LIBRARIES}")
    SET(CMAKE_FIND_FRAMEWORK "LAST")
    FIND_FILE(VEGA_GLUT_H_PATH glut.h PATH_SUFFIXES GL/ GLUT/)
    get_filename_component(VEGA_GLUT_H_DIRECTORY ${VEGA_GLUT_H_PATH} DIRECTORY)
    MESSAGE(STATUS "Path to glut.h: ${VEGA_GLUT_H_PATH}")
    MESSAGE(STATUS "glut.h directory: ${VEGA_GLUT_H_DIRECTORY}")
    LIST(APPEND TST_INCLUDES ${VEGA_GLUT_H_DIRECTORY})
    LIST(APPEND TST_LIBS ${GLUT_LIBRARIES})
ELSE (GLUT_FOUND)
    MESSAGE(FATAL_ERROR "I can't build this, missing GLUT")
ENDIF (GLUT_FOUND)

UNSET(OPENGL_LIBRARY_DIR)
SET(CMAKE_FIND_FRAMEWORK "LAST")

OPTION(USE_OPEN_TELEMETRY "Include and use opentelemetry-cpp" OFF)

# Detect available Linux Distros that can be built for
# 2021-09-18 stephengtuggy - This needs to happen before the boost_python detection logic,
# so that we can handle the special cases for Xenial and stretch
FIND_PACKAGE(LinuxDistro REQUIRED)

FIND_PACKAGE(Python3 3.7...<3.14 REQUIRED COMPONENTS Development Interpreter)

MESSAGE(STATUS "Python library : ${Python3_LIBRARIES} (${Python3_VERSION})")
MESSAGE(STATUS "Python include dir : ${Python3_INCLUDE_DIRS}")
IF (Python3_FOUND)
    LIST(APPEND TST_LIBS ${Python3_LIBRARIES})
    SET(HAVE_PYTHON 1)
    SET(TMP_Python_SITELIB ${Python3_SITELIB})
    STRING(REGEX REPLACE "\\\\" "/" TMP_Python_SITELIB ${TMP_Python_SITELIB})
    MESSAGE("TMP_Python_SITELIB = ${TMP_Python_SITELIB}")
    add_compile_definitions(Python_SITELIB=${TMP_Python_SITELIB})
ELSE (Python3_FOUND)
    MESSAGE(FATAL_ERROR "Can't find python")
ENDIF (Python3_FOUND)

# Find Boost with boost_python library
# Note: we now use Cmake 3.21 and Boost 1.75.
MESSAGE(STATUS "From now on, always using System Boost (ver. 1.75 or higher)")
SET(BOOST_PYTHON_COMPONENT "python${Python3_VERSION_MAJOR}${Python3_VERSION_MINOR}")
FIND_PACKAGE(Boost COMPONENTS ${BOOST_PYTHON_COMPONENT} json log log_setup date_time system filesystem thread chrono atomic program_options REQUIRED)
MESSAGE(STATUS "Found System Boost version ${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION}")
SET(BOOST_PYTHON_NO_PY_SIGNATURES 1)

LIST(APPEND TST_INCLUDES ${Python3_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
MESSAGE("** Boost_LIBRARIES: ${Boost_LIBRARIES}")
LIST(APPEND TST_LIBS ${Boost_LIBRARIES})

# Set the default build type
IF (NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Release, RelWithDebInfo, Debug, Profiler" FORCE )
ENDIF (NOT CMAKE_BUILD_TYPE)
MESSAGE("** Build Type: ${CMAKE_BUILD_TYPE}")

# Should we install gtest?
OPTION(INSTALL_GTEST "Should we download and install GTest?" ON)

# Should we run gtest?
OPTION(USE_GTEST "Should we build and run the unit tests using GTest?" ON)

IF (INSTALL_GTEST OR USE_GTEST)
    MESSAGE(STATUS "Configuring Unit Tests")
    ENABLE_TESTING()

    IF (INSTALL_GTEST)
        INCLUDE(FetchContent)
        FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG        v1.15.2
        )

        # Set custom variables, policies, etc.

        # For Windows: Prevent overriding the parent project's compiler/linker settings
        SET(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

        FetchContent_MakeAvailable(googletest)
    ENDIF (INSTALL_GTEST)
ENDIF (INSTALL_GTEST OR USE_GTEST)

INCLUDE(GNUInstallDirs)

SET(VS_SUBDIRECTORIES "libraries" "engine")
FOREACH(VS_SUBDIR IN LISTS VS_SUBDIRECTORIES)
    ADD_SUBDIRECTORY(${VS_SUBDIR})
ENDFOREACH(VS_SUBDIR)
