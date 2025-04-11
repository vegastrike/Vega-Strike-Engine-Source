/*
 * matrix.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file. Specifically:
 * Alan Shieh and pyramid3d
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
#ifndef VEGA_STRIKE_ENGINE_GFX_MATRIX_H
#define VEGA_STRIKE_ENGINE_GFX_MATRIX_H

#include "gfx_generic/vec.h"
#include "src/endianness.h"
#include "root_generic/vs_globals.h"

class Matrix {
private:
    float operator[](int i);
public:
    float r[9];
    QVector p;

    inline Matrix() : p(0, 0, 0) {
        r[0] = r[1] = r[2] = r[3] = r[4] = r[5] = r[6] = r[7] = r[8] = 0;
    }

//Convert the matrix into network byte order
    void netswap() {
        for (int i = 0; i < 9; i++) {
            r[i] = VSSwapHostFloatToLittle(r[i]);
        }
        p.netswap();
    }

    inline Vector getR() const {
        return Vector(r[6], r[7], r[8]);
    }

    inline Vector getQ() const {
        return Vector(r[3], r[4], r[5]);
    }

    inline Vector getP() const {
        return Vector(r[0], r[1], r[2]);
    }

    inline Matrix(float r0,
            float r1,
            float r2,
            float r3,
            float r4,
            float r5,
            float r6,
            float r7,
            float r8,
            QVector pos) {
        r[0] = r0;
        r[1] = r1;
        r[2] = r2;
        r[3] = r3;
        r[4] = r4;
        r[5] = r5;
        r[6] = r6;
        r[7] = r7;
        r[8] = r8;
        p = pos;
    }

    inline void InvertRotationInto(Matrix &result) const {
        result.r[0] = r[0];
        result.r[1] = r[3];
        result.r[2] = r[6];
        result.r[3] = r[1];
        result.r[4] = r[4];
        result.r[5] = r[7];
        result.r[6] = r[2];
        result.r[7] = r[5];
        result.r[8] = r[8];
    }

    inline Matrix(const Vector &v1, const Vector &v2, const Vector &v3) : p(0, 0, 0) {
        this->r[0] = v1.i;
        this->r[1] = v1.j;
        this->r[2] = v1.k;

        this->r[3] = v2.i;
        this->r[4] = v2.j;
        this->r[5] = v2.k;

        this->r[6] = v3.i;
        this->r[7] = v3.j;
        this->r[8] = v3.k;
    }

    inline Matrix(const Vector &v1, const Vector &v2, const Vector &v3, const QVector &pos);
    inline Matrix operator*(const Matrix &m2) const;
};

const Matrix identity_matrix(1, 0, 0,
        0, 1, 0,
        0, 0, 1,
        QVector(0, 0, 0));

/** moves a vector struct to a matrix */

inline void ScaleMatrix(Matrix &RESTRICT matrix, const Vector &RESTRICT scale) {
    matrix.r[0] *= scale.i;
    matrix.r[1] *= scale.i;
    matrix.r[2] *= scale.i;
    matrix.r[3] *= scale.j;
    matrix.r[4] *= scale.j;
    matrix.r[5] *= scale.j;
    matrix.r[6] *= scale.k;
    matrix.r[7] *= scale.k;
    matrix.r[8] *= scale.k;
}

inline void VectorAndPositionToMatrix(Matrix &matrix,
        const Vector &v1,
        const Vector &v2,
        const Vector &v3,
        const QVector &pos) {
    matrix.r[0] = v1.i;
    matrix.r[1] = v1.j;
    matrix.r[2] = v1.k;

    matrix.r[3] = v2.i;
    matrix.r[4] = v2.j;
    matrix.r[5] = v2.k;

    matrix.r[6] = v3.i;
    matrix.r[7] = v3.j;
    matrix.r[8] = v3.k;
    matrix.p = pos;
}

inline Matrix::Matrix(const Vector &RESTRICT v1,
        const Vector &RESTRICT v2,
        const Vector &RESTRICT v3,
        const QVector &RESTRICT pos) {
    VectorAndPositionToMatrix(*this, v1, v2, v3, pos);
}

/** zeros out a 4x4 matrix quickly
 */
inline void Zero(Matrix &RESTRICT matrix) {
    for (unsigned int i = 0; i < 9; i++) {
        matrix.r[i] = 0;
    }
    matrix.p.Set(0, 0, 0);
}

/** Computes a 4x4 identity matrix
 */
inline void Identity(Matrix &RESTRICT matrix) {
    Zero(matrix);
    matrix.r[0] = matrix.r[4] = matrix.r[8] = 1;
}

/** Computes a Translation matrix based on x,y,z translation
 */

