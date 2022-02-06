/*
 * Copyright (C) 2001-2022 Roy Falk, Nachum Barcohen, David Wales,
 * Stephen G. Tuggy, and other Vega Strike contributors.
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
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */


#ifndef TVECTOR_H
#define TVECTOR_H

#ifdef __cplusplus

//#include "vectorizable.h"

class Matrix;

/*
 * XVector = Vector, QFLOAT = float, YVector = QVector
*/


// Moved to start to prevent 'specialization ... after instantiation' error
template<typename T>
T _netswap(T);

template<typename T>
T _sqrt(T);

// Note that S is QFLOAT and the main typename. T is used for defining YVector.
template<typename S, typename T>
class TVector {
// Fields
public:
    union {
        S i;
        S x;
    };
    union {
        S j;
        S y;
    };
    union {
        S k;
        S z;
    };

// Constructors
    TVector() : i(0), j(0), k(0)
    {
    }

    TVector(const TVector<T, S> &a);
    TVector(const TVector<S, T> &a);
    TVector(S i, S j, S k);
    TVector(struct _object *);

// Operators
public:
    TVector operator+(const TVector &obj) const;
    TVector operator-(const TVector &obj) const;
    S operator*(const TVector &b) const;
    TVector &operator=(const TVector &other);

    TVector operator-() const;
    bool operator==(const TVector &b) const;

    TVector &operator+=(const TVector &obj);
    TVector &operator-=(const TVector &obj);
    TVector &operator*=(const T &obj);

    const TVector<T, S> &operator=(const TVector<T, S> &a);

// Methods
private:
    friend class Quadsquare;
    friend class QuadTree;
    friend class CoordinateSelect;
    friend class AIScript;
    friend class SphericalTransform;

public:
    TVector<T, S> Cast() const;
    TVector Cross(const TVector &v) const;
    S Dot(const TVector &b) const;
    S Magnitude() const;
    S MagnitudeSquared() const;
    TVector Normalize();
    void netswap();
    TVector Scale(S s) const;
    void Set(S x, S y, S z);
    TVector Transform(const TVector &p, const TVector &q, const TVector &r);
    TVector Vabs() const;

    TVector Min(const TVector &other) const;
    TVector Max(const TVector &other) const;

    void Yaw(S rad); //only works with unit vector
    void Roll(S rad);
    void Pitch(S rad);

    // Vector::Transform definition moved from matrix.h
    // This is the unspecialized version, but it's really only for Vector
    const TVector Transform(const Matrix &m1) const;

    // Matrix::Transform definition moved from matrix.h and into the class
    // Now that we're using templates, we only need one
    TVector Transform(const Matrix &t, const TVector &v);
};

template<typename S, typename T>
TVector<S, T> operator*(TVector<S, T> vector, const float obj);

template<typename S, typename T>
TVector<S, T> operator*(const float obj, TVector<S, T> vector);

template<typename S, typename T>
TVector<S, T> operator*(TVector<S, T> vector, const double obj);

template<typename S, typename T>
TVector<S, T> operator*(const double obj, TVector<S, T> vector);

template<typename S, typename T>
TVector<S, T> operator*(TVector<S, T> vector, const int obj);

template<typename S, typename T>
TVector<S, T> operator*(const int obj, TVector<S, T> vector);

template<typename S, typename T>
TVector<S, T> operator/(const TVector<S, T> &lval, const S obj);

template<typename S, typename T>
TVector<S, T> CrossProduct(const TVector<S, T> &v1, const TVector<S, T> &v2);

template<typename S, typename T>
void CrossProduct(const TVector<S, T> &a, const TVector<S, T> &b, TVector<S, T> &RES);

template<typename S, typename T>
void ScaledCrossProduct(const TVector<S, T> &a, const TVector<S, T> &b, TVector<S, T> &r);

template<typename S, typename T>
S DotProduct(const TVector<S, T> &a, const TVector<S, T> &b);

template<typename S, typename T>
void Normalize(TVector<S, T> &r);

template<typename S, typename T>
bool IsShorterThan(const TVector<S, T> &a, S delta);

template<typename S, typename T>
bool IsShorterThan(const TVector<S, T> &a, T delta);

template<typename S, typename T>
TVector<S, T> PolygonNormal(TVector<S, T> v1, TVector<S, T> v2, TVector<S, T> v3);

template<typename S, typename T>
void ResetVectors(TVector<S, T> &p, TVector<S, T> &q, TVector<S, T> &r);

template<typename S, typename T>
void MakeRVector(TVector<S, T> &p, TVector<S, T> &q, TVector<S, T> &r);

//Makes a non-colinear vector q to given r. assumes magnitude of r is nonzero, does not check
template<typename S, typename T>
TVector<S, T> MakeNonColinearVector(const TVector<S, T> &p);

template<typename S, typename T>
void Orthogonize(TVector<S, T> &p, TVector<S, T> &q, TVector<S, T> &r);

template<typename S, typename T>
TVector<S, T> Transform(TVector<S, T> p, TVector<S, T> q, TVector<S, T> r, TVector<S, T> v);

template<typename S, typename T>
void Yaw(float rad, TVector<S, T> &p, TVector<S, T> &q, TVector<S, T> &r);

template<typename S, typename T>
void Pitch(float rad, TVector<S, T> &p, TVector<S, T> &q, TVector<S, T> &r);

template<typename S, typename T>
void Roll(float rad, TVector<S, T> &p, TVector<S, T> &q, TVector<S, T> &r);

typedef TVector<float, double> Vector;
typedef TVector<double, float> QVector;

#endif // C++
#endif // TVECTOR_H
