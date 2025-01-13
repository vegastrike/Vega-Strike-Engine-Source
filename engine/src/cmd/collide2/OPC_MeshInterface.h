/*
 * OPC_MeshInterface.h
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
 *	Contains a mesh interface.
 *	\file		OPC_MeshInterface.h
 *	\author		Pierre Terdiman
 *	\date		November, 27, 2002
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
#ifndef VEGA_STRIKE_ENGINE_CMD_COLLSION2_OPC_MESHINTERFACE_H
#define VEGA_STRIKE_ENGINE_CMD_COLLSION2_OPC_MESHINTERFACE_H

struct VertexPointers {
    const Point *Vertex[3];

    bool BackfaceCulling(const Point &source) {
        const Point &p0 = *Vertex[0];
        const Point &p1 = *Vertex[1];
        const Point &p2 = *Vertex[2];

        // Compute normal direction
        Point Normal = (p2 - p1) ^ (p0 - p1);

        // Backface culling
        return (Normal | (source - p0)) >= 0.0f;
    }
};

#ifdef OPC_USE_CALLBACKS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	User-callback, called by OPCODE to request vertices from the app.
 *	\param		triangle_index	[in] face index for which the system is requesting the vertices
 *	\param		triangle		[out] triangle's vertices (must be provided by the user)
 *	\param		user_data		[in] user-defined data from SetCallback()
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef void    (*RequestCallback)(uint32_t triangle_index, VertexPointers &triangle, void *user_data);
#endif

class OPCODE_API MeshInterface {
public:
    // Constructor / Destructor
    MeshInterface();
    ~MeshInterface();
    // Common settings
    inline_            uint32_t GetNbTriangles() const {
        return mNbTris;
    }

    inline_            uint32_t GetNbVertices() const {
        return mNbVerts;
    }

    inline_            void SetNbTriangles(uint32_t nb) {
        mNbTris = nb;
    }

    inline_            void SetNbVertices(uint32_t nb) {
        mNbVerts = nb;
    }

#ifdef OPC_USE_CALLBACKS
    // Callback settings

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Callback control: setups object callback. Must provide triangle-vertices for a given triangle index.
     *	\param		callback	[in] user-defined callback
     *	\param		user_data	[in] user-defined data
     *	\return		true if success
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool SetCallback(RequestCallback callback, void *user_data);

    inline_            void *GetUserData() const {
        return mUserData;
    }

    inline_            RequestCallback GetCallback() const {
        return mObjCallback;
    }

#else
    // Pointers settings

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Pointers control: setups object pointers. Must provide access to faces and vertices for a given object.
     *	\param		tris	[in] pointer to triangles
     *	\param		verts	[in] pointer to vertices
     *	\return		true if success
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                    bool				SetPointers(const IndexedTriangle* tris, const Point* verts);
    inline_	const	IndexedTriangle*	GetTris()			const	{ return mTris;			}
    inline_	const	Point*				GetVerts()			const	{ return mVerts;		}

#ifdef OPC_USE_STRIDE
    // Strides settings

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Strides control
     *	\param		tri_stride		[in] size of a triangle in bytes. The first sizeof(IndexedTriangle) bytes are used to get vertex indices.
     *	\param		vertex_stride	[in] size of a vertex in bytes. The first sizeof(Point) bytes are used to get vertex position.
     *	\return		true if success
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                    bool				SetStrides(uint32_t tri_stride=sizeof(IndexedTriangle), uint32_t vertex_stride=sizeof(Point));
    inline_			uint32_t				GetTriStride()		const	{ return mTriStride;	}
    inline_			uint32_t				GetVertexStride()	const	{ return mVertexStride;	}
#endif
#endif

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Fetches a triangle given a triangle index.
     *	\param		vp		[out] required triangle's vertex pointers
     *	\param		index	[in] triangle index
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    inline_            void GetTriangle(VertexPointers &vp, uint32_t index) const {
#ifdef OPC_USE_CALLBACKS
        (mObjCallback)(index, vp, mUserData);
#else
#ifdef OPC_USE_STRIDE
        const IndexedTriangle* T = (const IndexedTriangle*)(((uint8_t*)mTris) + index * mTriStride);
        vp.Vertex[0] = (const Point*)(((uint8_t*)mVerts) + T->mVRef[0] * mVertexStride);
        vp.Vertex[1] = (const Point*)(((uint8_t*)mVerts) + T->mVRef[1] * mVertexStride);
        vp.Vertex[2] = (const Point*)(((uint8_t*)mVerts) + T->mVRef[2] * mVertexStride);
#else
        const IndexedTriangle* T = &mTris[index];
        vp.Vertex[0] = &mVerts[T->mVRef[0]];
        vp.Vertex[1] = &mVerts[T->mVRef[1]];
        vp.Vertex[2] = &mVerts[T->mVRef[2]];
#endif
#endif
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Remaps client's mesh according to a permutation.
     *	\param		nb_indices	[in] number of indices in the permutation (will be checked against number of triangles)
     *	\param		permutation	[in] list of triangle indices
     *	\return		true if success
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool RemapClient(uint32_t nb_indices, const uint32_t *permutation) const;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Checks the mesh interface is valid, i.e. things have been setup correctly.
     *	\return		true if valid
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool IsValid() const;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Checks the mesh itself is valid.
     *	Currently we only look for degenerate faces.
     *	\return		number of degenerate faces
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    uint32_t CheckTopology() const;
private:

#ifdef OPC_USE_CALLBACKS
    // User callback
    void *mUserData;            //!< User-defined data sent to callback
    RequestCallback mObjCallback;        //!< Object callback
#else
    // User pointers
            const	IndexedTriangle*	mTris;				//!< Array of indexed triangles
            const	Point*				mVerts;				//!< Array of vertices
#ifdef OPC_USE_STRIDE
                    uint32_t				mTriStride;			//!< Possible triangle stride in bytes [Opcode 1.3]
                    uint32_t				mVertexStride;		//!< Possible vertex stride in bytes [Opcode 1.3]
#endif
#endif
    uint32_t mNbTris;            //!< Number of triangles in the input model
    uint32_t mNbVerts;            //!< Number of vertices in the input model
};

#endif //VEGA_STRIKE_ENGINE_CMD_COLLSION2_OPC_MESHINTERFACE_H
