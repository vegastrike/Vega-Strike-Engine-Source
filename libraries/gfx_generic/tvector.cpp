/*
 * tvector.cpp
 *
 * Copyright (C) 2001-2023 Daniel Horn, Roy Falk, Nachum Barcohen,
 * Stephen G. Tuggy, Davie Wales, and other Vega Strike contributors
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */


#define PY_SSIZE_T_CLEAN
#include <boost/python.hpp>
#include "tvector.h"
#include "matrix.h"

#include <boost/python/class.hpp>
#include <boost/python/to_python_converter.hpp>
#include <boost/python/to_python_indirect.hpp>
#include <boost/python/to_python_value.hpp>
#include <boost/python/converter/builtin_converters.hpp>
#include "vegastrike.h"
#include <math.h>
#define _CZ (761.465325527)

// Specialization comes first (actually only for declaration)
template<typename T>
T _netswap(T t) {
    return t;
}

template<>
float _netswap(float f) {
    return VSSwapHostFloatToLittle(f);
}

template<>
double _netswap(double d) {
    return VSSwapHostDoubleToLittle(d);
}

template<typename T>
T _sqrt(T t) {
    return sqrtf(t);
}

template<>
float _sqrt(float f) {
    return sqrtf(f);
}

template<>
double _sqrt(double d) {
    return sqrt(d);
}

// Constructors
template<typename S, typename T>
TVector<S, T>::TVector(const TVector<T, S> &a) {
    i = static_cast<S>(a.i);
    j = static_cast<S>(a.j);
    k = static_cast<S>(a.k);
}

template<typename S, typename T>
TVector<S, T>::TVector(const TVector<S, T> &a) {
    i = a.i;
    j = a.j;
    k = a.k;
}

template<typename S, typename T>
TVector<S, T>::TVector(S i, S j, S k) {
    this->i = i;
    this->j = j;
    this->k = k;
}

template<>
TVector<float, double>::TVector(PyObject *p) {
    TVector<float, double> vec(0, 0, 0);
    static char fff[4] = "fff";
    PyArg_ParseTuple(p, fff, &vec.i, &vec.j, &vec.k);
    *this = vec;
}

template<>
TVector<double, float>::TVector(PyObject *p) {
    TVector<double, float> vec(0, 0, 0);
    static char ddd[4] = "ddd";
    PyArg_ParseTuple(p, ddd, &vec.i, &vec.j, &vec.k);
    *this = vec;
}

// Operators
template<typename S, typename T>
TVector<S, T> TVector<S, T>::operator+(const TVector<S, T> &obj) const {
    return TVector(i + obj.i, j + obj.j, k + obj.k);
}

template<typename S, typename T>
TVector<S, T> TVector<S, T>::operator-(const TVector<S, T> &obj) const {
    return TVector(i - obj.i, j - obj.j, k - obj.k);
}

template<typename S, typename T>
S TVector<S, T>::operator*(const TVector &b) const {
    return i * b.i + j * b.j + k * b.k;
}

template<typename S, typename T>
TVector<S, T> operator/(const TVector<S, T> &lval, const S obj) {
    return lval * (S(1) / obj);
}

template<typename S, typename T>
TVector<S, T> &TVector<S, T>::operator=(const TVector<S, T> &other) {
    this->i = other.i;
    this->j = other.j;
    this->k = other.k;
    return *this;
}

template<typename S, typename T>
const TVector<T, S> &TVector<S, T>::operator=(const TVector<T, S> &a) {
    i = static_cast<S>(a.i);
    j = static_cast<S>(a.j);
    k = static_cast<S>(a.k);
    return a;
}

template<typename S, typename T>
TVector<S, T> TVector<S, T>::operator-() const {
    return TVector(-i, -j, -k);
}

template<typename S, typename T>
bool TVector<S, T>::operator==(const TVector<S, T> &b) const {
    return i == b.i && j == b.j && k == b.k;
}

