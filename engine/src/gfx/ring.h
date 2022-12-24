/*
 * ring.h
 *
 * Copyright (c) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike Contributors
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


#ifndef _RING_H_
#define _RING_H_
#include "mesh.h"
#include "vegastrike.h"

class RingMesh : public Mesh {
    virtual Mesh *AllocNewMeshesEachInSizeofMeshSpace(int num) {
        static_assert(sizeof(Mesh) == sizeof(*this), "Are RingMesh and Mesh the same size in memory?");
        return new RingMesh[num];
    }

    virtual void InitRing(float iradius,
            float oradius,
            int slices,
            const char *texture,
            const QVector &r,
            const QVector &s,
            int texwrapx = 1,
            int texwrapy = 1,
            const BLENDFUNC a = ONE,
            const BLENDFUNC b = ZERO,
            bool envMap = false,
            float theta_min = 0.0,
            float theta_max = 2 * M_PI,
            FILTER mipmap = MIPMAP);

public:
    RingMesh() : Mesh() {
    }

    RingMesh(float iradius,
            float oradius,
            int slices,
            const char *texture,
            const QVector &r,
            const QVector &s,
            int texwrapx = 1,
            int texwrapy = 1,
            const BLENDFUNC a = SRCALPHA,
            const BLENDFUNC b = INVSRCALPHA,
            bool envMap = false,
            float theta_min = 0.0,
            float theta_max = 2 * M_PI,
            FILTER mipmap = TRILINEAR) {
        InitRing(iradius,
                oradius,
                slices,
                texture,
                r,
                s,
                texwrapx,
                texwrapy,
                a,
                b,
                envMap,
                theta_min,
                theta_max,
                mipmap);
    }

    virtual float clipRadialSize() const;
};
#endif

