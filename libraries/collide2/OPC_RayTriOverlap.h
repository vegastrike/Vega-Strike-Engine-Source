/**
 * OPC_RayTriOverlap.h
 *
 * Copyright (C) 2001 Pierre Terdiman
 * Copyright (C) 2021, 2022, 2025 Stephen G. Tuggy
 * Copyright (C) 2023 Benjamen R. Meyer
 *
 * This file is part of OPCODE - Optimized Collision Detection
 * (http://www.codercorner.com/Opcode.htm) and has been
 * incorporated into Vega Strike
 * (https://github.com/vegastrike/Vega-Strike-Engine-Source).
 *
 * Public Domain
 */

// NO HEADER GUARD
#define LOCAL_EPSILON 0.000001f

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Computes a ray-triangle intersection test.
 *	Original code from Tomas Möller's "Fast Minimum Storage Ray-Triangle Intersection".
 *	It's been optimized a bit with integer code, and modified to return a non-intersection if distance from
 *	ray origin to triangle is negative.
 *
 *  \param		vert0	[in] triangle vertex
 *  \param		vert1	[in] triangle vertex
 *  \param		vert2	[in] triangle vertex
 *  \return		true on overlap. mStabbedFace is filled with relevant info.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline_ bool RayCollider::RayTriOverlap(const Point &vert0, const Point &vert1, const Point &vert2) {
    // Stats
    mNbRayPrimTests++;

    // Find vectors for two edges sharing vert0
    Point edge1 = vert1 - vert0;
    Point edge2 = vert2 - vert0;

    // Begin calculating determinant - also used to calculate U parameter
    Point pvec = mDir ^ edge2;

    // If determinant is near zero, ray lies in plane of triangle
    float det = edge1 | pvec;

    if (mCulling) {
        if (det < LOCAL_EPSILON) {
            return FALSE;
        }
        // From here, det is > 0. So we can use integer cmp.

        // Calculate distance from vert0 to ray origin
        Point tvec = mOrigin - vert0;

        // Calculate U parameter and test bounds
        mStabbedFace.mU = tvec | pvec;
//		if(IR(u)&0x80000000 || u>det)					return FALSE;
        if ((mStabbedFace.mU < 0.0f) || IR(mStabbedFace.mU) > IR(det)) {
            return FALSE;
        }

        // Prepare to test V parameter
        Point qvec = tvec ^ edge1;

        // Calculate V parameter and test bounds
        mStabbedFace.mV = mDir | qvec;
        if ((mStabbedFace.mV < 0.0f) || mStabbedFace.mU + mStabbedFace.mV > det) {
            return FALSE;
        }

        // Calculate t, scale parameters, ray intersects triangle
        mStabbedFace.mDistance = edge2 | qvec;
        // Det > 0 so we can early exit here
        // Intersection point is valid if distance is positive (else it can just be a face behind the orig point)
        if ((mStabbedFace.mDistance < 0.0f)) {
            return FALSE;
        }
        // Else go on
        float OneOverDet = 1.0f / det;
        mStabbedFace.mDistance *= OneOverDet;
        mStabbedFace.mU *= OneOverDet;
        mStabbedFace.mV *= OneOverDet;
    } else {
        // the non-culling branch
        if (det > -LOCAL_EPSILON && det < LOCAL_EPSILON) {
            return FALSE;
        }
        float OneOverDet = 1.0f / det;

        // Calculate distance from vert0 to ray origin
        Point tvec = mOrigin - vert0;

        // Calculate U parameter and test bounds
        mStabbedFace.mU = (tvec | pvec) * OneOverDet;
//		if(IR(u)&0x80000000 || u>1.0f)					return FALSE;
        if ((mStabbedFace.mU < 0.0f) || (mStabbedFace.mU > 1.0f)) {
            return FALSE;
        }

        // prepare to test V parameter
        Point qvec = tvec ^ edge1;

        // Calculate V parameter and test bounds
        mStabbedFace.mV = (mDir | qvec) * OneOverDet;
        if ((mStabbedFace.mV < 0.0f) || mStabbedFace.mU + mStabbedFace.mV > 1.0f) {
            return FALSE;
        }

        // Calculate t, ray intersects triangle
        mStabbedFace.mDistance = (edge2 | qvec) * OneOverDet;
        // Intersection point is valid if distance is positive (else it can just be a face behind the orig point)
        if ((mStabbedFace.mDistance < 0.0f)) {
            return FALSE;
        }
    }
    return TRUE;
}
