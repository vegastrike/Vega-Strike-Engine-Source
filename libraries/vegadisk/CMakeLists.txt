##
# CMakeLists.txt
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

MESSAGE(STATUS "PROCESSING DIRECTORY: ${CMAKE_CURRENT_SOURCE_DIR}")

ADD_LIBRARY(vegastrike_vegadisk STATIC
    diskobject.h
    diskobject.cpp
    pk3.cpp
    pk3.h
    savegame.cpp
    savegame.h
    vsfilesystem.cpp
    vsfilesystem.h
)

#TARGET_COMPILE_FEATURES(vegastrike_gfx_generic PUBLIC cxx_std_11)
SET_PROPERTY(TARGET vegastrike_vegadisk PROPERTY CXX_STANDARD 14)
SET_PROPERTY(TARGET vegastrike_vegadisk PROPERTY CXX_STANDARD_REQUIRED TRUE)
SET_PROPERTY(TARGET vegastrike_vegadisk PROPERTY CXX_EXTENSIONS ON)
SET_PROPERTY(TARGET vegastrike_vegadisk PROPERTY POSITION_INDEPENDENT_CODE TRUE)

TARGET_COMPILE_DEFINITIONS(vegastrike_vegadisk PUBLIC "BOOST_ALL_DYN_LINK" "$<$<CONFIG:Debug>:BOOST_DEBUG_PYTHON>")
IF (WIN32)
    TARGET_COMPILE_DEFINITIONS(vegastrike_vegadisk PUBLIC BOOST_USE_WINAPI_VERSION=0x0A00)
    TARGET_COMPILE_DEFINITIONS(vegastrike_vegadisk PUBLIC _WIN32_WINNT=0x0A00)
    TARGET_COMPILE_DEFINITIONS(vegastrike_vegadisk PUBLIC WINVER=0x0A00)
    TARGET_COMPILE_DEFINITIONS(vegastrike_vegadisk PUBLIC "$<$<CONFIG:Debug>:Py_DEBUG>")
ENDIF()

TARGET_INCLUDE_DIRECTORIES(vegastrike_vegadisk SYSTEM PRIVATE ${TST_INCLUDES})
TARGET_INCLUDE_DIRECTORIES(vegastrike_vegadisk PRIVATE
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
IF (NEED_LINKING_AGAINST_LIBM)
    TARGET_LINK_LIBRARIES(vegastrike_vegadisk PRIVATE m)
ENDIF()
TARGET_LINK_LIBRARIES(vegastrike_vegadisk PRIVATE ${TST_LIBS} GLUT::GLUT)
TARGET_LINK_LIBRARIES(vegastrike_vegadisk PUBLIC vegastrike_root_generic)

IF (USE_GTEST)
    SET(TEST_NAME ${PROJECT_NAME}_vegadisk_tests)
    ADD_EXECUTABLE(
        ${TEST_NAME}
        tests/diskobject_tests.cpp
    )
    TARGET_INCLUDE_DIRECTORIES(${TEST_NAME} SYSTEM PRIVATE ${VSE_TST_INCLUDES})
    TARGET_INCLUDE_DIRECTORIES(${TEST_NAME} PRIVATE
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
    SET_TARGET_PROPERTIES(${TEST_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${Vega_Strike_BINARY_DIR}/)

    #TARGET_COMPILE_FEATURES(${TEST_NAME} PUBLIC cxx_std_11)
    SET_PROPERTY(TARGET ${TEST_NAME} PROPERTY CXX_STANDARD 14)
    SET_PROPERTY(TARGET ${TEST_NAME} PROPERTY CXX_STANDARD_REQUIRED TRUE)
    SET_PROPERTY(TARGET ${TEST_NAME} PROPERTY POSITION_INDEPENDENT_CODE TRUE)
    IF (NEED_LINKING_AGAINST_LIBM)
        TARGET_LINK_LIBRARIES(${TEST_NAME} m)
    ENDIF()

    TARGET_LINK_LIBRARIES(
            ${TEST_NAME}
            ${OPENAL_LIBRARY}
            ${Vorbis_LIBRARIES}
            ${JPEG_LIBRARIES}
            ${PNG_LIBRARIES}
            ${Boost_LIBRARIES}
            ${Python3_LIBRARIES}
            gtest_main
            #$<TARGET_OBJECTS:vegastrike-testing>
            vegastrike_cmd
            vegastrike_gfx_generic
            Boost::log
            Boost::log_setup
            Boost::json
    )
    TARGET_COMPILE_DEFINITIONS(${TEST_NAME} PUBLIC "BOOST_ALL_DYN_LINK" "$<$<CONFIG:Debug>:BOOST_DEBUG_PYTHON>")
    IF (WIN32)
        TARGET_COMPILE_DEFINITIONS(${TEST_NAME} PUBLIC BOOST_USE_WINAPI_VERSION=0x0A00)
        TARGET_COMPILE_DEFINITIONS(${TEST_NAME} PUBLIC _WIN32_WINNT=0x0A00)
        TARGET_COMPILE_DEFINITIONS(${TEST_NAME} PUBLIC WINVER=0x0A00)
        TARGET_COMPILE_DEFINITIONS(${TEST_NAME} PUBLIC "$<$<CONFIG:Debug>:Py_DEBUG>")
    ENDIF()

    INCLUDE(GoogleTest)
    gtest_discover_tests(${TEST_NAME} PROPERTIES DISCOVERY_TIMEOUT 30)
ENDIF (USE_GTEST)