template<typename S, typename T>
TVector<S, T> &TVector<S, T>::operator+=(const TVector<S, T> &obj) {
    i += obj.i;
    j += obj.j;
    k += obj.k;
    return *this;
}

template<typename S, typename T>
TVector<S, T> &TVector<S, T>::operator-=(const TVector<S, T> &obj) {
    i -= obj.i;
    j -= obj.j;
    k -= obj.k;
    return *this;
}

template<typename S, typename T>
TVector<S, T> &TVector<S, T>::operator*=(const T &obj) {
    i *= obj;
    j *= obj;
    k *= obj;
    return *this;
}

// missing     const TVector<T,S>& operator=( const TVector<T,S> &a );


// Methods

template<typename S, typename T>
TVector<T, S> TVector<S, T>::Cast() const {
    return TVector<T, S>(i, j, k);
}

template<typename S, typename T>
TVector<S, T> TVector<S, T>::Cross(const TVector<S, T> &v) const {
    return TVector(this->j * v.k - this->k * v.j,
            this->k * v.i - this->i * v.k,
            this->i * v.j - this->j * v.i);
}

template<typename S, typename T>
S TVector<S, T>::Dot(const TVector &b) const {
    return DotProduct(*this, b);
}

// Not used. Specialized below.
template<typename S, typename T>
S TVector<S, T>::Magnitude() const {
    return sqrtf(i * i + j * j + k * k);
}

template<>
float TVector<float, double>::Magnitude() const {
    return sqrtf(i * i + j * j + k * k);
}

template<>
double TVector<double, float>::Magnitude() const {
    return sqrt(i * i + j * j + k * k);
}

template<typename S, typename T>
S TVector<S, T>::MagnitudeSquared() const {
    return i * i + j * j + k * k;
}

template<typename S, typename T>
void TVector<S, T>::netswap() {
    this->i = _netswap(this->i);
    this->j = _netswap(this->j);
    this->k = _netswap(this->k);
}

template<typename S, typename T>
TVector<S, T> TVector<S, T>::Normalize() {
    ::Normalize(*this);
    return *this;
}

template<typename S, typename T>
TVector<S, T> TVector<S, T>::Scale(S s) const {
    return TVector(s * i, s * j, s * k);
}

template<typename S, typename T>
void TVector<S, T>::Set(S x, S y, S z) {
    i = x;
    j = y;
    k = z;
}

template<typename S, typename T>
TVector<S, T> TVector<S, T>::Transform(const TVector<S, T> &p, const TVector<S, T> &q, const TVector<S, T> &r) {
    TVector tvect = TVector(DotProduct(*this, p), DotProduct(*this, q), DotProduct(*this, r));
    *this = tvect;
    return *this;
}

template<typename S, typename T>
TVector<S, T> TVector<S, T>::Vabs() const {
    return TVector(i >= 0 ? i : -i,
            j >= 0 ? j : -j,
            k >= 0 ? k : -k);
}

template<typename S, typename T>
TVector<S, T> TVector<S, T>::Min(const TVector<S, T> &other) const {
    return TVector((i < other.i) ? i : other.i,
            (j < other.j) ? j : other.j,
            (k < other.k) ? k : other.k);
}

template<typename S, typename T>
TVector<S, T> TVector<S, T>::Max(const TVector<S, T> &other) const {
    return TVector((i > other.i) ? i : other.i,
            (j > other.j) ? j : other.j,
            (k > other.k) ? k : other.k);
}



/////////////////////////////////////////////////////////////
// Yaw Roll and Pitch a unit vector
/////////////////////////////////////////////////////////////