inline void RotateAxisAngle(Matrix &RESTRICT tmp, const Vector &RESTRICT axis, const float angle) {
    float c = cosf(angle);
    float s = sinf(angle);
#define M(a, b) (tmp.r[b*3+a])
    M(0, 0) = axis.i * axis.i * (1 - c) + c;
    M(0, 1) = axis.i * axis.j * (1 - c) - axis.k * s;
    M(0, 2) = axis.i * axis.k * (1 - c) + axis.j * s;
    //M(0,3)=0;
    M(1, 0) = axis.j * axis.i * (1 - c) + axis.k * s;
    M(1, 1) = axis.j * axis.j * (1 - c) + c;
    M(1, 2) = axis.j * axis.k * (1 - c) - axis.i * s;
    //M(1,3)=0;
    M(2, 0) = axis.i * axis.k * (1 - c) - axis.j * s;
    M(2, 1) = axis.j * axis.k * (1 - c) + axis.i * s;
    M(2, 2) = axis.k * axis.k * (1 - c) + c;
    //M(2,3)=0;
#undef M
    tmp.p.Set(0, 0, 0);
}

inline void Translate(Matrix &matrix, const QVector &v) {
    for (unsigned int i = 0; i < 9; i++) {
        matrix.r[i] = 0;
    }
    matrix.r[0] = matrix.r[4] = matrix.r[8] = 1;
    matrix.p = v;
}

/** Multiplies m1 and m2 and pops the result into dest;
 *  dest != m1, dest !=m2
 */
inline void MultMatrix(Matrix &RESTRICT dest, const Matrix &RESTRICT m1, const Matrix &RESTRICT m2) {
    dest.r[0] = m1.r[0] * m2.r[0] + m1.r[3] * m2.r[1] + m1.r[6] * m2.r[2];
    dest.r[1] = m1.r[1] * m2.r[0] + m1.r[4] * m2.r[1] + m1.r[7] * m2.r[2];
    dest.r[2] = m1.r[2] * m2.r[0] + m1.r[5] * m2.r[1] + m1.r[8] * m2.r[2];

    dest.r[3] = m1.r[0] * m2.r[3] + m1.r[3] * m2.r[4] + m1.r[6] * m2.r[5];
    dest.r[4] = m1.r[1] * m2.r[3] + m1.r[4] * m2.r[4] + m1.r[7] * m2.r[5];
    dest.r[5] = m1.r[2] * m2.r[3] + m1.r[5] * m2.r[4] + m1.r[8] * m2.r[5];

    dest.r[6] = m1.r[0] * m2.r[6] + m1.r[3] * m2.r[7] + m1.r[6] * m2.r[8];
    dest.r[7] = m1.r[1] * m2.r[6] + m1.r[4] * m2.r[7] + m1.r[7] * m2.r[8];
    dest.r[8] = m1.r[2] * m2.r[6] + m1.r[5] * m2.r[7] + m1.r[8] * m2.r[8];

    dest.p.i = m1.r[0] * m2.p.i + m1.r[3] * m2.p.j + m1.r[6] * m2.p.k + m1.p.i;
    dest.p.j = m1.r[1] * m2.p.i + m1.r[4] * m2.p.j + m1.r[7] * m2.p.k + m1.p.j;
    dest.p.k = m1.r[2] * m2.p.i + m1.r[5] * m2.p.j + m1.r[8] * m2.p.k + m1.p.k;
}

inline Matrix Matrix::operator*(const Matrix &m2) const {
    Matrix res;
    MultMatrix(res, *this, m2);
    return res;
}

/**
 * Copies Matrix source into the destination Matrix
 */
inline void CopyMatrix(Matrix &dest, const Matrix &source) {
    dest = source;
}

/**
 * moves a vector in the localspace to world space through matrix t
 */
inline QVector Transform(const Matrix &RESTRICT t, const QVector &RESTRICT v) {
    return QVector(t.p.i + v.i * t.r[0] + v.j * t.r[3] + v.k * t.r[6],
            t.p.j + v.i * t.r[1] + v.j * t.r[4] + v.k * t.r[7],
            t.p.k + v.i * t.r[2] + v.j * t.r[5] + v.k * t.r[8]);
}

inline Vector Transform(const Matrix &t, const Vector &v) {
    const double v_dot_i_temp = v.i;
    const double v_dot_j_temp = v.j;
    const double v_dot_k_temp = v.k;
    return Vector(t.p.i + v_dot_i_temp * t.r[0] + v_dot_j_temp * t.r[3] + v_dot_k_temp * t.r[6],
            t.p.j + v_dot_i_temp * t.r[1] + v_dot_j_temp * t.r[4] + v_dot_k_temp * t.r[7],
            t.p.k + v_dot_i_temp * t.r[2] + v_dot_j_temp * t.r[5] + v_dot_k_temp * t.r[8]);
}

//these vectors are going to be just normals...
inline Vector InvTransformNormal(const Matrix &t, const Vector &v) {
#define M(A, B) (t.r[B*3+A])
    return Vector(v.i * M(0, 0) + v.j * M(1, 0) + v.k * M(2, 0),
            v.i * M(0, 1) + v.j * M(1, 1) + v.k * M(2, 1),
            v.i * M(0, 2) + v.j * M(1, 2) + v.k * M(2, 2));

#undef M
}

inline QVector InvTransformNormal(const Matrix &t, const QVector &v) {
#define M(A, B) (t.r[B*3+A])
    return QVector(v.i * M(0, 0) + v.j * M(1, 0) + v.k * M(2, 0),
            v.i * M(0, 1) + v.j * M(1, 1) + v.k * M(2, 1),
            v.i * M(0, 2) + v.j * M(1, 2) + v.k * M(2, 2));

#undef M
}

