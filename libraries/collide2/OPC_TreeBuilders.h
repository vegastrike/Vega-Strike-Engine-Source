/**
 * OPC_TreeBuilders.h
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
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/Opcode.htm
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for tree builders.
 *  \file		OPC_TreeBuilders.h
 *  \author		Pierre Terdiman
 *  \date		March, 20, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_OPC_TREE_BUILDERS_H
#define VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_OPC_TREE_BUILDERS_H

//! Tree splitting rules
enum SplittingRules {
    // Primitive split
    SPLIT_LARGEST_AXIS = (1 << 0),        //!< Split along the largest axis
    SPLIT_SPLATTER_POINTS = (1 << 1),        //!< Splatter primitive centers (QuickCD-style)
    SPLIT_BEST_AXIS = (1 << 2),        //!< Try largest axis, then second, then last
    SPLIT_BALANCED = (1 << 3),        //!< Try to keep a well-balanced tree
    SPLIT_FIFTY = (1 << 4),        //!< Arbitrary 50-50 split
    // Node split
    SPLIT_GEOM_CENTER = (1 << 5),        //!< Split at geometric center (else split in the middle)
    //
    SPLIT_FORCE_DWORD = 0x7fffffff
};

//! Simple wrapper around build-related settings [Opcode 1.3]
struct OPCODE_API BuildSettings {
    inline_ BuildSettings() : mLimit(1), mRules(SPLIT_FORCE_DWORD) {
    }

    uint32_t mLimit;        //!< Limit number of primitives / node. If limit is 1, build a complete tree (2*N-1 nodes)
    uint32_t mRules;        //!< Building/Splitting rules (a combination of SplittingRules flags)
};

class OPCODE_API AABBTreeBuilder {
public:
    //! Constructor
    AABBTreeBuilder() :
            mNbPrimitives(0),
            mNodeBase(nullptr),
            mCount(0),
            mNbInvalidSplits(0) {
    }

    //! Destructor
    virtual                                        ~AABBTreeBuilder() {
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Computes the AABB of a set of primitives.
     *	\param		primitives		[in] list of indices of primitives
     *	\param		nb_prims		[in] number of indices
     *	\param		global_box		[out] global AABB enclosing the set of input primitives
     *	\return		true if success
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual bool ComputeGlobalBox(const uint32_t *primitives, uint32_t nb_prims, AABB &global_box) const = 0;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Computes the splitting value along a given axis for a given primitive.
     *	\param		index			[in] index of the primitive to split
     *	\param		axis			[in] axis index (0,1,2)
     *	\return		splitting value
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual float GetSplittingValue(uint32_t index, uint32_t axis) const = 0;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Computes the splitting value along a given axis for a given node.
     *	\param		primitives		[in] list of indices of primitives
     *	\param		nb_prims		[in] number of indices
     *	\param		global_box		[in] global AABB enclosing the set of input primitives
     *	\param		axis			[in] axis index (0,1,2)
     *	\return		splitting value
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual float GetSplittingValue(const uint32_t * /*primitives*/,
            uint32_t /*nb_prims*/,
            const AABB &global_box,
            uint32_t axis) const {
        // Default split value = middle of the axis (using only the box)
        return global_box.GetCenter(axis);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Validates node subdivision. This is called each time a node is considered for subdivision, during tree building.
     *	\param		primitives		[in] list of indices of primitives
     *	\param		nb_prims		[in] number of indices
     *	\param		global_box		[in] global AABB enclosing the set of input primitives
     *	\return		TRUE if the node should be subdivised
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual BOOL ValidateSubdivision(const uint32_t * /*primitives*/, uint32_t nb_prims, const AABB & /*global_box*/) {
        // Check the user-defined limit
        if (nb_prims <= mSettings.mLimit) {
            return FALSE;
        }

        return TRUE;
    }

    BuildSettings mSettings;            //!< Splitting rules & split limit [Opcode 1.3]
    uint32_t mNbPrimitives;        //!< Total number of primitives.
    void *mNodeBase;            //!< Address of node pool [Opcode 1.3]
    // Stats
    inline_                        void SetCount(uint32_t nb) {
        mCount = nb;
    }

    inline_                        void IncreaseCount(uint32_t nb) {
        mCount += nb;
    }

    inline_                        uint32_t GetCount() const {
        return mCount;
    }

    inline_                        void SetNbInvalidSplits(uint32_t nb) {
        mNbInvalidSplits = nb;
    }

    inline_                        void IncreaseNbInvalidSplits() {
        mNbInvalidSplits++;
    }

    inline_                        uint32_t GetNbInvalidSplits() const {
        return mNbInvalidSplits;
    }

private:
    uint32_t mCount;                //!< Stats: number of nodes created
    uint32_t mNbInvalidSplits;    //!< Stats: number of invalid splits
};

class OPCODE_API AABBTreeOfVerticesBuilder : public AABBTreeBuilder {
public:
    //! Constructor
    AABBTreeOfVerticesBuilder() : mVertexArray(nullptr) {
    }

    //! Destructor
    virtual                                        ~AABBTreeOfVerticesBuilder() {
    }

    override(AABBTreeBuilder) bool ComputeGlobalBox(const uint32_t *primitives,
            uint32_t nb_prims,
            AABB &global_box) const;
    override(AABBTreeBuilder) float GetSplittingValue(uint32_t index, uint32_t axis) const;
    override(AABBTreeBuilder) float GetSplittingValue(const uint32_t *primitives,
            uint32_t nb_prims,
            const AABB &global_box,
            uint32_t axis) const;

    const Point *mVertexArray;        //!< Shortcut to an app-controlled array of vertices.
};

class OPCODE_API AABBTreeOfAABBsBuilder : public AABBTreeBuilder {
public:
    //! Constructor
    AABBTreeOfAABBsBuilder() : mAABBArray(nullptr) {
    }

    //! Destructor
    virtual                                        ~AABBTreeOfAABBsBuilder() {
    }

    override(AABBTreeBuilder) bool ComputeGlobalBox(const uint32_t *primitives,
            uint32_t nb_prims,
            AABB &global_box) const;
    override(AABBTreeBuilder) float GetSplittingValue(uint32_t index, uint32_t axis) const;

    const AABB *mAABBArray;            //!< Shortcut to an app-controlled array of AABBs.
};

class OPCODE_API AABBTreeOfTrianglesBuilder : public AABBTreeBuilder {
public:
    //! Constructor
    AABBTreeOfTrianglesBuilder() : mIMesh(nullptr) {
    }

    //! Destructor
    virtual                                        ~AABBTreeOfTrianglesBuilder() {
    }

    override(AABBTreeBuilder) bool ComputeGlobalBox(const uint32_t *primitives,
            uint32_t nb_prims,
            AABB &global_box) const;
    override(AABBTreeBuilder) float GetSplittingValue(uint32_t index, uint32_t axis) const;
    override(AABBTreeBuilder) float GetSplittingValue(const uint32_t *primitives,
            uint32_t nb_prims,
            const AABB &global_box,
            uint32_t axis) const;

    const MeshInterface *mIMesh;            //!< Shortcut to an app-controlled mesh interface
};

#endif //VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_OPC_TREE_BUILDERS_H
