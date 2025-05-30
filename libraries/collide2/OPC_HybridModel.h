///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *  OPC_HybridModel.h
 *
 *  OPCODE - Optimized Collision Detection
 *  Copyright (C) 2001 Pierre Terdiman
 *  Copyright (C) 2021, 2022, 2025 Stephen G. Tuggy
 *  Copyright (C) 2023 Benjamen R. Meyer
 *  Public Domain
 *  Homepage: http://www.codercorner.com/Opcode.htm
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for hybrid models.
 *  \file		OPC_HybridModel.h
 *  \author		Pierre Terdiman
 *  \date		May, 18, 2003
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef VEGA_STRIKE_ENGINE_CMD_COLLSION2_OPC_HYBRIDMODEL_H
#define VEGA_STRIKE_ENGINE_CMD_COLLSION2_OPC_HYBRIDMODEL_H

//! Leaf descriptor
struct LeafTriangles {
    uint32_t Data;        //!< Packed data

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Gets number of triangles in the leaf.
     *	\return		number of triangles N, with 0 < N <= 16
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    inline_    uint32_t GetNbTriangles() const {
        return (Data & 15) + 1;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Gets triangle index for this leaf. Indexed model's array of indices retrieved with HybridModel::GetIndices()
     *	\return		triangle index
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    inline_    uint32_t GetTriangleIndex() const {
        return Data >> 4;
    }

    inline_    void SetData(uint32_t nb, uint32_t index) {
        OPASSERT(nb > 0 && nb <= 16);
        nb--;
        Data = (index << 4) | (nb & 15);
    }
};

class OPCODE_API HybridModel : public BaseModel {
public:
    // Constructor/Destructor
    HybridModel();
    virtual                                        ~HybridModel();

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Builds a collision model.
     *	\param		create		[in] model creation structure
     *	\return		true if success
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    override(BaseModel) bool Build(const OPCODECREATE &create);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Gets the number of bytes used by the tree.
     *	\return		amount of bytes used
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    override(BaseModel) size_t GetUsedBytes() const;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Refits the collision model. This can be used to handle dynamic meshes. Usage is:
     *	1. modify your mesh vertices (keep the topology constant!)
     *	2. refit the tree (call this method)
     *	\return		true if success
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    override(BaseModel) bool Refit();

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Gets array of triangles.
     *	\return		array of triangles
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    inline_                const LeafTriangles *GetLeafTriangles() const {
        return mTriangles;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Gets array of indices.
     *	\return		array of indices
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    inline_                const uint32_t *GetIndices() const {
        return mIndices;
    }

private:
    uint32_t mNbLeaves;        //!< Number of leaf nodes in the model
    LeafTriangles *mTriangles;        //!< Array of mNbLeaves leaf descriptors
    uint32_t mNbPrimitives;    //!< Number of primitives in the model
    uint32_t *mIndices;        //!< Array of primitive indices

    // Internal methods
    void Release();
};

#endif //VEGA_STRIKE_ENGINE_CMD_COLLSION2_OPC_HYBRIDMODEL_H
