/*
 * Copyright (C) 2001-2025 Pierre Terdiman, Daniel Horn, pyramid3d,
 * Stephen G. Tuggy, Benjamen R. Meyer, and other Vega Strike contributors.
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
 *	Contains base volume collider class.
 *	\file		OPC_VolumeCollider.cpp
 *	\author		Pierre Terdiman
 *	\date		June, 2, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains the abstract class for volume colliders.
 *
 *	\class		VolumeCollider
 *	\author		Pierre Terdiman
 *	\version	1.3
 *	\date		June, 2, 2001
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "cmd/collide2/Opcode.h"

using namespace Opcode;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VolumeCollider::VolumeCollider() :
        mTouchedPrimitives(nullptr),
        mNbVolumeBVTests(0),
        mNbVolumePrimTests(0) {
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VolumeCollider::~VolumeCollider() {
    mTouchedPrimitives = nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Validates current settings. You should call this method after all the settings / callbacks have been defined for a collider.
 *	\return		nullptr if everything is ok, else a string describing the problem
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char *VolumeCollider::ValidateSettings() {
    return nullptr;
}

// Pretty dumb way to dump - to do better - one day...

#define IMPLEMENT_NOLEAFDUMP(type)                                              \
void VolumeCollider::_Dump(const type* node)                                    \
{                                                                               \
    if(node->HasPosLeaf())    mTouchedPrimitives->Add(node->GetPosPrimitive()); \
    else                    _Dump(node->GetPos());                              \
                                                                                \
    if(ContactFound()) return;                                                  \
                                                                                \
    if(node->HasNegLeaf())    mTouchedPrimitives->Add(node->GetNegPrimitive()); \
    else                    _Dump(node->GetNeg());                              \
}

#define IMPLEMENT_LEAFDUMP(type)                                                \
void VolumeCollider::_Dump(const type* node)                                    \
{                                                                               \
    if(node->IsLeaf())                                                          \
    {                                                                           \
        mTouchedPrimitives->Add(node->GetPrimitive());                          \
    }                                                                           \
    else                                                                        \
    {                                                                           \
        _Dump(node->GetPos());                                                  \
                                                                                \
        if(ContactFound()) return;                                              \
                                                                                \
        _Dump(node->GetNeg());                                                  \
    }                                                                           \
}

IMPLEMENT_NOLEAFDUMP(AABBNoLeafNode)

IMPLEMENT_NOLEAFDUMP(AABBQuantizedNoLeafNode)

IMPLEMENT_LEAFDUMP(AABBCollisionNode)

IMPLEMENT_LEAFDUMP(AABBQuantizedNode)
