/*
 * unit_csv_factory.cpp
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


#include "cmd/unit_csv_factory.h"

#include <iostream>
#include <vector>
#include <string>

// Required definition of static variable
std::map<std::string, std::map<std::string, std::string>> UnitCSVFactory::units;

// This is probably unique enough to ensure no collision
std::string UnitCSVFactory::DEFAULT_ERROR_VALUE = "UnitCSVFactory::_GetVariable DEFAULT_ERROR_VALUE";


// TODO: place this somewhere else with similar code from unit_csv
std::string GetUnitKeyFromNameAndFaction(const std::string unit_name, const std::string unit_faction) {
    std::string hash_name = unit_name + "__" + unit_faction;

    if(UnitCSVFactory::HasUnit(hash_name)) {
        return hash_name;
    }

    if(UnitCSVFactory::HasUnit(unit_name)) {
        return unit_name;
    }

    return std::string();
}

void UnitCSVFactory::LoadUnit(std::string key,
                              std::map<std::string,std::string> unit_map) {
    UnitCSVFactory::units[key] = unit_map;
}
