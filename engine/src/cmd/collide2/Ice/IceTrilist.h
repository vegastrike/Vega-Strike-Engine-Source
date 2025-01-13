/*
 * IceTrilist.h
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
 *	Contains code for a triangle container.
 *	\file		IceTrilist.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
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
#ifndef VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_TRILIST_H
#define VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_TRILIST_H

class ICEMATHS_API TriList : public Container {
public:
    // Constructor / Destructor
    TriList() {
    }

    ~TriList() {
    }

    inline_    uint32_t GetNbTriangles() const {
        return GetNbEntries() / 9;
    }

    inline_    Triangle *GetTriangles() const {
        return (Triangle *) GetEntries();
    }

    void AddTri(const Triangle &tri) {
        Add(tri.mVerts[0].x).Add(tri.mVerts[0].y).Add(tri.mVerts[0].z);
        Add(tri.mVerts[1].x).Add(tri.mVerts[1].y).Add(tri.mVerts[1].z);
        Add(tri.mVerts[2].x).Add(tri.mVerts[2].y).Add(tri.mVerts[2].z);
    }

    void AddTri(const Point &p0, const Point &p1, const Point &p2) {
        Add(p0.x).Add(p0.y).Add(p0.z);
        Add(p1.x).Add(p1.y).Add(p1.z);
        Add(p2.x).Add(p2.y).Add(p2.z);
    }
};

class ICEMATHS_API TriangleList : public Container {
public:
    // Constructor / Destructor
    TriangleList() {
    }

    ~TriangleList() {
    }

    inline_    uint32_t GetNbTriangles() const {
        return GetNbEntries() / 3;
    }

    inline_    IndexedTriangle *GetTriangles() const {
        return (IndexedTriangle *) GetEntries();
    }

    void AddTriangle(const IndexedTriangle &tri) {
        Add(tri.mVRef[0]).Add(tri.mVRef[1]).Add(tri.mVRef[2]);
    }

    void AddTriangle(uint32_t vref0, uint32_t vref1, uint32_t vref2) {
        Add(vref0).Add(vref1).Add(vref2);
    }
};

#endif //VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_TRILIST_H
