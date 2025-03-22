///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/Opcode.htm
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for an LSS collider.
 *	\file		OPC_LSSCollider.h
 *	\author		Pierre Terdiman
 *	\date		December, 28, 2002
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
#ifndef VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_OPC_LSS_COLLIDER_H
#define VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_OPC_LSS_COLLIDER_H

struct OPCODE_API LSSCache : VolumeCache {
    LSSCache() {
        Previous.mP0 = Point(0.0f, 0.0f, 0.0f);
        Previous.mP1 = Point(0.0f, 0.0f, 0.0f);
        Previous.mRadius = 0.0f;
        FatCoeff = 1.1f;
    }

    // Cached faces signature
    LSS Previous;    //!< LSS used when performing the query resulting in cached faces
    // User settings
    float FatCoeff;    //!< mRadius2 multiplier used to create a fat LSS
};

class OPCODE_API LSSCollider : public VolumeCollider {
public:
    // Constructor / Destructor
    LSSCollider();
    virtual                                ~LSSCollider();

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Generic collision query for generic OPCODE models. After the call, access the results:
     *	- with GetContactStatus()
     *	- with GetNbTouchedPrimitives()
     *	- with GetTouchedPrimitives()
     *
     *	\param		cache			[in/out] an lss cache
     *	\param		lss				[in] collision lss in local space
     *	\param		model			[in] Opcode model to collide with
     *	\param		worldl			[in] lss world matrix, or nullptr
     *	\param		worldm			[in] model's world matrix, or nullptr
     *	\return		true if success
     *	\warning	SCALE NOT SUPPORTED. The matrices must contain rotation & translation parts only.
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool Collide(LSSCache &cache,
            const LSS &lss,
            const Model &model,
            const Matrix4x4 *worldl = nullptr,
            const Matrix4x4 *worldm = nullptr);
    //
    bool Collide(LSSCache &cache, const LSS &lss, const AABBTree *tree);
protected:
    // LSS in model space
    Segment mSeg;            //!< Segment
    float mRadius2;        //!< LSS radius squared
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
    inline_                bool LSSContainsBox(const Point &bc, const Point &be);
    inline_                bool LSSAABBOverlap(const Point &center, const Point &extents);
    inline_                bool LSSTriOverlap(const Point &vert0, const Point &vert1, const Point &vert2);
    // Init methods
    bool InitQuery(LSSCache &cache,
            const LSS &lss,
            const Matrix4x4 *worldl = nullptr,
            const Matrix4x4 *worldm = nullptr);
};

class OPCODE_API HybridLSSCollider : public LSSCollider {
public:
    // Constructor / Destructor
    HybridLSSCollider();
    virtual                                ~HybridLSSCollider();

    bool Collide(LSSCache &cache,
            const LSS &lss,
            const HybridModel &model,
            const Matrix4x4 *worldl = nullptr,
            const Matrix4x4 *worldm = nullptr);
protected:
    Container mTouchedBoxes;
};

#endif //VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_OPC_LSS_COLLIDER_H
