/*
 * quaternion.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
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
#ifndef VEGA_STRIKE_ENGINE_GFX_QUATERNION_H
#define VEGA_STRIKE_ENGINE_GFX_QUATERNION_H

#include "vec.h"
#include "matrix.h"

struct Quaternion {
    float s;
    Vector v;

    inline Quaternion() : s(0), v(0, 0, 0) {
    }

    inline Quaternion(float s, Vector v) {
        this->s = s;
        this->v = v;
    }

    //inline Quaternion(float s, Vector v) {this->s = s; this->v = v;};
    inline Quaternion Conjugate() const {
        return Quaternion(s, Vector(-v.i, -v.j, -v.k));
    }

    inline void netswap() {
        s = VSSwapHostFloatToLittle(s);
        v.netswap();
    }

    inline float Magnitude() const {
        return sqrtf(s * s + v.i * v.i + v.j * v.j + v.k * v.k);
    }

    inline Quaternion operator*(const Quaternion &rval) const {
        return Quaternion(s * rval.s - DotProduct(v, rval.v),
                s * rval.v + rval.s * v + v.Cross(rval.v));
    }

    inline Quaternion &operator*=(const Quaternion &rval) {
        return *this = *this * rval;
    }

    Quaternion &Normalize() {
        float rcpmag = 1.0f / Magnitude();
        v *= rcpmag;
        s *= rcpmag;
        return *this;
    }

    static Quaternion from_vectors(const Vector &v1, const Vector &v2, const Vector &v3);
    static Quaternion from_axis_angle(const Vector &axis, float angle);

    void to_matrix(Matrix &mat) const {
        const float GFXEPSILON = ((float) 10e-6);
        float W = v.i * v.i + v.j * v.j + v.k * v.k + s * s;         //norm
        W = (W < 0 + GFXEPSILON && W > 0 - GFXEPSILON) ? 0 : 2.0 / W;

        float xw = v.i * W;
        float yw = v.j * W;
        float zw = v.k * W;

        float sx = s * xw;
        float sy = s * yw;
        float sz = s * zw;

        float xx = v.i * xw;
        float xy = v.i * yw;
        float xz = v.i * zw;

        float yy = v.j * yw;
        float yz = v.j * zw;
        float zz = v.k * zw;

#define M(B, A) mat.r[B*3+A]

        M(0, 0) = 1.0f - (yy + zz);
        M(0, 1) = (xy - sz);
        M(0, 2) = (xz + sy);

        M(1, 0) = (xy + sz);
        M(1, 1) = 1.0f - (xx + zz);
        M(1, 2) = (yz - sx);

        M(2, 0) = (xz - sy);
        M(2, 1) = (yz + sx);
        M(2, 2) = 1.0f - (xx + yy);
        //M(3,0)  = M(3,1) = M(3,2) = M(0,3) = M(1,3) = M(2,3) = 0;
        //M(3,3) = 1;

#undef M

    }
};

inline Quaternion operator-(const Quaternion &a, const Quaternion &b) {
    return Quaternion(a.s - b.s, a.v - b.v);
}

inline Quaternion operator+(const Quaternion &a, const Quaternion &b) {
    return Quaternion(a.s + b.s, a.v + b.v);
}

inline Quaternion operator*(const Quaternion &a, const float &b) {
    return Quaternion(a.s * b, a.v * b);
}

const Quaternion identity_quaternion(1, Vector(0, 0, 0));

struct Transformation {
    Quaternion orientation;
    QVector position;

    inline Transformation() : orientation(identity_quaternion), position(0, 0, 0) {
    }

    inline Transformation(const Quaternion &orient, const QVector &pos) : orientation(orient), position(pos) {
    }
    //inline Transformation(const Quaternion &orient, const QVector &pos) : orientation(orient), position(pos) { }

    inline void netswap() {
        orientation.netswap();
        position.netswap();
    }

    inline void to_matrix(Matrix &m) const {
        orientation.to_matrix(m);
        m.p = position;
    }

    inline void Compose(const Transformation &b, const Matrix &m) {
        orientation *= b.orientation;
        position = Transform(m, position);
    }

    inline void InvertOrientationRevPos() {
        orientation = orientation.Conjugate();
        position = -position;
    }

    inline void InvertAndToMatrix(Matrix &m) {
        InvertOrientationRevPos();
        to_matrix(m);
        m.p = TransformNormal(m, position);
    }

    static Transformation from_matrix(Matrix &m) {
        Vector p, q, r;
        QVector c;
        MatrixToVectors(m, p, q, r, c);
        return Transformation(Quaternion::from_vectors(p, q, r), c);
    }
};

const Transformation identity_transformation(identity_quaternion, QVector(0, 0, 0));

#endif //VEGA_STRIKE_ENGINE_GFX_QUATERNION_H
