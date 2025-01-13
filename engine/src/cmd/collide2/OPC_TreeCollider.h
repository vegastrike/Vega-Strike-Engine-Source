/*
 * OPC_TreeCollider.h
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
 *	Contains code for a tree collider.
 *	\file		OPC_TreeCollider.h
 *	\author		Pierre Terdiman
 *	\date		March, 20, 2001
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
#ifndef VEGA_STRIKE_ENGINE_CMD_COLLSION2_OPC_TREECOLLIDER_H
#define VEGA_STRIKE_ENGINE_CMD_COLLSION2_OPC_TREECOLLIDER_H

//! This structure holds cached information used by the algorithm.
//! Two model pointers and two colliding primitives are cached. Model pointers are assigned
//! to their respective meshes, and the pair of colliding primitives is used for temporal
//! coherence. That is, in case temporal coherence is enabled, those two primitives are
//! tested for overlap before everything else. If they still collide, we're done before
//! even entering the recursive collision code.
struct OPCODE_API BVTCache : Pair {
    //! Constructor
    BVTCache() {
        ResetCache();
        ResetCountDown();
    }

    void ResetCache() {
        Model0 = nullptr;
        Model1 = nullptr;
        id0 = 0;
        id1 = 1;
#ifdef __MESHMERIZER_H__        // Collision hulls only supported within ICE !
        HullTest		= true;
        SepVector.pid	= 0;
        SepVector.qid	= 0;
        SepVector.SV	= Point(1.0f, 0.0f, 0.0f);
#endif // __MESHMERIZER_H__
    }

    void ResetCountDown() {
#ifdef __MESHMERIZER_H__        // Collision hulls only supported within ICE !
        CountDown		= 50;
#endif // __MESHMERIZER_H__
    }

    const Model *Model0;    //!< Model for first object
    const Model *Model1;    //!< Model for second object

#ifdef __MESHMERIZER_H__    // Collision hulls only supported within ICE !
    SVCache				SepVector;
    uint32_t				CountDown;
    bool				HullTest;
#endif // __MESHMERIZER_H__
};

class OPCODE_API AABBTreeCollider : public Collider {
public:
    // Constructor / Destructor
    AABBTreeCollider();
    virtual                                ~AABBTreeCollider();
    // Generic collision query

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Generic collision query for generic OPCODE models. After the call, access the results with:
     *	- GetContactStatus()
     *	- GetNbPairs()
     *	- GetPairs()
     *
     *	\param		cache			[in] collision cache for model pointers and a colliding pair of primitives
     *	\param		world0			[in] world matrix for first object, or nullptr
     *	\param		world1			[in] world matrix for second object, or nullptr
     *	\return		true if success
     *	\warning	SCALE NOT SUPPORTED. The matrices must contain rotation & translation parts only.
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool Collide(BVTCache &cache, const Matrix4x4 *world0 = nullptr, const Matrix4x4 *world1 = nullptr);

    // Collision queries
    bool Collide(const AABBCollisionTree *tree0,
            const AABBCollisionTree *tree1,
            const Matrix4x4 *world0 = nullptr,
            const Matrix4x4 *world1 = nullptr,
            Pair *cache = nullptr);
    bool Collide(const AABBNoLeafTree *tree0,
            const AABBNoLeafTree *tree1,
            const Matrix4x4 *world0 = nullptr,
            const Matrix4x4 *world1 = nullptr,
            Pair *cache = nullptr);
    bool Collide(const AABBQuantizedTree *tree0,
            const AABBQuantizedTree *tree1,
            const Matrix4x4 *world0 = nullptr,
            const Matrix4x4 *world1 = nullptr,
            Pair *cache = nullptr);
    bool Collide(const AABBQuantizedNoLeafTree *tree0,
            const AABBQuantizedNoLeafTree *tree1,
            const Matrix4x4 *world0 = nullptr,
            const Matrix4x4 *world1 = nullptr,
            Pair *cache = nullptr);
    // Settings

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Settings: selects between full box-box tests or "SAT-lite" tests (where Class III axes are discarded)
     *	\param		flag		[in] true for full tests, false for coarse tests
     *	\see		SetFullPrimBoxTest(bool flag)
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    inline_                void SetFullBoxBoxTest(bool flag) {
        mFullBoxBoxTest = flag;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Settings: selects between full triangle-box tests or "SAT-lite" tests (where Class III axes are discarded)
     *	\param		flag		[in] true for full tests, false for coarse tests
     *	\see		SetFullBoxBoxTest(bool flag)
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void SetFullPrimBoxTest(bool flag) {
        mFullPrimBoxTest = flag;
    }

    // Stats

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Stats: gets the number of BV-BV overlap tests after a collision query.
     *	\see		GetNbPrimPrimTests()
     *	\see		GetNbBVPrimTests()
     *	\return		the number of BV-BV tests performed during last query
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    inline_                uint32_t GetNbBVBVTests() const {
        return mNbBVBVTests;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Stats: gets the number of Triangle-Triangle overlap tests after a collision query.
     *	\see		GetNbBVBVTests()
     *	\see		GetNbBVPrimTests()
     *	\return		the number of Triangle-Triangle tests performed during last query
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    inline_                uint32_t GetNbPrimPrimTests() const {
        return mNbPrimPrimTests;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Stats: gets the number of BV-Triangle overlap tests after a collision query.
     *	\see		GetNbBVBVTests()
     *	\see		GetNbPrimPrimTests()
     *	\return		the number of BV-Triangle tests performed during last query
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    inline_                uint32_t GetNbBVPrimTests() const {
        return mNbBVPrimTests;
    }

    // Data access

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Gets the number of contacts after a collision query.
     *	\see		GetContactStatus()
     *	\see		GetPairs()
     *	\return		the number of contacts / colliding pairs.
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    inline_                uint32_t GetNbPairs() const {
        return mPairs.GetNbEntries() >> 1;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Gets the pairs of colliding triangles after a collision query.
     *	\see		GetContactStatus()
     *	\see		GetNbPairs()
     *	\return		the list of colliding pairs (triangle indices)
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    inline_                const Pair *GetPairs() const {
        return (const Pair *) mPairs.GetEntries();
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Validates current settings. You should call this method after all the settings and callbacks have been defined for a collider.
     *	\return		nullptr if everything is ok, else a string describing the problem
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    override(Collider) const char *ValidateSettings();

protected:
    // Colliding pairs
    Container mPairs;                //!< Pairs of colliding primitives
    // User mesh interfaces
    const MeshInterface *mIMesh0;            //!< User-defined mesh interface for object0
    const MeshInterface *mIMesh1;            //!< User-defined mesh interface for object1
    // Stats
    uint32_t mNbBVBVTests;        //!< Number of BV-BV tests
    uint32_t mNbPrimPrimTests;    //!< Number of Primitive-Primitive tests
    uint32_t mNbBVPrimTests;        //!< Number of BV-Primitive tests
    // Precomputed data
    Matrix3x3 mAR;                //!< Absolute rotation matrix
    Matrix3x3 mR0to1;                //!< Rotation from object0 to object1
    Matrix3x3 mR1to0;                //!< Rotation from object1 to object0
    Point mT0to1;                //!< Translation from object0 to object1
    Point mT1to0;                //!< Translation from object1 to object0
    // Dequantization coeffs
    Point mCenterCoeff0;
    Point mExtentsCoeff0;
    Point mCenterCoeff1;
    Point mExtentsCoeff1;
    // Leaf description
    Point mLeafVerts[3];        //!< Triangle vertices
    uint32_t mLeafIndex;            //!< Triangle index
    // Settings
    bool mFullBoxBoxTest;    //!< Perform full BV-BV tests (true) or SAT-lite tests (false)
    bool mFullPrimBoxTest;    //!< Perform full Primitive-BV tests (true) or SAT-lite tests (false)
    // Internal methods

    // Standard AABB trees
    void _Collide(const AABBCollisionNode *b0, const AABBCollisionNode *b1);
    // Quantized AABB trees
    void _Collide(const AABBQuantizedNode *b0,
            const AABBQuantizedNode *b1,
            const Point &a,
            const Point &Pa,
            const Point &b,
            const Point &Pb);
    // No-leaf AABB trees
    void _CollideTriBox(const AABBNoLeafNode *b);
    void _CollideBoxTri(const AABBNoLeafNode *b);
    void _Collide(const AABBNoLeafNode *a, const AABBNoLeafNode *b);
    // Quantized no-leaf AABB trees
    void _CollideTriBox(const AABBQuantizedNoLeafNode *b);
    void _CollideBoxTri(const AABBQuantizedNoLeafNode *b);
    void _Collide(const AABBQuantizedNoLeafNode *a, const AABBQuantizedNoLeafNode *b);
    // Overlap tests
    void PrimTest(uint32_t id0, uint32_t id1);
    inline_            void PrimTestTriIndex(uint32_t id1);
    inline_            void PrimTestIndexTri(uint32_t id0);

    inline_            bool BoxBoxOverlap(const Point &ea, const Point &ca, const Point &eb, const Point &cb);
    inline_            bool TriBoxOverlap(const Point &center, const Point &extents);
    inline_            bool TriTriOverlap(const Point &V0,
            const Point &V1,
            const Point &V2,
            const Point &U0,
            const Point &U1,
            const Point &U2);
    // Init methods
    void InitQuery(const Matrix4x4 *world0 = nullptr, const Matrix4x4 *world1 = nullptr);
    bool CheckTemporalCoherence(Pair *cache);

    inline_                bool Setup(const MeshInterface *mi0, const MeshInterface *mi1) {
        mIMesh0 = mi0;
        mIMesh1 = mi1;

        if (!mIMesh0 || !mIMesh1) {
            return FALSE;
        }

        return TRUE;
    }
};

#endif //VEGA_STRIKE_ENGINE_CMD_COLLSION2_OPC_TREECOLLIDER_H
