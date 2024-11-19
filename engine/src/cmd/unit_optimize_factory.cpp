/*
 * unit_csv_factory.cpp
 *
 * Copyright (C) 2020-2022 Daniel Horn, Roy Falk, Stephen G. Tuggy, and
 * other Vega Strike contributors
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
                   const std::string& json_text) {
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
        } 
    }

    if(unit_attributes.count("Key")) {
        std::string unit_key = unit_attributes["Key"];
        UnitCSVFactory::units[unit_key] = unit_attributes;
    }
    

    // Parse the units array
    if(json.has_key("units")) {
        std::vector<std::string> units = json::parsing::parse_array(json.get("units").c_str());
        // Iterate over root
        for (const std::string &unit_text : units) {
            RecursiveParse(unit_attributes, unit_text);
        }
    } 
}


void UnitOptimizeFactory::ParseJSON(VSFileSystem::VSFile &file) {
    const std::string json_text = file.ReadFull();



    std::map<std::string, std::string> unit_attributes;

    // Add root
    unit_attributes["root"] = file.GetRoot();

    RecursiveParse(unit_attributes, json_text);
}
