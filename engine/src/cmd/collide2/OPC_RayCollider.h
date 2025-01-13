/*
 * OPC_RayCollider.h
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
 *	Contains code for a ray collider.
 *	\file		OPC_RayCollider.h
 *	\author		Pierre Terdiman
 *	\date		June, 2, 2001
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
#ifndef VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_OPC_RAY_COLLIDER_H
#define VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_OPC_RAY_COLLIDER_H

class OPCODE_API CollisionFace {
public:
    //! Constructor
    inline_ CollisionFace() {
    }
    //! Destructor
    inline_                ~CollisionFace() {
    }

    uint32_t mFaceID;                //!< Index of touched face
    float mDistance;                //!< Distance from collider to hitpoint
    float mU, mV;                    //!< Impact barycentric coordinates
};

class OPCODE_API CollisionFaces : private Container {
public:
    //! Constructor
    CollisionFaces() {
    }

    //! Destructor
    ~CollisionFaces() {
    }

    inline_    uint32_t GetNbFaces() const {
        return GetNbEntries() >> 2;
    }

    inline_    const CollisionFace *GetFaces() const {
        return (const CollisionFace *) GetEntries();
    }

    inline_    void Reset() {
        Container::Reset();
    }

    inline_    void AddFace(const CollisionFace &face) {
        Add(face.mFaceID).Add(face.mDistance).Add(face.mU).Add(face.mV);
    }
};

#ifdef OPC_RAYHIT_CALLBACK
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	User-callback, called by OPCODE to record a hit.
 *	\param		hit			[in] current hit
 *	\param		user_data	[in] user-defined data from SetCallback()
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef void    (*HitCallback)(const CollisionFace &hit, void *user_data);
#endif

class OPCODE_API RayCollider : public Collider {
public:
    // Constructor / Destructor
    RayCollider();
    virtual                                ~RayCollider();

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Generic stabbing query for generic OPCODE models. After the call, access the results:
     *	- with GetContactStatus()
     *	- in the user-provided destination array
     *
     *	\param		world_ray		[in] stabbing ray in world space
     *	\param		model			[in] Opcode model to collide with
     *	\param		world			[in] model's world matrix, or nullptr
     *	\param		cache			[in] a possibly cached face index, or nullptr
     *	\return		true if success
     *	\warning	SCALE NOT SUPPORTED. The matrices must contain rotation & translation parts only.
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool Collide(const Ray &world_ray, const Model &model, const Matrix4x4 *world = nullptr, uint32_t *cache = nullptr);
    //
    bool Collide(const Ray &world_ray, const AABBTree *tree, Container &box_indices);
    // Settings

#ifndef OPC_RAYHIT_CALLBACK
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Settings: enable or disable "closest hit" mode.
     *	\param		flag		[in] true to report closest hit only
     *	\see		SetCulling(bool flag)
     *	\see		SetMaxDist(float max_dist)
     *	\see		SetDestination(StabbedFaces* sf)
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    inline_				void			SetClosestHit(bool flag)				{ mClosestHit	= flag;		}
#endif
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Settings: enable or disable backface culling.
     *	\param		flag		[in] true to enable backface culling
     *	\see		SetClosestHit(bool flag)
     *	\see		SetMaxDist(float max_dist)
     *	\see		SetDestination(StabbedFaces* sf)
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    inline_                void SetCulling(bool flag) {
        mCulling = flag;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Settings: sets the higher distance bound.
     *	\param		max_dist	[in] higher distance bound. Default = maximal value, for ray queries (else segment)
     *	\see		SetClosestHit(bool flag)
     *	\see		SetCulling(bool flag)
     *	\see		SetDestination(StabbedFaces* sf)
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    inline_                void SetMaxDist(float max_dist = MAX_FLOAT) {
        mMaxDist = max_dist;
    }

#ifdef OPC_RAYHIT_CALLBACK

    inline_                void SetHitCallback(HitCallback cb) {
        mHitCallback = cb;
    }

    inline_                void SetUserData(void *user_data) {
        mUserData = user_data;
    }

#else
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Settings: sets the destination array for stabbed faces.
     *	\param		cf			[in] destination array, filled during queries
     *	\see		SetClosestHit(bool flag)
     *	\see		SetCulling(bool flag)
     *	\see		SetMaxDist(float max_dist)
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    inline_				void			SetDestination(CollisionFaces* cf)		{ mStabbedFaces	= cf;		}
#endif
    // Stats
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Stats: gets the number of Ray-BV overlap tests after a collision query.
     *	\see		GetNbRayPrimTests()
     *	\see		GetNbIntersections()
     *	\return		the number of Ray-BV tests performed during last query
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    inline_                uint32_t GetNbRayBVTests() const {
        return mNbRayBVTests;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Stats: gets the number of Ray-Triangle overlap tests after a collision query.
     *	\see		GetNbRayBVTests()
     *	\see		GetNbIntersections()
     *	\return		the number of Ray-Triangle tests performed during last query
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    inline_                uint32_t GetNbRayPrimTests() const {
        return mNbRayPrimTests;
    }

    // In-out test
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Stats: gets the number of intersection found after a collision query. Can be used for in/out tests.
     *	\see		GetNbRayBVTests()
     *	\see		GetNbRayPrimTests()
     *	\return		the number of valid intersections during last query
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    inline_                uint32_t GetNbIntersections() const {
        return mNbIntersections;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Validates current settings. You should call this method after all the settings and callbacks have been defined for a collider.
     *	\return		nullptr if everything is ok, else a string describing the problem
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    override(Collider) const char *ValidateSettings();

protected:
    // Ray in local space
    Point mOrigin;            //!< Ray origin
    Point mDir;                //!< Ray direction (normalized)
    Point mFDir;                //!< fabsf(mDir)
    Point mData, mData2;
    // Stabbed faces
    CollisionFace mStabbedFace;        //!< Current stabbed face
#ifdef OPC_RAYHIT_CALLBACK
    HitCallback mHitCallback;        //!< Callback used to record a hit
    void *mUserData;            //!< User-defined data
#else
    CollisionFaces*	mStabbedFaces;		//!< List of stabbed faces
#endif
    // Stats
    uint32_t mNbRayBVTests;        //!< Number of Ray-BV tests
    uint32_t mNbRayPrimTests;    //!< Number of Ray-Primitive tests
    // In-out test
    uint32_t mNbIntersections;    //!< Number of valid intersections
    // Dequantization coeffs
    Point mCenterCoeff;
    Point mExtentsCoeff;
    // Settings
    float mMaxDist;            //!< Valid segment on the ray
#ifndef OPC_RAYHIT_CALLBACK
    bool			mClosestHit;		//!< Report closest hit only
#endif
    bool mCulling;            //!< Stab culled faces or not
    // Internal methods
    void _SegmentStab(const AABBCollisionNode *node);
    void _SegmentStab(const AABBNoLeafNode *node);
    void _SegmentStab(const AABBQuantizedNode *node);
    void _SegmentStab(const AABBQuantizedNoLeafNode *node);
    void _SegmentStab(const AABBTreeNode *node, Container &box_indices);
    void _RayStab(const AABBCollisionNode *node);
    void _RayStab(const AABBNoLeafNode *node);
    void _RayStab(const AABBQuantizedNode *node);
    void _RayStab(const AABBQuantizedNoLeafNode *node);
    void _RayStab(const AABBTreeNode *node, Container &box_indices);
    // Overlap tests
    inline_                bool RayAABBOverlap(const Point &center, const Point &extents);
    inline_                bool SegmentAABBOverlap(const Point &center, const Point &extents);
    inline_                bool RayTriOverlap(const Point &vert0, const Point &vert1, const Point &vert2);
    // Init methods
    bool InitQuery(const Ray &world_ray, const Matrix4x4 *world = nullptr, uint32_t *face_id = nullptr);
};

#endif //VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_OPC_RAY_COLLIDER_H
