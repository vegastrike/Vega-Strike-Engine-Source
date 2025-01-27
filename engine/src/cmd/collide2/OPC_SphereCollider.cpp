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
 *	\file		OPC_SphereCollider.cpp
 *	\author		Pierre Terdiman
 *	\date		June, 2, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a sphere-vs-tree collider.
 *	This class performs a collision test between a sphere and an AABB tree. You can use this to do a standard player vs world collision,
 *	in a Nettle/Telemachos way. It doesn't suffer from all reported bugs in those two classic codes - the "new" one by Paul Nettle is a
 *	debuggued version I think. Collision response can be driven by reported collision data - it works extremely well for me. In sake of
 *	efficiency, all meshes (that is, all AABB trees) should of course also be kept in an extra hierarchical structure (octree, whatever).
 *
 *	\class		SphereCollider
 *	\author		Pierre Terdiman
 *	\version	1.3
 *	\date		June, 2, 2001
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Updated by Stephen G. Tuggy 2021-07-03
 * Updated by Stephen G. Tuggy 2022-01-06
 * Updated by Benjamen R. Meyer 2025-01-14
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "cmd/collide2/Opcode.h"

using namespace Opcode;

#include "OPC_SphereAABBOverlap.h"
#include "OPC_SphereTriOverlap.h"

#define SET_CONTACT(prim_index, flag)                                       \
    /* Set contact status */                                                \
    mFlags |= flag;                                                         \
    mTouchedPrimitives->Add(prim_index);

