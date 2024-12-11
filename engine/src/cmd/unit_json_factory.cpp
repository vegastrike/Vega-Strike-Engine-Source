/*
 * unit_csv_factory.cpp
 *
 * Copyright (C) 2021 Roy Falk
 * Copyright (C) 2022 Stephen G. Tuggy
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

#include "unit_json_factory.h"
#include "unit_csv_factory.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>

#include "json.h"


void UnitJSONFactory::ParseJSON(VSFileSystem::VSFile &file, bool player_ship) {
    const std::string json_text = file.ReadFull();

    std::vector<std::string> units = json::parsing::parse_array(json_text.c_str());
    // Iterate over root
    for (const std::string &unit_text : units) {
        json::jobject unit = json::jobject::parse(unit_text);
        std::map<std::string, std::string> unit_attributes;

        for (const std::string &key : keys) {
            // For some reason, parser adds quotes
            if(unit.has_key(key)) {
                const std::string attribute = unit.get(key);
                const std::string stripped_attribute = attribute.substr(1, attribute.size() - 2);
                unit_attributes[key] = stripped_attribute;
            } else {
                unit_attributes[key] = "";
            }
        }

        // Add root
        unit_attributes["root"] = file.GetRoot();

        

        if(player_ship) {
            UnitCSVFactory::units["player_ship"] = unit_attributes;
        } else {
            std::string unit_key = unit.get("Key");
            std::string stripped_unit_key = unit_key.substr(1, unit_key.size() - 2);
            UnitCSVFactory::units[stripped_unit_key] = unit_attributes;
        }
    }
}
