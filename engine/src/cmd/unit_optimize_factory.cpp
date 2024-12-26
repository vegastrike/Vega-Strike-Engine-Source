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


void UnitOptimizeFactory::RecursiveParse(std::map<std::string, std::string> unit_attributes,
                                         const boost::json::object object) {
    // Parse the data section
    if(object.if_contains("data")) {
        const boost::json::object data_object = object.at("data").as_object();

        for(const boost::json::key_value_pair& pair : data_object) {
            const std::string value = boost::json::value_to<std::string>(pair.value());
            unit_attributes[pair.key()] = value;
        }

        if(unit_attributes.count("Key")) {
            std::string unit_key = unit_attributes["Key"];
            UnitCSVFactory::units[unit_key] = unit_attributes;
        }
    }


    if(object.if_contains("units")) {
        const boost::json::array units_array = object.at("units").as_array();

        for(const boost::json::value& value : units_array) {
            RecursiveParse(unit_attributes, value.as_object());
        } 
    }
}


void UnitOptimizeFactory::ParseJSON(VSFileSystem::VSFile &file) {
    const std::string json_text = file.ReadFull();

    boost::json::value json_value = boost::json::parse(json_text);
    boost::json::object root_object = json_value.as_object();


    std::map<std::string, std::string> unit_attributes;

    // Add root
    unit_attributes["root"] = file.GetRoot();

    RecursiveParse(unit_attributes, root_object);
}
