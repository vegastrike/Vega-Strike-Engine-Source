///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/Opcode.htm
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for OPCODE models.
 *	\file		OPC_Model.cpp
 *	\author		Pierre Terdiman
 *	\date		March, 20, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Updated by Stephen G. Tuggy 2021-07-03
 * Updated by Stephen G. Tuggy 2022-01-06
 * Updated by Benjamen R. Meyer 2025-01-14
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	The main collision wrapper, for all trees. Supported trees are:
 *	- Normal trees (2*N-1 nodes, full size)
 *	- No-leaf trees (N-1 nodes, full size)
 *	- Quantized trees (2*N-1 nodes, half size)
 *	- Quantized no-leaf trees (N-1 nodes, half size)
 *
 *	Usage:
 *
 *	1) Create a static mesh interface using callbacks or pointers. (see OPC_MeshInterface.cpp).
 *	Keep it around in your app, since a pointer to this interface is saved internally and
 *	used until you release the collision structures.
 *
 *	2) Build a Model using a creation structure:
 *
 *	\code
 *		Model Sample;
 *
 *		OPCODECREATE OPCC;
 *		OPCC.IMesh			= ...;
 *		OPCC.Rules			= ...;
 *		OPCC.NoLeaf			= ...;
 *		OPCC.Quantized		= ...;
 *		OPCC.KeepOriginal	= ...;
 *		bool Status = Sample.Build(OPCC);
 *	\endcode
 *
 *	3) Create a tree collider and set it up:
 *
 *	\code
 *		AABBTreeCollider TC;
 *		TC.SetFirstContact(...);
 *		TC.SetFullBoxBoxTest(...);
 *		TC.SetFullPrimBoxTest(...);
 *		TC.SetTemporalCoherence(...);
 *	\endcode
 *
 *	4) Perform a collision query
 *
 *	\code
 *		// Setup cache
 *		static BVTCache ColCache;
 *		ColCache.Model0 = &Model0;
 *		ColCache.Model1 = &Model1;
 *
 *		// Collision query
 *		bool IsOk = TC.Collide(ColCache, World0, World1);
 *
 *		// Get collision status => if true, objects overlap
 *		BOOL Status = TC.GetContactStatus();
 *
 *		// Number of colliding pairs and list of pairs
 *		uint32_t NbPairs = TC.GetNbPairs();
 *		const Pair* p = TC.GetPairs()
 *	\endcode
 *
 *	5) Stats
 *
 *	\code
 *		Model0.GetUsedBytes()	= number of bytes used for this collision tree
 *		TC.GetNbBVBVTests()		= number of BV-BV overlap tests performed during last query
 *		TC.GetNbPrimPrimTests()	= number of Triangle-Triangle overlap tests performed during last query
 *		TC.GetNbBVPrimTests()	= number of Triangle-BV overlap tests performed during last query
 *	\endcode
 *
 *	\class		Model
 *	\author		Pierre Terdiman
 *	\version	1.3
 *	\date		March, 20, 2001
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "cmd/collide2/Opcode.h"

using namespace Opcode;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Model::Model() {
#ifdef __MESHMERIZER_H__    // Collision hulls only supported within ICE !
    mHull	= nullptr;
#endif // __MESHMERIZER_H__
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Model::~Model() {
    Release();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Releases the model.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Model::Release() {
    ReleaseBase();
#ifdef __MESHMERIZER_H__    // Collision hulls only supported within ICE !
    DELETESINGLE(mHull);
#endif // __MESHMERIZER_H__
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Builds a collision model.
 *	\param		create		[in] model creation structure
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Model::Build(const OPCODECREATE &create) {
    // 1) Checkings
    if (!create.mIMesh || !create.mIMesh->IsValid()) {
        return false;
    }

    // For this model, we only support complete trees
//	if(create.mSettings.mLimit!=1)	return SetIceError ("OPCODE WARNING: supports complete trees only! Use mLimit = 1.", nullptr);
    if (create.mSettings.mLimit != 1) {
        return (false);
    }
    // Look for degenerate faces.
    create.mIMesh->CheckTopology();
    // We continue nonetheless....

    Release();    // Make sure previous tree has been discarded [Opcode 1.3, thanks Adam]

    // 1-1) Setup mesh interface automatically [Opcode 1.3]
    SetMeshInterface(create.mIMesh);

    // Special case for 1-triangle meshes [Opcode 1.3]
    uint32_t NbTris = create.mIMesh->GetNbTriangles();
    if (NbTris == 1) {
        // We don't need to actually create a tree here, since we'll only have a single triangle to deal with anyway.
        // It's a waste to use a "model" for this but at least it will work.
        mModelCode |= OPC_SINGLE_NODE;
        return true;
    }

    // 2) Build a generic AABB Tree.
    mSource = new AABBTree;
    CHECKALLOC(mSource);

    // 2-1) Setup a builder. Our primitives here are triangles from input mesh,
    // so we use an AABBTreeOfTrianglesBuilder.....
    {
        AABBTreeOfTrianglesBuilder TB;
        TB.mIMesh = create.mIMesh;
        TB.mSettings = create.mSettings;
        TB.mNbPrimitives = NbTris;
        if (!mSource->Build(&TB)) {
            return false;
        }
    }

    // 3) Create an optimized tree according to user-settings
    if (!CreateTree(create.mNoLeaf, create.mQuantized)) {
        return false;
    }

    // 3-2) Create optimized tree
    if (!mTree->Build(mSource)) {
        return false;
    }

    // 3-3) Delete generic tree if needed
    if (!create.mKeepOriginal) {
        DELETESINGLE(mSource);
    }

#ifdef __MESHMERIZER_H__
    // 4) Convex hull
    if(create.mCollisionHull)
    {
        // Create hull
        mHull = new CollisionHull;
        CHECKALLOC(mHull);

        CONVEXHULLCREATE CHC;
        // ### doesn't work with strides
        CHC.NbVerts			= create.mIMesh->GetNbVertices();
        CHC.Vertices		= create.mIMesh->GetVerts();
        CHC.UnifyNormals	= true;
        CHC.ReduceVertices	= true;
        CHC.WordFaces		= false;
        mHull->Compute(CHC);
    }
#endif // __MESHMERIZER_H__

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Gets the number of bytes used by the tree.
 *	\return		amount of bytes used
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
size_t Model::GetUsedBytes() const {
    if (!mTree) {
        return 0;
    }
    return mTree->GetUsedBytes();
}

