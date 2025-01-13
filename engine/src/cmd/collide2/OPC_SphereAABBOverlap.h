/*
 * OPC_SphereAABBOverlap.h
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
/**
 * Updated by Stephen G. Tuggy 2021-07-03
 * Updated by Stephen G. Tuggy 2022-01-06
 * Updated by Benjamen R. Meyer 2023-05-27
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NO HEADER GUARD

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Sphere-AABB overlap test, based on Jim Arvo's code.
 *	\param		center		[in] box center
 *	\param		extents		[in] box extents
 *	\return		TRUE on overlap
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