inline QVector InvTransform(const Matrix &t, const QVector &v) {
    return InvTransformNormal(t, QVector(v.i - t.p.i, v.j - t.p.j, v.k - t.p.k));
}

inline Vector InvTransform(const Matrix &t, const Vector &v) {
    return InvTransformNormal(t, QVector(v.i - t.p.i, v.j - t.p.j, v.k - t.p.k)).Cast();
}

inline Vector TransformNormal(const Matrix &t, const Vector &v) {
    return Vector(v.i * t.r[0] + v.j * t.r[3] + v.k * t.r[6],
            v.i * t.r[1] + v.j * t.r[4] + v.k * t.r[7],
            v.i * t.r[2] + v.j * t.r[5] + v.k * t.r[8]);
}

inline QVector TransformNormal(const Matrix &t, const QVector &v) {
    return QVector(v.i * t.r[0] + v.j * t.r[3] + v.k * t.r[6],
            v.i * t.r[1] + v.j * t.r[4] + v.k * t.r[7],
            v.i * t.r[2] + v.j * t.r[5] + v.k * t.r[8]);
}

int invert(float b[], float a[]);

inline void MatrixToVectors(const Matrix &m, Vector &p, Vector &q, Vector &r, QVector &c) {
    p.Set(m.r[0], m.r[1], m.r[2]);
    q.Set(m.r[3], m.r[4], m.r[5]);
    r.Set(m.r[6], m.r[7], m.r[8]);
    c = m.p;
}

inline QVector InvScaleTransform(const Matrix &trans, QVector pos) {
    pos = pos - trans.p;
#define a (trans.r[0])
#define b (trans.r[3])
#define c (trans.r[6])
#define d (trans.r[1])
#define e (trans.r[4])
#define f (trans.r[7])
#define g (trans.r[2])
#define h (trans.r[5])
#define i (trans.r[8])
    double factor = 1.0F / (-c * e * g + b * f * g + c * d * h - a * f * h - b * d * i + a * e * i);
    return QVector(pos.Dot(QVector(e * i - f * h, c * h - b * i,
                    b * f - c * e)),
            pos.Dot(QVector(f * g - d * i, a * i - c * g, c * d - a * f)),
            pos.Dot(QVector(d * h - e * g, b * g - a * h, a * e - b * d))) * factor;

#undef a
#undef b
#undef c
#undef d
#undef e
#undef f
#undef g
#undef h
#undef i
}

inline void InvertMatrix(Matrix &o, const Matrix &trans) {
#define a (trans.r[0])
#define b (trans.r[3])
#define c (trans.r[6])
#define d (trans.r[1])
#define e (trans.r[4])
#define f (trans.r[7])
#define g (trans.r[2])
#define h (trans.r[5])
#define i (trans.r[8])
    float factor = 1.0F / (-c * e * g + b * f * g + c * d * h - a * f * h - b * d * i + a * e * i);
    o.r[0] = factor * (e * i - f * h);
    o.r[3] = factor * (c * h - b * i);
    o.r[6] = factor * (b * f - c * e);
    o.r[1] = factor * (f * g - d * i);
    o.r[4] = factor * (a * i - c * g);
    o.r[7] = factor * (c * d - a * f);
    o.r[2] = factor * (d * h - e * g);
    o.r[5] = factor * (b * g - a * h);
    o.r[8] = factor * (a * e - b * d);
#undef a
#undef b
#undef c
#undef d
#undef e
#undef f
#undef g
#undef h
#undef i
    o.p = TransformNormal(o, QVector(-trans.p));
}

inline void Rotate(Matrix &tmp, const Vector &axis, float angle) {
    float c = cos(angle);
    float s = sin(angle);
//Row, COl
#define M(a, b) (tmp.r[b*3+a])
    M(0, 0) = axis.i * axis.i * (1 - c) + c;
    M(0, 1) = axis.i * axis.j * (1 - c) - axis.k * s;
    M(0, 2) = axis.i * axis.k * (1 - c) + axis.j * s;
    //M(0,3)=0;
    M(1, 0) = axis.j * axis.i * (1 - c) + axis.k * s;
    M(1, 1) = axis.j * axis.j * (1 - c) + c;
    M(1, 2) = axis.j * axis.k * (1 - c) - axis.i * s;
    //M(1,3)=0;
    M(2, 0) = axis.i * axis.k * (1 - c) - axis.j * s;
    M(2, 1) = axis.j * axis.k * (1 - c) + axis.i * s;
    M(2, 2) = axis.k * axis.k * (1 - c) + c;
    //M(2,3)=0;
#undef M
    tmp.p.Set(0, 0, 0);
}

struct DrawContext {
    Matrix m;
    class GFXVertexList *vlist;

    DrawContext() {
    }

    DrawContext(const Matrix &a, GFXVertexList *vl) : m(a), vlist(vl) {
    }
};

#endif //VEGA_STRIKE_ENGINE_GFX_MATRIX_H
