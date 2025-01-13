/*
 * IceSegment.h
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
 *	Contains code for segments.
 *	\file		IceSegment.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
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
#ifndef VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_SEGMENT_H
#define VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_SEGMENT_H

class ICEMATHS_API Segment {
public:
    //! Constructor
    inline_ Segment() {
    }
    //! Constructor
    inline_ Segment(const Point &p0, const Point &p1) : mP0(p0), mP1(p1) {
    }
    //! Copy constructor
    inline_ Segment(const Segment &seg) : mP0(seg.mP0), mP1(seg.mP1) {
    }
    //! Destructor
    inline_                    ~Segment() {
    }

    inline_    const Point &GetOrigin() const {
        return mP0;
    }

    inline_            Point ComputeDirection() const {
        return mP1 - mP0;
    }

    inline_            void ComputeDirection(Point &dir) const {
        dir = mP1 - mP0;
    }

    inline_            float ComputeLength() const {
        return mP1.Distance(mP0);
    }

    inline_            float ComputeSquareLength() const {
        return mP1.SquareDistance(mP0);
    }

    inline_            void SetOriginDirection(const Point &origin, const Point &direction) {
        mP0 = mP1 = origin;
        mP1 += direction;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *	Computes a point on the segment
     *	\param		pt	[out] point on segment
     *	\param		t	[in] point's parameter [t=0 => pt = mP0, t=1 => pt = mP1]
     */
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    inline_            void ComputePoint(Point &pt, float t) const {
        pt = mP0 + t * (mP1 - mP0);
    }

    float SquareDistance(const Point &point, float *t = nullptr) const;

    inline_            float Distance(const Point &point, float *t = nullptr) const {
        return sqrtf(SquareDistance(point, t));
    }

    Point mP0;        //!< Start of segment
    Point mP1;        //!< End of segment
};

#endif //VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_SEGMENT_H
