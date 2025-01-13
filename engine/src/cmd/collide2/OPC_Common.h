/*
 * OPC_Common.h
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
 *	Contains common classes & defs used in OPCODE.
 *	\file		OPC_Common.h
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
#ifndef VEGA_STRIKE_ENGINE_CMD_COLLSION2_OPC_COMMON_H
#define VEGA_STRIKE_ENGINE_CMD_COLLSION2_OPC_COMMON_H

// [GOTTFRIED]: Just a small change for readability.
#ifdef OPC_CPU_COMPARE
#define GREATER(x, y)	AIR(x) > IR(y)
#else
#define GREATER(x, y)    fabsf(x) > (y)
#endif

class OPCODE_API CollisionAABB {
public:
    //! Constructor
    inline_ CollisionAABB() {
    }
    //! Constructor
    inline_ CollisionAABB(const AABB &b) {
        b.GetCenter(mCenter);
        b.GetExtents(mExtents);
    }
    //! Destructor
    inline_                ~CollisionAABB() {
    }

    //! Get min point of the box
    inline_    void GetMin(Point &min) const {
        min = mCenter - mExtents;
    }
    //! Get max point of the box
    inline_    void GetMax(Point &max) const {
        max = mCenter + mExtents;
    }

    //! Get component of the box's min point along a given axis
    inline_    float GetMin(uint32_t axis) const {
        return mCenter[axis] - mExtents[axis];
    }
    //! Get component of the box's max point along a given axis
    inline_    float GetMax(uint32_t axis) const {
        return mCenter[axis] + mExtents[axis];
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Setups an AABB from min & max vectors.
     *	\param		min			[in] the min point
     *	\param		max			[in] the max point
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    inline_    void SetMinMax(const Point &min, const Point &max) {
        mCenter = (max + min) * 0.5f;
        mExtents = (max - min) * 0.5f;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Checks a box is inside another box.
     *	\param		box		[in] the other box
     *	\return		true if current box is inside input box
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    inline_    BOOL IsInside(const CollisionAABB &box) const {
        if (box.GetMin(0) > GetMin(0)) {
            return FALSE;
        }
        if (box.GetMin(1) > GetMin(1)) {
            return FALSE;
        }
        if (box.GetMin(2) > GetMin(2)) {
            return FALSE;
        }
        if (box.GetMax(0) < GetMax(0)) {
            return FALSE;
        }
        if (box.GetMax(1) < GetMax(1)) {
            return FALSE;
        }
        if (box.GetMax(2) < GetMax(2)) {
            return FALSE;
        }
        return TRUE;
    }

    Point mCenter;                //!< Box center
    Point mExtents;                //!< Box extents
};

class OPCODE_API QuantizedAABB {
public:
    //! Constructor
    inline_ QuantizedAABB() {
    }
    //! Destructor
    inline_                ~QuantizedAABB() {
    }

    int16_t mCenter[3];                //!< Quantized center
    uint16_t mExtents[3];            //!< Quantized extents
};

//! Quickly rotates & translates a vector
inline_ void TransformPoint(Point &dest, const Point &source, const Matrix3x3 &rot, const Point &trans) {
    dest.x = trans.x + source.x * rot.m[0][0] + source.y * rot.m[1][0] + source.z * rot.m[2][0];
    dest.y = trans.y + source.x * rot.m[0][1] + source.y * rot.m[1][1] + source.z * rot.m[2][1];
    dest.z = trans.z + source.x * rot.m[0][2] + source.y * rot.m[1][2] + source.z * rot.m[2][2];
}

#endif //VEGA_STRIKE_ENGINE_CMD_COLLSION2_OPC_COMMON_H
