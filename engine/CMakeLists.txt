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
# Copyright (C) 2008-2022 safemode, Anth0rx, pyramid, Roy Falk,
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


CMAKE_MINIMUM_REQUIRED(VERSION 3.13 FATAL_ERROR)

SET(VEGASTRIKE_VERSION_MAJOR "0")
SET(VEGASTRIKE_VERSION_MINOR "9")
SET(VEGASTRIKE_VERSION_PATCH "0")

IF (DEFINED ENV{SHORT_SHA} AND NOT "$ENV{SHORT_SHA}" STREQUAL "")
    SET(VEGASTRIKE_VERSION_TWEAK "$ENV{SHORT_SHA}")
ELSE ()
    SET (GIT_ROOT_DIR "${CMAKE_SOURCE_DIR}/..")
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

IF (COMMAND cmake_policy)
    CMAKE_POLICY(SET CMP0003 NEW)
ENDIF (COMMAND cmake_policy)

# VERSION Policy
IF (POLICY CMP0048)
    CMAKE_POLICY (SET CMP0048 NEW)
ENDIF (POLICY CMP0048)

# Used to be that without this, CMake couldn't find pow() -- at least not on my Windows machine.
IF (POLICY CMP0066)
    CMAKE_POLICY (SET CMP0066 NEW)  #OLD
ENDIF ()

# FindOpenGL prefers GLVND by default when available. But there is an option to override that.
IF (POLICY CMP0072)
    CMAKE_POLICY (SET CMP0072 NEW)
ENDIF (POLICY CMP0072)

# find_package() uses <PackageName>_ROOT variables
IF (POLICY CMP0074)
    CMAKE_POLICY (SET CMP0074 NEW)
ENDIF (POLICY CMP0074)

# Include file check macros honor CMAKE_REQUIRED_LIBRARIES
IF (POLICY CMP0075)
    CMAKE_POLICY (SET CMP0075 NEW)
ENDIF ()

# MSVC runtime library flags are selected by an abstraction. Supported by CMake 3.15 and up
IF (POLICY CMP0091)
    CMAKE_POLICY (SET CMP0091 NEW)
ENDIF ()

PROJECT(Vega_Strike
    VERSION
        "${VEGASTRIKE_VERSION_MAJOR}.${VEGASTRIKE_VERSION_MINOR}.${VEGASTRIKE_VERSION_PATCH}"   #.${VEGASTRIKE_VERSION_TWEAK}   # CMake only allows numeric version components, unfortunately.
    LANGUAGES
        CXX C
)

# One version header generator to rule them all
CONFIGURE_FILE(src/version.h.in ${CMAKE_SOURCE_DIR}/src/version.h)
CONFIGURE_FILE(src/version.h.in ${CMAKE_SOURCE_DIR}/setup/src/include/version.h)

MESSAGE("== Vega Strike Version: ${VEGASTRIKE_VERSION_LONG_STR}")

SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_STANDARD_REQUIRED TRUE)
SET(CMAKE_CXX_EXTENSIONS ON)
SET(CMAKE_C_STANDARD 11)
SET(CMAKE_C_STANDARD_REQUIRED TRUE)
SET(CMAKE_C_EXTENSIONS ON)

UNSET(PYTHONLIBS_FOUND)
UNSET(Boost_FOUND)
UNSET(Boost_python3_FOUND)
UNSET(OPENGL_FOUND)
UNSET(OPENGL_GLU_FOUND)
UNSET(GLUT_FOUND)
UNSET(OPENAL_FOUND)
UNSET(SDL_FOUND)
UNSET(VORBIS_FOUND)
UNSET(JPEG_FOUND)
UNSET(PNG_FOUND)
UNSET(ZLIB_FOUND)
UNSET(EXPAT_FOUND)
UNSET(MATH_FOUND)
UNSET(FFMPEG_FOUND)
UNSET(OGRE_FOUND)
UNSET(Boost_DIR)

