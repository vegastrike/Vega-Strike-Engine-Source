/*
 * enhancement.h
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
#ifndef VEGA_STRIKE_ENGINE_CMD_ENHANCER_H
#define VEGA_STRIKE_ENGINE_CMD_ENHANCER_H

#include "vegadisk/savegame.h"
#include "src/universe.h"
#include "cmd/unit_generic.h"

class Enhancement : public Unit {
public:
    Enhancement(const char *filename,
            int faction,
            const string &modifications,
            Flightgroup *flightgrp = NULL,
            int fg_subnumber = 0) :
            Unit(filename, false, faction, modifications, flightgrp, fg_subnumber) {
        string file(filename);
        this->filename = filename;
    }

protected:
    std::string filename;

public:
    enum Vega_UnitType isUnit() const override {
        return Vega_UnitType::enhancement;
    }

    Vega_UnitType getUnitType() const override {
        return Vega_UnitType::enhancement;
    }

    /// default constructor forbidden
    Enhancement() = delete;
    /// copy constructor forbidden
    Enhancement(const Enhancement &) = delete;
    /// assignment operator forbidden
    Enhancement &operator=(const Enhancement &) = delete;

    ~Enhancement() override = default;
};

#endif //VEGA_STRIKE_ENGINE_CMD_ENHANCER_H
