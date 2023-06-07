/**
 * Vector.h
 *
 * Copyright (C) Daniel Horn
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike
 * contributors
 * Copyright (C) 2022-2023 Stephen G. Tuggy, Benjamen R. Meyer
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
#ifndef VEGA_STRIKE_ENGINE_AUDIO_VECTOR_H
#define VEGA_STRIKE_ENGINE_AUDIO_VECTOR_H

//
// C++ Interface: Audio::Codec
//

#include <math.h>

namespace Audio {

template<typename T>
class TVector3 {
public:
    T x, y, z;

    TVector3() {
    }

    TVector3(T xx, T yy, T zz) : x(xx), y(yy), z(zz) {
    }

    explicit TVector3(T s) : x(s), y(s), z(s) {
    }

    template<typename Y>
    TVector3(const TVector3<Y> &other) : x(T(other.x)), y(T(other.y)), z(T(other.z)) {
    }

    template<typename Y>
    TVector3<T> &operator=(const TVector3<Y> &other) {
        x = T(other.x);
        y = T(other.y);
        z = T(other.z);
        return *this;
    }

    TVector3<T> &operator+=(const TVector3<T> &other) {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    TVector3<T> &operator-=(const TVector3<T> &other) {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    TVector3<T> &operator*=(const TVector3<T> &other) {
        x *= other.x;
        y *= other.y;
        z *= other.z;
        return *this;
    }

    TVector3<T> &operator*=(T t) {
        x *= t;
        y *= t;
        z *= t;
        return *this;
    }

    TVector3<T> &operator/=(const TVector3<T> &other) {
        x /= other.x;
        y /= other.y;
        z /= other.z;
        return *this;
    }

    TVector3<T> &operator/=(T t) {
        x /= t;
        y /= t;
        z /= t;
        return *this;
    }

    TVector3<T> operator+(const TVector3<T> &other) const {
        return TVector3<T>(x + other.x, y + other.y, z + other.z);
    }

    TVector3<T> operator-(const TVector3<T> &other) const {
        return TVector3<T>(x - other.x, y - other.y, z - other.z);
    }

    TVector3<T> operator-() const {
        return TVector3<T>(-x, -y, -z);
    }

    TVector3<T> operator*(const TVector3<T> &other) const {
        return TVector3<T>(x * other.x, y * other.y, z * other.z);
    }

    TVector3<T> operator/(const TVector3<T> &other) const {
        return TVector3<T>(x / other.x, y / other.y, z / other.z);
    }

    TVector3<T> operator*(T t) const {
        return TVector3<T>(x * t, y * t, z * t);
    }

    TVector3<T> operator/(T t) const {
        return TVector3<T>(x / t, y / t, z / t);
    }

    T dot(TVector3<T> other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    T normSquared() const {
        return dot(*this);
    }

    T norm() const {
        return sqrt(normSquared());
    }

    T lengthSquared() const {
        return normSquared();
    }

    T length() const {
        return norm();
    }

    T distanceSquared(const TVector3<T> &other) const {
        return (other - *this).normSquared();
    }

    T distance(const TVector3<T> &other) const {
        return sqrt(distanceSquared(other));
    }

    TVector3<T> cross(const TVector3<T> &v) const {
        return TVector3<T>(
                y * v.z - z * v.y,
                z * v.x - x * v.z,
                x * v.y - y * v.x
        );
    }

    void normalize() {
        *this /= norm();
    }

    TVector3<T> normalized() const {
        return *this / norm();
    }
};

};

#endif //VEGA_STRIKE_ENGINE_AUDIO_VECTOR_H
