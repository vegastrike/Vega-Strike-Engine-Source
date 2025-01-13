/*
 * OPC_AABBTree.h
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
 *	Contains code for a versatile AABB tree.
 *	\file		OPC_AABBTree.h
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
#ifndef VEGA_STRIKE_ENGINE_CMD_COLLSION2_OPC_AABBTREE_H
#define VEGA_STRIKE_ENGINE_CMD_COLLSION2_OPC_AABBTREE_H

#ifdef OPC_NO_NEG_VANILLA_TREE
//! TO BE DOCUMENTED
#define IMPLEMENT_TREE(base_class, volume)                                                                                      \
        public:                                                                                                                 \
        /* Constructor / Destructor */                                                                                          \
                                    base_class();                                                                               \
                                    ~base_class();                                                                              \
        /* Data access */                                                                                                       \
        inline_    const volume*        Get##volume()    const    { return &mBV;                            }                   \
        /* Clear the last bit */                                                                                                \
        inline_    const base_class*    GetPos()        const    { return (const base_class*)(mPos&~1);    }                    \
        inline_    const base_class*    GetNeg()        const    { const base_class* P = GetPos(); return P ? P+1 : nullptr;}   \
                                                                                                                                \
        /* We don't need to test both nodes since we can't have one without the other	*/                                      \
        inline_    bool                IsLeaf()        const    { return !GetPos();                        }                    \
                                                                                                                                \
        /* Stats */                                                                                                             \
        inline_    size_t                GetNodeSize()    const    { return SIZEOFOBJECT;                    }                  \
        protected:                                                                                                              \
        /* Tree-independent data */                                                                                             \
        /* Following data always belong to the BV-tree, regardless of what the tree actually contains.*/                        \
        /* Whatever happens we need the two children and the enclosing volume.*/                                                \
                volume                mBV;        /* Global bounding-volume enclosing all the node-related primitives */        \
                uintptr_t            mPos;        /* "Positive" & "Negative" children */
#else
//! TO BE DOCUMENTED
	#define IMPLEMENT_TREE(base_class, volume)																			    \
		public:																											    \
		/* Constructor / Destructor */																					    \
									base_class();																		    \
									~base_class();																		    \

		/* Data access */																								    \
		inline_	const volume*		Get##volume()	const	{ return &mBV;							}					    \
		/* Clear the last bit */																						    \
		inline_	const base_class*	GetPos()		const	{ return (const base_class*)(mPos&~1);	}					    \
		inline_	const base_class*	GetNeg()		const	{ return (const base_class*)(mNeg&~1);	}					    \
																														    \
/*		inline_	bool				IsLeaf()		const	{ return (!GetPos() && !GetNeg());	}	*/					    \
		/* We don't need to test both nodes since we can't have one without the other	*/								    \
		inline_	bool				IsLeaf()		const	{ return !GetPos();						}					    \
																														    \
		/* Stats */																										    \
		inline_	size_t				GetNodeSize()	const	{ return SIZEOFOBJECT;					}					    \
		protected:																										    \
		/* Tree-independent data */																						    \
		/* Following data always belong to the BV-tree, regardless of what the tree actually contains.*/				    \
		/* Whatever happens we need the two children and the enclosing volume.*/										    \
				volume				mBV;		/* Global bounding-volume enclosing all the node-related primitives */	    \
				uintptr_t			mPos;		/* "Positive" child */													    \
				uintptr_t			mNeg;		/* "Negative" child */
#endif

typedef void
(*CullingCallback)(uint32_t nb_primitives, uint32_t *node_primitives, BOOL need_clipping, void *user_data);

class OPCODE_API AABBTreeNode {
IMPLEMENT_TREE(AABBTreeNode, AABB)
public:
    // Data access
    inline_    const uint32_t *GetPrimitives() const {
        return mNodePrimitives;
    }

    inline_    uint32_t GetNbPrimitives() const {
        return mNbPrimitives;
    }

protected:
    // Tree-dependent data
    uint32_t *mNodePrimitives;    //!< Node-related primitives (shortcut to a position in mIndices below)
    uint32_t mNbPrimitives;        //!< Number of primitives for this node
    // Internal methods
    uint32_t Split(uint32_t axis, AABBTreeBuilder *builder);
    bool Subdivide(AABBTreeBuilder *builder);
    void _BuildHierarchy(AABBTreeBuilder *builder);
    void _Refit(AABBTreeBuilder *builder);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	User-callback, called for each node by the walking code.
 *	\param		current		[in] current node
 *	\param		depth		[in] current node's depth
 *	\param		user_data	[in] user-defined data
 *	\return		true to recurse through children, else false to bypass them
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef bool                (*WalkingCallback)(const AABBTreeNode *current, uint32_t depth, void *user_data);

class OPCODE_API AABBTree : public AABBTreeNode {
public:
    // Constructor / Destructor
    AABBTree();
    ~AABBTree();
    // Build
    bool Build(AABBTreeBuilder *builder);
    void Release();

    // Data access
    inline_    const uint32_t *GetIndices() const {
        return mIndices;
    }    //!< Catch the indices
    inline_    uint32_t GetNbNodes() const {
        return mTotalNbNodes;
    }    //!< Catch the number of nodes

    // Infos
    bool IsComplete() const;
    // Stats
    uint32_t ComputeDepth() const;
    size_t GetUsedBytes() const;
    uint32_t Walk(WalkingCallback callback, void *user_data) const;

    bool Refit(AABBTreeBuilder *builder);
    bool Refit2(AABBTreeBuilder *builder);
private:
    uint32_t *mIndices;            //!< Indices in the app list. Indices are reorganized during build (permutation).
    AABBTreeNode *mPool;                //!< Linear pool of nodes for complete trees. Null otherwise. [Opcode 1.3]
    // Stats
    uint32_t mTotalNbNodes;        //!< Number of nodes in the tree.
};

#endif //VEGA_STRIKE_ENGINE_CMD_COLLSION2_OPC_AABBTREE_H
