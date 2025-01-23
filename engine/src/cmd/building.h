/*
 * building.h
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
#ifndef VEGA_STRIKE_ENGINE_CMD_BUILDING_H
#define VEGA_STRIKE_ENGINE_CMD_BUILDING_H

#include "unit_generic.h"

class Terrain;
class ContinuousTerrain;
class Flightgroup;

class Building : public Unit {
    // Fields
protected:
    union Buildingparent {
        Terrain *terrain;
        ContinuousTerrain *plane;
    }
            parent;
    bool continuous;
    bool vehicle;

    // Constructors
public:
    Building(ContinuousTerrain *parent,
            bool vehicle,
            const char *filename,
            bool SubUnit,
            int faction,
            const std::string &unitModifications = std::string(""),
            Flightgroup *fg = NULL);

    Building(Terrain *parent,
            bool vehicle,
            const char *filename,
            bool SubUnit,
            int faction,
            const std::string &unitModifications = std::string(""),
            Flightgroup *fg = NULL);

public:

    enum Vega_UnitType isUnit() const {
        return Vega_UnitType::building;
    }

    virtual void UpdatePhysics2(const Transformation &trans,
            const Transformation &old_physical_state,
            const Vector &accel,
            float difficulty,
            const Matrix &transmat,
            const Vector &CumulativeVelocity,
            bool ResolveLast,
            UnitCollection *uc = NULL);

    bool ownz(void *parent) {
        return this->parent.terrain == (Terrain *) parent;
    }

protected:
/// default constructor forbidden
    Building();
/// copy constructor forbidden
    Building(const Building &);
/// assignment operator forbidden
    Building &operator=(const Building &);
};

#endif //VEGA_STRIKE_ENGINE_CMD_BUILDING_H
