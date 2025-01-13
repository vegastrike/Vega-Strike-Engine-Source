/*
 * IcePlane.h
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
 *	Contains code for planes.
 *	\file		IcePlane.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 *
 *  Updated by Stephen G. Tuggy 2022-01-06
 *  Updated by Benjamen R. Meyer 2023-05-27
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_PLANE_H
#define VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_PLANE_H

#define PLANE_EPSILON        (1.0e-7f)

class ICEMATHS_API Plane {
public:
    //! Constructor
    inline_ Plane() {
    }
    //! Constructor from a normal and a distance
    inline_ Plane(float nx, float ny, float nz, float d) {
        Set(nx, ny, nz, d);
    }
    //! Constructor from a point on the plane and a normal
    inline_ Plane(const Point &p, const Point &n) {
        Set(p, n);
    }
    //! Constructor from three points
    inline_ Plane(const Point &p0, const Point &p1, const Point &p2) {
        Set(p0, p1, p2);
    }
    //! Constructor from a normal and a distance
    inline_ Plane(const Point &_n, float _d) {
        n = _n;
        d = _d;
    }
    //! Copy constructor
    inline_ Plane(const Plane &plane) : n(plane.n), d(plane.d) {
    }
    //! Destructor
    inline_            ~Plane() {
    }

    inline_    Plane &Zero() {
        n.Zero();
        d = 0.0f;
        return *this;
    }

    inline_    Plane &Set(float nx, float ny, float nz, float _d) {
        n.Set(nx, ny, nz);
        d = _d;
        return *this;
    }

    inline_    Plane &Set(const Point &p, const Point &_n) {
        n = _n;
        d = -p | _n;
        return *this;
    }

    Plane &Set(const Point &p0, const Point &p1, const Point &p2);

    inline_    float Distance(const Point &p) const {
        return (p | n) + d;
    }

    inline_    bool Belongs(const Point &p) const {
        return fabsf(Distance(p)) < PLANE_EPSILON;
    }

    inline_    void Normalize() {
        float Denom = 1.0f / n.Magnitude();
        n.x *= Denom;
        n.y *= Denom;
        n.z *= Denom;
        d *= Denom;
    }

public:
    // Members
    Point n;        //!< The normal to the plane
    float d;        //!< The distance from the origin

    // Cast operators
    inline_            operator Point() const {
        return n;
    }

    inline_            operator HPoint() const {
        return HPoint(n, d);
    }

    // Arithmetic operators
    inline_    Plane operator*(const Matrix4x4 &m) const {
        // Old code from Irion. Kept for reference.
        Plane Ret(*this);
        return Ret *= m;
    }

    inline_    Plane &operator*=(const Matrix4x4 &m) {
        // Old code from Irion. Kept for reference.
        Point n2 = HPoint(n, 0.0f) * m;
        d = -((Point) (HPoint(-d * n, 1.0f) * m) | n2);
        n = n2;
        return *this;
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Transforms a plane by a 4x4 matrix. Same as Plane * Matrix4x4 operator, but faster.
 *	\param		transformed	[out] transformed plane
 *	\param		plane		[in] source plane
 *	\param		transform	[in] transform matrix
 *	\warning	the plane normal must be unit-length
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline_    void TransformPlane(Plane &transformed, const Plane &plane, const Matrix4x4 &transform) {
    // Rotate the normal using the rotation part of the 4x4 matrix
    transformed.n = plane.n * Matrix3x3(transform);

    // Compute new d
    transformed.d = plane.d - (Point(transform.GetTrans()) | transformed.n);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Transforms a plane by a 4x4 matrix. Same as Plane * Matrix4x4 operator, but faster.
 *	\param		plane		[in/out] source plane (transformed on return)
 *	\param		transform	[in] transform matrix
 *	\warning	the plane normal must be unit-length
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline_    void TransformPlane(Plane &plane, const Matrix4x4 &transform) {
    // Rotate the normal using the rotation part of the 4x4 matrix
    plane.n *= Matrix3x3(transform);

    // Compute new d
    plane.d -= Point(transform.GetTrans()) | plane.n;
}

#endif //VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_PLANE_H
