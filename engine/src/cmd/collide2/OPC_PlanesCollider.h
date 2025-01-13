/*
 * OPC_PlanesCollider.h
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
 *	Contains code for a planes collider.
 *	\file		OPC_PlanesCollider.h
 *	\author		Pierre Terdiman
 *	\date		January, 1st, 2002
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
#ifndef VEGA_STRIKE_ENGINE_CMD_COLLSION2_OPC_PLANESCOLLIDER_H
#define VEGA_STRIKE_ENGINE_CMD_COLLSION2_OPC_PLANESCOLLIDER_H

struct OPCODE_API PlanesCache : VolumeCache {
    PlanesCache() {
    }
};

class OPCODE_API PlanesCollider : public VolumeCollider {
public:
    // Constructor / Destructor
    PlanesCollider();
    virtual                                ~PlanesCollider();

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Generic collision query for generic OPCODE models. After the call, access the results:
     *	- with GetContactStatus()
     *	- with GetNbTouchedPrimitives()
     *	- with GetTouchedPrimitives()
     *
     *	\param		cache			[in/out] a planes cache
     *	\param		planes			[in] list of planes in world space
     *	\param		nb_planes		[in] number of planes
     *	\param		model			[in] Opcode model to collide with
     *	\param		worldm			[in] model's world matrix, or nullptr
     *	\return		true if success
     *	\warning	SCALE NOT SUPPORTED. The matrices must contain rotation & translation parts only.
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool Collide(PlanesCache &cache,
            const Plane *planes,
            uint32_t nb_planes,
            const Model &model,
            const Matrix4x4 *worldm = nullptr);

    // Mutant box-with-planes collision queries
    inline_                bool Collide(PlanesCache &cache,
            const OBB &box,
            const Model &model,
            const Matrix4x4 *worldb = nullptr,
            const Matrix4x4 *worldm = nullptr) {
        Plane PL[6];

        if (worldb) {
            // Create a new OBB in world space
            OBB WorldBox;
            box.Rotate(*worldb, WorldBox);
            // Compute planes from the sides of the box
            WorldBox.ComputePlanes(PL);
        } else {
            // Compute planes from the sides of the box
            box.ComputePlanes(PL);
        }

        // Collide with box planes
        return Collide(cache, PL, 6, model, worldm);
    }
    // Settings

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Validates current settings. You should call this method after all the settings and callbacks have been defined for a collider.
     *	\return		nullptr if everything is ok, else a string describing the problem
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    override(Collider) const char *ValidateSettings();

protected:
    // Planes in model space
    uint32_t mNbPlanes;
    Plane *mPlanes;
    // Leaf description
    VertexPointers mVP;
    // Internal methods
    void _Collide(const AABBCollisionNode *node, uint32_t clip_mask);
    void _Collide(const AABBNoLeafNode *node, uint32_t clip_mask);
    void _Collide(const AABBQuantizedNode *node, uint32_t clip_mask);
    void _Collide(const AABBQuantizedNoLeafNode *node, uint32_t clip_mask);
    void _CollideNoPrimitiveTest(const AABBCollisionNode *node, uint32_t clip_mask);
    void _CollideNoPrimitiveTest(const AABBNoLeafNode *node, uint32_t clip_mask);
    void _CollideNoPrimitiveTest(const AABBQuantizedNode *node, uint32_t clip_mask);
    void _CollideNoPrimitiveTest(const AABBQuantizedNoLeafNode *node, uint32_t clip_mask);
    // Overlap tests
    inline_                bool PlanesAABBOverlap(const Point &center,
            const Point &extents,
            uint32_t &out_clip_mask,
            uint32_t in_clip_mask);
    inline_                bool PlanesTriOverlap(uint32_t in_clip_mask);
    // Init methods
    bool InitQuery(PlanesCache &cache, const Plane *planes, uint32_t nb_planes, const Matrix4x4 *worldm = nullptr);
};

class OPCODE_API HybridPlanesCollider : public PlanesCollider {
public:
    // Constructor / Destructor
    HybridPlanesCollider();
    virtual                                ~HybridPlanesCollider();

    bool Collide(PlanesCache &cache,
            const Plane *planes,
            uint32_t nb_planes,
            const HybridModel &model,
            const Matrix4x4 *worldm = nullptr);
protected:
    Container mTouchedBoxes;
};

#endif //VEGA_STRIKE_ENGINE_CMD_COLLSION2_OPC_PLANESCOLLIDER_H
