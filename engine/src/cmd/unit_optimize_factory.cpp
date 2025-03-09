/*
 * unit_optimize_factory.cpp
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

#include "unit_optimize_factory.h"
#include "unit_csv_factory.h"

#include "json.h"

void UnitOptimizeFactory::RecursiveParse(std::map<std::string, std::string> unit_attributes,
                   const std::string& json_text, bool is_root) {
    json::jobject json = json::jobject::parse(json_text);

    // Parse the data section
    std::string data = json.get("data");
    json::jobject data_json = json::jobject::parse(data);

    for (const std::string &key : keys) {
        // For some reason, parser adds quotes
        if(data_json.has_key(key)) {
            const std::string attribute = data_json.get(key);
            const std::string stripped_attribute = attribute.substr(1, attribute.size() - 2);
            unit_attributes[key] = stripped_attribute;
        } else {
            // If we do this for non-root, we'll overwrite existing attributes
            if(is_root) {
                unit_attributes[key] = "";
            }
        }
    }

    // Parse the units array
    if(json.has_key("units")) {
        std::vector<std::string> units = json::parsing::parse_array(json.get("units").c_str());
        // Iterate over root
        for (const std::string &unit_text : units) {
            RecursiveParse(unit_attributes, unit_text, false);
        }
    } else {
        // Add moment of intertia
        if(unit_attributes.count("Mass")) {
            unit_attributes["Moment_Of_Inertia"] = unit_attributes["Mass"];
        }

        std::string unit_key = unit_attributes["Key"];

        UnitCSVFactory::units[unit_key] = unit_attributes;
    }
}


void UnitOptimizeFactory::ParseJSON(VSFileSystem::VSFile &file) {
    const std::string json_text = file.ReadFull();



    std::map<std::string, std::string> unit_attributes;

    // Add root
    unit_attributes["root"] = file.GetRoot();

    RecursiveParse(unit_attributes, json_text, true);
}
