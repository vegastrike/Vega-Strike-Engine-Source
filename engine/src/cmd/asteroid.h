/*
 * asteroid.h
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
#ifndef VEGA_STRIKE_ENGINE_CMD_ASTEROID_H
#define VEGA_STRIKE_ENGINE_CMD_ASTEROID_H

#include "gfx/quaternion.h"
#include "gfx/vec.h"
#include "gfx/matrix.h"
#include "cmd/script/flightgroup.h"
#include "cmd/collection.h"
#include "unit_generic.h"

class Asteroid : public Unit {
private:
    unsigned int asteroid_physics_offset;

public:
    Asteroid(const char *filename, int faction, Flightgroup *fg = NULL, int fg_snumber = 0, float difficulty = .01);

    enum Vega_UnitType isUnit() const {
        return Vega_UnitType::asteroid;
    }

private:
/// default constructor forbidden
    Asteroid();

    Asteroid(std::vector<Mesh *> m, bool b, int i) : Unit(m, b, i) {
    }

/// copy constructor forbidden
    Asteroid(const Asteroid &);

/// assignment operator forbidden
    Asteroid &operator=(const Asteroid &);
};

#endif //VEGA_STRIKE_ENGINE_CMD_ASTEROID_H