template<typename S, typename T>
void TVector<S, T>::Yaw(S rad) //only works with unit vector
{
    float theta;
    theta = 0.0f; //FIXME This line added temporarily by chuck_starchaser
    if (i > 0) {
        theta = (float) atan(k / i);
    } else if (i < 0) {
        theta = PI + (float) atan(k / i);
    } else if (k <= 0 && i == 0) {
        theta = -PI / 2;
    } else if (k > 0 && i == 0) {
        theta = PI / 2;
    }
    theta += rad; //FIXME If none of the if's is true, theta is uninitialized!
    i = cosf(theta);
    k = sinf(theta);
}

template<typename S, typename T>
void TVector<S, T>::Roll(S rad) {
    float theta;
    theta = 0.0f; //FIXME This line added temporarily by chuck_starchaser
    if (i > 0) {
        theta = (float) atan(j / i);
    } else if (i < 0) {
        theta = PI + (float) atan(j / i);
    } else if (j <= 0 && i == 0) {
        theta = -PI / 2;
    } else if (j > 0 && i == 0) {
        theta = PI / 2;
    }
    theta += rad; //FIXME If none of the if's is true, theta is uninitialized!
    i = cosf(theta);
    j = sinf(theta);
}

template<typename S, typename T>
void TVector<S, T>::Pitch(S rad) {
    float theta;
    theta = 0.0f; //FIXME This line added temporarily by chuck_starchaser
    if (k > 0) {
        theta = (float) atan(j / k);
    } else if (k < 0) {
        theta = PI + (float) atan(j / k);
    } else if (j <= 0 && k == 0) {
        theta = -PI / 2;
    } else if (j > 0 && k == 0) {
        theta = PI / 2;
    }
    theta += rad; //FIXME If none of the if's is true, theta is uninitialized!
    k = cosf(theta);
    j = sinf(theta);
}

template<typename S, typename T>
void Yaw(float rad, TVector<S, T> &p, TVector<S, T> &q, TVector<S, T> &r) {
    TVector<S, T> temp1, temp2, temp3;
    temp1.i = 1;
    temp1.j = 0;
    temp1.k = 0;
    temp1.Yaw(rad);
    temp2.i = temp1.i * p.i + temp1.j * q.i + temp1.k * r.i;
    temp2.j = temp1.i * p.j + temp1.j * q.j + temp1.k * r.j;
    temp2.k = temp1.i * p.k + temp1.j * q.k + temp1.k * r.k;
    temp1.i = 0;
    temp1.j = 0;
    temp1.k = 1;
    temp1.Yaw(rad);
    temp3.i = temp1.i * p.i + temp1.j * q.i + temp1.k * r.i;
    temp3.j = temp1.i * p.j + temp1.j * q.j + temp1.k * r.j;
    temp3.k = temp1.i * p.k + temp1.j * q.k + temp1.k * r.k;
    p = temp2;
    r = temp3;
}

template<typename S, typename T>
void Roll(float rad, TVector<S, T> &p, TVector<S, T> &q, TVector<S, T> &r) {
    TVector<S, T> temp1, temp2, temp3;
    temp1.i = 1;
    temp1.j = 0;
    temp1.k = 0;
    temp1.Roll(rad);
    temp2.i = temp1.i * p.i + temp1.j * q.i + temp1.k * r.i;
    temp2.j = temp1.i * p.j + temp1.j * q.j + temp1.k * r.j;
    temp2.k = temp1.i * p.k + temp1.j * q.k + temp1.k * r.k;
    temp1.i = 0;
    temp1.j = 1;
    temp1.k = 0;
    temp1.Roll(rad);
    temp3.i = temp1.i * p.i + temp1.j * q.i + temp1.k * r.i;
    temp3.j = temp1.i * p.j + temp1.j * q.j + temp1.k * r.j;
    temp3.k = temp1.i * p.k + temp1.j * q.k + temp1.k * r.k;
    p = temp2;
    q = temp3;
}