SET(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
SET(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
INCLUDE(InstallRequiredSystemLibraries)

IF (MSVC)
INCLUDE_DIRECTORIES(
    ${Vega_Strike_SOURCE_DIR}/src
    ${Vega_Strike_SOURCE_DIR}/src/cmd
    ${Vega_Strike_SOURCE_DIR}/src/damage
    ${Vega_Strike_SOURCE_DIR}/src/resource
    ${Vega_Strike_BINARY_DIR}
)
ELSE (MSVC)
INCLUDE_DIRECTORIES(
    ${Vega_Strike_SOURCE_DIR}/src
    ${Vega_Strike_SOURCE_DIR}/src/cmd
    ${Vega_Strike_SOURCE_DIR}/src/damage
    ${Vega_Strike_SOURCE_DIR}/src/resource
    ${Vega_Strike_BINARY_DIR}
    /usr/include/harfbuzz/
)
ENDIF (MSVC)

# The source files used to be listed here...


INCLUDE(CheckIncludeFile)
INCLUDE(CheckIncludeFileCXX)
INCLUDE(CheckTypeSize)
INCLUDE(CMakePushCheckState)
# cmake_push_check_state()
UNSET(COMPILER_SPECIFIC_FLAGS)
SET(SIZEOF_VOID_P CMAKE_SIZEOF_VOID_P)
# SET(CMAKE_REQUIRED_LINK_OPTIONS "${TOOLCHAIN_SPECIFIC_LINK_FLAGS}")
# CHECK_TYPE_SIZE("void*" SIZEOF_VOID_P) #BUILTIN_TYPES_ONLY)
# cmake_pop_check_state()


#Find Math
INCLUDE(CheckSymbolExists)
IF(NOT POW_FUNCTION_EXISTS AND NOT NEED_LINKING_AGAINST_LIBM)
    # cmake_push_check_state()
    UNSET(COMPILER_SPECIFIC_FLAGS)
    check_symbol_exists(pow math.h POW_FUNCTION_EXISTS)
    IF(NOT POW_FUNCTION_EXISTS)
        UNSET(POW_FUNCTION_EXISTS CACHE)
        LIST(APPEND CMAKE_REQUIRED_LIBRARIES m)
        # LIST(APPEND CMAKE_REQUIRED_INCLUDES math.h)
        check_symbol_exists(pow math.h POW_FUNCTION_EXISTS)
        IF(POW_FUNCTION_EXISTS)
            SET(NEED_LINKING_AGAINST_LIBM True CACHE BOOL "" FORCE)
        ELSE(POW_FUNCTION_EXISTS)
            MESSAGE(FATAL_ERROR "Failed making the pow() function available")
        ENDIF(POW_FUNCTION_EXISTS)
    ENDIF(NOT POW_FUNCTION_EXISTS)
    # cmake_pop_check_state()
ENDIF(NOT POW_FUNCTION_EXISTS AND NOT NEED_LINKING_AGAINST_LIBM)

# Set the default build type
IF (NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Release, RelWithDebInfo, Debug, Profiler" FORCE )
ENDIF (NOT CMAKE_BUILD_TYPE)

#IF (NOT BUILD_OPT)
#    SET(BUILD_OPT -O2 CACHE STRING "-O0, -O1, -O2, -O3, -Os, -Ofast" FORCE )
#ENDIF (NOT BUILD_OPT)

# Option to turn off compiling vegastrike bin
OPTION(DISABLE_CLIENT "Disable building the vegastrike bin" OFF )

# Should we prefer the Mesa OpenGL implementation, or GLVND?
# OPTION(VEGA_STRIKE_PREFER_LEGACY_OPENGL "Prefer legacy OpenGL implementation (such as Mesa's)? Or prefer GLVND?" OFF )
IF (OpenGL_GL_PREFERENCE STREQUAL "LEGACY")
    SET (VEGA_STRIKE_GLU_DEPENDENCY "libglu1-mesa")
ELSE ()
    SET (VEGA_STRIKE_GLU_DEPENDENCY "libglvnd0")
ENDIF ()
MESSAGE("** OpenGL_GL_PREFERENCE: ${OpenGL_GL_PREFERENCE}")

# Should we install gtest?
OPTION(INSTALL_GTEST "Should we download and install GTest?" ON)

# Should we run gtest?
OPTION(USE_GTEST "Should we build and run the unit tests using GTest?" ON)

# Provide boolean options for enabling various cpu optimizations.
OPTION(CPUAMD_k8  "Enable AMD K8 optimizations (Athlon through athlon64)" OFF )
OPTION(CPUAMD_k9  "Enable AMD K9 (sse3) optimizations (Athlon64 AM3) " OFF )
OPTION(CPUAMD_k10  "Enable AMD K10 optimizations (Athlon2)" OFF )
OPTION(CPUAMD_vishera  "Enable AMD Vishera optimizations" OFF )
OPTION(CPUINTEL_p4 "Enable Intel P4 optimizations (1st gen P4's)" OFF )
OPTION(CPUINTEL_prescott  "Enable Intel P4 (sse3) optimizations" OFF )
OPTION(CPUINTEL_nocona "Enable Intel P4 (64bit) optimizations" OFF )
OPTION(CPUINTEL_core2 "Enable Intel Core2 optimizations" OFF )
OPTION(CPUINTEL_native "Enable host-specific optimizations for this machine" OFF )

SET(DATADIR "" CACHE STRING "Default vegastrike data directory")

# Hide our temporary variable and initialize it to empty
SET(CPU_OPTS  )
SET(DEFINES "-DNV_CUBE_MAP -DBOOST_PYTHON_NO_PY_SIGNATURES -DBOOST_ALL_DYN_LINK ")
MARK_AS_ADVANCED( CPU_OPTS )
MARK_AS_ADVANCED( DEFINES )
SET(CPU_SMP "1" CACHE STRING "Number of CPU's in system")

# Depending on the boolean chosen above, set CPU_OPTS to the correct cpu opts
IF (CPUAMD_k8)
    SET(CPU_OPTS "-mtune=athlon64 -mfpmath=sse -msse2 -mmmx -m3dnow -ftree-vectorize -ffast-math -fassociative-math -fprefetch-loop-arrays ")
ELSEIF (CPUAMD_K9)
    SET(CPU_OPTS "-mtune=k8-sse3 -mfpmath=sse -msse3 -mmmx -m3dnow -ftree-vectorize -ffast-math -fassociative-math -funroll-loops ")
ELSEIF (CPUINTEL_p4)
    SET(CPU_OPTS "-mtune=pentium4 -mfpmath=sse -msse2 -mmmx -ftree-vectorize -ffast-math -fassociative-math -funroll-loops ")
ELSEIF (CPUINTEL_prescott)
    SET(CPU_OPTS "-mtune=prescott -mfpmath=sse -msse3 -mmmx -ftree-vectorize -ffast-math -fassociative-math -funroll-loops " )
ELSEIF (CPUINTEL_nocona)
    SET(CPU_OPTS "-mtune=nocona -mfpmath=sse -msse3 -mmmx -ftree-vectorize -ffast-math -fassociative-math -funroll-loops ")
ELSEIF (CPUINTEL_core2)
    SET(CPU_OPTS "-mtune=core2 -mfpmath=sse -msse3 -mmmx -ftree-vectorize -ffast-math -fassociative-math -funroll-loops ")
ELSEIF (CPUINTEL_native)
    SET(CPU_OPTS "-mtune=native -march=native -mfpmath=sse -msse3 -mmmx -ftree-vectorize -ffast-math -fassociative-math -funroll-loops ")
ELSEIF (CPUAMD_vishera)
    SET(CPU_OPTS "-march=bdver2 -mfpmath=both -mmmx -msse4a -msse4.2 -maes -mpclmul -mfsgsbase -mrdrnd -mf16c -mfma -mfma4 -m3dnow -mxop -mlwp -mtbm -mavx2 -mvzeroupper -msahf -mcx16 ")
ENDIF (CPUAMD_k8)

IF (CPU_SMP GREATER 1)
    SET(CPU_OPTS " ${CPU_OPTS} -ftree-parallelize-loops=${CPU_SMP} ")
ENDIF (CPU_SMP GREATER 1)

IF (DATADIR)
    SET(DEFINES "${DEFINES} -DDATA_DIR='\"${DATADIR}\"'")
ENDIF (DATADIR)

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
IF (CMAKE_VERSION VERSION_LESS "3.14")
    MESSAGE("-- CMake version older than 3.14; falling back to old PIE logic")
    IF (ENABLE_PIE)
        MESSAGE("!! Enabling Position Independent Executables/Shared Libraries (NOT RECOMMENDED on Ubuntu/Mint) !!")
        ADD_COMPILE_OPTIONS("-fpie")
        LINK_LIBRARIES("-pie")
    ELSE (ENABLE_PIE)
        MESSAGE("++ Disabling Position Independent Executables/Shared Libraries (Recommended on Ubuntu/Mint)")
        IF (NOT MSVC)
            ADD_COMPILE_OPTIONS("-fno-pie")
            LINK_LIBRARIES("-no-pie")
        ENDIF (NOT MSVC)
    ENDIF (ENABLE_PIE)
ELSE (CMAKE_VERSION VERSION_LESS "3.14")
    MESSAGE("-- CMake version 3.14 or later; using preferred PIE logic")
    UNSET(CMAKE_POSITION_INDEPENDENT_CODE)
    IF (ENABLE_PIE)
        MESSAGE("!! Enabling Position Independent Executables/Shared Libraries (NOT RECOMMENDED on Ubuntu/Mint) !!")
        SET(CMAKE_POSITION_INDEPENDENT_CODE ON)
    ELSE (ENABLE_PIE)
        MESSAGE("++ Disabling Position Independent Executables/Shared Libraries (Recommended on Ubuntu/Mint)")
        SET(CMAKE_POSITION_INDEPENDENT_CODE OFF)
    ENDIF (ENABLE_PIE)
ENDIF (CMAKE_VERSION VERSION_LESS "3.14")

#set_property(DIRECTORY APPEND PROPERTY
#        COMPILE_DEFINITIONS $<MSVC:/FI\"${Vega_Strike_BINARY_DIR}\"/config.h >
#        )

add_compile_definitions("NV_CUBE_MAP"
        "BOOST_PYTHON_NO_PY_SIGNATURES"
        "BOOST_ALL_DYN_LINK"
        "$<$<CONFIG:Debug>:BOOST_DEBUG_PYTHON>"
        "$<$<CXX_COMPILER_ID:MSVC>:__x86_64__>"
        "$<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>"
        "$<$<CXX_COMPILER_ID:MSVC>:_CRT_NONSTDC_NO_DEPRECATE>"
        )
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/FI${Vega_Strike_BINARY_DIR}/config.h>"
        "$<$<CXX_COMPILER_ID:MSVC>:/wd4244>"
        "$<$<CXX_COMPILER_ID:MSVC>:/wd4267>"
        "$<$<CXX_COMPILER_ID:MSVC>:/wd4305>"
        "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Debug>>:/Z7>"
        )
add_link_options("$<$<CXX_COMPILER_ID:MSVC>:/DEBUG>")
add_compile_options("$<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-pipe>"
        "$<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-Wall>"
        "$<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-fvisibility=hidden>"
        "$<$<AND:$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>,$<OR:$<CONFIG:Debug>,$<CONFIG:Profiler>,$<CONFIG:RelWithDebInfo>>>:-Og>"
        "$<$<AND:$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>,$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>>:-g3>"
        "$<$<AND:$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>,$<CONFIG:Profiler>>:-pg>"
        "$<$<AND:$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>,$<CONFIG:Profiler>>:-g2>"
        "$<$<AND:$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>,$<CONFIG:Release>>:-O3>"
        )

# Let cmake find our in-tree modules
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${Vega_Strike_SOURCE_DIR})

# Detect available Linux Distros that can be built for
# 2021-09-18 stephengtuggy - This needs to happen before the boost_python detection logic,
# so that we can handle the special cases for Xenial and stretch
FIND_PACKAGE(LinuxDistro REQUIRED)

FIND_PACKAGE(Python3 REQUIRED COMPONENTS Development)
MESSAGE("++ Python library : ${Python3_LIBRARIES} (${Python3_VERSION})")
MESSAGE("++ Python include dir : ${Python3_INCLUDE_DIRS}")
IF (Python3_FOUND)
    SET(TST_INCLUDES ${Python3_INCLUDE_DIRS})
    SET(TST_LIBS ${Python3_LIBRARIES})
    SET(HAVE_PYTHON 1)
ELSE (Python3_FOUND)
    MESSAGE(FATAL_ERROR "Can't find python")
ENDIF (Python3_FOUND)

# Find Boost with boost_python library
OPTION(USE_SYSTEM_BOOST "Use system boost" ON)
IF (USE_SYSTEM_BOOST)
    MESSAGE("++ Using System Boost")
    FIND_PACKAGE(Boost)
    IF (Boost_FOUND)

        # VERSION_EQUAL / VERSION_GREATER requires a version in the form X.Y.Z
        # Until CMake v3.15, Boost_VERSION is in the form as written in boost/version.hpp (e.g. 106700)
        # Thus, build the version from parts, which should work across all supported CMake versions
        SET(Boost_DERIVED_VERSION "${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION}")
        MESSAGE("++ Found System Boost version : ${Boost_DERIVED_VERSION}")

        # With boost version 1.67 and above, python components are specified with a version suffix
        # (`python27`, `python38`) instead of their invocation name (`python`, `python3`)
        IF (Boost_DERIVED_VERSION VERSION_EQUAL 1.67.0)
            SET(Boost_1_67_Or_Later_Result TRUE)
        ELSEIF (Boost_DERIVED_VERSION VERSION_GREATER 1.67.0)
            SET(Boost_1_67_Or_Later_Result TRUE)
        ELSE (Boost_DERIVED_VERSION VERSION_EQUAL 1.67.0)
            SET(Boost_1_67_Or_Later_Result FALSE)
        ENDIF (Boost_DERIVED_VERSION VERSION_EQUAL 1.67.0)
        MESSAGE("-- Boost_1_67_Or_Later_Result: ${Boost_1_67_Or_Later_Result}")
        IF (Boost_1_67_Or_Later_Result)
            SET(BOOST_PYTHON_COMPONENT "python${Python3_VERSION_MAJOR}${Python3_VERSION_MINOR}")
        ELSE (Boost_1_67_Or_Later_Result)
            SET(BOOST_PYTHON_COMPONENT "python3")

            # Debian and Debian-based systems up to Boost 1.67 lack libboost_python3.{so|a}
            # but have libboost_python-py{major}{minor}.{so|a}. E.g., Ubuntu xenial and
            # Debian stretch.
            IF (CMAKE_SYSTEM_NAME STREQUAL Linux)
                IF (LINUX_CODENAME STREQUAL xenial OR LINUX_CODENAME STREQUAL stretch)
                    SET(BOOST_PYTHON_COMPONENT "python-py3${Python3_VERSION_MINOR}")
                ENDIF (LINUX_CODENAME STREQUAL xenial OR LINUX_CODENAME STREQUAL stretch)
            ENDIF (CMAKE_SYSTEM_NAME STREQUAL Linux)
        ENDIF (Boost_1_67_Or_Later_Result)
        UNSET(Boost_1_67_Or_Later_Result)

        UNSET(Boost_${BOOST_PYTHON_COMPONENT}_FOUND)
        MESSAGE("++ Looking for System Boost::python")
        FIND_PACKAGE(Boost COMPONENTS ${BOOST_PYTHON_COMPONENT} log log_setup date_time system filesystem thread chrono atomic REQUIRED)
        IF (Boost_python3_FOUND)
            MESSAGE("++ Found System Boost::python3 : ${Boost_PYTHON3_LIBRARY}")
        ELSEIF (Boost_${BOOST_PYTHON_COMPONENT}_FOUND)
            MESSAGE("++ Found System Boost::python")
        ENDIF (Boost_python3_FOUND)

        SET(BOOST_PYTHON_NO_PY_SIGNATURES 1)
        SET(TST_INCLUDES ${TST_INCLUDES} ${Boost_INCLUDE_DIRS})
        SET(TST_LIBS ${TST_LIBS} ${Boost_LIBRARIES})
        INCLUDE_DIRECTORIES(${TST_INCLUDES})
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
    SET(BOOST_PYTHON_NO_PY_SIGNATURES 1)
    SET(TST_INCLUDES ${TST_INCLUDES} ${Vega_Strike_SOURCE_DIR}/${Boost_DIR})
    MESSAGE("++ Using Internal Boost::python3")
    SET(TST_LIBS ${TST_LIBS} boost_python3)
    INCLUDE_DIRECTORIES(${TST_INCLUDES})
    ADD_SUBDIRECTORY(${Boost_DIR} build)
    MESSAGE("++ boost dir: ${Boost_DIR}")
    #IF (NOT DISABLE_CLIENT)
    #  ADD_DEPENDENCIES(vegastrike-engine boost_python)
    #ENDIF (NOT DISABLE_CLIENT)
    #IF (NOT DISABLE_SERVER)
    #  ADD_DEPENDENCIES(vegaserver boost_python)
    #ENDIF (NOT DISABLE_SERVER)
ENDIF (NOT USE_SYSTEM_BOOST)

IF (NOT DISABLE_CLIENT) ##########

    #Find GL
    FIND_PACKAGE(OpenGL REQUIRED)
    IF (OPENGL_FOUND AND OPENGL_GLU_FOUND)
        SET(TST_INCLUDES ${TST_INCLUDES} ${OPENGL_INCLUDE_DIR})
        # SET(TST_LIBS ${TST_LIBS} "${OpenGL::GL}" "${OpenGL::GLU}")
        MESSAGE("++ OpenGL found")
    ELSE (OPENGL_FOUND AND OPENGL_GLU_FOUND)
        MESSAGE("!! Why you no have GL?")
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
    FIND_PACKAGE(GLUT REQUIRED)
    IF (GLUT_FOUND)
        SET(TST_INCLUDES ${TST_INCLUDES} ${GLUT_INCLUDE_DIR})
        SET(TST_LIBS ${TST_LIBS} ${GLUT_LIBRARIES})
        MESSAGE("++ GLUT found : ${GLUT_LIBRARIES}")
    ELSE (GLUT_FOUND)
        MESSAGE("!! I can't build this, missing GLUT")
    ENDIF (GLUT_FOUND)

    UNSET(OPENGL_LIBRARY_DIR)

    #Find OpenAL
    FIND_PACKAGE(OpenAL REQUIRED)
    IF (OPENAL_FOUND)
        MESSAGE("++ Found OpenAL")
        SET(TST_INCLUDES ${TST_INCLUDES} ${OPENAL_INCLUDE_DIR})
        SET(TST_LIBS ${TST_LIBS} ${OPENAL_LIBRARY})
        SET(HAVE_AL 1)
    ELSE (OPENAL_FOUND)
        MESSAGE("!! We aint got no sound")
    ENDIF (OPENAL_FOUND)

    IF (NOT BEOS)
        #Find SDL
        FIND_PACKAGE(SDL)
        IF (SDL_FOUND)
            SET(TST_INCLUDES ${TST_INCLUDES} ${SDL_INCLUDE_DIR})
            SET(TST_LIBS ${TST_LIBS} ${SDL_LIBRARY})
            MESSAGE("++ SDL Found")
            SET(HAVE_SDL 1)
            SET(SDL_WINDOWING 1)
        ELSE (SDL_FOUND)
            MESSAGE("!! How will we render to OpenGL without SDL?")
        ENDIF (SDL_FOUND)
    ENDIF (NOT BEOS)

    #find Vorbis
    FIND_PACKAGE(Vorbis REQUIRED)
    IF (Vorbis_FOUND)
        SET(TST_INCLUDES ${TST_INCLUDES} ${Vorbis_INCLUDE_DIRS})
        SET(TST_LIBS ${TST_LIBS} ${Vorbis_LIBRARIES})
        SET(HAVE_OGG 1)
    ELSE (Vorbis_FOUND)
        MESSAGE("!! Can't find Vorbis libs")
    ENDIF (Vorbis_FOUND)

    #Find JPEG
    FIND_PACKAGE(JPEG REQUIRED)
    IF (JPEG_FOUND)
        SET(TST_INCLUDES ${TST_INCLUDES} ${JPEG_INCLUDE_DIR})
        SET(TST_LIBS ${TST_LIBS} ${JPEG_LIBRARIES})
    ELSE (JPEG_FOUND)
        MESSAGE("!! How are we gonna open jpegs?")
    ENDIF (JPEG_FOUND)

    #Find PNG
    FIND_PACKAGE(PNG REQUIRED)
    IF (PNG_FOUND)
        SET(TEST_INCLUDES ${TST_INCLUDES} ${PNG_INCLUDE_DIRS})
        SET(TST_LIBS ${TST_LIBS} ${PNG_LIBRARIES})
    ELSE (PNG_FOUND)
        MESSAGE("!! Can't find PNG lib")
    ENDIF (PNG_FOUND)

ENDIF(NOT DISABLE_CLIENT) ##########

#find Zlib
FIND_PACKAGE(ZLIB REQUIRED)
IF (ZLIB_FOUND)
    SET(TST_INCLUDES ${TST_INCLUDES} ${ZLIB_INCLUDE_DIR})
    SET(TST_LIBS ${TST_LIBS} ${ZLIB_LIBRARIES})
ELSE (ZLIB_FOUND)
    MESSAGE("!! For the love of god, no zlib here")
ENDIF (ZLIB_FOUND)

#find Expat
FIND_PACKAGE(EXPAT REQUIRED)
IF (EXPAT_FOUND)
    SET(TST_INCLUDES ${TST_INCLUDES} ${EXPAT_INCLUDE_DIRS})
    SET(TST_LIBS ${TST_LIBS} ${EXPAT_LIBRARIES})
ELSE (EXPAT_FOUND)
    MESSAGE("!! We can't find expat for XML crap")
ENDIF (EXPAT_FOUND)

#Find FFMpeg
OPTION(DISABLE_FFMPEG "Disable ffmpeg codecs" ON)
IF (NOT DISABLE_FFMPEG)
    FIND_PACKAGE(FFMPEG)
    IF (FFMPEG_FOUND)
        SET(TST_INCLUDES ${TST_INCLUDES} ${FFMPEG_INCLUDE_DIRS})
        SET(TST_LIBS ${TST_LIBS} ${FFMPEG_LIBRARIES})
        ADD_DEFINITIONS(${FFMPEG_DEFINITIONS})
        SET(HAVE_FFMPEG 1 )
        MESSAGE("++ FFmpeg found.")
        IF (swscale1_FOUND)
            MESSAGE("++ FFmpeg's libswscale found.")
        ELSE (swscale1_FOUND)
            MESSAGE("-- FFMpeg's libswscale not found... depending on your ffmpeg version, VS might not build.")
        ENDIF (swscale1_FOUND)
    ELSE (FFMPEG_FOUND)
        MESSAGE("-- FFMPEG Not Found")
    ENDIF (FFMPEG_FOUND)
ELSE (NOT DISABLE_FFMPEG)
    MESSAGE("-- FFMPEG disabled")
ENDIF (NOT DISABLE_FFMPEG)

#Find Ogre
# current OGRE 1.12 is not compatible
OPTION(DISABLE_OGRE "Disable Ogre-mesh support in mesh tool" ON)
IF (NOT DISABLE_OGRE)
    FIND_PACKAGE(OGRE)
    IF (OGRE_FOUND)
        SET(TST_INCLUDES ${TST_INCLUDES} ${OGRE_INCLUDE_DIR})
        SET(TST_LIBS ${TST_LIBS} ${OGRE_LIBRARY})
        ADD_DEFINITIONS(${OGRE_DEFINITIONS})
        SET(HAVE_OGRE 1)
        MESSAGE("++ Found Ogre: ${OGRE_VERSION}")
    ELSE (OGRE_FOUND)
        MESSAGE("-- Ogre Not Found: compiling without")
    ENDIF (OGRE_FOUND)
ELSE (NOT DISABLE_OGRE)
    MESSAGE("-- Ogre disabled")
ENDIF (NOT DISABLE_OGRE)

IF (NOT BEOS)
    FIND_LIBRARY(UTIL_LIB util)
ELSEIF (WIN32)
    # Don't need it?
ELSE (NOT BEOS)
    FIND_LIBRARY(UTIL_LIB network)
ENDIF (NOT BEOS)

IF (NOT WIN32)
    SET(TST_LIBS ${TST_LIBS} ${UTIL_LIB})
ENDIF (NOT WIN32)

# Still need to add CEGUI and OGRE find packages

# Multiplayer networking option
OPTION(USE_NET_THREAD_POSIX "Enable Posix net threads" OFF)
IF (NOT USE_NET_THREAD_POSIX)
    SET(USE_NET_THREAD_NONE 1)
ENDIF (NOT USE_NET_THREAD_POSIX)


#config.h generation
INCLUDE(CheckFunctionExists)
INCLUDE(CheckSymbolExists)
CHECK_INCLUDE_FILE(wchar.h HAVE_WCHAR_H)
CHECK_INCLUDE_FILE(float.h HAVE_FLOAT_H)
CHECK_INCLUDE_FILE(wctype.h HAVE_WCTYPE_H)
CHECK_INCLUDE_FILE(inttypes.h HAVE_INTTYPES_H)
CHECK_INCLUDE_FILE(zlib.h HAVE_ZLIB_H)
CHECK_INCLUDE_FILE(math.h HAVE_MATH_H)
CHECK_INCLUDE_FILE_CXX(tr1/unordered_map HAVE_TR1_UNORDERED_MAP)

CHECK_FUNCTION_EXISTS(gettimeofday HAVE_GETTIMEOFDAY)
IF (HAVE_MATH_H)
    CHECK_FUNCTION_EXISTS(isnan HAVE_ISNAN)
    IF (NOT HAVE_ISNAN)
        CHECK_FUNCTION_EXISTS(_isnan HAVE__ISNAN)
    ENDIF (NOT HAVE_ISNAN)
    CHECK_FUNCTION_EXISTS(finite HAVE_FINITE)
    IF (NOT HAVE_FINITE)
        CHECK_FUNCTION_EXISTS(_finite HAVE__finite)
    ENDIF (NOT HAVE_FINITE)
ENDIF (HAVE_MATH_H)

CHECK_INCLUDE_FILE(sys/stat.h HAVE_STAT_H)
IF (HAVE_STAT_H)
    CHECK_FUNCTION_EXISTS(lstat HAVE_LSTAT)
ENDIF (HAVE_STAT_H)

CHECK_SYMBOL_EXISTS(GLX_ARB_get_proc_address GL/glx.h HAVE_GLXGETPROCADDRESSARB)

IF (UNIX)
    IF (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        SET(HOSTOS "LINUX")
        SET(_GNU_SOURCE 1)
    ELSE (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        SET(HOSTOS "BSD")
    ENDIF (CMAKE_SYSTEM_NAME STREQUAL "Linux")
ELSEIF (WIN32)
    SET(HOSTOS "WIN32")
ELSEIF (BEOS)
    SET(HOSTOS "HAIKU")
ELSE (UNIX)
    SET(HOSTOS "APPLE")
ENDIF (UNIX)

CONFIGURE_FILE(${Vega_Strike_SOURCE_DIR}/cmake-config.h.in ${Vega_Strike_BINARY_DIR}/config.h)

#end config.h generation

INCLUDE_DIRECTORIES(APPEND ${TST_INCLUDES})
include_directories(APPEND ${CMAKE_CURRENT_BINARY_DIR})
add_compile_options("$<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-include>" "$<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:config.h>")
add_link_options("$<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-Bsymbolic>")

# Now the source files are listed starting here!
SET(LIBVS_LOGGING
    src/vs_logging.cpp
)

SET(LIBCONFIG
    src/abstract_config.cpp
    src/configuration/configuration.cpp
    src/configuration/game_config.cpp
)

SET(LIBDAMAGE
    src/damage/damageable_layer.cpp
    src/damage/damageable_object.cpp
    src/damage/health.cpp
    )

SET(LIBRESOURCE
    src/resource/resource.cpp
    src/resource/store.cpp
    src/resource/product.cpp
    )

SET(LIBGUI_SOURCES
    src/gui/button.cpp
    src/gui/control.cpp
    src/gui/eventmanager.cpp
    src/gui/eventresponder.cpp
    src/gui/font.cpp
    src/gui/glut_support.cpp
    src/gui/groupcontrol.cpp
    src/gui/guidefs.cpp
    src/gui/guitexture.cpp
    src/gui/modaldialog.cpp
    src/gui/newbutton.cpp
    src/gui/painttext.cpp
    src/gui/picker.cpp
    src/gui/scroller.cpp
    src/gui/simplepicker.cpp
    src/gui/slider.cpp
    src/gui/staticdisplay.cpp
    src/gui/text_area.cpp
    src/gui/textinputdisplay.cpp
    src/gui/window.cpp
    src/gui/windowcontroller.cpp
)

SET(LIBPYTHON_SOURCES
    src/python/init.cpp
    src/python/python_compile.cpp
    src/python/unit_exports.cpp
    src/python/unit_exports1.cpp
    src/python/unit_exports2.cpp
    src/python/unit_exports3.cpp
    src/python/unit_method_defs.cpp
    src/python/unit_wrapper.cpp
    src/python/universe_util_export.cpp
)

SET(LIBSCRIPT_SOURCES
    src/cmd/script/director_generic.cpp
    src/cmd/script/mission_script.cpp
    src/cmd/script/mission.cpp
    src/cmd/script/msgcenter.cpp
    src/cmd/script/pythonmission.cpp
    src/cmd/script/script_call_olist.cpp
    src/cmd/script/script_call_omap.cpp
    src/cmd/script/script_call_order.cpp
    src/cmd/script/script_call_string.cpp
    src/cmd/script/script_call_unit_generic.cpp
    src/cmd/script/script_callbacks.cpp
    src/cmd/script/script_expression.cpp
    src/cmd/script/script_generic.cpp
    src/cmd/script/script_statement.cpp
    src/cmd/script/script_util.cpp
    src/cmd/script/script_variables.cpp
)

SET(LIBCMDSCRIPTSERVER_SOURCES
    src/cmd/script/flightgroup_server.cpp
    src/cmd/script/script_call_unit_server.cpp
    src/cmd/script/c_alike/c_alike.tab.cpp
    src/cmd/script/c_alike/lex.yy.cpp
)

SET(LIBAI_SOURCES
    src/cmd/ai/aggressive.cpp
    src/cmd/ai/autodocking.cpp
    src/cmd/ai/comm_ai.cpp
    src/cmd/ai/communication_xml.cpp
    src/cmd/ai/communication.cpp
    src/cmd/ai/docking.cpp
    src/cmd/ai/event_xml.cpp
    src/cmd/ai/fire.cpp
    src/cmd/ai/fireall.cpp
    src/cmd/ai/flybywire.cpp
    src/cmd/ai/hard_coded_scripts.cpp
    src/cmd/ai/ikarus.cpp
    src/cmd/ai/missionscript.cpp
    src/cmd/ai/navigation.cpp
    src/cmd/ai/order_comm.cpp
    src/cmd/ai/order.cpp
    src/cmd/ai/script.cpp
    src/cmd/ai/tactics.cpp
    src/cmd/ai/turretai.cpp
    src/cmd/ai/warpto.cpp
    src/cmd/ai/flykeyboard_generic.cpp
)

ADD_LIBRARY(vegastrike-OPcollide
    src/cmd/collide2/Ice/IceAABB.cpp
    src/cmd/collide2/Ice/IceContainer.cpp
    src/cmd/collide2/Ice/IceHPoint.cpp
    src/cmd/collide2/Ice/IceIndexedTriangle.cpp
    src/cmd/collide2/Ice/IceMatrix3x3.cpp
    src/cmd/collide2/Ice/IceMatrix4x4.cpp
    src/cmd/collide2/Ice/IceOBB.cpp
    src/cmd/collide2/Ice/IcePlane.cpp
    src/cmd/collide2/Ice/IcePoint.cpp
    src/cmd/collide2/Ice/IceRandom.cpp
    src/cmd/collide2/Ice/IceRay.cpp
    src/cmd/collide2/Ice/IceRevisitedRadix.cpp
    src/cmd/collide2/Ice/IceSegment.cpp
    src/cmd/collide2/Ice/IceTriangle.cpp
    src/cmd/collide2/Ice/IceUtils.cpp
    src/cmd/collide2/csgeom2/opbox.cpp
    src/cmd/collide2/csgeom2/opmatrix3.cpp
    src/cmd/collide2/csgeom2/opvector3.cpp
    src/cmd/collide2/OPC_AABBCollider.cpp
    src/cmd/collide2/OPC_AABBTree.cpp
    src/cmd/collide2/OPC_BaseModel.cpp
    src/cmd/collide2/OPC_BoxPruning.cpp
    src/cmd/collide2/OPC_Collider.cpp
    src/cmd/collide2/OPC_Common.cpp
    src/cmd/collide2/OPC_HybridModel.cpp
    src/cmd/collide2/OPC_LSSCollider.cpp
    src/cmd/collide2/OPC_MeshInterface.cpp
    src/cmd/collide2/OPC_Model.cpp
    src/cmd/collide2/OPC_OBBCollider.cpp
    src/cmd/collide2/OPC_OptimizedTree.cpp
    src/cmd/collide2/OPC_Picking.cpp
    src/cmd/collide2/OPC_PlanesCollider.cpp
    src/cmd/collide2/OPC_RayCollider.cpp
    src/cmd/collide2/OPC_SphereCollider.cpp
    src/cmd/collide2/OPC_SweepAndPrune.cpp
    src/cmd/collide2/OPC_TreeBuilders.cpp
    src/cmd/collide2/OPC_TreeCollider.cpp
    src/cmd/collide2/OPC_VolumeCollider.cpp
    src/cmd/collide2/CSopcodecollider.cpp
)

TARGET_COMPILE_FEATURES(vegastrike-OPcollide PUBLIC cxx_std_11)
IF (NEED_LINKING_AGAINST_LIBM)
    TARGET_LINK_LIBRARIES(vegastrike-OPcollide m)
ENDIF()

SET(LIBCMD_SOURCES
    src/cmd/alphacurve.cpp
    src/cmd/cargo.cpp
    src/cmd/carrier.cpp
    src/cmd/collection.cpp
    src/cmd/collide_map.cpp
    src/cmd/collide.cpp
    src/cmd/container.cpp
    src/cmd/csv.cpp
    src/cmd/mount.cpp
    src/cmd/armed.cpp
    src/cmd/audible.cpp
    src/cmd/jump_capable.cpp
    src/cmd/role_bitmask.cpp
    src/cmd/unit_collide.cpp
    src/cmd/unit_const_cache.cpp
    src/cmd/unit_csv.cpp
    src/cmd/unit_csv_factory.cpp
    src/cmd/unit_json_factory.cpp
    src/cmd/unit_optimize_factory.cpp
    src/cmd/json.cpp
    src/cmd/unit_functions_generic.cpp
    src/cmd/unit_generic.cpp
    src/cmd/fg_util.cpp
    src/cmd/unit_util_generic.cpp
    src/cmd/unit_xml.cpp
    src/cmd/engineVersion.cpp

    src/cmd/collision.cpp
    src/cmd/damageable.cpp
    src/cmd/drawable.cpp
    src/cmd/movable.cpp
    src/cmd/computer.cpp

    src/cmd/intelligent.cpp
    src/cmd/energetic.cpp

    src/cmd/planetary_orbit.cpp

    src/cmd/weapon_factory.cpp
    src/cmd/weapon_info.cpp
    src/cmd/mount_size.cpp
    src/cmd/weapon_type.cpp
    src/vega_py_run.cpp
)

SET(LIBCMDSERVER_SOURCES
    src/cmd/atmosphere_server.cpp
    src/cmd/base_util_server.cpp
    src/cmd/beam_server.cpp
    src/cmd/bolt_server.cpp
    src/cmd/unit_factory_server.cpp
    src/cmd/unit_functions_server.cpp
    src/cmd/unit_util_server.cpp
    src/cmd/unit_server.cpp
    src/cmd/pilot_server.cpp
    src/cmd/images_server.cpp
    src/cmd/faction_xml_server.cpp
)

SET(LIBGFXGENERIC_SOURCES
    src/gfx/texture_manager.cpp
    src/gfx/boltdrawmanager.cpp
    src/gfx/cockpit_generic.cpp
    src/gfx/lerp.cpp
    src/gfx/matrix.cpp
    src/gfx/mesh_bxm.cpp
    src/gfx/mesh_poly.cpp
    src/gfx/mesh_xml.cpp
    src/gfx/mesh.cpp
    src/gfx/quaternion.cpp
    src/gfx/soundcontainer_generic.cpp
    src/gfx/sphere_generic.cpp
    src/gfx/tvector.cpp
)

SET(LIBGFXCLIENT_SOURCES
    src/gfx/ani_texture.cpp
    src/gfx/animation.cpp
    src/gfx/aux_logo.cpp
    src/gfx/aux_palette.cpp
    src/gfx/aux_texture.cpp
    src/gfx/background.cpp
    src/gfx/camera.cpp
    src/gfx/cockpit_xml.cpp
    src/gfx/cockpit.cpp
    src/gfx/cockpit_gfx.cpp
    src/gfx/cockpit_gfx_utils.cpp
    src/gfx/coord_select.cpp
    src/gfx/env_map_gent.cpp
    src/gfx/gauge.cpp
    src/gfx/halo_system.cpp
    src/gfx/halo.cpp
    src/gfx/hud.cpp
    src/gfx/jpeg_memory.cpp
    src/gfx/loc_select.cpp
    src/gfx/masks.cpp
    src/gfx/mesh_bin.cpp
    src/gfx/mesh_fx.cpp
    src/gfx/mesh_gfx.cpp
    src/gfx/nav/criteria_xml.cpp
    src/gfx/nav/criteria.cpp
    src/gfx/nav/drawgalaxy.cpp
    src/gfx/nav/drawlist.cpp
    src/gfx/nav/drawsystem.cpp
    src/gfx/nav/navcomputer.cpp
    src/gfx/nav/navgetxmldata.cpp
    src/gfx/nav/navpath.cpp
    src/gfx/nav/navscreen.cpp
    src/gfx/nav/navscreenoccupied.cpp
    src/gfx/radar/bubble_display.cpp
    src/gfx/radar/bubble_display.h
    src/gfx/radar/dual_display.cpp
    src/gfx/radar/dual_display.h
    src/gfx/radar/null_display.h
    src/gfx/radar/plane_display.cpp
    src/gfx/radar/plane_display.h
    src/gfx/radar/radar.cpp
    src/gfx/radar/radar.h
    src/gfx/radar/sensor.cpp
    src/gfx/radar/sensor.h
    src/gfx/radar/sphere_display.cpp
    src/gfx/radar/sphere_display.h
    src/gfx/radar/track.cpp
    src/gfx/radar/track.h
    src/gfx/radar/viewarea.cpp
    src/gfx/radar/viewarea.h
    src/gfx/particle.cpp
    src/gfx/pipelined_texture.cpp
    src/gfx/quadsquare_cull.cpp
    src/gfx/quadsquare_render.cpp
    src/gfx/quadsquare_update.cpp
    src/gfx/quadsquare.cpp
    src/gfx/quadtree_xml.cpp
    src/gfx/quadtree.cpp
    src/gfx/ring.cpp
    src/gfx/occlusion.cpp
    src/gfx/screenshot.cpp
    src/gfx/soundcontainer.cpp
    src/gfx/soundcontainer_aldrv.cpp
    src/gfx/sphere.cpp
    src/gfx/sprite.cpp
    src/gfx/star.cpp
    src/gfx/stream_texture.cpp
    src/gfx/technique.cpp
    src/gfx/pass.cpp
    src/gfx/tex_transform.cpp
    src/gfx/vdu.cpp
    src/gfx/vid_file.cpp
    src/ffmpeg_init.cpp
    src/gfx/vsbox.cpp
    src/gfx/vsimage.cpp
    src/gfx/warptrail.cpp
)

SET(LIBGFXSERVER_SOURCES
    src/gfx/sphere_server.cpp
    src/gfx/vsbox_server.cpp
    src/gfx/mesh_server.cpp
    src/gfx/background_server.cpp
    src/gfx/mesh_bin_server.cpp
)

SET(LIBROOTGENERIC_SOURCES
    src/atmospheric_fog_mesh.cpp
    src/configxml.cpp
    src/easydom.cpp
    src/endianness.cpp
    src/macosx_math.cpp
    src/faction_generic.cpp
    src/faction_util_generic.cpp
    src/galaxy.cpp
    src/galaxy_gen.cpp
    src/galaxy_xml.cpp
    src/galaxy_utils.cpp
    src/hashtable.cpp
    src/lin_time.cpp
    src/load_mission.cpp
    src/pk3.cpp
    src/posh.cpp
    src/savegame.cpp
    src/system_factory.cpp
    src/star_system_generic.cpp
    src/star_system_xml.cpp
    src/stardate.cpp
    src/universe_globals.cpp
    src/universe_util_generic.cpp
    src/vs_globals.cpp
    src/vsfilesystem.cpp
    src/xml_serializer.cpp
    src/xml_support.cpp
    src/XMLDocument.cpp
    src/options.cpp
)

SET(LIBROOTCLIENT_SOURCES
    src/cg_global.cpp
    src/command.cpp
    src/config_xml.cpp
    src/debug_vs.cpp
    src/faction_util.cpp
    src/force_feedback.cpp
    src/gamemenu.cpp
    src/gfxlib_struct.cpp
    src/in_joystick.cpp
    src/in_kb.cpp
    src/in_main.cpp
    src/in_mouse.cpp
    src/in_sdl.cpp
    src/main_loop.cpp
    src/physics.cpp
    src/rendertext.cpp
    src/ship_commands.cpp
    src/star_system_jump.cpp
    src/star_system.cpp
    src/universe_util.cpp
    src/universe.cpp
)

SET(LIBROOTSERVER_SOURCES
    src/force_feedback_server.cpp
    src/faction_util_server.cpp
    src/gfxlib_struct_server.cpp
    src/libaudioserver.cpp
    src/libserver.cpp
    src/universe_util_server.cpp
)

SET(LIBAUDIO_SOURCES
    src/audio/CodecRegistry.cpp
    src/audio/Listener.cpp
    src/audio/RenderableListener.cpp
    src/audio/RenderableSource.cpp
    src/audio/Renderer.cpp
    src/audio/Scene.cpp
    src/audio/SceneManager.cpp
    src/audio/SimpleScene.cpp
    src/audio/SimpleSound.cpp
    src/audio/SimpleSource.cpp
    src/audio/SoundBuffer.cpp
    src/audio/Sound.cpp
    src/audio/Source.cpp
    src/audio/SourceTemplate.cpp
    src/audio/Stream.cpp
    src/audio/test.cpp
    src/audio/utils.cpp
    src/audio/codecs/Codec.cpp
    src/audio/codecs/FFCodec.cpp
    src/audio/codecs/FFStream.cpp
    src/audio/codecs/OggCodec.cpp
    src/audio/codecs/OggData.cpp
    src/audio/codecs/OggStream.cpp
    src/audio/renderers/OpenAL/OpenALHelpers.cpp
    src/audio/renderers/OpenAL/OpenALRenderableListener.cpp
    src/audio/renderers/OpenAL/OpenALRenderableSource.cpp
    src/audio/renderers/OpenAL/OpenALRenderableStreamingSource.cpp
    src/audio/renderers/OpenAL/OpenALRenderer.cpp
    src/audio/renderers/OpenAL/OpenALSimpleSound.cpp
    src/audio/renderers/OpenAL/OpenALStreamingSound.cpp
)


ADD_LIBRARY(vegastrike-engine_com
    ${LIBVS_LOGGING}
    ${LIBCONFIG}
    ${LIBDAMAGE}
    ${LIBRESOURCE}
    ${LIBAI_SOURCES}
    ${LIBCMD_SOURCES}
    ${LIBNET_SOURCES}
    ${LIBPYTHON_SOURCES}
    ${LIBROOTGENERIC_SOURCES}
    ${LIBSCRIPT_SOURCES}
    ${LIBGFXGENERIC_SOURCES}
)

TARGET_COMPILE_FEATURES(vegastrike-engine_com PUBLIC cxx_std_11)
IF (NEED_LINKING_AGAINST_LIBM)
    TARGET_LINK_LIBRARIES(vegastrike-engine_com m)
ENDIF()

SET(VEGASTRIKE_SOURCES
    ${LIBGUI_SOURCES}
    ${LIBNETCLIENT_SOURCES}
    ${LIBROOTCLIENT_SOURCES}
    ${LIBGFXCLIENT_SOURCES}
    ${LIBAUDIO_SOURCES}
    src/aldrv/al_globals.cpp
    src/aldrv/al_init.cpp
    src/aldrv/al_listen.cpp
    src/aldrv/al_sound.cpp
    src/cmd/ai/firekeyboard.cpp
    src/cmd/ai/flyjoystick.cpp
    src/cmd/ai/flykeyboard.cpp
    src/cmd/asteroid.cpp
    src/cmd/atmosphere.cpp
    src/cmd/base_init.cpp
    src/cmd/base_interface.cpp
    src/cmd/base_util.cpp
    src/cmd/base_write_python.cpp
    src/cmd/base_write_xml.cpp
    src/cmd/base_xml.cpp
    src/cmd/basecomputer.cpp
    src/cmd/beam.cpp
    src/cmd/bolt.cpp
    src/cmd/briefing.cpp
    src/cmd/building.cpp
    src/cmd/click_list.cpp
    src/cmd/cont_terrain.cpp
    src/cmd/missile.cpp
    src/cmd/music.cpp
    src/cmd/nebula.cpp
    src/cmd/planet.cpp
    src/cmd/script/c_alike/c_alike.tab.cpp
    src/cmd/script/c_alike/lex.yy.cpp
    src/cmd/script/director.cpp
    src/cmd/script/flightgroup.cpp
    src/cmd/script/script_call_briefing.cpp
    src/cmd/script/script_call_unit.cpp
    src/cmd/terrain.cpp
    src/cmd/unit_functions.cpp
    src/cmd/unit_util.cpp
    src/cmd/faction_xml.cpp
    src/cmd/faction_factory.cpp
    src/cmd/pilot.cpp
    src/cmd/images.cpp
    src/cmd/turret.cpp
    src/gldrv/gl_program.cpp
    src/gldrv/gl_clip.cpp
    src/gldrv/gl_fog.cpp
    src/gldrv/gl_globals.cpp
    src/gldrv/gl_init.cpp
    src/gldrv/gl_light_pick.cpp
    src/gldrv/gl_light_state.cpp
    src/gldrv/gl_light.cpp
    src/gldrv/gl_material.cpp
    src/gldrv/gl_matrix.cpp
    src/gldrv/gl_misc.cpp
    src/gldrv/gl_quad_list.cpp
    src/gldrv/gl_sphere_list.cpp
    src/gldrv/gl_state.cpp
    src/gldrv/sdds.cpp
    src/gldrv/gl_texture.cpp
    src/gldrv/gl_vertex_list.cpp
    src/gldrv/winsys.cpp
    src/main.cpp
    src/python/briefing_wrapper.cpp
)


SET(TST_LIBS vegastrike-engine_com vegastrike-OPcollide ${TST_LIBS})

IF (NOT DISABLE_CLIENT)
    ADD_EXECUTABLE(vegastrike-engine ${VEGASTRIKE_SOURCES})

    TARGET_COMPILE_FEATURES(vegastrike-engine PUBLIC cxx_std_11)
    IF (NEED_LINKING_AGAINST_LIBM)
        TARGET_LINK_LIBRARIES(vegastrike-engine m)
    ENDIF()

    ADD_DEPENDENCIES(vegastrike-engine OpenGL::GL OpenGL::GLU)

    TARGET_LINK_LIBRARIES(vegastrike-engine OpenGL::GL OpenGL::GLU ${TST_LIBS})
    IF (MSVC)
        SET_TARGET_PROPERTIES(vegastrike-engine PROPERTIES LINK_FLAGS "${TST_LFLAGS}")
    ELSE (MSVC)
        SET_TARGET_PROPERTIES(vegastrike-engine PROPERTIES LINK_FLAGS "-L/usr/lib -L/usr/local/lib ${TST_LFLAGS}")
    ENDIF (MSVC)
ENDIF (NOT DISABLE_CLIENT)

# Vssetup Sub build file
ADD_SUBDIRECTORY(setup)

# Add other utilies here
ADD_SUBDIRECTORY(objconv)

## show debug output
#GET_DIRECTORY_PROPERTY(TEMP_DIRECTORY INCLUDE_DIRECTORIES)
#MESSAGE("-- Default build type is RelWithDebInfo, no cpu opts enabled. ")
#MESSAGE("++ Building with BUILD_OPT: ${BUILD_OPT}")
#MESSAGE("++ Building with CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
#MESSAGE("Linking with : ${TST_LIBS}")
#MESSAGE("including : ${TEMP_DIRECTORY}")
## end debug output

MARK_AS_ADVANCED(FORCE
    Boost_DIR
    AVCODEC_FOUND
    AVFORMAT_FOUND
    AVUTIL_FOUND
    AVCODEC_LIBRARY
    AVCODEC_INCLUDE_DIR
    AVUTIL_INCLUDE_DIR
    AVFORMAT_LIBRARY
    AVUTIL_LIBRARY
    SWSCALE_INCLUDE_DIR
    SWSCALE_LIBRARY
    AVFORMAT_INCLUDE_DIR
    HAVE_AVFORMAT_H
    HAVE_AVCODEC_H
    HAVE_AVIO_H
    HAVE_FFMPEG_SWSCALE_H
    CMAKE_BACKWARDS_COMPATIBILITY
    FFMPEG_INCLUDE_DIR
    FFMPEG_FOUND
    FFMPEG_LIBRARIES
    FFMPEG_DEFINITIONS
    GTK3_ATK_INCLUDE_DIR
    GTK3_ATK_LIBRARY
    GTK3_CAIRO_INCLUDE_DIR
    GTK3_CAIRO_LIBRARY
    GTK3_GDK_INCLUDE_DIR
    GTK3_GDK_LIBRARY
    GTK3_GDK_PIXBUF_LIBRARY
    GTK3_GDK_PIXBUF_INCLUDE_DIR
    GTK3_GLIBCONFIG_INCLUDE_DIR
    GTK3_GLIB_INCLUDE_DIR
    GTK3_GLIB_LIBRARY
    GTK3_GMODULE_LIBRARY
    GTK3_GOBJECT_LIBRARY
    GTK3_PANGO_INCLUDE_DIR
    GTK3_PANGO_LIBRARY
    GTK3_GTHREAD_LIBRARY
    GTK3_GTKGL_INCLUDE_DIR
    GTK3_GTKGL_LIBRARY
    GTK3_GTK_INCLUDE_DIR
    GTK3_GTK_LIBRARY
    LIBRARY_OUTPUT_PATH
    OPENAL_INCLUDE_DIR
    OPENAL_LIBRARY
    SDLMAIN_LIBRARY
    SDL_INCLUDE_DIR
    SDL_LIBRARY
    UTIL_LIB
    ogg_INCLUDE_DIR
    ogg_LIBRARY
    vorbis_INCLUDE_DIR
    vorbis_LIBRARY
    vorbisfile_LIBRARY
)
INSTALL(TARGETS vegastrike-engine DESTINATION bin)

# CMake doesn't support doing symlinks directly; however,
# it's easy enough to mimic such behavior via CMake by installing
# the executable a second time using a different name.
IF (WIN32)
    INSTALL(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/vegastrike-engine" RENAME vegastrike.exe DESTINATION bin)
ELSE (WIN32)
    INSTALL(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/vegastrike-engine" RENAME vegastrike DESTINATION bin)
ENDIF (WIN32)


ADD_SUBDIRECTORY("../doc" "./doc")

# As per https://braintrekking.wordpress.com/2013/04/27/dll-hell-how-to-include-microsoft-redistributable-runtime-libraries-in-your-cmakecpack-project/
INSTALL(FILES ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} DESTINATION bin COMPONENT Libraries)

SET(CPACK_PACKAGE_NAME "Vega-Strike")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "3D OpenGL spaceflight simulator")
SET(CPACK_PACKAGE_VENDOR "Vega Strike")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../README.md")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE")
SET(CPACK_PACKAGE_VERSION_MAJOR "${VEGASTRIKE_VERSION_MAJOR}")
SET(CPACK_PACKAGE_VERSION_MINOR "${VEGASTRIKE_VERSION_MINOR}")
SET(CPACK_PACKAGE_VERSION_PATCH "${VEGASTRIKE_VERSION_PATCH}")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "VegaStrike-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}")
SET(CPACK_PACKAGE_EXECUTABLES "vegastrike" "vegastrike-engine" "vega-meshtool" "vegasettings")
SET(CPACK_CONTACT "maintainers@vega-strike.org")

SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_v${VEGASTRIKE_PKG_VERSION_STR}-Source")
SET(CPACK_SOURCE_GENERATOR "TBZ2" "TXZ")

# See https://gitlab.kitware.com/cmake/community/-/wikis/doc/cpack/PackageGenerators for generator lists
IF (WIN32 AND NOT UNIX)
    MESSAGE("-- Configuring Packaging for Windows")
    # NSIS - See https://cmake.org/cmake/help/v3.3/module/CPackNSIS.html
    # NSI bug requires at least 1 set of 4 forwards slashes??
    SET(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}\\\\icon4.ico")
    SET(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\VegaStrike.exe")
    SET(CPACK_NSIS_DISPLAY_NAME "Vega Strike")
    SET(CPACK_NSIS_HELP_LINK "https:\\\\vega-strike.org")
    SET(CPACK_NSIS_URL_INFO_ABOUT "https:\\\\vega-strike.org/about")
    SET(CPACK_NSIS_CONTACT "TBD@vega-strike.org")
    SET(CPACK_NSIS_MODIFY_PATH ON)

    # NSIS, Wix, and compressed archives (7z, Zip)
    SET(CPACK_GENERATOR "NSIS" "NSIS64" "WIX" "7Z" "ZIP")
    SET(CPACK_PACKAGE_EXECUTABLES "vegastrike.exe" "vegastrike-engine.exe" "vega-meshtool.exe" "vegasettings.exe")
ELSEIF (XCODE)
    MESSAGE("-- Configuring Packaging for Mac OS X")
    # Mac OS X
    # There's a few options for MacOSX; not sure what we want to use
    # DragNDrop -> DMG
    #  See https://cmake.org/cmake/help/v3.3/module/CPackDMG.html
    # Bundle -> Compressed Disk Image
    # PackageMaker - see https://cmake.org/cmake/help/v3.3/module/CPackPackageMaker.html
    SET(CPACK_GENERATOR "DragNDrop" "BUNDLE" "TBZ2" "TGZ" "TXZ")
ELSEIF (CMAKE_SYSTEM_NAME STREQUAL Linux)
    MESSAGE("-- Configuring Packaging for Linux")
    SET(CPACK_GENERATOR "TBZ2" "TGZ" "TXZ")
    # Linux

    SET(VEGA_STRIKE_PYTHON_VERSION_STR "py3")

    # Moved up above the boost_python detection logic - stephengtuggy 2021-09-18
    # # Detect available Linux Distros that can be built for
    # FIND_PACKAGE(LinuxDistro REQUIRED)

    # "DEB"
    IF(VS_CAN_BUILD_DEB)
        MESSAGE("-- Configuring Debian Packaging")
        # See https://cmake.org/cmake/help/v3.3/module/CPackDeb.html
        SET(CPACK_DEBIAN_PACKAGE_NAME "Vega-Strike")
        SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "maintainers@vega-strike.org")
        SET(CPACK_DEBIAN_PACKAGE_VERSION "${VEGASTRIKE_PKG_VERSION_STR}")
        # Debian Version: /etc/debian_version - <name>/<junk>
        IF (NOT LSB_EXISTS)
            SET(USE_DEBIAN_VERSION TRUE)
            FILE(READ "/etc/debian_version" DEBIAN_VERSION_DATA)
            STRING(REGEX MATCH "^(.+)\/.*$" DEBIAN_VERSION_DATA DEBIAN_RELEASE_VERSION)
        ELSE (NOT LSB_EXISTS)
            SET(USE_DEBIAN_VERSION FALSE)
            SET(DEBIAN_RELEASE_VERSION "Debian Derivative Release Version ${LSB_LINUX_DISTRIBUTION_CODENAME}")
        ENDIF (NOT LSB_EXISTS)

        # dependency.list is generated by `script/packages`. It requires the binaries to already be built to work.
        # if using `script/package` to build the packages, then it will automatically re-run `cmake` to update the data
        # for the dependencies.
        #
        # If dependency.list is not available, then a hard-coded set of dependencies is provided below.
        # Thanks to stephengtuggy for providing the static list for known distributions.
        #
        # Prefer the auto-generated list by default as it'll be more accurate
        # Fall back to the static lists when it's not available.
        MESSAGE("Looking for ${CMAKE_BINARY_DIR}/dependency.list")
        IF (EXISTS "${CMAKE_BINARY_DIR}/dependency.list")
            MESSAGE("Found Dependency file at ${CMAKE_BINARY_DIR}/dependency.list")
            # Load the dependencies from the file into the CPack variable:
            FILE(STRINGS "${CMAKE_BINARY_DIR}/dependency.list" AUTOMATED_DEBIAN_DEPENDENCY_LIST)
            FOREACH(DLD_DEPENDENCY IN LISTS AUTOMATED_DEBIAN_DEPENDENCY_LIST)
                MESSAGE("** Found Dependency ${DLD_DEPENDENCY}")
                IF (DEFINED CPACK_DEBIAN_PACKAGE_DEPENDS)
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, ${DLD_DEPENDENCY}")
                ELSE(DEFINED CPACK_DEBIAN_PACKAGE_DEPENDS)
                    IF (NOT "${DLD_DEPENDENCY}" STREQUAL "")
                        SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${DLD_DEPENDENCY}")
                    ENDIF (NOT "${DLD_DEPENDENCY}" STREQUAL "")
                ENDIF(DEFINED CPACK_DEBIAN_PACKAGE_DEPENDS)
            ENDFOREACH(DLD_DEPENDENCY)

        ELSE (EXISTS "${CMAKE_BINARY_DIR}/dependency.list")
            MESSAGE("Still need to generate dependency list")
            IF (USE_DEBIAN_VERSION)
                # Debian Dependency Chain
                IF (DEBIAN_RELEASE_VERSION STREQUAL "bullseye")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libpython3.9")
                    IF (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "1.74.0")
                    ELSE (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "${Boost_DERIVED_VERSION}")
                    ENDIF (Boost_DERIVED_VERSION STREQUAL "")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libjpeg62-turbo, libpng16-16, freeglut3, libgtk-3-0, libvorbis0a, libopenal1, libsdl-gfx1.2-5, xdg-utils, libgl1, libopengl0, ${VEGA_STRIKE_GLU_DEPENDENCY}, libboost-atomic${BOOST_VER}, libboost-chrono${BOOST_VER}, libboost-date-time${BOOST_VER}, libboost-filesystem${BOOST_VER}, libboost-log${BOOST_VER}, libboost-python${BOOST_VER}, libboost-regex${BOOST_VER}, libboost-system${BOOST_VER}, libboost-thread${BOOST_VER}")
                ELSEIF (DEBIAN_RELEASE_VERSION STREQUAL "buster")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libpython3.7")
                    IF (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "1.67.0")
                    ELSE (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "${Boost_DERIVED_VERSION}")
                    ENDIF (Boost_DERIVED_VERSION STREQUAL "")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libjpeg62-turbo, libpng16-16, freeglut3, libgtk-3-0, libvorbis0a, libopenal1, libsdl-gfx1.2-5, xdg-utils, libgl1, libopengl0, ${VEGA_STRIKE_GLU_DEPENDENCY}, libboost-atomic${BOOST_VER}, libboost-chrono${BOOST_VER}, libboost-date-time${BOOST_VER}, libboost-filesystem${BOOST_VER}, libboost-log${BOOST_VER}, libboost-python${BOOST_VER}, libboost-regex${BOOST_VER}, libboost-system${BOOST_VER}, libboost-thread${BOOST_VER}")
                ELSEIF (DEBIAN_RELEASE_VERSION STREQUAL "stretch")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libpython3.5")
                    IF (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "1.62.0")
                    ELSE (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "${Boost_DERIVED_VERSION}")
                    ENDIF (Boost_DERIVED_VERSION STREQUAL "")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libjpeg62-turbo, libpng16-16, freeglut3, libgtk-3-0, libvorbis0a, libopenal1, libsdl-gfx1.2-5, xdg-utils, libgl1, libopengl0, libboost-atomic${BOOST_VER}, libboost-chrono${BOOST_VER}, libboost-date-time${BOOST_VER}, libboost-filesystem${BOOST_VER}, libboost-log${BOOST_VER}, libboost-python${BOOST_VER}, libboost-regex${BOOST_VER}, libboost-system${BOOST_VER}, libboost-thread${BOOST_VER}")
                ELSE (DEBIAN_RELEASE_VERSION STREQUAL "bullseye")
                    MESSAGE(SEND_WARNING "!! Unsupported Debian version.")
                ENDIF (DEBIAN_RELEASE_VERSION STREQUAL "bullseye")

            ELSE (USE_DEBIAN_VERSION)
                # Ubuntu Dependency Chain
                IF (LSB_LINUX_DISTRIBUTION_CODENAME STREQUAL "bionic")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libpython3.7")
                    IF (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "1.65.1")
                    ELSE (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "${Boost_DERIVED_VERSION}")
                    ENDIF (Boost_DERIVED_VERSION STREQUAL "")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libjpeg62, libpng16-16, freeglut3, libgtk-3-0, libvorbis0a, libopenal1, libsdl-gfx1.2-5, xdg-utils, libgl1, ${VEGA_STRIKE_GLU_DEPENDENCY}, libopengl0, libboost-atomic${BOOST_VER}, libboost-chrono${BOOST_VER}, libboost-date-time${BOOST_VER}, libboost-filesystem${BOOST_VER}, libboost-log${BOOST_VER}, libboost-python${BOOST_VER}, libboost-regex${BOOST_VER}, libboost-system${BOOST_VER}, libboost-thread${BOOST_VER}")
                ELSEIF (LSB_LINUX_DISTRIBUTION_CODENAME STREQUAL "focal")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libpython3.8")
                    IF (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "1.67.0")
                    ELSE (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "${Boost_DERIVED_VERSION}")
                    ENDIF (Boost_DERIVED_VERSION STREQUAL "")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libjpeg62, libpng16-16, freeglut3, libgtk-3-0, libvorbis0a, libopenal1, libsdl-gfx1.2-5, xdg-utils, libgl1, libopengl0, libboost-atomic${BOOST_VER}, libboost-chrono${BOOST_VER}, libboost-date-time${BOOST_VER}, libboost-filesystem${BOOST_VER}, libboost-log${BOOST_VER}, libboost-python${BOOST_VER}, libboost-regex${BOOST_VER}, libboost-system${BOOST_VER}, libboost-thread${BOOST_VER}")
                ELSEIF (LSB_LINUX_DISTRIBUTION_CODENAME STREQUAL "buster") # Apparently Debian Buster has lsb_release after all
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libpython3.7")
                    IF (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "1.67.0")
                    ELSE (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "${Boost_DERIVED_VERSION}")
                    ENDIF (Boost_DERIVED_VERSION STREQUAL "")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libjpeg62-turbo, libpng16-16, freeglut3, libgtk-3-0, libvorbis0a, libopenal1, libsdl-gfx1.2-5, xdg-utils, libgl1, libopengl0, ${VEGA_STRIKE_GLU_DEPENDENCY}, libboost-atomic${BOOST_VER}, libboost-chrono${BOOST_VER}, libboost-date-time${BOOST_VER}, libboost-filesystem${BOOST_VER}, libboost-log${BOOST_VER}, libboost-python${BOOST_VER}, libboost-regex${BOOST_VER}, libboost-system${BOOST_VER}, libboost-thread${BOOST_VER}")
                ELSEIF (LSB_LINUX_DISTRIBUTION_CODENAME STREQUAL "bullseye")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libpython3.9")
                    IF (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "1.74.0")
                    ELSE (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "${Boost_DERIVED_VERSION}")
                    ENDIF (Boost_DERIVED_VERSION STREQUAL "")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libjpeg62-turbo, libpng16-16, freeglut3, libgtk-3-0, libvorbis0a, libopenal1, libsdl-gfx1.2-5, xdg-utils, libgl1, libopengl0, ${VEGA_STRIKE_GLU_DEPENDENCY}, libboost-atomic${BOOST_VER}, libboost-chrono${BOOST_VER}, libboost-date-time${BOOST_VER}, libboost-filesystem${BOOST_VER}, libboost-log${BOOST_VER}, libboost-python${BOOST_VER}, libboost-regex${BOOST_VER}, libboost-system${BOOST_VER}, libboost-thread${BOOST_VER}")
                ELSEIF (DEBIAN_RELEASE_VERSION STREQUAL "stretch")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libpython3.5")
                    IF (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "1.62.0")
                    ELSE (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "${Boost_DERIVED_VERSION}")
                    ENDIF (Boost_DERIVED_VERSION STREQUAL "")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libjpeg62-turbo, libpng16-16, freeglut3, libgtk-3-0, libvorbis0a, libopenal1, libsdl-gfx1.2-5, xdg-utils, libgl1, libopengl0, libboost-atomic${BOOST_VER}, libboost-chrono${BOOST_VER}, libboost-date-time${BOOST_VER}, libboost-filesystem${BOOST_VER}, libboost-log${BOOST_VER}, libboost-python${BOOST_VER}, libboost-regex${BOOST_VER}, libboost-system${BOOST_VER}, libboost-thread${BOOST_VER}")
                ELSEIF (LSB_LINUX_DISTRIBUTION_CODENAME STREQUAL "groovy")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libpython3.9")
                    IF (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "1.71.0")
                    ELSE (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "${Boost_DERIVED_VERSION}")
                    ENDIF (Boost_DERIVED_VERSION STREQUAL "")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libjpeg62-turbo, libpng16-16, freeglut3, libgtk-3-0, libvorbis0a, libopenal1, libsdl-gfx1.2-5, xdg-utils, libgl1, libopengl0, ${VEGA_STRIKE_GLU_DEPENDENCY}, libboost-atomic${BOOST_VER}, libboost-chrono${BOOST_VER}, libboost-date-time${BOOST_VER}, libboost-filesystem${BOOST_VER}, libboost-log${BOOST_VER}, libboost-python${BOOST_VER}, libboost-regex${BOOST_VER}, libboost-system${BOOST_VER}, libboost-thread${BOOST_VER}")
                ELSEIF (LSB_LINUX_DISTRIBUTION_CODENAME STREQUAL "hirsute")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libpython3.9")
                    IF (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "1.74.0")
                    ELSE (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "${Boost_DERIVED_VERSION}")
                    ENDIF (Boost_DERIVED_VERSION STREQUAL "")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libjpeg62-turbo, libpng16-16, freeglut3, libgtk-3-0, libvorbis0a, libopenal1, libsdl-gfx1.2-5, xdg-utils, libgl1, libopengl0, ${VEGA_STRIKE_GLU_DEPENDENCY}, libboost-atomic${BOOST_VER}, libboost-chrono${BOOST_VER}, libboost-date-time${BOOST_VER}, libboost-filesystem${BOOST_VER}, libboost-log${BOOST_VER}, libboost-python${BOOST_VER}, libboost-regex${BOOST_VER}, libboost-system${BOOST_VER}, libboost-thread${BOOST_VER}")
                ELSEIF (LSB_LINUX_DISTRIBUTION_CODENAME STREQUAL "ulyana")  # Linux Mint 20 codename "ulyana"
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libpython3.8")
                    IF (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "1.67.0")
                    ELSE (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "${Boost_DERIVED_VERSION}")
                    ENDIF (Boost_DERIVED_VERSION STREQUAL "")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libjpeg62-turbo, libpng16-16, freeglut3, libgtk-3-0, libvorbis0a, libopenal1, libsdl-gfx1.2-5, xdg-utils, libgl1, libopengl0, ${VEGA_STRIKE_GLU_DEPENDENCY}, libboost-atomic${BOOST_VER}, libboost-chrono${BOOST_VER}, libboost-date-time${BOOST_VER}, libboost-filesystem${BOOST_VER}, libboost-log${BOOST_VER}, libboost-python${BOOST_VER}, libboost-regex${BOOST_VER}, libboost-system${BOOST_VER}, libboost-thread${BOOST_VER}")
                ELSEIF (LSB_LINUX_DISTRIBUTION_CODENAME STREQUAL "impish")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libpython3.10")
                    IF (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "1.74.0")
                    ELSE (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "${Boost_DERIVED_VERSION}")
                    ENDIF (Boost_DERIVED_VERSION STREQUAL "")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libjpeg62-turbo, libpng16-16, freeglut3, libgtk-3-0, libvorbis0a, libopenal1, libsdl-gfx1.2-5, xdg-utils, libgl1, libopengl0, ${VEGA_STRIKE_GLU_DEPENDENCY}, libboost-atomic${BOOST_VER}, libboost-chrono${BOOST_VER}, libboost-date-time${BOOST_VER}, libboost-filesystem${BOOST_VER}, libboost-log${BOOST_VER}, libboost-python${BOOST_VER}, libboost-regex${BOOST_VER}, libboost-system${BOOST_VER}, libboost-thread${BOOST_VER}")
                ELSEIF (LSB_LINUX_DISTRIBUTION_CODENAME STREQUAL "jammy")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libpython3.10")
                    IF (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "1.74.0")
                    ELSE (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "${Boost_DERIVED_VERSION}")
                    ENDIF (Boost_DERIVED_VERSION STREQUAL "")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libjpeg62-turbo, libpng16-16, freeglut3, libgtk-3-0, libvorbis0a, libopenal1, libsdl-gfx1.2-5, xdg-utils, libgl1, libopengl0, ${VEGA_STRIKE_GLU_DEPENDENCY}, libboost-atomic${BOOST_VER}, libboost-chrono${BOOST_VER}, libboost-date-time${BOOST_VER}, libboost-filesystem${BOOST_VER}, libboost-log${BOOST_VER}, libboost-python${BOOST_VER}, libboost-regex${BOOST_VER}, libboost-system${BOOST_VER}, libboost-thread${BOOST_VER}")
                ELSE (LSB_LINUX_DISTRIBUTION_CODENAME STREQUAL "bionic")
                    MESSAGE(SEND_WARNING "!! Unsupported Debian derivative.")
                ENDIF (LSB_LINUX_DISTRIBUTION_CODENAME STREQUAL "bionic")

            ENDIF (USE_DEBIAN_VERSION)
        ENDIF (EXISTS "${CMAKE_BINARY_DIR}/dependency.list")

        IF (USE_DEBIAN_VERSION)
            # Add the platform (Ubuntu, Debian, etc) to the filename, including the version (stretch, jessie, etc)
            SET(CPACK_PACKAGE_FILE_NAME "${CPACK_DEBIAN_PACKAGE_NAME}_v${VEGASTRIKE_PKG_VERSION_STR}-${OpenGL_GL_PREFERENCE}-debian-${DEBIAN_RELEASE_VERSION}_${CMAKE_SYSTEM_PROCESSOR}")

        ELSE (USE_DEBIAN_VERSION)
            # Add the platform (Ubuntu, Debian, etc) to the filename, including the version (stretch, jessie, etc)
            SET(CPACK_PACKAGE_FILE_NAME "${CPACK_DEBIAN_PACKAGE_NAME}_v${VEGASTRIKE_PKG_VERSION_STR}-${OpenGL_GL_PREFERENCE}-${LSB_LINUX_DISTRIBUTION}-${LSB_LINUX_DISTRIBUTION_CODENAME}_${CMAKE_SYSTEM_PROCESSOR}")
        ENDIF (USE_DEBIAN_VERSION)
        SET(CPACK_DEBIAN_PACKAGE_SECTION "Amusements/Games")
        SET(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://www.vega-strike.org")
        SET(CPACK_GENERATOR "DEB")
    ENDIF (VS_CAN_BUILD_DEB)

    #  "RPM"
    IF (VS_CAN_BUILD_RPM)
        MESSAGE("-- Configuring RPM Packaging")
        # See https://cmake.org/cmake/help/v3.3/module/CPackRPM.html
        SET(CPACK_RPM_PACKAGE_LICENSE "GPLv3") # See ../LICENSE
        SET(CPACK_RPM_PACKAGE_URL "https://www.vega-strike.org")
        SET(CPACK_RPM_PACKAGE_NAME "Vega-Strike")

        # Figure out the filename
        IF (LINUX_CODENAME)
            SET(VEGA_STRIKE_LINUX_VERSION_STR "${LINUX_CODENAME}")
        ELSE (LINUX_CODENAME)
            SET(VEGA_STRIKE_LINUX_VERSION_STR "${LINUX_VERSION_ID}")
        ENDIF (LINUX_CODENAME)

        SET(CPACK_PACKAGE_FILE_NAME "${CPACK_RPM_PACKAGE_NAME}_v${VEGASTRIKE_PKG_VERSION_STR}-${OpenGL_GL_PREFERENCE}-${LINUX_ID}-${VEGA_STRIKE_LINUX_VERSION_STR}_${CMAKE_SYSTEM_PROCESSOR}")

        # TODO: Finish porting the script/package script over for RPM-based distros
        # Detect whether SuSe or RH/CentOS/Fedora as deps may change
        IF (LINUX_ID STREQUAL opensuse-leap)
            IF (LINUX_VERSION_ID VERSION_EQUAL 15.2 OR LINUX_VERSION_ID VERSION_EQUAL 15.3)
                SET(CPACK_RPM_PACKAGE_REQUIRES "libboost_python-py3-1_66_0, libpython3_6m1_0")
                SET(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libjpeg-turbo, libpng16-16, libglut3, libgtk-3-0, libvorbis0, libopenal0, libSDL-1_2-0, libglvnd, expat, libboost_log1_66_0, libboost_system1_66_0, libboost_filesystem1_66_0, libboost_thread1_66_0, libboost_regex1_66_0, libboost_chrono1_66_0, libboost_atomic1_66_0")
            ELSEIF (LINUX_VERSION_ID VERSION_EQUAL 15.4)
                IF (Boost_DERIVED_VERSION STREQUAL "")
                    SET(BOOST_VER "1.75.0")
                ELSE (Boost_DERIVED_VERSION STREQUAL "")
                    SET(BOOST_VER "${Boost_DERIVED_VERSION}")
                ENDIF (Boost_DERIVED_VERSION STREQUAL "")
                SET(CPACK_RPM_PACKAGE_REQUIRES "libboost_python-py3-${BOOST_VER}, libpython3_10-1_0")
                SET(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libjpeg-turbo, libpng16-16, libglut3, libgtk-3-0, libvorbis0, libopenal0, libSDL-1_2-0, libglvnd, expat, libboost_log${BOOST_VER}, libboost_system${BOOST_VER}, libboost_filesystem${BOOST_VER}, libboost_thread${BOOST_VER}, libboost_regex${BOOST_VER}, libboost_chrono${BOOST_VER}, libboost_atomic${BOOST_VER}")
            ELSE (LINUX_VERSION_ID VERSION_EQUAL 15.2 OR LINUX_VERSION_ID VERSION_EQUAL 15.3)
                MESSAGE (SEND_WARNING "!! Unsupported version of openSUSE")
                SET(CPACK_RPM_PACKAGE_REQUIRES "")
            ENDIF (LINUX_VERSION_ID VERSION_EQUAL 15.2 OR LINUX_VERSION_ID VERSION_EQUAL 15.3)
        ELSEIF (LINUX_ID STREQUAL fedora AND LINUX_VERSION_ID VERSION_GREATER_EQUAL 30 AND LINUX_VERSION_ID VERSION_LESS_EQUAL 36)
            IF (LINUX_VERSION_ID VERSION_GREATER_EQUAL 34)
                SET(CPACK_RPM_PACKAGE_REQUIRES "python3, boost-python3")
            ELSEIF (LINUX_VERSION_ID VERSION_EQUAL 32 OR LINUX_VERSION_ID VERSION_EQUAL 33)
                SET(CPACK_RPM_PACKAGE_REQUIRES "python38, boost-python3")       # python39
            ELSEIF (LINUX_VERSION_ID VERSION_EQUAL 31)
                SET(CPACK_RPM_PACKAGE_REQUIRES "python38, boost-python3")
            ELSE (LINUX_VERSION_ID VERSION_GREATER_EQUAL 34)
                SET(CPACK_RPM_PACKAGE_REQUIRES "python37, boost-python3")
            ENDIF (LINUX_VERSION_ID VERSION_GREATER_EQUAL 34)
            SET(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libjpeg-turbo, libpng, freeglut, gtk3, libvorbis, openal-soft, SDL, libglvnd, expat, boost-log, boost-system, boost-filesystem, boost-thread, boost-regex, boost-chrono, boost-atomic")
        ELSEIF (LINUX_ID STREQUAL centos AND LINUX_VERSION_ID STREQUAL "8")
            SET(CPACK_RPM_PACKAGE_REQUIRES "boost-python3")
            SET(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libjpeg-turbo, libpng, freeglut, gtk3, libvorbis, openal-soft, SDL, libglvnd, expat, boost-log, boost-system, boost-filesystem, boost-thread, boost-regex, boost-chrono, boost-atomic")
        ELSEIF (LINUX_ID STREQUAL rhel AND LINUX_VERSION_ID STREQUAL "8")
            SET(CPACK_RPM_PACKAGE_REQUIRES "boost-python3")
            SET(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libjpeg-turbo, libpng, freeglut, gtk3, libvorbis, openal-soft, SDL, libglvnd, expat, boost-log, boost-system, boost-filesystem, boost-thread, boost-regex, boost-chrono, boost-atomic")
        ELSEIF (LINUX_ID STREQUAL rocky)
            IF (LINUX_VERSION_ID VERSION_GREATER_EQUAL 8.4 AND LINUX_VERSION_ID VERSION_LESS_EQUAL 8.5)
                SET(CPACK_RPM_PACKAGE_REQUIRES "python39, boost-python3")
                SET(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libjpeg-turbo, libpng, freeglut, gtk3, libvorbis, openal-soft, SDL, libglvnd, expat, boost-log, boost-system, boost-filesystem, boost-thread, boost-regex, boost-chrono, boost-atomic")
            ELSE (LINUX_VERSION_ID VERSION_GREATER_EQUAL 8.4 AND LINUX_VERSION_ID VERSION_LESS_EQUAL 8.5)
                MESSAGE(SEND_WARNING "!! This version of Rocky Linux is not currently supported.")
                SET(CPACK_RPM_PACKAGE_REQUIRES "")
            ENDIF (LINUX_VERSION_ID VERSION_GREATER_EQUAL 8.4 AND LINUX_VERSION_ID VERSION_LESS_EQUAL 8.5)
        ELSE (LINUX_ID STREQUAL opensuse-leap)
            MESSAGE(SEND_WARNING "!! Unrecognized RPM-based Linux distribution. RPM Dependencies may be wrong for this platform.")
            SET(CPACK_RPM_PACKAGE_REQUIRES "python3")
            SET(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libjpeg, libpng, freeglut, gtk3, libvorbis, openal, SDL_mixer")
        ENDIF (LINUX_ID STREQUAL opensuse-leap)
        SET(CPACK_GENERATOR "RPM")
    ENDIF (VS_CAN_BUILD_RPM)
ELSE ()
    MESSAGE("-- Configuring Packaging for Unknown Platforms - \"${CMAKE_SYSTEM_NAME}\"")
    # Unknown Platform --> Just do the various compressed tarballs
    SET(CPACK_GENERATOR "TBZ2" "TGZ" "TXZ")
ENDIF (WIN32 AND NOT UNIX)

INCLUDE(CPack)

IF (INSTALL_GTEST OR USE_GTEST)
    MESSAGE("-- Configuring Unit Tests")
ENDIF (INSTALL_GTEST OR USE_GTEST)

IF (INSTALL_GTEST)
    INCLUDE(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        release-1.11.0
    )

    FetchContent_GetProperties(googletest)
    STRING(TOLOWER "googletest" lcName)
    IF(NOT ${lcName}_POPULATED)
        # Fetch the content using previously declared details
        FetchContent_Populate(googletest)

        # Set custom variables, policies, etc.

        # For Windows: Prevent overriding the parent project's compiler/linker settings
        SET(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

        # Bring the populated content into the build
        ADD_SUBDIRECTORY(${${lcName}_SOURCE_DIR} ${${lcName}_BINARY_DIR})
    ENDIF(NOT ${lcName}_POPULATED)
ENDIF (INSTALL_GTEST)

IF (USE_GTEST)
    ENABLE_TESTING()

    SET(TEST_NAME ${PROJECT_NAME}_tests)

    ADD_EXECUTABLE(
        ${TEST_NAME}
        src/cmd/tests/csv_tests.cpp
        src/cmd/tests/json_tests.cpp
        src/configuration/tests/configuration_tests.cpp
        src/damage/tests/health_tests.cpp
        src/damage/tests/layer_tests.cpp
        src/damage/tests/object_tests.cpp
        src/resource/tests/buy_sell.cpp
        src/resource/tests/resource_test.cpp
        src/exit_unit_tests.cpp
    )

    ADD_LIBRARY(vegastrike-testing
        ${LIBCONFIG}
        ${LIBDAMAGE}
        ${LIBRESOURCE}
        ${LIBCMD_SOURCES}
        ${LIBVS_LOGGING}
    )

    TARGET_LINK_LIBRARIES(
        ${TEST_NAME}
        gtest_main
        vegastrike-testing
        Boost::log
        Boost::log_setup
    )

    FILE(
        COPY "src/configuration/tests/vegastrike.config"
        DESTINATION ${CMAKE_BINARY_DIR}/test_assets
    )

    FILE(
        COPY "src/cmd/tests/units.json"
        DESTINATION ${CMAKE_BINARY_DIR}/test_assets
    )

    INCLUDE(GoogleTest)
    gtest_discover_tests(${TEST_NAME})
ENDIF (USE_GTEST)
