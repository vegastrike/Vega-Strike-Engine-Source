/*
 * Opcode.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 Pierre Terdiman, Daniel Horn, pyramid3d,
 * Stephen G. Tuggy, Benjamen R. Meyer, and other Vega Strike contributors.
 * Copyright (C) 2026 The Vega Strike Contributors
 *
 * This file is part of OPCODE - Optimized Collision Detection
 * (http://www.codercorner.com/Opcode.htm) and has been
 * incorporated into Vega Strike
 * (https://github.com/vegastrike/Vega-Strike-Engine-Source).
 *
 * Public Domain
 */

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Main file for Opcode.dll.
 *  \file		Opcode.h
 *  \author		Pierre Terdiman
 *  \date		March, 20, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef VEGA_STRIKE_ENGINE_CMD_COLLSION2_OPCODE_H
#define VEGA_STRIKE_ENGINE_CMD_COLLSION2_OPCODE_H

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "collide2/opcodesysdef.h"
#include "collide2/opcodetypes.h"
#include "gfx_generic/quaternion.h"
#include "gfx_generic/tvector.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Preprocessor

#define OPCODE_API

#define ICE_NO_DLL
#define ICE_DONT_CHECK_COMPILER_OPTIONS

#include "Ice/IceContainer.h"
#include "Ice/IcePairs.h"
#include "Ice/IceRandom.h"
#include "Ice/IceRevisitedRadix.h"
#include "Ice/IceUtils.h"
#include "OPC_IceHook.h"

namespace Opcode {

#include "Ice/IceAABB.h"
#include "Ice/IceAxes.h"
#include "Ice/IceBoundingSphere.h"
#include "Ice/IceHPoint.h"
#include "Ice/IceIndexedTriangle.h"
#include "Ice/IceLSS.h"
#include "Ice/IceMatrix3x3.h"
#include "Ice/IceMatrix4x4.h"
#include "Ice/IceOBB.h"
#include "Ice/IcePlane.h"
#include "Ice/IcePoint.h"
#include "Ice/IceRay.h"
#include "Ice/IceSegment.h"
#include "Ice/IceTriangle.h"
#include "Ice/IceTrilist.h"

// Bulk-of-the-work
#include "OPC_Common.h"
#include "OPC_MeshInterface.h"
#include "OPC_Settings.h"
// Builders
#include "OPC_TreeBuilders.h"
// Trees
#include "OPC_AABBTree.h"
#include "OPC_OptimizedTree.h"
// Models
#include "OPC_BaseModel.h"
#include "OPC_HybridModel.h"
#include "OPC_Model.h"
// Colliders
#include "OPC_AABBCollider.h"
#include "OPC_Collider.h"
#include "OPC_LSSCollider.h"
#include "OPC_OBBCollider.h"
#include "OPC_PlanesCollider.h"
#include "OPC_RayCollider.h"
#include "OPC_SphereCollider.h"
#include "OPC_TreeCollider.h"
#include "OPC_VolumeCollider.h"
// Usages
#include "OPC_Picking.h"
// Sweep-and-prune
#include "OPC_BoxPruning.h"
#include "OPC_SweepAndPrune.h"

}  // namespace Opcode

#endif  // VEGA_STRIKE_ENGINE_CMD_COLLSION2_OPCODE_H