//! Sphere-triangle overlap test
#define SPHERE_PRIM(prim_index, flag)                                       \
    /* Request vertices from the app */                                     \
    VertexPointers VP;    mIMesh->GetTriangle(VP, prim_index);              \
                                                                            \
    /* Perform sphere-tri overlap test */                                   \
    if(SphereTriOverlap(*VP.Vertex[0], *VP.Vertex[1], *VP.Vertex[2]))       \
    {                                                                       \
        SET_CONTACT(prim_index, flag)                                       \
    }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SphereCollider::SphereCollider() {
    mCenter.Zero();
    mRadius2 = 0.0f;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SphereCollider::~SphereCollider() {
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Generic collision query for generic OPCODE models. After the call, access the results:
 *	- with GetContactStatus()
 *	- with GetNbTouchedPrimitives()
 *	- with GetTouchedPrimitives()
 *
 *	\param		cache		[in/out] a sphere cache
 *	\param		sphere		[in] collision sphere in local space
 *	\param		model		[in] Opcode model to collide with
 *	\param		worlds		[in] sphere's world matrix, or nullptr
 *	\param		worldm		[in] model's world matrix, or nullptr
 *	\return		true if success
 *	\warning	SCALE NOT SUPPORTED. The matrices must contain rotation & translation parts only.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool SphereCollider::Collide(SphereCache &cache,
        const Sphere &sphere,
        const Model &model,
        const Matrix4x4 *worlds,
        const Matrix4x4 *worldm) {
    // Checkings
    if (!Setup(&model)) {
        return false;
    }

    // Init collision query
    if (InitQuery(cache, sphere, worlds, worldm)) {
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
 *	- setup matrices
 *	- check temporal coherence
 *
 *	\param		cache		[in/out] a sphere cache
 *	\param		sphere		[in] sphere in local space
 *	\param		worlds		[in] sphere's world matrix, or nullptr
 *	\param		worldm		[in] model's world matrix, or nullptr
 *	\return		TRUE if we can return immediately
 *	\warning	SCALE NOT SUPPORTED. The matrices must contain rotation & translation parts only.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool SphereCollider::InitQuery(SphereCache &cache,
        const Sphere &sphere,
        const Matrix4x4 *worlds,
        const Matrix4x4 *worldm) {
    // 1) Call the base method
    VolumeCollider::InitQuery();

    // 2) Compute sphere in model space:
    // - Precompute R^2
    mRadius2 = sphere.mRadius * sphere.mRadius;
    // - Compute center position
    mCenter = sphere.mCenter;
    // -> to world space
    if (worlds) {
        mCenter *= *worlds;
    }
    // -> to model space
    if (worldm) {
        // Invert model matrix
        Matrix4x4 InvWorldM;
        InvertPRMatrix(InvWorldM, *worldm);

        mCenter *= InvWorldM;
    }

    // 3) Setup destination pointer
    mTouchedPrimitives = &cache.TouchedPrimitives;

    // 4) Special case: 1-triangle meshes [Opcode 1.3]
    if (mCurrentModel && mCurrentModel->HasSingleNode()) {
        if (!SkipPrimitiveTests()) {
            // We simply perform the BV-Prim overlap test each time. We assume single triangle has index 0.
            mTouchedPrimitives->Reset();

            // Perform overlap test between the unique triangle and the sphere (and set contact status if needed)
            SPHERE_PRIM(uint32_t(0), OPC_CONTACT)

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

                // Perform overlap test between the cached triangle and the sphere (and set contact status if needed)
                SPHERE_PRIM(PreviouslyTouchedFace, OPC_TEMPORAL_CONTACT)

                // Return immediately if possible
                if (GetContactStatus()) {
                    return TRUE;
                }
            }
            // else no face has been touched during previous query
            // => we'll have to perform a normal query
        } else {
            // We're interested in all contacts =>test the new real sphere N(ew) against the previous fat sphere P(revious):
            float r = sqrtf(cache.FatRadius2) - sphere.mRadius;
            if (IsCacheValid(cache) && cache.Center.SquareDistance(mCenter) < r * r) {
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

                // Make a fat sphere so that coherence will work for subsequent frames
                mRadius2 *= cache.FatCoeff;
//				mRadius2 = (sphere.mRadius * cache.FatCoeff)*(sphere.mRadius * cache.FatCoeff);

                // Update cache with query data (signature for cached faces)
                cache.Center = mCenter;
                cache.FatRadius2 = mRadius2;
            }
        }
    } else {
        // Here we don't use temporal coherence => do a normal query
        mTouchedPrimitives->Reset();
    }

    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Collision query for vanilla AABB trees.
 *	\param		cache		[in/out] a sphere cache
 *	\param		sphere		[in] collision sphere in world space
 *	\param		tree		[in] AABB tree
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool SphereCollider::Collide(SphereCache &cache, const Sphere &sphere, const AABBTree *tree) {
    // This is typically called for a scene tree, full of -AABBs-, not full of triangles.
    // So we don't really have "primitives" to deal with. Hence it doesn't work with
    // "FirstContact" + "TemporalCoherence".
    OPASSERT(!(FirstContactEnabled() && TemporalCoherenceEnabled()));

    // Checkings
    if (!tree) {
        return false;
    }

    // Init collision query
    if (InitQuery(cache, sphere)) {
        return true;
    }

    // Perform collision query
    _Collide(tree);

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Checks the sphere completely contains the box. In which case we can end the query sooner.
 *	\param		bc	[in] box center
 *	\param		be	[in] box extents
 *	\return		true if the sphere contains the whole box
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline_ bool SphereCollider::SphereContainsBox(const Point &bc, const Point &be) {
    // I assume if all 8 box vertices are inside the sphere, so does the whole box.
    // Sounds ok but maybe there's a better way?
    Point p;
    p.x = bc.x + be.x;
    p.y = bc.y + be.y;
    p.z = bc.z + be.z;
    if (mCenter.SquareDistance(p) >= mRadius2) {
        return FALSE;
    }
    p.x = bc.x - be.x;
    if (mCenter.SquareDistance(p) >= mRadius2) {
        return FALSE;
    }
    p.x = bc.x + be.x;
    p.y = bc.y - be.y;
    if (mCenter.SquareDistance(p) >= mRadius2) {
        return FALSE;
    }
    p.x = bc.x - be.x;
    if (mCenter.SquareDistance(p) >= mRadius2) {
        return FALSE;
    }
    p.x = bc.x + be.x;
    p.y = bc.y + be.y;
    p.z = bc.z - be.z;
    if (mCenter.SquareDistance(p) >= mRadius2) {
        return FALSE;
    }
    p.x = bc.x - be.x;
    if (mCenter.SquareDistance(p) >= mRadius2) {
        return FALSE;
    }
    p.x = bc.x + be.x;
    p.y = bc.y - be.y;
    if (mCenter.SquareDistance(p) >= mRadius2) {
        return FALSE;
    }
    p.x = bc.x - be.x;
    if (mCenter.SquareDistance(p) >= mRadius2) {
        return FALSE;
    }

    return TRUE;
}

#define TEST_BOX_IN_SPHERE(center, extents) \
    if(SphereContainsBox(center, extents))  \
    {                                       \
        /* Set contact status */            \
        mFlags |= OPC_CONTACT;              \
        _Dump(node);                        \
        return;                             \
    }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive collision query for normal AABB trees.
 *	\param		node	[in] current collision node
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SphereCollider::_Collide(const AABBCollisionNode *node) {
    // Perform Sphere-AABB overlap test
    if (!SphereAABBOverlap(node->mAABB.mCenter, node->mAABB.mExtents)) {
        return;
    }

    TEST_BOX_IN_SPHERE(node->mAABB.mCenter, node->mAABB.mExtents)

    if (node->IsLeaf()) {
        SPHERE_PRIM(node->GetPrimitive(), OPC_CONTACT)
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
void SphereCollider::_CollideNoPrimitiveTest(const AABBCollisionNode *node) {
    // Perform Sphere-AABB overlap test
    if (!SphereAABBOverlap(node->mAABB.mCenter, node->mAABB.mExtents)) {
        return;
    }

    TEST_BOX_IN_SPHERE(node->mAABB.mCenter, node->mAABB.mExtents)

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
void SphereCollider::_Collide(const AABBQuantizedNode *node) {
    // Dequantize box
    const QuantizedAABB &Box = node->mAABB;
    const Point Center(float(Box.mCenter[0]) * mCenterCoeff.x,
            float(Box.mCenter[1]) * mCenterCoeff.y,
            float(Box.mCenter[2]) * mCenterCoeff.z);
    const Point Extents(float(Box.mExtents[0]) * mExtentsCoeff.x,
            float(Box.mExtents[1]) * mExtentsCoeff.y,
            float(Box.mExtents[2]) * mExtentsCoeff.z);

    // Perform Sphere-AABB overlap test
    if (!SphereAABBOverlap(Center, Extents)) {
        return;
    }

    TEST_BOX_IN_SPHERE(Center, Extents)

    if (node->IsLeaf()) {
        SPHERE_PRIM(node->GetPrimitive(), OPC_CONTACT)
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
void SphereCollider::_CollideNoPrimitiveTest(const AABBQuantizedNode *node) {
    // Dequantize box
    const QuantizedAABB &Box = node->mAABB;
    const Point Center(float(Box.mCenter[0]) * mCenterCoeff.x,
            float(Box.mCenter[1]) * mCenterCoeff.y,
            float(Box.mCenter[2]) * mCenterCoeff.z);
    const Point Extents(float(Box.mExtents[0]) * mExtentsCoeff.x,
            float(Box.mExtents[1]) * mExtentsCoeff.y,
            float(Box.mExtents[2]) * mExtentsCoeff.z);

    // Perform Sphere-AABB overlap test
    if (!SphereAABBOverlap(Center, Extents)) {
        return;
    }

    TEST_BOX_IN_SPHERE(Center, Extents)

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
void SphereCollider::_Collide(const AABBNoLeafNode *node) {
    // Perform Sphere-AABB overlap test
    if (!SphereAABBOverlap(node->mAABB.mCenter, node->mAABB.mExtents)) {
        return;
    }

    TEST_BOX_IN_SPHERE(node->mAABB.mCenter, node->mAABB.mExtents)

    if (node->HasPosLeaf()) {
        SPHERE_PRIM(node->GetPosPrimitive(), OPC_CONTACT)
    } else {
        _Collide(node->GetPos());
    }

    if (ContactFound()) {
        return;
    }

    if (node->HasNegLeaf()) {
        SPHERE_PRIM(node->GetNegPrimitive(), OPC_CONTACT)
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
void SphereCollider::_CollideNoPrimitiveTest(const AABBNoLeafNode *node) {
    // Perform Sphere-AABB overlap test
    if (!SphereAABBOverlap(node->mAABB.mCenter, node->mAABB.mExtents)) {
        return;
    }

    TEST_BOX_IN_SPHERE(node->mAABB.mCenter, node->mAABB.mExtents)

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
void SphereCollider::_Collide(const AABBQuantizedNoLeafNode *node) {
    // Dequantize box
    const QuantizedAABB &Box = node->mAABB;
    const Point Center(float(Box.mCenter[0]) * mCenterCoeff.x,
            float(Box.mCenter[1]) * mCenterCoeff.y,
            float(Box.mCenter[2]) * mCenterCoeff.z);
    const Point Extents(float(Box.mExtents[0]) * mExtentsCoeff.x,
            float(Box.mExtents[1]) * mExtentsCoeff.y,
            float(Box.mExtents[2]) * mExtentsCoeff.z);

    // Perform Sphere-AABB overlap test
    if (!SphereAABBOverlap(Center, Extents)) {
        return;
    }

    TEST_BOX_IN_SPHERE(Center, Extents)

    if (node->HasPosLeaf()) {
        SPHERE_PRIM(node->GetPosPrimitive(), OPC_CONTACT)
    } else {
        _Collide(node->GetPos());
    }

    if (ContactFound()) {
        return;
    }

    if (node->HasNegLeaf()) {
        SPHERE_PRIM(node->GetNegPrimitive(), OPC_CONTACT)
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
void SphereCollider::_CollideNoPrimitiveTest(const AABBQuantizedNoLeafNode *node) {
    // Dequantize box
    const QuantizedAABB &Box = node->mAABB;
    const Point Center(float(Box.mCenter[0]) * mCenterCoeff.x,
            float(Box.mCenter[1]) * mCenterCoeff.y,
            float(Box.mCenter[2]) * mCenterCoeff.z);
    const Point Extents(float(Box.mExtents[0]) * mExtentsCoeff.x,
            float(Box.mExtents[1]) * mExtentsCoeff.y,
            float(Box.mExtents[2]) * mExtentsCoeff.z);

    // Perform Sphere-AABB overlap test
    if (!SphereAABBOverlap(Center, Extents)) {
        return;
    }

    TEST_BOX_IN_SPHERE(Center, Extents)

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
void SphereCollider::_Collide(const AABBTreeNode *node) {
    // Perform Sphere-AABB overlap test
    Point Center, Extents;
    node->GetAABB()->GetCenter(Center);
    node->GetAABB()->GetExtents(Extents);
    if (!SphereAABBOverlap(Center, Extents)) {
        return;
    }

    if (node->IsLeaf() || SphereContainsBox(Center, Extents)) {
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
HybridSphereCollider::HybridSphereCollider() {
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
HybridSphereCollider::~HybridSphereCollider() {
}

bool HybridSphereCollider::Collide(SphereCache &cache,
        const Sphere &sphere,
        const HybridModel &model,
        const Matrix4x4 *worlds,
        const Matrix4x4 *worldm) {
    // We don't want primitive tests here!
    mFlags |= OPC_NO_PRIMITIVE_TESTS;

    // Checkings
    if (!Setup(&model)) {
        return false;
    }

    // Init collision query
    if (InitQuery(cache, sphere, worlds, worldm)) {
        return true;
    }

    // Special case for 1-leaf trees
    if (mCurrentModel && mCurrentModel->HasSingleNode()) {
        // Here we're supposed to perform a normal query, except our tree has a single node, i.e. just a few triangles
        uint32_t Nb = mIMesh->GetNbTriangles();

        // Loop through all triangles
        for (uint32_t i = 0; i < Nb; i++) {
            SPHERE_PRIM(i, OPC_CONTACT)
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
                    SPHERE_PRIM(TriangleIndex, OPC_CONTACT)
                }
            } else {
                uint32_t BaseIndex = CurrentLeaf.GetTriangleIndex();

                // Loop through triangles and test each of them
                while (NbTris--) {
                    uint32_t TriangleIndex = BaseIndex++;
                    SPHERE_PRIM(TriangleIndex, OPC_CONTACT)
                }
            }
        }
    }

    return true;
}

