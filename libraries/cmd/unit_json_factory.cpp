/*
 * unit_json_factory.cpp
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

#include "cmd/unit_json_factory.h"
#include "cmd/unit_csv_factory.h"

#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <boost/json.hpp>


void UnitJSONFactory::ParseJSON(VSFileSystem::VSFile &file, bool player_ship) {
    const std::string json_text = file.ReadFull();

    boost::json::value json_value;
    try {
        json_value = boost::json::parse(json_text);
    } catch (std::exception const& e) {
        VS_LOG_FLUSH_EXIT(fatal, (boost::format("Error parsing JSON in UnitJSONFactory::ParseJSON(): %1%") % e.what()), 42);
    }
    if (json_value.is_object()) {
        boost::json::object obj = json_value.as_object();
        std::map<std::string, std::string> unit_attributes{};

        for (boost::json::key_value_pair& pair : obj) {
            const std::string value = boost::json::value_to<std::string>(pair.value());
            unit_attributes[pair.key()] = value;
        }

        // Add root
        unit_attributes["root"] = file.GetRoot();

        if (player_ship) {
            UnitCSVFactory::units["player_ship"] = unit_attributes;
        } else {
            UnitCSVFactory::units[unit_attributes["Key"]] = unit_attributes;
        }
    } else if (json_value.is_array()) {
        boost::json::array json_root = json_value.as_array();
        for (boost::json::value & unit_value : json_root) {
            boost::json::object unit_object = unit_value.get_object();
            std::map<std::string, std::string> unit_attributes{};

            for (boost::json::key_value_pair& pair : unit_object) {
                const std::string value = boost::json::value_to<std::string>(pair.value());
                unit_attributes[pair.key()] = value;
            }

            // Add root
            unit_attributes["root"] = file.GetRoot();

            if (player_ship) {
                UnitCSVFactory::units["player_ship"] = unit_attributes;
            } else {
                UnitCSVFactory::units[unit_attributes["Key"]] = unit_attributes;
            }
        }
    } else {
        VS_LOG_FLUSH_EXIT(fatal, (boost::format("File '%1%' had an unexpected JSON structure. We don't know how to process it.") % file.GetFilename()), 42);
    }

}
