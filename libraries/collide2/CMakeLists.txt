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

ADD_LIBRARY(vegastrike-OPcollide SHARED
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
TARGET_INCLUDE_DIRECTORIES(vegastrike-OPcollide PRIVATE
        # VS engine headers
        $<BUILD_INTERFACE:${Vega_Strike_SOURCE_DIR}/engine>
        $<BUILD_INTERFACE:${Vega_Strike_SOURCE_DIR}/engine/src>
        $<BUILD_INTERFACE:${Vega_Strike_SOURCE_DIR}/engine/src/cmd>
        $<BUILD_INTERFACE:${Vega_Strike_SOURCE_DIR}/engine/src/components>
        $<BUILD_INTERFACE:${Vega_Strike_SOURCE_DIR}/engine/src/configuration>
        $<BUILD_INTERFACE:${Vega_Strike_SOURCE_DIR}/engine/src/damage>
        $<BUILD_INTERFACE:${Vega_Strike_SOURCE_DIR}/engine/src/resource>
        $<BUILD_INTERFACE:${Vega_Strike_SOURCE_DIR}/engine/src/python/base_computer>
        $<BUILD_INTERFACE:${Vega_Strike_SOURCE_DIR}/engine/src/python/config>
        # Library Headers
        $<BUILD_INTERFACE:${Vega_Strike_SOURCE_DIR}/libraries/collide2>
        $<BUILD_INTERFACE:${Vega_Strike_SOURCE_DIR}/libraries/collide2/csgeom2>
        $<BUILD_INTERFACE:${Vega_Strike_SOURCE_DIR}/libraries/collide2/Ice>
        # CMake Artifacts
        $<BUILD_INTERFACE:${Vega_Strike_BINARY_DIR}>
        $<BUILD_INTERFACE:${Vega_Strike_BINARY_DIR}/src>
        $<BUILD_INTERFACE:${Vega_Strike_BINARY_DIR}/engine>
        $<BUILD_INTERFACE:${Vega_Strike_BINARY_DIR}/engine/src>
)
TARGET_INCLUDE_DIRECTORIES(vegastrike-OPcollide SYSTEM PRIVATE ${TST_INCLUDES})
TARGET_INCLUDE_DIRECTORIES(vegastrike-OPcollide SYSTEM PRIVATE ${VSE_TST_INCLUDES})

#TARGET_COMPILE_FEATURES(vegastrike-OPcollide PUBLIC cxx_std_11)
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
