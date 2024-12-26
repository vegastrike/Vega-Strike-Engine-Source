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
#include <boost/json.hpp>

#include "resource/json_utils.h"


void UnitJSONFactory::ParseJSON(VSFileSystem::VSFile &file, bool player_ship) {
    const std::string json_text = file.ReadFull();

    boost::json::value json_value = boost::json::parse(json_text);
    boost::json::array root_array = json_value.get_array();

    for(boost::json::value& unit_value : root_array) {
        boost::json::object unit_object = unit_value.get_object();
        std::map<std::string, std::string> unit_attributes;

        for(boost::json::key_value_pair& pair : unit_object) {
            const std::string value = boost::json::value_to<std::string>(pair.value());
            unit_attributes[pair.key()] = value;
        }

        // Add root
        unit_attributes["root"] = file.GetRoot();  

        if(player_ship) {
            UnitCSVFactory::units["player_ship"] = unit_attributes;
        } else {
            UnitCSVFactory::units[unit_attributes["Key"]] = unit_attributes;
        }
    }
}
