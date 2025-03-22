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

MESSAGE(STATUS "PROCESSING DIRECTORY: ${CMAKE_CURRENT_SOURCE_DIR}")

ADD_LIBRARY(vegastrike-OPcollide STATIC
        Ice/IceAABB.cpp
        Ice/IceAABB.h
        Ice/IceAxes.h
        Ice/IceBoundingSphere.h
        Ice/IceContainer.cpp
        Ice/IceContainer.h
        Ice/IceFPU.h
        Ice/IceHPoint.cpp
        Ice/IceHPoint.h
        Ice/IceIndexedTriangle.cpp
        Ice/IceIndexedTriangle.h
        Ice/IceLSS.h
        Ice/IceMatrix3x3.cpp
        Ice/IceMatrix3x3.h
        Ice/IceMatrix4x4.cpp
        Ice/IceMatrix4x4.h
        Ice/IceMemoryMacros.h
        Ice/IceOBB.cpp
        Ice/IceOBB.h
        Ice/IcePairs.h
        Ice/IcePlane.cpp
        Ice/IcePlane.h
        Ice/IcePoint.cpp
        Ice/IcePoint.h
        Ice/IcePreprocessor.h
        Ice/IceRandom.cpp
        Ice/IceRandom.h
        Ice/IceRay.cpp
        Ice/IceRay.h
        Ice/IceRevisitedRadix.cpp
        Ice/IceRevisitedRadix.h
        Ice/IceSegment.cpp
        Ice/IceSegment.h
        Ice/IceTriangle.cpp
        Ice/IceTriangle.h
        Ice/IceTrilist.h
        Ice/IceTypes.h
        Ice/IceUtils.cpp
        Ice/IceUtils.h
        csgeom2/opmath.h
        csgeom2/optransfrm.h
        csgeom2/opbox.cpp
        csgeom2/opbox.h
        csgeom2/opmatrix3.cpp
        csgeom2/opmatrix3.h
        csgeom2/opvector3.cpp
        csgeom2/opvector3.h
        basecollider.h
        OPC_Common.h
        OPC_IceHook.h
        OPC_BoxBoxOverlap.h
        OPC_LSSAABBOverlap.h
        OPC_LSSTriOverlap.h
        OPC_PlanesAABBOverlap.h
        OPC_PlanesTriOverlap.h
        OPC_RayAABBOverlap.h
        OPC_RayTriOverlap.h
        OPC_SphereAABBOverlap.h
        OPC_SphereTriOverlap.h
        OPC_TriBoxOverlap.h
        OPC_TriTriOverlap.h
        OPC_AABBCollider.cpp
        OPC_AABBCollider.h
        OPC_AABBTree.cpp
        OPC_AABBTree.h
        OPC_BaseModel.cpp
        OPC_BaseModel.h
        OPC_BoxPruning.cpp
        OPC_BoxPruning.h
        OPC_Collider.cpp
        OPC_Collider.h
        OPC_HybridModel.cpp
        OPC_HybridModel.h
        OPC_LSSCollider.cpp
        OPC_LSSCollider.h
        OPC_MeshInterface.cpp
        OPC_MeshInterface.h
        OPC_Model.cpp
        OPC_Model.h
        OPC_OBBCollider.cpp
        OPC_OBBCollider.h
        OPC_OptimizedTree.cpp
        OPC_OptimizedTree.h
        OPC_Picking.cpp
        OPC_Picking.h
        OPC_PlanesCollider.cpp
        OPC_PlanesCollider.h
        OPC_RayCollider.cpp
        OPC_RayCollider.h
        OPC_SphereCollider.cpp
        OPC_SphereCollider.h
        OPC_SweepAndPrune.cpp
        OPC_SweepAndPrune.h
        OPC_TreeBuilders.cpp
        OPC_TreeBuilders.h
        OPC_TreeCollider.cpp
        OPC_TreeCollider.h
        OPC_VolumeCollider.cpp
        OPC_VolumeCollider.h
        CSopcodecollider.cpp
        CSopcodecollider.h
        Opcode.h
        opcodealgorithms.h
        opcodeextern.h
        opcodegarray.h
        opcodeqint.h
        opcodeqsqrt.h
        opcodesysdef.h
        opcodetypes.h
        segmentcell.h
)
TARGET_INCLUDE_DIRECTORIES(vegastrike-OPcollide PRIVATE
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
        ${Vega_Root_SOURCE_DIR}/engine/src/python
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
TARGET_INCLUDE_DIRECTORIES(vegastrike-OPcollide SYSTEM PRIVATE ${TST_INCLUDES})

#TARGET_COMPILE_FEATURES(vegastrike-OPcollide PUBLIC cxx_std_11)
SET_PROPERTY(TARGET vegastrike-OPcollide PROPERTY POSITION_INDEPENDENT_CODE TRUE)
IF (NEED_LINKING_AGAINST_LIBM)
    TARGET_LINK_LIBRARIES(vegastrike-OPcollide m)
ENDIF()
TARGET_LINK_LIBRARIES(vegastrike-OPcollide ${TST_LIBS} GLUT::GLUT vegastrike_gfx_generic)

TARGET_COMPILE_DEFINITIONS(vegastrike-OPcollide PUBLIC "BOOST_ALL_DYN_LINK" "$<$<CONFIG:Debug>:BOOST_DEBUG_PYTHON>")
IF (WIN32)
    TARGET_COMPILE_DEFINITIONS(vegastrike-OPcollide PUBLIC BOOST_USE_WINAPI_VERSION=0x0A00)
    TARGET_COMPILE_DEFINITIONS(vegastrike-OPcollide PUBLIC _WIN32_WINNT=0x0A00)
    TARGET_COMPILE_DEFINITIONS(vegastrike-OPcollide PUBLIC WINVER=0x0A00)
    TARGET_COMPILE_DEFINITIONS(vegastrike-OPcollide PUBLIC "$<$<CONFIG:Debug>:Py_DEBUG>")
ENDIF()
