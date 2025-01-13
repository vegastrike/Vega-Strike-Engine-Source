/*
 * OPC_SphereCollider.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
 *
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
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/Opcode.htm
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for a sphere collider.
 *	\file		OPC_SphereCollider.h
 *	\author		Pierre Terdiman
 *	\date		June, 2, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Updated by Stephen G. Tuggy 2021-07-03
 * Updated by Stephen G. Tuggy 2022-01-06
 * Updated by Benjamen R. Meyer 2023-05-27
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef VEGA_STRIKE_ENGINE_CMD_COLLSION2_OPC_SPHERE_COLLIDER_H
#define VEGA_STRIKE_ENGINE_CMD_COLLSION2_OPC_SPHERE_COLLIDER_H

struct OPCODE_API SphereCache : VolumeCache {
    SphereCache() : Center(0.0f, 0.0f, 0.0f), FatRadius2(0.0f), FatCoeff(1.1f) {
    }

    ~SphereCache() {
    }

    // Cached faces signature
    Point Center;        //!< Sphere used when performing the query resulting in cached faces
    float FatRadius2;    //!< Sphere used when performing the query resulting in cached faces
    // User settings
    float FatCoeff;    //!< mRadius2 multiplier used to create a fat sphere
};

class OPCODE_API SphereCollider : public VolumeCollider {
public:
    // Constructor / Destructor
    SphereCollider();
    virtual                                ~SphereCollider();

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Generic collision query for generic OPCODE models. After the call, access the results:
     *	- with GetContactStatus()
     *	- with GetNbTouchedPrimitives()
     *	- with GetTouchedPrimitives()
     *
     *	\param		cache			[in/out] a sphere cache
     *	\param		sphere			[in] collision sphere in local space
     *	\param		model			[in] Opcode model to collide with
     *	\param		worlds			[in] sphere's world matrix, or nullptr
     *	\param		worldm			[in] model's world matrix, or nullptr
     *	\return		true if success
     *	\warning	SCALE NOT SUPPORTED. The matrices must contain rotation & translation parts only.
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool Collide(SphereCache &cache,
            const Sphere &sphere,
            const Model &model,
            const Matrix4x4 *worlds = nullptr,
            const Matrix4x4 *worldm = nullptr);

    //
    bool Collide(SphereCache &cache, const Sphere &sphere, const AABBTree *tree);
protected:
    // Sphere in model space
    Point mCenter;            //!< Sphere center
    float mRadius2;            //!< Sphere radius squared
    // Internal methods
    void _Collide(const AABBCollisionNode *node);
    void _Collide(const AABBNoLeafNode *node);
    void _Collide(const AABBQuantizedNode *node);
    void _Collide(const AABBQuantizedNoLeafNode *node);
    void _Collide(const AABBTreeNode *node);
    void _CollideNoPrimitiveTest(const AABBCollisionNode *node);
    void _CollideNoPrimitiveTest(const AABBNoLeafNode *node);
    void _CollideNoPrimitiveTest(const AABBQuantizedNode *node);
    void _CollideNoPrimitiveTest(const AABBQuantizedNoLeafNode *node);
    // Overlap tests
    inline_                bool SphereContainsBox(const Point &bc, const Point &be);
    inline_                bool SphereAABBOverlap(const Point &center, const Point &extents);
    bool SphereTriOverlap(const Point &vert0, const Point &vert1, const Point &vert2);
    // Init methods
    bool InitQuery(SphereCache &cache,
            const Sphere &sphere,
            const Matrix4x4 *worlds = nullptr,
            const Matrix4x4 *worldm = nullptr);
};

class OPCODE_API HybridSphereCollider : public SphereCollider {
public:
    // Constructor / Destructor
    HybridSphereCollider();
    virtual                                ~HybridSphereCollider();

    bool Collide(SphereCache &cache,
            const Sphere &sphere,
            const HybridModel &model,
            const Matrix4x4 *worlds = nullptr,
            const Matrix4x4 *worldm = nullptr);
protected:
    Container mTouchedBoxes;
};

#endif //VEGA_STRIKE_ENGINE_CMD_COLLSION2_OPC_SPHERE_COLLIDER_H
