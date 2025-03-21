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

ADD_LIBRARY(vegastrike_cmd STATIC
        alphacurve.cpp
        alphacurve.h
        carrier.cpp
        carrier.h
        collection.cpp
        collection.h
        collide_map.cpp
        collide_map.h
        collide.cpp
        collide.h
        container.cpp
        container.h
        csv.cpp
        csv.h
        mount.cpp
        mount.h
        armed.cpp
        armed.h
        audible.cpp
        audible.h
        jump_capable.cpp
        jump_capable.h
        role_bitmask.cpp
        role_bitmask.h
        unit_collide.cpp
        unit_collide.h
        unit_const_cache.cpp
        unit_const_cache.h
        unit_csv.cpp
        unit_csv.h
        unit_csv_factory.cpp
        unit_csv_factory.h
        unit_json_factory.cpp
        unit_json_factory.h
        unit_optimize_factory.cpp
        unit_optimize_factory.h
        unit_functions_generic.cpp
        unit_generic.cpp
        unit_generic.h
        upgradeable_unit.cpp
        upgradeable_unit.h
        fg_util.cpp
        fg_util.h
        unit_util_generic.cpp
        unit_xml.cpp
        unit_xml.h
        engineVersion.cpp
        engineVersion.h

        collision.cpp
        collision.h
        damageable.cpp
        damageable.h
        drawable.cpp
        drawable.h
        movable.cpp
        movable.h
        computer.cpp
        computer.h

        intelligent.cpp
        intelligent.h
        energetic.cpp
        energetic.h

        planetary_orbit.cpp
        planetary_orbit.h

        weapon_factory.cpp
        weapon_factory.h
        weapon_info.cpp
        weapon_info.h
        mount_size.cpp
        mount_size.h
        weapon_type.cpp
        weapon_type.h

        vega_py_run.cpp
        vega_py_run.h
)
TARGET_INCLUDE_DIRECTORIES(vegastrike_cmd PRIVATE
        # VS engine headers
        ${Vega_Root_SOURCE_DIR}
        ${Vega_Root_SOURCE_DIR}/engine
        ${Vega_Root_SOURCE_DIR}/engine/src
        ${Vega_Root_SOURCE_DIR}/engine/src/cmd
        ${Vega_Root_SOURCE_DIR}/engine/src/components
        ${Vega_Root_SOURCE_DIR}/engine/src/configuration
        ${Vega_Root_SOURCE_DIR}/engine/src/damage
        ${Vega_Root_SOURCE_DIR}/engine/src/gfx
        ${Vega_Root_SOURCE_DIR}/engine/src/resource
        ${Vega_Root_SOURCE_DIR}/engine/src/python/base_computer
        ${Vega_Root_SOURCE_DIR}/engine/src/python/config
        # Library Headers
        ${Vega_Root_SOURCE_DIR}/libraries
        ${Vega_Root_SOURCE_DIR}/libraries/cmd
        ${Vega_Root_SOURCE_DIR}/libraries/collide2
        ${Vega_Root_SOURCE_DIR}/libraries/collide2/csgeom2
        ${Vega_Root_SOURCE_DIR}/libraries/collide2/Ice
        ${Vega_Root_SOURCE_DIR}/libraries/gfx_generic
        ${Vega_Root_SOURCE_DIR}/libraries/root_generic
        # CMake Artifacts
        ${Vega_Root_BINARY_DIR}
        ${Vega_Root_BINARY_DIR}/src
        ${Vega_Root_BINARY_DIR}/engine
        ${Vega_Root_BINARY_DIR}/engine/src
)
TARGET_INCLUDE_DIRECTORIES(vegastrike_cmd SYSTEM PRIVATE ${TST_INCLUDES})

#TARGET_COMPILE_FEATURES(vegastrike_cmd PUBLIC cxx_std_11)
SET_PROPERTY(TARGET vegastrike_cmd PROPERTY POSITION_INDEPENDENT_CODE TRUE)

IF (NEED_LINKING_AGAINST_LIBM)
    TARGET_LINK_LIBRARIES(vegastrike_cmd m)
ENDIF()
TARGET_LINK_LIBRARIES(vegastrike_cmd ${TST_LIBS})

TARGET_COMPILE_DEFINITIONS(vegastrike_cmd PUBLIC "BOOST_ALL_DYN_LINK" "$<$<CONFIG:Debug>:BOOST_DEBUG_PYTHON>")
IF (WIN32)
    TARGET_COMPILE_DEFINITIONS(vegastrike_cmd PUBLIC BOOST_USE_WINAPI_VERSION=0x0A00)
    TARGET_COMPILE_DEFINITIONS(vegastrike_cmd PUBLIC _WIN32_WINNT=0x0A00)
    TARGET_COMPILE_DEFINITIONS(vegastrike_cmd PUBLIC WINVER=0x0A00)
    TARGET_COMPILE_DEFINITIONS(vegastrike_cmd PUBLIC "$<$<CONFIG:Debug>:Py_DEBUG>")
ENDIF()
