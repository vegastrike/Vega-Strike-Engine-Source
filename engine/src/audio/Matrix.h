/**
 * Matrix.h
 *
 * Copyright (C) Daniel Horn
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike
 * contributors
 * Copyright (C) 2022 Stephen G. Tuggy
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


//
// C++ Interface: Audio::Codec
//
#ifndef __AUDIO_MATRIX_H__INCLUDED__
#define __AUDIO_MATRIX_H__INCLUDED__

#include <cmath>
#include "Vector.h"

namespace Audio {

template<typename T>
class TMatrix3 {
public:
    T m[3][3];

    TMatrix3() {
    }

    TMatrix3(T m11, T m12, T m13, T m21, T m22, T m23, T m31, T m32, T m33) {
        m[0][0] = m11;
        m[0][1] = m12;
        m[0][2] = m13;
        m[1][0] = m21;
        m[1][1] = m22;
        m[1][2] = m23;
        m[2][0] = m31;
        m[2][1] = m32;
        m[2][2] = m33;
    }

    explicit TMatrix3(T s) {
        m[0][0] = s;
        m[0][1] = 0;
        m[0][2] = 0;
        m[1][0] = 0;
        m[1][1] = s;
        m[1][2] = 0;
        m[2][0] = 0;
        m[2][1] = 0;
        m[2][2] = s;
    }

    template<typename Y>
    TMatrix3(const TMatrix3<Y> &o) {
        m[0][0] = o.m[0][0];
        m[0][1] = o.m[0][1];
        m[0][2] = o.m[0][2];
        m[1][0] = o.m[1][0];
        m[1][1] = o.m[1][1];
        m[1][2] = o.m[1][2];
        m[2][0] = o.m[2][0];
        m[2][1] = o.m[2][1];
        m[2][2] = o.m[2][2];
    }

    template<typename Y>
    TMatrix3(const TVector3<Y> &c1, const TVector3<Y> &c2, const TVector3<Y> &c3) {
        m[0][0] = c1.x;
        m[0][1] = c2.x;
        m[0][2] = c3.x;
        m[1][0] = c1.y;
        m[1][1] = c2.y;
        m[1][2] = c3.y;
        m[2][0] = c1.z;
        m[2][1] = c2.z;
        m[2][2] = c3.z;
    }

    template<typename Y>
    TMatrix3<T> &operator=(const TMatrix3<Y> &o) {
        m[0][0] = o.m[0][0];
        m[0][1] = o.m[0][1];
        m[0][2] = o.m[0][2];
        m[1][0] = o.m[1][0];
        m[1][1] = o.m[1][1];
        m[1][2] = o.m[1][2];
        m[2][0] = o.m[2][0];
        m[2][1] = o.m[2][1];
        m[2][2] = o.m[2][2];
        return *this;
    }

    TMatrix3<T> &operator+=(const TMatrix3<T> &o) {
        m[0][0] += o.m[0][0];
        m[0][1] += o.m[0][1];
        m[0][2] += o.m[0][2];
        m[1][0] += o.m[1][0];
        m[1][1] += o.m[1][1];
        m[1][2] += o.m[1][2];
        m[2][0] += o.m[2][0];
        m[2][1] += o.m[2][1];
        m[2][2] += o.m[2][2];
        return *this;
    }

    TMatrix3<T> &operator-=(const TMatrix3<T> &o) {
        m[0][0] -= o.m[0][0];
        m[0][1] -= o.m[0][1];
        m[0][2] -= o.m[0][2];
        m[1][0] -= o.m[1][0];
        m[1][1] -= o.m[1][1];
        m[1][2] -= o.m[1][2];
        m[2][0] -= o.m[2][0];
        m[2][1] -= o.m[2][1];
        m[2][2] -= o.m[2][2];
        return *this;
    }

    TMatrix3<T> &operator*=(const TMatrix3<T> &o) {
        #define DOT(m, om, r, c) (m[r][0]*om[0][c] + m[r][1]*om[1][c] + m[r][2]*om[2][c])
        TMatrix3<T> rv;
        rv.m[0][0] = DOT(m, o.m, 0, 0);
        rv.m[0][1] = DOT(m, o.m, 0, 1);
        rv.m[0][2] = DOT(m, o.m, 0, 2);
        rv.m[1][0] = DOT(m, o.m, 1, 0);
        rv.m[1][1] = DOT(m, o.m, 1, 1);
        rv.m[1][2] = DOT(m, o.m, 1, 2);
        rv.m[2][0] = DOT(m, o.m, 2, 0);
        rv.m[2][1] = DOT(m, o.m, 2, 1);
        rv.m[2][2] = DOT(m, o.m, 2, 2);
        #undef DOT
        return *this;
    }

    TVector3<T> &operator*=(T t) {
        m[0][0] *= t;
        m[0][1] *= t;
        m[0][2] *= t;
        m[1][0] *= t;
        m[1][1] *= t;
        m[1][2] *= t;
        m[2][0] *= t;
        m[2][1] *= t;
        m[2][2] *= t;
        return *this;
    }

    TMatrix3<T> &operator/=(const TMatrix3<T> &other) {
        (*this) *= other.inverse();
        return *this;
    }

    TMatrix3<T> &operator/=(T t) {
        (*this) *= T(1) / t;
        return *this;
    }

    TMatrix3<T> operator+(const TMatrix3<T> &o) const {
        return TMatrix3<T>(
                m[0][0] + o.m[0][0], m[0][1] + o.m[0][1], m[0][2] + o.m[0][2],
                m[1][0] + o.m[1][0], m[1][1] + o.m[1][1], m[1][2] + o.m[1][2],
                m[2][0] + o.m[2][0], m[2][1] + o.m[2][1], m[2][2] + o.m[2][2]
        );
    }

    TMatrix3<T> operator-(const TMatrix3<T> &o) const {
        return TMatrix3<T>(
                m[0][0] - o.m[0][0], m[0][1] - o.m[0][1], m[0][2] - o.m[0][2],
                m[1][0] - o.m[1][0], m[1][1] - o.m[1][1], m[1][2] - o.m[1][2],
                m[2][0] - o.m[2][0], m[2][1] - o.m[2][1], m[2][2] - o.m[2][2]
        );
    }

    TMatrix3<T> operator*(const TMatrix3<T> &o) const {
        #define DOT(m, om, r, c) (m[r][0]*om[0][c] + m[r][1]*om[1][c] + m[r][2]*om[2][c])
        return TMatrix3<T>(
                DOT(m, o.m, 0, 0), DOT(m, o.m, 0, 1), DOT(m, o.m, 0, 2),
                DOT(m, o.m, 1, 0), DOT(m, o.m, 1, 1), DOT(m, o.m, 1, 2),
                DOT(m, o.m, 2, 0), DOT(m, o.m, 2, 1), DOT(m, o.m, 2, 2)
        );
        #undef DOT
    }

    TMatrix3<T> operator/(const TMatrix3<T> &other) const {
        return (*this) * other.inverse();
    }

    TMatrix3<T> operator*(T t) const {
        return TMatrix3<T>(
                m[0][0] * t, m[0][1] * t, m[0][2] * t,
                m[1][0] * t, m[1][1] * t, m[1][2] * t,
                m[2][0] * t, m[2][1] * t, m[2][2] * t
        );
    }

    TMatrix3<T> operator/(T t) const {
        return (*this) * (T(1) / t);
    }

    template<typename Y>
    TVector3<Y> operator*(const TVector3<Y> &t) const {
        return TVector3<Y>(
                m[0][0] * t.x + m[0][1] * t.y + m[0][2] * t.z,
                m[1][0] * t.x + m[1][1] * t.y + m[1][2] * t.z,
                m[2][0] * t.x + m[2][1] * t.y + m[2][2] * t.z
        );
    }

    TMatrix3<T> inverse() const {
        TMatrix3<T> rv;

        /*
        compute adjoint matrix,
        then divide by the determinant
        */

        //determinant
        float det =
                +m[0][0] * m[1][1] * m[2][2]
                        + m[1][0] * m[2][1] * m[0][2]
                        + m[2][0] * m[0][1] * m[1][2]

                        - m[0][2] * m[1][1] * m[2][0]
                        - m[1][2] * m[2][1] * m[0][0]
                        - m[2][2] * m[0][1] * m[1][0];
        float idet = T(1) / det;

        rv.m[0][0] = +(m[1][1] * m[2][2] - m[1][2] * m[2][1]) * idet;
        rv.m[0][1] = -(m[1][0] * m[2][2] - m[1][2] * m[2][0]) * idet;
        rv.m[0][2] = +(m[1][0] * m[2][1] - m[1][1] * m[2][0]) * idet;

        rv.m[1][0] = -(m[0][1] * m[2][2] - m[0][2] * m[2][1]) * idet;
        rv.m[1][1] = +(m[0][0] * m[2][2] - m[0][2] * m[2][0]) * idet;
        rv.m[1][2] = -(m[0][0] * m[2][1] - m[0][1] * m[2][0]) * idet;

        rv.m[2][0] = +(m[0][1] * m[1][2] - m[0][2] * m[1][1]) * idet;
        rv.m[2][1] = -(m[0][0] * m[1][2] - m[0][2] * m[1][0]) * idet;
        rv.m[2][2] = +(m[0][0] * m[1][1] - m[0][1] * m[1][0]) * idet;

        return rv;
    }

    TMatrix3<T> transpose() const {
        return TMatrix3<T>(
                m[0][0], m[1][0], m[2][0],
                m[0][1], m[1][1], m[2][1],
                m[0][2], m[1][2], m[2][2]
        );
    }
};

};

#endif//__AUDIO_MATRIX_H__INCLUDED__
