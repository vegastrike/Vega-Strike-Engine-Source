# Copyright (C) 2025 Stephen G. Tuggy and other Vega Strike contributors.
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

ADD_LIBRARY(vegastrike_gfx_generic STATIC
        texture_manager.cpp
        texture_manager.h
        boltdrawmanager.cpp
        boltdrawmanager.h
        cockpit_generic.cpp
        cockpit_generic.h
        lerp.cpp
        lerp.h
        matrix.cpp
        matrix.h
        mesh_bxm.cpp
        mesh_bxm.h
        mesh_poly.cpp
        mesh_xml.cpp
        mesh_xml.h
        mesh.cpp
        mesh.h
        quaternion.cpp
        quaternion.h
        soundcontainer_generic.cpp
        soundcontainer_generic.h
        sphere_generic.cpp
        sphere.h
        tvector.cpp
        tvector.h
        vec.h
        mesh_io.h
)

#TARGET_COMPILE_FEATURES(vegastrike_gfx_generic PUBLIC cxx_std_11)
SET_PROPERTY(TARGET vegastrike_gfx_generic PROPERTY CXX_STANDARD 14)
SET_PROPERTY(TARGET vegastrike_gfx_generic PROPERTY CXX_STANDARD_REQUIRED TRUE)
SET_PROPERTY(TARGET vegastrike_gfx_generic PROPERTY CXX_EXTENSIONS ON)
SET_PROPERTY(TARGET vegastrike_gfx_generic PROPERTY POSITION_INDEPENDENT_CODE TRUE)

TARGET_COMPILE_DEFINITIONS(vegastrike_gfx_generic PUBLIC "BOOST_ALL_DYN_LINK" "$<$<CONFIG:Debug>:BOOST_DEBUG_PYTHON>")
IF (WIN32)
    TARGET_COMPILE_DEFINITIONS(vegastrike_gfx_generic PUBLIC BOOST_USE_WINAPI_VERSION=0x0A00)
    TARGET_COMPILE_DEFINITIONS(vegastrike_gfx_generic PUBLIC _WIN32_WINNT=0x0A00)
    TARGET_COMPILE_DEFINITIONS(vegastrike_gfx_generic PUBLIC WINVER=0x0A00)
    TARGET_COMPILE_DEFINITIONS(vegastrike_gfx_generic PUBLIC "$<$<CONFIG:Debug>:Py_DEBUG>")
ENDIF()

TARGET_INCLUDE_DIRECTORIES(vegastrike_gfx_generic SYSTEM PRIVATE ${TST_INCLUDES})
TARGET_INCLUDE_DIRECTORIES(vegastrike_gfx_generic PRIVATE
        # VS engine headers
        ${Vega_Root_SOURCE_DIR}
        ${Vega_Root_SOURCE_DIR}/engine
        ${Vega_Root_SOURCE_DIR}/engine/src
        # Library Headers
        ${Vega_Root_SOURCE_DIR}/libraries
        # CMake Artifacts
        ${Vega_Root_BINARY_DIR}
        ${Vega_Root_BINARY_DIR}/src
        ${Vega_Root_BINARY_DIR}/engine
        ${Vega_Root_BINARY_DIR}/engine/src
)
IF (NEED_LINKING_AGAINST_LIBM)
    TARGET_LINK_LIBRARIES(vegastrike_gfx_generic PRIVATE m)
ENDIF()
TARGET_LINK_LIBRARIES(vegastrike_gfx_generic PRIVATE ${TST_LIBS} GLUT::GLUT)
TARGET_LINK_LIBRARIES(vegastrike_gfx_generic PUBLIC vegastrike_root_generic)