template<typename S, typename T>
void Pitch(float rad, TVector<S, T> &p, TVector<S, T> &q, TVector<S, T> &r) {
    TVector<S, T> temp1, temp2, temp3;
    temp1.i = 0;
    temp1.j = 1;
    temp1.k = 0;
    temp1.Pitch(rad);
    temp2.i = temp1.i * p.i + temp1.j * q.i + temp1.k * r.i;
    temp2.j = temp1.i * p.j + temp1.j * q.j + temp1.k * r.j;
    temp2.k = temp1.i * p.k + temp1.j * q.k + temp1.k * r.k;
    temp1.i = 0;
    temp1.j = 0;
    temp1.k = 1;
    temp1.Pitch(rad);
    temp3.i = temp1.i * p.i + temp1.j * q.i + temp1.k * r.i;
    temp3.j = temp1.i * p.j + temp1.j * q.j + temp1.k * r.j;
    temp3.k = temp1.i * p.k + temp1.j * q.k + temp1.k * r.k;
    q = temp2;
    r = temp3;
}







////////////////////////////////////////////////////////////

template<typename S, typename T>
TVector<S, T> operator*(TVector<S, T> vector, const float obj) {
    return TVector<S, T>(vector.i * obj, vector.j * obj, vector.k * obj);
}

template<typename S, typename T>
TVector<S, T> operator*(const float obj, TVector<S, T> vector) {
    return TVector<S, T>(vector.i * obj, vector.j * obj, vector.k * obj);
}

template<typename S, typename T>
TVector<S, T> operator*(TVector<S, T> vector, const double obj) {
    return TVector<S, T>(vector.i * obj, vector.j * obj, vector.k * obj);
}

template<typename S, typename T>
TVector<S, T> operator*(const double obj, TVector<S, T> vector) {
    return TVector<S, T>(vector.i * obj, vector.j * obj, vector.k * obj);
}

template<typename S, typename T>
TVector<S, T> operator*(TVector<S, T> vector, const int obj) {
    return TVector<S, T>(vector.i * obj, vector.j * obj, vector.k * obj);
}

template<typename S, typename T>
TVector<S, T> operator*(const int obj, TVector<S, T> vector) {
    return TVector<S, T>(vector.i * obj, vector.j * obj, vector.k * obj);
}

template<typename S, typename T>
TVector<S, T> CrossProduct(const TVector<S, T> &v1, const TVector<S, T> &v2) {
    TVector<S, T> result;
    result.i = v1.j * v2.k - v1.k * v2.j;
    result.j = v1.k * v2.i - v1.i * v2.k;
    result.k = v1.i * v2.j - v1.j * v2.i;
    return result;
}

template<typename S, typename T>
void CrossProduct(const TVector<S, T> &a, const TVector<S, T> &b, TVector<S, T> &RES) {
    RES = a.Cross(b);
}

template<typename S, typename T>
void ScaledCrossProduct(const TVector<S, T> &a, const TVector<S, T> &b, TVector<S, T> &r) {
    r.i = a.j * b.k - a.k * b.j;
    r.j = a.k * b.i - a.i * b.k;
    r.k = a.i * b.j - a.j * b.i;
    T size = _sqrt(r.i * r.i + r.j * r.j + r.k * r.k);
    if (size < 0.00001) {
        r.i = r.j = r.k = 0;
    } else {
        r.i /= size;
        r.j /= size;
        r.k /= size;
    }
}

template<typename S, typename T>
S DotProduct(const TVector<S, T> &a, const TVector<S, T> &b) {
    return a.i * b.i + a.j * b.j + a.k * b.k;
}

// Matrix::Transform definition moved from matrix.h and into the class
// Now that we're using templates, we only need one
TVector<double, float> _Transform(const Matrix &t, const TVector<double, float> &v) {
    return TVector<double, float>(t.p.i + v.i * t.r[0] + v.j * t.r[3] + v.k * t.r[6],
            t.p.j + v.i * t.r[1] + v.j * t.r[4] + v.k * t.r[7],
            t.p.k + v.i * t.r[2] + v.j * t.r[5] + v.k * t.r[8]);
}

