# Copyright (C) 2025 Benjamen Meyer and other Vega Strike contributors.
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

MESSAGE("PROCESSING DIRECTORY: ${CMAKE_CURRENT_SOURCE_DIR}")

LIST(APPEND
    OPCOLLIDE_INCLUDES
    # collide2 headers
    ${CMAKE_CURRENT_SOURCE_DIR}
    # VS engine headers
    #${CMAKE_SOURCE_DIR}/engine
    ${CMAKE_SOURCE_DIR}/engine/src
    ${CMAKE_SOURCE_DIR}/engine/src/cmd
    ${CMAKE_SOURCE_DIR}/engine/src/components
    ${CMAKE_SOURCE_DIR}/engine/src/damage
    ${CMAKE_SOURCE_DIR}/engine/src/resource
    ${CMAKE_SOURCE_DIR}/engine/src/python/base_computer
    ${CMAKE_SOURCE_DIR}/engine/src/python/config
    # CMake Artifacts
    ${Vega_Strike_BINARY_DIR}
    ${Vega_Strike_BINARY_DIR}/engine
    ${Vega_Strike_BINARY_DIR}/src
)
MESSAGE("OpCollide Includes: ${OPCOLLIDE_INCLUDES}")

ADD_LIBRARY(vegastrike-OPcollide STATIC
    Ice/IceAABB.cpp
    Ice/IceContainer.cpp
    Ice/IceHPoint.cpp
    Ice/IceIndexedTriangle.cpp
    Ice/IceMatrix3x3.cpp
    Ice/IceMatrix4x4.cpp
    Ice/IceOBB.cpp
    Ice/IcePlane.cpp
    Ice/IcePoint.cpp
    Ice/IceRandom.cpp
    Ice/IceRay.cpp
    Ice/IceRevisitedRadix.cpp
    Ice/IceSegment.cpp
    Ice/IceTriangle.cpp
    Ice/IceUtils.cpp
    csgeom2/opbox.cpp
    csgeom2/opmatrix3.cpp
    csgeom2/opvector3.cpp
    OPC_AABBCollider.cpp
    OPC_AABBTree.cpp
    OPC_BaseModel.cpp
    OPC_BoxPruning.cpp
    OPC_Collider.cpp
    OPC_HybridModel.cpp
    OPC_LSSCollider.cpp
    OPC_MeshInterface.cpp
    OPC_Model.cpp
    OPC_OBBCollider.cpp
    OPC_OptimizedTree.cpp
    OPC_Picking.cpp
    OPC_PlanesCollider.cpp
    OPC_RayCollider.cpp
    OPC_SphereCollider.cpp
    OPC_SweepAndPrune.cpp
    OPC_TreeBuilders.cpp
    OPC_TreeCollider.cpp
    OPC_VolumeCollider.cpp
    CSopcodecollider.cpp
)
TARGET_INCLUDE_DIRECTORIES(vegastrike-OPcollide PUBLIC ${OPCOLLIDE_INCLUDES})

TARGET_COMPILE_FEATURES(vegastrike-OPcollide PUBLIC cxx_std_11)
SET_PROPERTY(TARGET vegastrike-OPcollide PROPERTY POSITION_INDEPENDENT_CODE TRUE)
IF (NEED_LINKING_AGAINST_LIBM)
    TARGET_LINK_LIBRARIES(vegastrike-OPcollide m)
ENDIF()

TARGET_COMPILE_DEFINITIONS(vegastrike-OPcollide PUBLIC "BOOST_ALL_DYN_LINK" "$<$<CONFIG:Debug>:BOOST_DEBUG_PYTHON>")
IF (WIN32)
    TARGET_COMPILE_DEFINITIONS(vegastrike-OPcollide PUBLIC BOOST_USE_WINAPI_VERSION=0x0A00)
    TARGET_COMPILE_DEFINITIONS(vegastrike-OPcollide PUBLIC _WIN32_WINNT=0x0A00)
    TARGET_COMPILE_DEFINITIONS(vegastrike-OPcollide PUBLIC WINVER=0x0A00)
    TARGET_COMPILE_DEFINITIONS(vegastrike-OPcollide PUBLIC "$<$<CONFIG:Debug>:Py_DEBUG>")
ENDIF()
