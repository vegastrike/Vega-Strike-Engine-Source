/**
 * quaternion.cpp
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
 * Copyright (C) 2022 Stephen G. Tuggy
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */


#include "quaternion.h"

Quaternion Quaternion::from_vectors(const Vector &v1, const Vector &v2, const Vector &v3)
{
    float T = v1.i + v2.j + v3.k + 1, S, W, X, Y, Z;
    if (T > 0) {
        S = 0.5 / sqrtf(T);
        W = 0.25 / S;
        X = (v3.j - v2.k) * S;
        Y = (v1.k - v3.i) * S;
        Z = (v2.i - v1.j) * S;
    } else {
        int max = (v1.i > v2.j) ? 1 : 2;
        if (max == 1) {
            max = (v1.i > v3.k) ? 1 : 3;
        } else {
            max = (v2.j > v3.k) ? 2 : 3;
        }
        switch (max) {
            case 1:
                //column 0
                S = sqrtf((v2.i - (v2.j + v3.k)) + 1);
                X = S * .5;
                S = .5 / S;
                W = (v3.j - v2.k) * S;
                Y = (v2.i + v1.j) * S;
                Z = (v3.i + v1.k) * S;
                break;
            case 2:
                //column 1
                S = sqrtf((v3.j - (v3.k + v1.i)) + 1);
                Y = 0.5 * S;
                S = .5 / S;
                W = (v1.k - v3.i);
                Z = (v3.j + v2.k);
                X = (v1.j + v2.i);
                break;
            case 3:
                //column 2
                S = sqrtf((v1.k - (v1.i + v2.j)) + 1);
                Z = 0.5 * S;
                S = .5 / S;
                W = (v2.i - v1.j);
                X = (v1.k + v3.i);
                Y = (v2.k + v3.j);
                break;
        }
#if 0
        DEPRECATED
        /*
         *  switch(max) {
         *  case 1:
         *  //column 0
         *  S  = QSQRT( (v1.j - (v2.j + v3.k ))+1);
         *  Y = 0.5 *  S;
         *  S = .5 / S;
         *  Z = (v1.j + v2.i ) * S;
         *  X = (v1.k + v3.i ) * S;
         *  W = (v2.k - v3.j ) * S;
         *
         *  break;
         *  case 2:
         *  //column 1
         *  S  = QSQRT( v2.k - (v3.k + v1.i )+1);
         *  Y = 0.5 *  S;
         *  S = .5 / S;
         *  Z = (v3.j + v2.k ) * S;
         *  X = (v2.i + v1.j ) * S;
         *  W = (v3.i - v1.k ) * S;
         *  break;
         *  case 3:
         *  //column 2
         *  S  = QSQRT( v3.i - (v1.i + v2.j )+1);
         *  Z = 0.5 *  S;
         *  S = .5 / S;
         *  X = (v1.k + v3.i ) * S;
         *  Y = (v3.j + v2.k ) * S;
         *  W = (v1.j - v2.i ) * S;
         *  break;
         *  }
         */
#endif
    }
    return Quaternion(W, Vector(X, Y, Z));
}

Quaternion Quaternion::from_axis_angle(const Vector &axis, float angle)
{
    float sin_a = sin(angle / 2);
    float cos_a = cos(angle / 2);
    return Quaternion(cos_a, Vector(axis.i * sin_a,
                                    axis.j * sin_a,
                                    axis.k * sin_a));
}

