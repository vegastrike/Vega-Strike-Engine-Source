/*
 * Copyright (C) 2001-2025 Daniel Horn, Pierre Terdiman, pyramid3d,
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
 *	Contains code for a versatile AABB tree.
 *	\file		OPC_AABBTree.cpp
 *	\author		Pierre Terdiman
 *	\date		March, 20, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a generic AABB tree node.
 *
 *	\class		AABBTreeNode
 *	\author		Pierre Terdiman
 *	\version	1.3
 *	\date		March, 20, 2001
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a generic AABB tree.
 *	This is a vanilla AABB tree, without any particular optimization. It contains anonymous references to
 *	user-provided primitives, which can theoretically be anything - triangles, boxes, etc. Each primitive
 *	is surrounded by an AABB, regardless of the primitive's nature. When the primitive is a triangle, the
 *	resulting tree can be converted into an optimized tree. If the primitive is a box, the resulting tree
 *	can be used for culling - VFC or occlusion -, assuming you cull on a mesh-by-mesh basis (modern way).
 *
 *	\class		AABBTree
 *	\author		Pierre Terdiman
 *	\version	1.3
 *	\date		March, 20, 2001
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Updated by Stephen G. Tuggy 2021-07-03
 * Updated by Stephen G. Tuggy 2022-01-06
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Precompiled Header
#include "cmd/collide2/Opcode.h"

using namespace Opcode;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AABBTreeNode::AABBTreeNode() :
        mPos(0),
#ifndef OPC_NO_NEG_VANILLA_TREE
        mNeg			(0),
#endif
        mNodePrimitives(nullptr),
        mNbPrimitives(0) {
#ifdef OPC_USE_TREE_COHERENCE
    mBitmask = 0;
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AABBTreeNode::~AABBTreeNode() {
    // Opcode 1.3:
    const AABBTreeNode *Pos = GetPos();
#ifndef OPC_NO_NEG_VANILLA_TREE
    const AABBTreeNode* Neg = GetNeg();
    if(!(mPos&1))	DELETESINGLE(Pos);
    if(!(mNeg&1))	DELETESINGLE(Neg);
#else
    if (!(mPos & 1)) {
        DELETEARRAY(Pos);
    }
#endif
    mNodePrimitives = nullptr;    // This was just a shortcut to the global list => no release
    mNbPrimitives = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Splits the node along a given axis.
 *	The list of indices is reorganized according to the split values.
 *	\param		axis		[in] splitting axis index
 *	\param		builder		[in] the tree builder
 *	\return		the number of primitives assigned to the first child
 *	\warning	this method reorganizes the internal list of primitives
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint32_t AABBTreeNode::Split(uint32_t axis, AABBTreeBuilder *builder) {
    // Get node split value
    float SplitValue = builder->GetSplittingValue(mNodePrimitives, mNbPrimitives, mBV, axis);

    uint32_t NbPos = 0;
    // Loop through all node-related primitives. Their indices range from mNodePrimitives[0] to mNodePrimitives[mNbPrimitives-1].
    // Those indices map the global list in the tree builder.
    for (uint32_t i = 0; i < mNbPrimitives; i++) {
        // Get index in global list
        uint32_t Index = mNodePrimitives[i];

        // Test against the splitting value. The primitive value is tested against the enclosing-box center.
        // [We only need an approximate partition of the enclosing box here.]
        float PrimitiveValue = builder->GetSplittingValue(Index, axis);

        // Reorganize the list of indices in this order: positive - negative.
        if (PrimitiveValue > SplitValue) {
            // Swap entries
            uint32_t Tmp = mNodePrimitives[i];
            mNodePrimitives[i] = mNodePrimitives[NbPos];
            mNodePrimitives[NbPos] = Tmp;
            // Count primitives assigned to positive space
            NbPos++;
        }
    }
    return NbPos;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Subdivides the node.
 *	
 *	          N
 *	        /   \
 *	      /       \
 *	   N/2         N/2
 *	  /   \       /   \
 *	N/4   N/4   N/4   N/4
 *	(etc)
 *
 *	A well-balanced tree should have a O(log n) depth.
 *	A degenerate tree would have a O(n) depth.
 *	Note a perfectly-balanced tree is not well-suited to collision detection anyway.
 *
 *	\param		builder		[in] the tree builder
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABBTreeNode::Subdivide(AABBTreeBuilder *builder) {
    // Checkings
    if (!builder) {
        return false;
    }

    // Stop subdividing if we reach a leaf node. This is always performed here,
    // else we could end in trouble if user overrides this.
    if (mNbPrimitives == 1) {
        return true;
    }

    // Let the user validate the subdivision
    if (!builder->ValidateSubdivision(mNodePrimitives, mNbPrimitives, mBV)) {
        return true;
    }

    bool ValidSplit = true;    // Optimism...
    uint32_t NbPos;
    if (builder->mSettings.mRules & SPLIT_LARGEST_AXIS) {
        // Find the largest axis to split along
        Point Extents;
        mBV.GetExtents(Extents);    // Box extents
        uint32_t Axis = Extents.LargestAxis();        // Index of largest axis

        // Split along the axis
        NbPos = Split(Axis, builder);

        // Check split validity
        if (!NbPos || NbPos == mNbPrimitives) {
            ValidSplit = false;
        }
    } else if (builder->mSettings.mRules & SPLIT_SPLATTER_POINTS) {
        uint32_t i;
        // Compute the means
        Point Means(0.0f, 0.0f, 0.0f);
        for (i = 0; i < mNbPrimitives; i++) {
            uint32_t Index = mNodePrimitives[i];
            Means.x += builder->GetSplittingValue(Index, 0);
            Means.y += builder->GetSplittingValue(Index, 1);
            Means.z += builder->GetSplittingValue(Index, 2);
        }
        Means /= float(mNbPrimitives);

        // Compute variances
        Point Vars(0.0f, 0.0f, 0.0f);
        for (i = 0; i < mNbPrimitives; i++) {
            uint32_t Index = mNodePrimitives[i];
            float Cx = builder->GetSplittingValue(Index, 0);
            float Cy = builder->GetSplittingValue(Index, 1);
            float Cz = builder->GetSplittingValue(Index, 2);
            Vars.x += (Cx - Means.x) * (Cx - Means.x);
            Vars.y += (Cy - Means.y) * (Cy - Means.y);
            Vars.z += (Cz - Means.z) * (Cz - Means.z);
        }
        Vars /= float(mNbPrimitives - 1);

        // Choose axis with greatest variance
        uint32_t Axis = Vars.LargestAxis();

        // Split along the axis
        NbPos = Split(Axis, builder);

        // Check split validity
        if (!NbPos || NbPos == mNbPrimitives) {
            ValidSplit = false;
        }
    } else if (builder->mSettings.mRules & SPLIT_BALANCED) {
        // Test 3 axis, take the best
        float Results[3];
        NbPos = Split(0, builder);
        Results[0] = float(NbPos) / float(mNbPrimitives);
        NbPos = Split(1, builder);
        Results[1] = float(NbPos) / float(mNbPrimitives);
        NbPos = Split(2, builder);
        Results[2] = float(NbPos) / float(mNbPrimitives);
        Results[0] -= 0.5f;
        Results[0] *= Results[0];
        Results[1] -= 0.5f;
        Results[1] *= Results[1];
        Results[2] -= 0.5f;
        Results[2] *= Results[2];
        uint32_t Min = 0;
        if (Results[1] < Results[Min]) {
            Min = 1;
        }
        if (Results[2] < Results[Min]) {
            Min = 2;
        }

        // Split along the axis
        NbPos = Split(Min, builder);

        // Check split validity
        if (!NbPos || NbPos == mNbPrimitives) {
            ValidSplit = false;
        }
    } else if (builder->mSettings.mRules & SPLIT_BEST_AXIS) {
        // Test largest, then middle, then smallest axis...

        // Sort axis
        Point Extents;
        mBV.GetExtents(Extents);    // Box extents
        uint32_t SortedAxis[] = {0, 1, 2};
        float *Keys = (float *) &Extents.x;
        for (uint32_t j = 0; j < 3; j++) {
            for (uint32_t i = 0; i < 2; i++) {
                if (Keys[SortedAxis[i]] < Keys[SortedAxis[i + 1]]) {
                    uint32_t Tmp = SortedAxis[i];
                    SortedAxis[i] = SortedAxis[i + 1];
                    SortedAxis[i + 1] = Tmp;
                }
            }
        }

        // Find the largest axis to split along
        uint32_t CurAxis = 0;
        ValidSplit = false;
        while (!ValidSplit && CurAxis != 3) {
            NbPos = Split(SortedAxis[CurAxis], builder);
            // Check the subdivision has been successful
            if (!NbPos || NbPos == mNbPrimitives) {
                CurAxis++;
            } else {
                ValidSplit = true;
            }
        }
    } else if (builder->mSettings.mRules & SPLIT_FIFTY) {
        // Don't even bother splitting (mainly a performance test)
        NbPos = mNbPrimitives >> 1;
    } else {
        return false;
    }    // Unknown splitting rules

    // Check the subdivision has been successful
    if (!ValidSplit) {
        // Here, all boxes lie in the same sub-space. Two strategies:
        // - if the tree *must* be complete, make an arbitrary 50-50 split
        // - else stop subdividing
//		if(builder->mSettings.mRules&SPLIT_COMPLETE)
        if (builder->mSettings.mLimit == 1) {
            builder->IncreaseNbInvalidSplits();
            NbPos = mNbPrimitives >> 1;
        } else {
            return true;
        }
    }

    // Now create children and assign their pointers.
    if (builder->mNodeBase) {
        // We use a pre-allocated linear pool for complete trees [Opcode 1.3]
        AABBTreeNode *Pool = (AABBTreeNode *) builder->mNodeBase;
        uint32_t Count = builder->GetCount() - 1;    // Count begins to 1...
        // Set last bit to tell it shouldn't be freed ### pretty ugly, find a better way. Maybe one bit in mNbPrimitives
        OPASSERT(!(uintptr_t(&Pool[Count + 0]) & 1));
        OPASSERT(!(uintptr_t(&Pool[Count + 1]) & 1));
        mPos = uintptr_t(&Pool[Count + 0]) | 1;
#ifndef OPC_NO_NEG_VANILLA_TREE
        mNeg = uintptr_t(&Pool[Count+1])|1;
#endif
    } else {
        // Non-complete trees and/or Opcode 1.2 allocate nodes on-the-fly
#ifndef OPC_NO_NEG_VANILLA_TREE
        mPos = (uintptr_t)new AABBTreeNode;	CHECKALLOC(mPos);
        mNeg = (uintptr_t)new AABBTreeNode;	CHECKALLOC(mNeg);
#else
        AABBTreeNode *PosNeg = new AABBTreeNode[2];
        CHECKALLOC(PosNeg);
        mPos = (uintptr_t) PosNeg;
#endif
    }

    // Update stats
    builder->IncreaseCount(2);

    // Assign children
    AABBTreeNode *Pos = (AABBTreeNode *) GetPos();
    AABBTreeNode *Neg = (AABBTreeNode *) GetNeg();
    Pos->mNodePrimitives = &mNodePrimitives[0];
    Pos->mNbPrimitives = NbPos;
    Neg->mNodePrimitives = &mNodePrimitives[NbPos];
    Neg->mNbPrimitives = mNbPrimitives - NbPos;

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive hierarchy building in a top-down fashion.
 *	\param		builder		[in] the tree builder
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AABBTreeNode::_BuildHierarchy(AABBTreeBuilder *builder) {
    // 1) Compute the global box for current node. The box is stored in mBV.
    builder->ComputeGlobalBox(mNodePrimitives, mNbPrimitives, mBV);

    // 2) Subdivide current node
    Subdivide(builder);

    // 3) Recurse
    AABBTreeNode *Pos = (AABBTreeNode *) GetPos();
    AABBTreeNode *Neg = (AABBTreeNode *) GetNeg();
    if (Pos) {
        Pos->_BuildHierarchy(builder);
    }
    if (Neg) {
        Neg->_BuildHierarchy(builder);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Refits the tree (top-down).
 *	\param		builder		[in] the tree builder
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AABBTreeNode::_Refit(AABBTreeBuilder *builder) {
    // 1) Recompute the new global box for current node
    builder->ComputeGlobalBox(mNodePrimitives, mNbPrimitives, mBV);

    // 2) Recurse
    AABBTreeNode *Pos = (AABBTreeNode *) GetPos();
    AABBTreeNode *Neg = (AABBTreeNode *) GetNeg();
    if (Pos) {
        Pos->_Refit(builder);
    }
    if (Neg) {
        Neg->_Refit(builder);
    }
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AABBTree::AABBTree() : mIndices(nullptr), mPool(nullptr), mTotalNbNodes(0) {
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AABBTree::~AABBTree() {
    Release();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Releases the tree.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AABBTree::Release() {
    DELETEARRAY(mPool);
    DELETEARRAY(mIndices);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Builds a generic AABB tree from a tree builder.
 *	\param		builder		[in] the tree builder
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABBTree::Build(AABBTreeBuilder *builder) {
    // Checkings
    if (!builder || !builder->mNbPrimitives) {
        return false;
    }

    // Release previous tree
    Release();

    // Init stats
    builder->SetCount(1);
    builder->SetNbInvalidSplits(0);

    // Initialize indices. This list will be modified during build.
    mIndices = new uint32_t[builder->mNbPrimitives];
    CHECKALLOC(mIndices);
    // Identity permutation
    for (uint32_t i = 0; i < builder->mNbPrimitives; i++) {
        mIndices[i] = i;
    }

    // Setup initial node. Here we have a complete permutation of the app's primitives.
    mNodePrimitives = mIndices;
    mNbPrimitives = builder->mNbPrimitives;

    // Use a linear array for complete trees (since we can predict the final number of nodes) [Opcode 1.3]
//	if(builder->mRules&SPLIT_COMPLETE)
    if (builder->mSettings.mLimit == 1) {
        // Allocate a pool of nodes
        mPool = new AABBTreeNode[builder->mNbPrimitives * 2 - 1];

        builder->mNodeBase = mPool;    // ### ugly !
    }

    // Build the hierarchy
    _BuildHierarchy(builder);

    // Get back total number of nodes
    mTotalNbNodes = builder->GetCount();

    // For complete trees, check the correct number of nodes has been created [Opcode 1.3]
    if (mPool) OPASSERT(mTotalNbNodes == builder->mNbPrimitives * 2 - 1);

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Computes the depth of the tree.
 *	A well-balanced tree should have a log(n) depth. A degenerate tree O(n) depth.
 *	\return		depth of the tree
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint32_t AABBTree::ComputeDepth() const {
    return Walk(nullptr, nullptr);    // Use the walking code without callback
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Walks the tree, calling the user back for each node.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint32_t AABBTree::Walk(WalkingCallback callback, void *user_data) const {
    // Call it without callback to compute max depth
    uint32_t MaxDepth = 0;
    uint32_t CurrentDepth = 0;

    struct Local {
        static void _Walk(const AABBTreeNode *current_node,
                uint32_t &max_depth,
                uint32_t &current_depth,
                WalkingCallback callback,
                void *user_data) {
            // Checkings
            if (!current_node) {
                return;
            }
            // Entering a new node => increase depth
            current_depth++;
            // Keep track of max depth
            if (current_depth > max_depth) {
                max_depth = current_depth;
            }

            // Callback
            if (callback && !(callback)(current_node, current_depth, user_data)) {
                return;
            }

            // Recurse
            if (current_node->GetPos()) {
                _Walk(current_node->GetPos(), max_depth, current_depth, callback, user_data);
                current_depth--;
            }
            if (current_node->GetNeg()) {
                _Walk(current_node->GetNeg(), max_depth, current_depth, callback, user_data);
                current_depth--;
            }
        }
    };
    Local::_Walk(this, MaxDepth, CurrentDepth, callback, user_data);
    return MaxDepth;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Refits the tree in a top-down way.
 *	\param		builder		[in] the tree builder
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABBTree::Refit(AABBTreeBuilder *builder) {
    if (!builder) {
        return false;
    }
    _Refit(builder);
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Refits the tree in a bottom-up way.
 *	\param		builder		[in] the tree builder
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABBTree::Refit2(AABBTreeBuilder *builder) {
    // Checkings
    if (!builder) {
        return false;
    }

    OPASSERT(mPool);

    // Bottom-up update
    Point Min, Max;
    Point Min_, Max_;
    uint32_t Index = mTotalNbNodes;
    while (Index--) {
        AABBTreeNode &Current = mPool[Index];

        if (Current.IsLeaf()) {
            builder->ComputeGlobalBox(Current.GetPrimitives(), Current.GetNbPrimitives(), *(AABB *) Current.GetAABB());
        } else {
            Current.GetPos()->GetAABB()->GetMin(Min);
            Current.GetPos()->GetAABB()->GetMax(Max);

            Current.GetNeg()->GetAABB()->GetMin(Min_);
            Current.GetNeg()->GetAABB()->GetMax(Max_);

            Min.Min(Min_);
            Max.Max(Max_);

            ((AABB *) Current.GetAABB())->SetMinMax(Min, Max);
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Computes the number of bytes used by the tree.
 *	\return		number of bytes used
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
size_t AABBTree::GetUsedBytes() const {
    // return SIZEOFOBJECT;
    size_t TotalSize = mTotalNbNodes * GetNodeSize();
    if (mIndices) {
        TotalSize += mNbPrimitives * sizeof(uint32_t);
    }
    return TotalSize;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Checks the tree is a complete tree or not.
 *	A complete tree is made of 2*N-1 nodes, where N is the number of primitives in the tree.
 *	\return		true for complete trees
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABBTree::IsComplete() const {
    return (GetNbNodes() == GetNbPrimitives() * 2 - 1);
}

