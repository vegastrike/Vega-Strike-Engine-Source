/*
 * IceTriangle.h
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
 *	Contains a handy triangle class.
 *	\file		IceTriangle.h
 *	\author		Pierre Terdiman
 *	\date		January, 17, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Updated by Stephen G. Tuggy 2021-07-03
 * Updated by Stephen G. Tuggy 2022-01-06
 * Updated by Benjamen R. Meyer 2023-05-27
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_TRIANGLE_H
#define VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_TRIANGLE_H

// Forward declarations
class Moment;

// Partitioning values
enum PartVal {
    TRI_MINUS_SPACE = 0,            //!< Triangle is in the negative space
    TRI_PLUS_SPACE = 1,            //!< Triangle is in the positive space
    TRI_INTERSECT = 2,            //!< Triangle intersects plane
    TRI_ON_PLANE = 3,            //!< Triangle and plane are coplanar

    TRI_FORCEDWORD = 0x7fffffff
};

// A triangle class.
class ICEMATHS_API Triangle {
public:
    //! Constructor
    inline_ Triangle() {
    }
    //! Constructor
    inline_ Triangle(const Point &p0, const Point &p1, const Point &p2) {
        mVerts[0] = p0;
        mVerts[1] = p1;
        mVerts[2] = p2;
    }
    //! Copy constructor
    inline_ Triangle(const Triangle &triangle) {
        mVerts[0] = triangle.mVerts[0];
        mVerts[1] = triangle.mVerts[1];
        mVerts[2] = triangle.mVerts[2];
    }
    //! Destructor
    inline_                    ~Triangle() {
    }

    //! Vertices
    Point mVerts[3];

    // Methods
    void Flip();
    float Area() const;
    float Perimeter() const;
    float Compacity() const;
    void Normal(Point &normal) const;
    void DenormalizedNormal(Point &normal) const;
    void Center(Point &center) const;

    inline_    Plane PlaneEquation() const {
        return Plane(mVerts[0], mVerts[1], mVerts[2]);
    }

    PartVal TestAgainstPlane(const Plane &plane, float epsilon) const;
//				float			Distance(Point& cp, Point& cq, Tri& tri);
    void ComputeMoment(Moment &m);
    float MinEdgeLength() const;
    float MaxEdgeLength() const;
    void ComputePoint(float u, float v, Point &pt, uint32_t *nearvtx = nullptr) const;
    void Inflate(float fat_coeff, bool constant_border);
};

#endif //VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_TRIANGLE_H
