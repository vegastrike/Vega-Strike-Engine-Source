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

ADD_LIBRARY(vegastrike_root_generic STATIC
        atmospheric_fog_mesh.cpp
        atmospheric_fog_mesh.h
        configxml.cpp
        configxml.h
        easydom.cpp
        easydom.h
        macosx_math.cpp
        macosx_math.h
        faction_generic.cpp
        faction_generic.h
        faction_util_generic.cpp
        galaxy.cpp
        galaxy.h
        galaxy_gen.cpp
        galaxy_gen.h
        galaxy_xml.cpp
        galaxy_xml.h
        galaxy_utils.cpp
        lin_time.cpp
        lin_time.h
        load_mission.cpp
        load_mission.h
        pk3.cpp
        pk3.h
        system_factory.cpp
        system_factory.h
        star_system_xml.cpp
        stardate.cpp
        stardate.h
        universe_globals.cpp
        universe_globals.h
        universe_util_generic.cpp
        vs_globals.cpp
        vs_globals.h
        xml_serializer.cpp
        xml_serializer.h
        xml_support.cpp
        xml_support.h
        XMLDocument.cpp
        XMLDocument.h
        options.cpp
        options.h
)

#TARGET_COMPILE_FEATURES(vegastrike_root_generic PUBLIC cxx_std_11)
SET_PROPERTY(TARGET vegastrike_root_generic PROPERTY CXX_STANDARD 14)
SET_PROPERTY(TARGET vegastrike_root_generic PROPERTY CXX_STANDARD_REQUIRED TRUE)
SET_PROPERTY(TARGET vegastrike_root_generic PROPERTY CXX_EXTENSIONS ON)
SET_PROPERTY(TARGET vegastrike_root_generic PROPERTY POSITION_INDEPENDENT_CODE TRUE)

TARGET_COMPILE_DEFINITIONS(vegastrike_root_generic PUBLIC "BOOST_ALL_DYN_LINK" "$<$<CONFIG:Debug>:BOOST_DEBUG_PYTHON>")
IF (WIN32)
    TARGET_COMPILE_DEFINITIONS(vegastrike_root_generic PUBLIC BOOST_USE_WINAPI_VERSION=0x0A00)
    TARGET_COMPILE_DEFINITIONS(vegastrike_root_generic PUBLIC _WIN32_WINNT=0x0A00)
    TARGET_COMPILE_DEFINITIONS(vegastrike_root_generic PUBLIC WINVER=0x0A00)
    TARGET_COMPILE_DEFINITIONS(vegastrike_root_generic PUBLIC "$<$<CONFIG:Debug>:Py_DEBUG>")
ENDIF()

TARGET_INCLUDE_DIRECTORIES(vegastrike_root_generic SYSTEM PRIVATE ${TST_INCLUDES})
TARGET_INCLUDE_DIRECTORIES(vegastrike_root_generic PRIVATE
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
    TARGET_LINK_LIBRARIES(vegastrike_root_generic PRIVATE m)
ENDIF()
TARGET_LINK_LIBRARIES(vegastrike_root_generic PRIVATE ${TST_LIBS} GLUT::GLUT)
