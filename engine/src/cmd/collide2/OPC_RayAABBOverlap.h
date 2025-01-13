/*
 * OPC_RayAABBOverlap.h
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
// Opcode 1.1: ray-AABB overlap tests based on Woo's code
// Opcode 1.2: ray-AABB overlap tests based on the separating axis theorem
//
// The point of intersection is not computed anymore. The distance to impact is not needed anymore
// since we now have two different queries for segments or rays.
// NO HEADER GUARD

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Computes a segment-AABB overlap test using the separating axis theorem. Segment is cached within the class.
 *	\param		center	[in] AABB center
 *	\param		extents	[in] AABB extents
 *	\return		true on overlap
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline_ bool RayCollider::SegmentAABBOverlap(const Point &center, const Point &extents) {
    // Stats
    mNbRayBVTests++;

    float Dx = mData2.x - center.x;
    if (fabsf(Dx) > extents.x + mFDir.x) {
        return FALSE;
    }
    float Dy = mData2.y - center.y;
    if (fabsf(Dy) > extents.y + mFDir.y) {
        return FALSE;
    }
    float Dz = mData2.z - center.z;
    if (fabsf(Dz) > extents.z + mFDir.z) {
        return FALSE;
    }

    float f;
    f = mData.y * Dz - mData.z * Dy;
    if (fabsf(f) > extents.y * mFDir.z + extents.z * mFDir.y) {
        return FALSE;
    }
    f = mData.z * Dx - mData.x * Dz;
    if (fabsf(f) > extents.x * mFDir.z + extents.z * mFDir.x) {
        return FALSE;
    }
    f = mData.x * Dy - mData.y * Dx;
    if (fabsf(f) > extents.x * mFDir.y + extents.y * mFDir.x) {
        return FALSE;
    }

    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Computes a ray-AABB overlap test using the separating axis theorem. Ray is cached within the class.
 *	\param		center	[in] AABB center
 *	\param		extents	[in] AABB extents
 *	\return		true on overlap
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline_ bool RayCollider::RayAABBOverlap(const Point &center, const Point &extents) {
    // Stats
    mNbRayBVTests++;

//	float Dx = mOrigin.x - center.x;	if(fabsf(Dx) > extents.x && Dx*mDir.x>=0.0f)	return FALSE;
//	float Dy = mOrigin.y - center.y;	if(fabsf(Dy) > extents.y && Dy*mDir.y>=0.0f)	return FALSE;
//	float Dz = mOrigin.z - center.z;	if(fabsf(Dz) > extents.z && Dz*mDir.z>=0.0f)	return FALSE;

    float Dx = mOrigin.x - center.x;
    if (GREATER(Dx, extents.x) && Dx * mDir.x >= 0.0f) {
        return FALSE;
    }
    float Dy = mOrigin.y - center.y;
    if (GREATER(Dy, extents.y) && Dy * mDir.y >= 0.0f) {
        return FALSE;
    }
    float Dz = mOrigin.z - center.z;
    if (GREATER(Dz, extents.z) && Dz * mDir.z >= 0.0f) {
        return FALSE;
    }

//	float Dx = mOrigin.x - center.x;	if(GREATER(Dx, extents.x) && ((SIR(Dx)-1)^SIR(mDir.x))>=0.0f)	return FALSE;
//	float Dy = mOrigin.y - center.y;	if(GREATER(Dy, extents.y) && ((SIR(Dy)-1)^SIR(mDir.y))>=0.0f)	return FALSE;
//	float Dz = mOrigin.z - center.z;	if(GREATER(Dz, extents.z) && ((SIR(Dz)-1)^SIR(mDir.z))>=0.0f)	return FALSE;

    float f;
    f = mDir.y * Dz - mDir.z * Dy;
    if (fabsf(f) > extents.y * mFDir.z + extents.z * mFDir.y) {
        return FALSE;
    }
    f = mDir.z * Dx - mDir.x * Dz;
    if (fabsf(f) > extents.x * mFDir.z + extents.z * mFDir.x) {
        return FALSE;
    }
    f = mDir.x * Dy - mDir.y * Dx;
    if (fabsf(f) > extents.x * mFDir.y + extents.y * mFDir.x) {
        return FALSE;
    }

    return TRUE;
}
