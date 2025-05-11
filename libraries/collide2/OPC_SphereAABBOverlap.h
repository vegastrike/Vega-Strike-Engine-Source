///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * OPC_SphereAABBOverlap.h
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
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// NO HEADER GUARD

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Sphere-AABB overlap test, based on Jim Arvo's code.
 *  \param		center		[in] box center
 *  \param		extents		[in] box extents
 *  \return		TRUE on overlap
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline_ bool SphereCollider::SphereAABBOverlap(const Point &center, const Point &extents) {
    // Stats
    mNbVolumeBVTests++;

    float d = 0.0f;

    //find the square of the distance
    //from the sphere to the box

//	float tmp = mCenter.x - center.x;
//	float s = tmp + extents.x;

    float tmp, s;

    tmp = mCenter.x - center.x;
    s = tmp + extents.x;

    if (s < 0.0f) {
        d += s * s;
        if (d > mRadius2) {
            return FALSE;
        }
    } else {
        s = tmp - extents.x;
        if (s > 0.0f) {
            d += s * s;
            if (d > mRadius2) {
                return FALSE;
            }
        }
    }

    tmp = mCenter.y - center.y;
    s = tmp + extents.y;

    if (s < 0.0f) {
        d += s * s;
        if (d > mRadius2) {
            return FALSE;
        }
    } else {
        s = tmp - extents.y;
        if (s > 0.0f) {
            d += s * s;
            if (d > mRadius2) {
                return FALSE;
            }
        }
    }

    tmp = mCenter.z - center.z;
    s = tmp + extents.z;

    if (s < 0.0f) {
        d += s * s;
        if (d > mRadius2) {
            return FALSE;
        }
    } else {
        s = tmp - extents.z;
        if (s > 0.0f) {
            d += s * s;
            if (d > mRadius2) {
                return FALSE;
            }
        }
    }
    return d <= mRadius2;
}
