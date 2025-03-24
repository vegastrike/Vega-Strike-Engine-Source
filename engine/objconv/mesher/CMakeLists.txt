##
# CMakeLists.txt
#
# Copyright (C) 2001-2023 Daniel Horn, pyramid3d, Stephen G. Tuggy,
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

FIND_PACKAGE(EXPAT REQUIRED)
IF (EXPAT_FOUND)
    SET(MESHER_SOURCES
        Converter.cpp
        from_obj.cpp
        to_BFXM.cpp
        from_BFXM.cpp
        main.cpp
        to_obj.cpp
        to_OgreMesh.cpp
        Modules/Convert.cpp
        Modules/OldSyntax.cpp
        Modules/Dims.cpp
        Modules/BFXM_to_Wavefront.cpp
        Modules/BFXM_to_XMesh.cpp
        Modules/XMesh_to_BFXM.cpp
        Modules/XMesh_to_Ogre.cpp
        Modules/Wavefront_to_BFXM.cpp
        PrecompiledHeaders/Converter.cpp
        ${Vega_Root_SOURCE_DIR}/engine/src/hashtable.cpp
        ${Vega_Root_SOURCE_DIR}/engine/src/xml_support.cpp
    )

    # Still need to add CEGUI and OGRE find packages
    #if have OGRE to_OgreMesh.cpp

    ADD_EXECUTABLE(mesher ${MESHER_SOURCES})
    TARGET_INCLUDE_DIRECTORIES(mesher SYSTEM PRIVATE ${TST_INCLUDES})
    TARGET_INCLUDE_DIRECTORIES(mesher PRIVATE
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
    SET_TARGET_PROPERTIES(mesher PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    SET_PROPERTY(TARGET mesher PROPERTY POSITION_INDEPENDENT_CODE TRUE)
    TARGET_COMPILE_DEFINITIONS(mesher PUBLIC "BOOST_ALL_DYN_LINK" "$<$<CONFIG:Debug>:BOOST_DEBUG_PYTHON>" "$<$<CONFIG:Debug>:Py_DEBUG>")
    IF (WIN32)
        TARGET_COMPILE_DEFINITIONS(mesher PUBLIC BOOST_USE_WINAPI_VERSION=0x0A00)
        TARGET_COMPILE_DEFINITIONS(mesher PUBLIC _WIN32_WINNT=0x0A00)
        TARGET_COMPILE_DEFINITIONS(mesher PUBLIC WINVER=0x0A00)
    ENDIF()

    # OGRE already looked for by parent
    # USE_SYSTEM_BOOST already set by main script
    MESSAGE(STATUS "${OGRE_FOUND} ${USE_SYSTEM_BOOST}")
    IF (OGRE_FOUND AND NOT USE_SYSTEM_BOOST)
        # Ogre usage in mesher requires boost::system
        ADD_DEPENDENCIES(mesher boost_system)
        SET(MESHER_LIBRARIES ${MESHER_LIBRARIES} boost_system)
    ENDIF (OGRE_FOUND AND NOT USE_SYSTEM_BOOST)

    TARGET_LINK_LIBRARIES(mesher ${MESHER_LIBRARIES} ${EXPAT_LIBRARIES} ${OGRE_LIBRARIES})
    SET_TARGET_PROPERTIES(mesher PROPERTIES LINK_FLAGS "-L/usr/lib")
ELSE (EXPAT_FOUND)
    MESSAGE(WARNING "Not going to be able to compile mesher, no expat found")
ENDIF(EXPAT_FOUND)
