/**
 * Opcode.h
 *
 * Copyright (C) 2001-2025 Pierre Terdiman, Daniel Horn, pyramid3d,
 * Stephen G. Tuggy, Benjamen R. Meyer, and other Vega Strike contributors.
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
 *  Main file for Opcode.dll.
 *  \file		Opcode.h
 *  \author		Pierre Terdiman
 *  \date		March, 20, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef VEGA_STRIKE_ENGINE_CMD_COLLSION2_OPCODE_H
#define VEGA_STRIKE_ENGINE_CMD_COLLSION2_OPCODE_H

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Preprocessor

#define OPCODE_API

#define ICE_NO_DLL
#define ICE_DONT_CHECK_COMPILER_OPTIONS

#include "OPC_IceHook.h"
#include "Ice/IceRandom.h"
#include "Ice/IceRevisitedRadix.h"
#include "Ice/IceUtils.h"
#include "Ice/IceContainer.h"
#include "Ice/IcePairs.h"

namespace Opcode {

#include "Ice/IceAxes.h"



#include "Ice/IcePoint.h"



#include "Ice/IceHPoint.h"



#include "Ice/IceMatrix3x3.h"



#include "Ice/IceMatrix4x4.h"



#include "Ice/IcePlane.h"



#include "Ice/IceRay.h"



#include "Ice/IceIndexedTriangle.h"



#include "Ice/IceTriangle.h"



#include "Ice/IceTrilist.h"



#include "Ice/IceAABB.h"



#include "Ice/IceOBB.h"



#include "Ice/IceBoundingSphere.h"



#include "Ice/IceSegment.h"



#include "Ice/IceLSS.h"

// Bulk-of-the-work
#include "OPC_Settings.h"



#include "OPC_Common.h"



#include "OPC_MeshInterface.h"
// Builders
#include "OPC_TreeBuilders.h"
// Trees
#include "OPC_AABBTree.h"



#include "OPC_OptimizedTree.h"
// Models
#include "OPC_BaseModel.h"



#include "OPC_Model.h"



#include "OPC_HybridModel.h"
// Colliders
#include "OPC_Collider.h"



#include "OPC_VolumeCollider.h"



#include "OPC_TreeCollider.h"



#include "OPC_RayCollider.h"



#include "OPC_SphereCollider.h"



#include "OPC_OBBCollider.h"



#include "OPC_AABBCollider.h"



#include "OPC_LSSCollider.h"



#include "OPC_PlanesCollider.h"
// Usages
#include "OPC_Picking.h"
// Sweep-and-prune
#include "OPC_BoxPruning.h"



#include "OPC_SweepAndPrune.h"

}

#endif //VEGA_STRIKE_ENGINE_CMD_COLLSION2_OPCODE_H