// Vector::Transform definition moved from matrix.h
// This is the specialized version, for QVector
template<>
const TVector<double, float> TVector<double, float>::Transform(const Matrix &m1) const {
    return _Transform(m1, *this);
}

// Vector::Transform definition moved from matrix.h
// This is the specialized version, for Vector
template<>
const TVector<float, double> TVector<float, double>::Transform(const Matrix &m1) const {
    TVector<double, float> ret = _Transform(m1, TVector(i, j, k));
    return TVector<float, double>(ret.i, ret.j, ret.k);
}

template<typename S, typename T>
void Normalize(TVector<S, T> &r) {
    S size = r.i * r.i + r.j * r.j + r.k * r.k;
    if (size > 0.00000000001) {
        S isize = S(1.0) / _sqrt(size);
        r.i *= isize;
        r.j *= isize;
        r.k *= isize;
    }
}

template<typename S, typename T>
bool IsShorterThan(const TVector<S, T> &a, S delta) {
    return (a.MagnitudeSquared() < (delta * delta));
}

template<typename S, typename T>
bool IsShorterThan(const TVector<S, T> &a, T delta) {
    return (a.MagnitudeSquared() < (delta * delta));
}

template<typename S, typename T>
TVector<S, T> PolygonNormal(TVector<S, T> v1, TVector<S, T> v2, TVector<S, T> v3) {
    TVector<S, T> temp;
    ScaledCrossProduct(v2 - v1, v3 - v1, temp);
    return temp;
}

template<typename S, typename T>
void ResetVectors(TVector<S, T> &p, TVector<S, T> &q, TVector<S, T> &r) {
    p.i = q.j = r.k = 1;
    p.j = p.k = q.i = q.k = r.i = r.j = 0;
}

template<typename S, typename T>
void MakeRVector(TVector<S, T> &p, TVector<S, T> &q, TVector<S, T> &r) {
    ScaledCrossProduct(p, q, r);
    ScaledCrossProduct(r, p, q);
    Normalize(p);
}

//Makes a non-colinear vector q to given r. assumes magnitude of r is nonzero, does not check
template<typename S, typename T>
TVector<S, T> MakeNonColinearVector(const TVector<S, T> &p) {
    TVector<S, T> q(p);
    if ((p.i == p.j) == 0) {
        q.i = p.k;
        q.j = p.i;
        q.k = p.j;
    } else {
        q.i = -p.j;
        q.j = p.i;
        q.k = p.k;
    }
    return q;
}

template<typename S, typename T>
void Orthogonize(TVector<S, T> &p, TVector<S, T> &q, TVector<S, T> &r) {
    Normalize(r);
    ScaledCrossProduct(r, p, q);     //result of scaled cross put into q
    ScaledCrossProduct(q, r, p);     //result of scaled cross put back into p
}

template<typename S, typename T>
TVector<S, T> Transform(TVector<S, T> p, TVector<S, T> q, TVector<S, T> r, TVector<S, T> v) {
    return TVector<S, T>(p.i * v.i + q.i * v.j + r.i * v.k,
            p.j * v.i + q.j * v.j + r.j * v.k,
            p.k * v.i + q.k * v.j + r.k * v.k);
}

// Template Instantiation
template
class TVector<float, double>;
template
class TVector<double, float>;

template TVector<float, double> operator*(TVector<float, double> vector, const float obj);
template TVector<double, float> operator*(TVector<double, float> vector, const float obj);

template TVector<float, double> operator*(const float obj, TVector<float, double> vector);
template TVector<double, float> operator*(const float obj, TVector<double, float> vector);

template TVector<float, double> operator*(TVector<float, double> vector, const double obj);
template TVector<double, float> operator*(TVector<double, float> vector, const double obj);

template TVector<float, double> operator*(const double obj, TVector<float, double> vector);
template TVector<double, float> operator*(const double obj, TVector<double, float> vector);

