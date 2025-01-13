/*
 * IceRay.cpp
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
 *	Contains code for rays.
 *	\file		IceRay.cpp
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 *
 *  Updated by Stephen G. Tuggy 2022-01-06
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Ray class.
 *	A ray is a half-line P(t) = mOrig + mDir * t, with 0 <= t <= +infinity
 *	\class		Ray
 *	\author		Pierre Terdiman
 *	\version	1.0
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
	O = Origin = impact point
	i = normalized vector along the x axis
	j = normalized vector along the y axis = actually the normal vector in O
	D = Direction vector, norm |D| = 1
	N = Projection of D on y axis, norm |N| = normal reaction
	T = Projection of D on x axis, norm |T| = tangential reaction
	R = Reflexion vector

              ^y
              |
              |
              |
       _  _  _| _ _ _
       *      *      *|
        \     |     /
         \    |N   /  |
         R\   |   /D
           \  |  /    |
            \ | /
    _________\|/______*_______>x
               O    T

	Let define theta = angle between D and N. Then cos(theta) = |N| / |D| = |N| since D is normalized.

	j|D = |j|*|D|*cos(theta) => |N| = j|D

	Then we simply have:

	D = N + T

	To compute tangential reaction :

	T = D - N

	To compute reflexion vector :

	R = N - T = N - (D-N) = 2*N - D
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Precompiled Header
#include "Stdafx.h"

using namespace Opcode;

float Ray::SquareDistance(const Point &point, float *t) const {
    Point Diff = point - mOrig;
    float fT = Diff | mDir;

    if (fT <= 0.0f) {
        fT = 0.0f;
    } else {
        fT /= mDir.SquareMagnitude();
        Diff -= fT * mDir;
    }

    if (t) {
        *t = fT;
    }

    return Diff.SquareMagnitude();
}

