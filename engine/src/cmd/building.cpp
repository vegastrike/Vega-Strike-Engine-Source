/**
 * building.cpp
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
 * Copyright (C) 2022 Stephen G. Tuggy
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */

#include "cmd/building.h"
#include "cmd/cont_terrain.h"
#include "unit_generic.h"

Building::Building(ContinuousTerrain *parent,
        bool vehicle,
        const char *filename,
        bool SubUnit,
        int faction,
        const string &modifications,
        Flightgroup *fg) : Unit(filename, SubUnit, faction, modifications, fg) {
    this->vehicle = vehicle;
    continuous = true;
    this->parent.plane = parent;
}

Building::Building(Terrain *parent,
        bool vehicle,
        const char *filename,
        bool SubUnit,
        int faction,
        const string &modifications,
        Flightgroup *fg) : Unit(filename, SubUnit, faction, modifications, fg) {
    this->vehicle = vehicle;
    continuous = false;
    this->parent.terrain = parent;
}

void Building::UpdatePhysics2(const Transformation &trans,
        const Transformation &old_physical_state,
        const Vector &accel,
        float difficulty,
        const Matrix &transmat,
        const Vector &CumulativeVelocity,
        bool ResolveLast,
        UnitCollection *uc) {
    Unit::UpdatePhysics2(trans,
            old_physical_state,
            accel,
            difficulty,
            transmat,
            CumulativeVelocity,
            ResolveLast,
            uc);
    QVector tmp(LocalPosition());
    Vector p, q, r;
    GetOrientation(p, q, r);
    if (continuous) {
        tmp = parent.plane->GetGroundPos(tmp, p);
    } else {
        parent.terrain->GetGroundPos(tmp, p, (float) 0, (float) 0);
    }
    if (vehicle) {
        Normalize(p);
        Vector tmp1;
#if 0
        if (k <= 0) {
            tmp1 = Vector( 0, 0, 1 );
            if ( k = tmp1.Magnitude() )
                tmp1 *= 800./k;
        } else
#endif
        {
            tmp1 = 200 * q.Cross(p);
        }
        NetLocalTorque += ((tmp1 - tmp1 * (tmp1.Dot(GetAngularVelocity()) / tmp1.Dot(tmp1)))) * 1. / Mass;
    }
    SetCurPosition(tmp);
}

