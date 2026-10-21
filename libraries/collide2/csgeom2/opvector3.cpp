/*
 * opvector3.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 1998,1999,2000 by Jorrit Tyberghein
 * Largely rewritten by Ivan Avramovic <ivan@avramovic.com>
 * Copyright (C) 2022-2025 Benjamen R. Meyer and Stephen G. Tuggy
 * Copyright (C) 2026 The Vega Strike Contributors
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see
 * <https://www.gnu.org/licenses/>.
 */


#include "collide2/csgeom2/opvector3.h"

#include "collide2/opcodeqsqrt.h"
#include "collide2/opcodesysdef.h"

//---------------------------------------------------------------------------

float csVector3::Norm() const {
    return qsqrt(x * x + y * y + z * z);
}

void csVector3::Normalize() {
    float sqlen = x * x + y * y + z * z;
    if (sqlen < SMALL_EPSILON) {
        return;
    }
    float invlen  = qisqrt(sqlen);
    *this        *= invlen;
}

//---------------------------------------------------------------------------
