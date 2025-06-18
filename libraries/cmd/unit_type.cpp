/*
 * unit_type.cpp
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

#include "unit_type.h"

#include <iostream>

int SerializeUnitType(Vega_UnitType type) {
    switch(type) {
        case Vega_UnitType::asteroid: return 0;
        case Vega_UnitType::building: return 1;
        case Vega_UnitType::enhancement: return 2;
        case Vega_UnitType::missile: return 3;
        case Vega_UnitType::nebula: return 4;
        case Vega_UnitType::planet: return 5;
        case Vega_UnitType::unit: return 6;
    }
}

Vega_UnitType DeserializeUnitType(int type) {
    switch(type) {
        case 0: return Vega_UnitType::asteroid;
        case 1: return Vega_UnitType::building;
        case 2: return Vega_UnitType::enhancement;
        case 3: return Vega_UnitType::missile;
        case 4: return Vega_UnitType::nebula;
        case 5: return Vega_UnitType::planet;
        default:
            std::cerr << "DeserializeUnitType: Illegal type " << type << std::endl;
            std::cerr << "Defaulting to unit" << std::endl;
            return Vega_UnitType::unit;
    }
}

Vega_UnitType DeserializeUnitType(std::string type) {
    if(type == "asteroid") {
        return Vega_UnitType::asteroid;
    } else if(type == "planet") {
        return Vega_UnitType::planet;
    } else if(type == "nebula") {
        return Vega_UnitType::nebula;
    } else {
        return Vega_UnitType::unit;
    }
}