/*
 * afterburner.h
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2023 Stephen G. Tuggy, Benjamen R. Meyer, Roy Falk and other Vega Strike Contributors
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

// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#ifndef AFTERBURNER_H
#define AFTERBURNER_H

#include <string>
#include <map>

#include "energy_container.h"
#include "energy_types.h"


class Afterburner : public EnergyConsumer
{
    double usage_factor;
public:
    Afterburner(EnergyType type, double usage_factor);
    Afterburner(EnergyType type, double usage_factor, double afterburner_level, 
                         double mass, double simulation_atom_var);
    //Afterburner(std::string unit_key);

    //void WriteUnitString(std::map<std::string, std::string> &unit);
    double UsageFactor() { return usage_factor; }
};

#endif // AFTERBURNER_H