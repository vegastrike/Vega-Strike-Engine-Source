/*
 * IceSegment.cpp
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
 *	\file		IceSegment.cpp
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 *
 *  Updated by Stephen G. Tuggy 2022-01-06
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Segment class.
 *	A segment is defined by S(t) = mP0 * (1 - t) + mP1 * t, with 0 <= t <= 1
 *	Alternatively, a segment is S(t) = Origin + t * Direction for 0 <= t <= 1.
 *	Direction is not necessarily unit length. The end points are Origin = mP0 and Origin + Direction = mP1.
 *
 *	\class		Segment
 *	\author		Pierre Terdiman
 *	\version	1.0
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Precompiled Header
#include "Stdafx.h"

using namespace Opcode;

float Segment::SquareDistance(const Point &point, float *t) const {
    Point Diff = point - mP0;
    Point Dir = mP1 - mP0;
    float fT = Diff | Dir;

    if (fT <= 0.0f) {
        fT = 0.0f;
    } else {
        float SqrLen = Dir.SquareMagnitude();
        if (fT >= SqrLen) {
            fT = 1.0f;
            Diff -= Dir;
        } else {
            fT /= SqrLen;
            Diff -= fT * Dir;
        }
    }

    if (t) {
        *t = fT;
    }

    return Diff.SquareMagnitude();
}

