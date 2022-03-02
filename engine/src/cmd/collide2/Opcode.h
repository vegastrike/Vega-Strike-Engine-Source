/*
 * Copyright (C) 2001-2022 Pierre Terdiman, Daniel Horn, pyramid3d,
 * Stephen G. Tuggy, and other Vega Strike contributors.
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Main file for Opcode.dll.
 *	\file		Opcode.h
 *	\author		Pierre Terdiman
 *	\date		March, 20, 2001
 *
 *  Updated by Stephen G. Tuggy 2022-01-06
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __OPCODE_H__
#define __OPCODE_H__

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

#endif // __OPCODE_H__
