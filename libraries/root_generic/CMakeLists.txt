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

ADD_LIBRARY(vegastrike_root_generic SHARED
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
        posh.cpp
        posh.h
        savegame.cpp
        savegame.h
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
        vsfilesystem.cpp
        vsfilesystem.h
        xml_serializer.cpp
        xml_serializer.h
        xml_support.cpp
        xml_support.h
        XMLDocument.cpp
        XMLDocument.h
        options.cpp
        options.h
)
TARGET_INCLUDE_DIRECTORIES(vegastrike_root_generic PRIVATE
        # VS engine headers
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
TARGET_INCLUDE_DIRECTORIES(vegastrike_root_generic SYSTEM PRIVATE ${TST_INCLUDES})
TARGET_INCLUDE_DIRECTORIES(vegastrike_root_generic SYSTEM PRIVATE ${VSE_TST_INCLUDES})
