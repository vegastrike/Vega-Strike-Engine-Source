///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/Opcode.htm
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for an AABB collider.
 *	\file		OPC_AABBCollider.cpp
 *	\author		Pierre Terdiman
 *	\date		January, 1st, 2002
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains an AABB-vs-tree collider.
 *
 *	\class		AABBCollider
 *	\author		Pierre Terdiman
 *	\version	1.3
 *	\date		January, 1st, 2002
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Updated by Stephen G. Tuggy 2021-06-27
 * Updated by Stephen G. Tuggy 2022-01-06
 * Updated by Benjamen R. Meyer 2025-01-14
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "cmd/collide2/Opcode.h"

using namespace Opcode;

#include "OPC_BoxBoxOverlap.h"
#include "OPC_TriBoxOverlap.h"

#define SET_CONTACT(prim_index, flag)                           \
    /* Set contact status */                                    \
    mFlags |= flag;                                             \
    mTouchedPrimitives->Add(prim_index);

//! AABB-triangle test
#define AABB_PRIM(prim_index, flag)                             \
    /* Request vertices from the app */                         \
    VertexPointers VP;    mIMesh->GetTriangle(VP, prim_index);  \
    mLeafVerts[0] = *VP.Vertex[0];                              \
    mLeafVerts[1] = *VP.Vertex[1];                              \
    mLeafVerts[2] = *VP.Vertex[2];                              \
    /* Perform triangle-box overlap test */                     \
    if(TriBoxOverlap())                                         \
    {                                                           \
        SET_CONTACT(prim_index, flag)                           \
    }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AABBCollider::AABBCollider() {
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AABBCollider::~AABBCollider() {
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Generic collision query for generic OPCODE models. After the call, access the results:
 *	- with GetContactStatus()
 *	- with GetNbTouchedPrimitives()
 *	- with GetTouchedPrimitives()
 *
 *	\param		cache		[in/out] a box cache
 *	\param		box			[in] collision AABB in world space
 *	\param		model		[in] Opcode model to collide with
 *	\return		true if success
 *	\warning	SCALE NOT SUPPORTED. The matrices must contain rotation & translation parts only.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABBCollider::Collide(AABBCache &cache, const CollisionAABB &box, const Model &model) {
    // Checkings
    if (!Setup(&model)) {
        return false;
    }

    // Init collision query
    if (InitQuery(cache, box)) {
        return true;
    }

    if (!model.HasLeafNodes()) {
        if (model.IsQuantized()) {
            const AABBQuantizedNoLeafTree *Tree = (const AABBQuantizedNoLeafTree *) model.GetTree();

            // Setup dequantization coeffs
            mCenterCoeff = Tree->mCenterCoeff;
            mExtentsCoeff = Tree->mExtentsCoeff;

            // Perform collision query
            if (SkipPrimitiveTests()) {
                _CollideNoPrimitiveTest(Tree->GetNodes());
            } else {
                _Collide(Tree->GetNodes());
            }
        } else {
            const AABBNoLeafTree *Tree = (const AABBNoLeafTree *) model.GetTree();

            // Perform collision query
            if (SkipPrimitiveTests()) {
                _CollideNoPrimitiveTest(Tree->GetNodes());
            } else {
                _Collide(Tree->GetNodes());
            }
        }
    } else {
        if (model.IsQuantized()) {
            const AABBQuantizedTree *Tree = (const AABBQuantizedTree *) model.GetTree();

            // Setup dequantization coeffs
            mCenterCoeff = Tree->mCenterCoeff;
            mExtentsCoeff = Tree->mExtentsCoeff;

            // Perform collision query
            if (SkipPrimitiveTests()) {
                _CollideNoPrimitiveTest(Tree->GetNodes());
            } else {
                _Collide(Tree->GetNodes());
            }
        } else {
            const AABBCollisionTree *Tree = (const AABBCollisionTree *) model.GetTree();

            // Perform collision query
            if (SkipPrimitiveTests()) {
                _CollideNoPrimitiveTest(Tree->GetNodes());
            } else {
                _Collide(Tree->GetNodes());
            }
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Initializes a collision query :
 *	- reset stats & contact status
 *	- check temporal coherence
 *
 *	\param		cache		[in/out] a box cache
 *	\param		box			[in] AABB in world space
 *	\return		TRUE if we can return immediately
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABBCollider::InitQuery(AABBCache &cache, const CollisionAABB &box) {
    // 1) Call the base method
    VolumeCollider::InitQuery();

    // 2) Keep track of the query box
    mBox = box;

    // 3) Setup destination pointer
    mTouchedPrimitives = &cache.TouchedPrimitives;

    // 4) Special case: 1-triangle meshes [Opcode 1.3]
    if (mCurrentModel && mCurrentModel->HasSingleNode()) {
        if (!SkipPrimitiveTests()) {
            // We simply perform the BV-Prim overlap test each time. We assume single triangle has index 0.
            mTouchedPrimitives->Reset();

            // Perform overlap test between the unique triangle and the box (and set contact status if needed)
            AABB_PRIM(uint32_t(0), OPC_CONTACT)

            // Return immediately regardless of status
            return TRUE;
        }
    }

    // 5) Check temporal coherence :
    if (TemporalCoherenceEnabled()) {
        // Here we use temporal coherence
        // => check results from previous frame before performing the collision query
        if (FirstContactEnabled()) {
            // We're only interested in the first contact found => test the unique previously touched face
            if (mTouchedPrimitives->GetNbEntries()) {
                // Get index of previously touched face = the first entry in the array
                uint32_t PreviouslyTouchedFace = mTouchedPrimitives->GetEntry(0);

                // Then reset the array:
                // - if the overlap test below is successful, the index we'll get added back anyway
                // - if it isn't, then the array should be reset anyway for the normal query
                mTouchedPrimitives->Reset();

                // Perform overlap test between the cached triangle and the box (and set contact status if needed)
                AABB_PRIM(PreviouslyTouchedFace, OPC_TEMPORAL_CONTACT)

                // Return immediately if possible
                if (GetContactStatus()) {
                    return TRUE;
                }
            }
            // else no face has been touched during previous query
            // => we'll have to perform a normal query
        } else {
            // We're interested in all contacts =>test the new real box N(ew) against the previous fat box P(revious):
            if (IsCacheValid(cache) && mBox.IsInside(cache.FatBox)) {
                // - if N is included in P, return previous list
                // => we simply leave the list (mTouchedFaces) unchanged

                // Set contact status if needed
                if (mTouchedPrimitives->GetNbEntries()) {
                    mFlags |= OPC_TEMPORAL_CONTACT;
                }

                // In any case we don't need to do a query
                return TRUE;
            } else {
                // - else do the query using a fat N

                // Reset cache since we'll about to perform a real query
                mTouchedPrimitives->Reset();

                // Make a fat box so that coherence will work for subsequent frames
                mBox.mExtents *= cache.FatCoeff;

                // Update cache with query data (signature for cached faces)
                cache.FatBox = mBox;
            }
        }
    } else {
        // Here we don't use temporal coherence => do a normal query
        mTouchedPrimitives->Reset();
    }

    // 5) Precompute min & max bounds if needed
    mMin = box.mCenter - box.mExtents;
    mMax = box.mCenter + box.mExtents;

    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Collision query for vanilla AABB trees.
 *	\param		cache		[in/out] a box cache
 *	\param		box			[in] collision AABB in world space
 *	\param		tree		[in] AABB tree
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABBCollider::Collide(AABBCache &cache, const CollisionAABB &box, const AABBTree *tree) {
    // This is typically called for a scene tree, full of -AABBs-, not full of triangles.
    // So we don't really have "primitives" to deal with. Hence it doesn't work with
    // "FirstContact" + "TemporalCoherence".
    OPASSERT(!(FirstContactEnabled() && TemporalCoherenceEnabled()));

    // Checkings
    if (!tree) {
        return false;
    }

    // Init collision query
    if (InitQuery(cache, box)) {
        return true;
    }

    // Perform collision query
    _Collide(tree);

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Checks the AABB completely contains the box. In which case we can end the query sooner.
 *	\param		bc	[in] box center
 *	\param		be	[in] box extents
 *	\return		true if the AABB contains the whole box
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline_ bool AABBCollider::AABBContainsBox(const Point &bc, const Point &be) {
    if (mMin.x > bc.x - be.x) {
        return FALSE;
    }
    if (mMin.y > bc.y - be.y) {
        return FALSE;
    }
    if (mMin.z > bc.z - be.z) {
        return FALSE;
    }

    if (mMax.x < bc.x + be.x) {
        return FALSE;
    }
    if (mMax.y < bc.y + be.y) {
        return FALSE;
    }
    if (mMax.z < bc.z + be.z) {
        return FALSE;
    }

    return TRUE;
}

#define TEST_BOX_IN_AABB(center, extents)       \
    if(AABBContainsBox(center, extents))        \
    {                                           \
        /* Set contact status */                \
        mFlags |= OPC_CONTACT;                  \
        _Dump(node);                            \
        return;                                 \
    }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive collision query for normal AABB trees.
 *	\param		node	[in] current collision node
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AABBCollider::_Collide(const AABBCollisionNode *node) {
    // Perform AABB-AABB overlap test
    if (!AABBAABBOverlap(node->mAABB.mExtents, node->mAABB.mCenter)) {
        return;
    }

    TEST_BOX_IN_AABB(node->mAABB.mCenter, node->mAABB.mExtents)

    if (node->IsLeaf()) {
        AABB_PRIM(node->GetPrimitive(), OPC_CONTACT)
    } else {
        _Collide(node->GetPos());

        if (ContactFound()) {
            return;
        }

        _Collide(node->GetNeg());
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive collision query for normal AABB trees, without primitive tests.
 *	\param		node	[in] current collision node
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AABBCollider::_CollideNoPrimitiveTest(const AABBCollisionNode *node) {
    // Perform AABB-AABB overlap test
    if (!AABBAABBOverlap(node->mAABB.mExtents, node->mAABB.mCenter)) {
        return;
    }

    TEST_BOX_IN_AABB(node->mAABB.mCenter, node->mAABB.mExtents)

    if (node->IsLeaf()) {
        SET_CONTACT(node->GetPrimitive(), OPC_CONTACT)
    } else {
        _CollideNoPrimitiveTest(node->GetPos());

        if (ContactFound()) {
            return;
        }

        _CollideNoPrimitiveTest(node->GetNeg());
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive collision query for quantized AABB trees.
 *	\param		node	[in] current collision node
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AABBCollider::_Collide(const AABBQuantizedNode *node) {
    // Dequantize box
    const QuantizedAABB &Box = node->mAABB;
    const Point Center(float(Box.mCenter[0]) * mCenterCoeff.x,
            float(Box.mCenter[1]) * mCenterCoeff.y,
            float(Box.mCenter[2]) * mCenterCoeff.z);
    const Point Extents(float(Box.mExtents[0]) * mExtentsCoeff.x,
            float(Box.mExtents[1]) * mExtentsCoeff.y,
            float(Box.mExtents[2]) * mExtentsCoeff.z);

    // Perform AABB-AABB overlap test
    if (!AABBAABBOverlap(Extents, Center)) {
        return;
    }

    TEST_BOX_IN_AABB(Center, Extents)

    if (node->IsLeaf()) {
        AABB_PRIM(node->GetPrimitive(), OPC_CONTACT)
    } else {
        _Collide(node->GetPos());

        if (ContactFound()) {
            return;
        }

        _Collide(node->GetNeg());
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive collision query for quantized AABB trees, without primitive tests.
 *	\param		node	[in] current collision node
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AABBCollider::_CollideNoPrimitiveTest(const AABBQuantizedNode *node) {
    // Dequantize box
    const QuantizedAABB &Box = node->mAABB;
    const Point Center(float(Box.mCenter[0]) * mCenterCoeff.x,
            float(Box.mCenter[1]) * mCenterCoeff.y,
            float(Box.mCenter[2]) * mCenterCoeff.z);
    const Point Extents(float(Box.mExtents[0]) * mExtentsCoeff.x,
            float(Box.mExtents[1]) * mExtentsCoeff.y,
            float(Box.mExtents[2]) * mExtentsCoeff.z);

    // Perform AABB-AABB overlap test
    if (!AABBAABBOverlap(Extents, Center)) {
        return;
    }

    TEST_BOX_IN_AABB(Center, Extents)

    if (node->IsLeaf()) {
        SET_CONTACT(node->GetPrimitive(), OPC_CONTACT)
    } else {
        _CollideNoPrimitiveTest(node->GetPos());

        if (ContactFound()) {
            return;
        }

        _CollideNoPrimitiveTest(node->GetNeg());
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive collision query for no-leaf AABB trees.
 *	\param		node	[in] current collision node
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AABBCollider::_Collide(const AABBNoLeafNode *node) {
    // Perform AABB-AABB overlap test
    if (!AABBAABBOverlap(node->mAABB.mExtents, node->mAABB.mCenter)) {
        return;
    }

    TEST_BOX_IN_AABB(node->mAABB.mCenter, node->mAABB.mExtents)

    if (node->HasPosLeaf()) {
        AABB_PRIM(node->GetPosPrimitive(), OPC_CONTACT)
    } else {
        _Collide(node->GetPos());
    }

    if (ContactFound()) {
        return;
    }

    if (node->HasNegLeaf()) {
        AABB_PRIM(node->GetNegPrimitive(), OPC_CONTACT)
    } else {
        _Collide(node->GetNeg());
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive collision query for no-leaf AABB trees, without primitive tests.
 *	\param		node	[in] current collision node
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AABBCollider::_CollideNoPrimitiveTest(const AABBNoLeafNode *node) {
    // Perform AABB-AABB overlap test
    if (!AABBAABBOverlap(node->mAABB.mExtents, node->mAABB.mCenter)) {
        return;
    }

    TEST_BOX_IN_AABB(node->mAABB.mCenter, node->mAABB.mExtents)

    if (node->HasPosLeaf()) {
        SET_CONTACT(node->GetPosPrimitive(), OPC_CONTACT)
    } else {
        _CollideNoPrimitiveTest(node->GetPos());
    }

    if (ContactFound()) {
        return;
    }

    if (node->HasNegLeaf()) {
        SET_CONTACT(node->GetNegPrimitive(), OPC_CONTACT)
    } else {
        _CollideNoPrimitiveTest(node->GetNeg());
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive collision query for quantized no-leaf AABB trees.
 *	\param		node	[in] current collision node
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AABBCollider::_Collide(const AABBQuantizedNoLeafNode *node) {
    // Dequantize box
    const QuantizedAABB &Box = node->mAABB;
    const Point Center(float(Box.mCenter[0]) * mCenterCoeff.x,
            float(Box.mCenter[1]) * mCenterCoeff.y,
            float(Box.mCenter[2]) * mCenterCoeff.z);
    const Point Extents(float(Box.mExtents[0]) * mExtentsCoeff.x,
            float(Box.mExtents[1]) * mExtentsCoeff.y,
            float(Box.mExtents[2]) * mExtentsCoeff.z);

    // Perform AABB-AABB overlap test
    if (!AABBAABBOverlap(Extents, Center)) {
        return;
    }

    TEST_BOX_IN_AABB(Center, Extents)

    if (node->HasPosLeaf()) {
        AABB_PRIM(node->GetPosPrimitive(), OPC_CONTACT)
    } else {
        _Collide(node->GetPos());
    }

    if (ContactFound()) {
        return;
    }

    if (node->HasNegLeaf()) {
        AABB_PRIM(node->GetNegPrimitive(), OPC_CONTACT)
    } else {
        _Collide(node->GetNeg());
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive collision query for quantized no-leaf AABB trees, without primitive tests.
 *	\param		node	[in] current collision node
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AABBCollider::_CollideNoPrimitiveTest(const AABBQuantizedNoLeafNode *node) {
    // Dequantize box
    const QuantizedAABB &Box = node->mAABB;
    const Point Center(float(Box.mCenter[0]) * mCenterCoeff.x,
            float(Box.mCenter[1]) * mCenterCoeff.y,
            float(Box.mCenter[2]) * mCenterCoeff.z);
    const Point Extents(float(Box.mExtents[0]) * mExtentsCoeff.x,
            float(Box.mExtents[1]) * mExtentsCoeff.y,
            float(Box.mExtents[2]) * mExtentsCoeff.z);

    // Perform AABB-AABB overlap test
    if (!AABBAABBOverlap(Extents, Center)) {
        return;
    }

    TEST_BOX_IN_AABB(Center, Extents)

    if (node->HasPosLeaf()) {
        SET_CONTACT(node->GetPosPrimitive(), OPC_CONTACT)
    } else {
        _CollideNoPrimitiveTest(node->GetPos());
    }

    if (ContactFound()) {
        return;
    }

    if (node->HasNegLeaf()) {
        SET_CONTACT(node->GetNegPrimitive(), OPC_CONTACT)
    } else {
        _CollideNoPrimitiveTest(node->GetNeg());
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive collision query for vanilla AABB trees.
 *	\param		node	[in] current collision node
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AABBCollider::_Collide(const AABBTreeNode *node) {
    // Perform AABB-AABB overlap test
    Point Center, Extents;
    node->GetAABB()->GetCenter(Center);
    node->GetAABB()->GetExtents(Extents);
    if (!AABBAABBOverlap(Center, Extents)) {
        return;
    }

    if (node->IsLeaf() || AABBContainsBox(Center, Extents)) {
        mFlags |= OPC_CONTACT;
        mTouchedPrimitives->Add(node->GetPrimitives(), node->GetNbPrimitives());
    } else {
        _Collide(node->GetPos());
        _Collide(node->GetNeg());
    }
}




///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
HybridAABBCollider::HybridAABBCollider() {
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
HybridAABBCollider::~HybridAABBCollider() {
}

bool HybridAABBCollider::Collide(AABBCache &cache, const CollisionAABB &box, const HybridModel &model) {
    // We don't want primitive tests here!
    mFlags |= OPC_NO_PRIMITIVE_TESTS;

    // Checkings
    if (!Setup(&model)) {
        return false;
    }

    // Init collision query
    if (InitQuery(cache, box)) {
        return true;
    }

    // Special case for 1-leaf trees
    if (mCurrentModel && mCurrentModel->HasSingleNode()) {
        // Here we're supposed to perform a normal query, except our tree has a single node, i.e. just a few triangles
        uint32_t Nb = mIMesh->GetNbTriangles();

        // Loop through all triangles
        for (uint32_t i = 0; i < Nb; i++) {
            AABB_PRIM(i, OPC_CONTACT)
        }
        return true;
    }

    // Override destination array since we're only going to get leaf boxes here
    mTouchedBoxes.Reset();
    mTouchedPrimitives = &mTouchedBoxes;

    // Now, do the actual query against leaf boxes
    if (!model.HasLeafNodes()) {
        if (model.IsQuantized()) {
            const AABBQuantizedNoLeafTree *Tree = (const AABBQuantizedNoLeafTree *) model.GetTree();

            // Setup dequantization coeffs
            mCenterCoeff = Tree->mCenterCoeff;
            mExtentsCoeff = Tree->mExtentsCoeff;

            // Perform collision query - we don't want primitive tests here!
            _CollideNoPrimitiveTest(Tree->GetNodes());
        } else {
            const AABBNoLeafTree *Tree = (const AABBNoLeafTree *) model.GetTree();

            // Perform collision query - we don't want primitive tests here!
            _CollideNoPrimitiveTest(Tree->GetNodes());
        }
    } else {
        if (model.IsQuantized()) {
            const AABBQuantizedTree *Tree = (const AABBQuantizedTree *) model.GetTree();

            // Setup dequantization coeffs
            mCenterCoeff = Tree->mCenterCoeff;
            mExtentsCoeff = Tree->mExtentsCoeff;

            // Perform collision query - we don't want primitive tests here!
            _CollideNoPrimitiveTest(Tree->GetNodes());
        } else {
            const AABBCollisionTree *Tree = (const AABBCollisionTree *) model.GetTree();

            // Perform collision query - we don't want primitive tests here!
            _CollideNoPrimitiveTest(Tree->GetNodes());
        }
    }

    // We only have a list of boxes so far
    if (GetContactStatus()) {
        // Reset contact status, since it currently only reflects collisions with leaf boxes
        Collider::InitQuery();

        // Change dest container so that we can use built-in overlap tests and get collided primitives
        cache.TouchedPrimitives.Reset();
        mTouchedPrimitives = &cache.TouchedPrimitives;

        // Read touched leaf boxes
        uint32_t Nb = mTouchedBoxes.GetNbEntries();
        const uint32_t *Touched = mTouchedBoxes.GetEntries();

        const LeafTriangles *LT = model.GetLeafTriangles();
        const uint32_t *Indices = model.GetIndices();

        // Loop through touched leaves
        while (Nb--) {
            const LeafTriangles &CurrentLeaf = LT[*Touched++];

            // Each leaf box has a set of triangles
            uint32_t NbTris = CurrentLeaf.GetNbTriangles();
            if (Indices) {
                const uint32_t *T = &Indices[CurrentLeaf.GetTriangleIndex()];

                // Loop through triangles and test each of them
                while (NbTris--) {
                    uint32_t TriangleIndex = *T++;
                    AABB_PRIM(TriangleIndex, OPC_CONTACT)
                }
            } else {
                uint32_t BaseIndex = CurrentLeaf.GetTriangleIndex();

                // Loop through triangles and test each of them
                while (NbTris--) {
                    uint32_t TriangleIndex = BaseIndex++;
                    AABB_PRIM(TriangleIndex, OPC_CONTACT)
                }
            }
        }
    }

    return true;
}