template TVector<float, double> operator*(TVector<float, double> vector, const int obj);
template TVector<double, float> operator*(TVector<double, float> vector, const int obj);

template TVector<float, double> operator*(const int obj, TVector<float, double> vector);
template TVector<double, float> operator*(const int obj, TVector<double, float> vector);

template TVector<float, double> operator/(const TVector<float, double> &lval, const float obj);
template TVector<double, float> operator/(const TVector<double, float> &lval, const double obj);

template TVector<float, double> CrossProduct(const TVector<float, double> &v1, const TVector<float, double> &v2);
template TVector<double, float> CrossProduct(const TVector<double, float> &v1, const TVector<double, float> &v2);

template void CrossProduct(const TVector<float, double> &a,
        const TVector<float, double> &b,
        TVector<float, double> &RES);
template void CrossProduct(const TVector<double, float> &a,
        const TVector<double, float> &b,
        TVector<double, float> &RES);

template void ScaledCrossProduct(const TVector<float, double> &a,
        const TVector<float, double> &b,
        TVector<float, double> &r);
template void ScaledCrossProduct(const TVector<double, float> &a,
        const TVector<double, float> &b,
        TVector<double, float> &r);

template float DotProduct(const TVector<float, double> &a, const TVector<float, double> &b);
template double DotProduct(const TVector<double, float> &a, const TVector<double, float> &b);

template void Normalize(TVector<float, double> &r);
template void Normalize(TVector<double, float> &r);

template bool IsShorterThan(const TVector<float, double> &a, float delta);
template bool IsShorterThan(const TVector<double, float> &a, float delta);
template bool IsShorterThan(const TVector<float, double> &a, double delta);
template bool IsShorterThan(const TVector<double, float> &a, double delta);

template TVector<float, double> PolygonNormal(TVector<float, double> v1,
        TVector<float, double> v2,
        TVector<float, double> v3);
template TVector<double, float> PolygonNormal(TVector<double, float> v1,
        TVector<double, float> v2,
        TVector<double, float> v3);

template void ResetVectors(TVector<float, double> &p, TVector<float, double> &q, TVector<float, double> &r);
template void ResetVectors(TVector<double, float> &p, TVector<double, float> &q, TVector<double, float> &r);

template void MakeRVector(TVector<float, double> &p, TVector<float, double> &q, TVector<float, double> &r);
template void MakeRVector(TVector<double, float> &p, TVector<double, float> &q, TVector<double, float> &r);

template TVector<float, double> MakeNonColinearVector(const TVector<float, double> &p);
template TVector<double, float> MakeNonColinearVector(const TVector<double, float> &p);

template void Orthogonize(TVector<float, double> &p, TVector<float, double> &q, TVector<float, double> &r);
template void Orthogonize(TVector<double, float> &p, TVector<double, float> &q, TVector<double, float> &r);

template TVector<float, double> Transform(TVector<float, double> p,
        TVector<float, double> q,
        TVector<float, double> r,
        TVector<float, double> v);
template TVector<double, float> Transform(TVector<double, float> p,
        TVector<double, float> q,
        TVector<double, float> r,
        TVector<double, float> v);

template void Yaw(float rad, TVector<float, double> &p, TVector<float, double> &q, TVector<float, double> &r);
template void Yaw(float rad, TVector<double, float> &p, TVector<double, float> &q, TVector<double, float> &r);

template void Pitch(float rad, TVector<float, double> &p, TVector<float, double> &q, TVector<float, double> &r);
template void Pitch(float rad, TVector<double, float> &p, TVector<double, float> &q, TVector<double, float> &r);

template void Roll(float rad, TVector<float, double> &p, TVector<float, double> &q, TVector<float, double> &r);
template void Roll(float rad, TVector<double, float> &p, TVector<double, float> &q, TVector<double, float> &r);
