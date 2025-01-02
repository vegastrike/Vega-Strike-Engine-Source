/*
 * json_utils.cpp
 *
 * Copyright (C) 2001-2024 Daniel Horn, Benjamen Meyer, Roy Falk, Stephen G. Tuggy,
 * and other Vega Strike contributors.
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
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */

#include "json_utils.h"

#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/json.hpp>





const std::string JsonGetStringWithDefault(boost::json::object object, 
                                           const std::string& key, 
                                           const char* default_value) {
    boost::json::value value = JsonGetValue(object, key);
    if(value != nullptr) {
        return boost::json::value_to<std::string>(value);
    } else {
        return default_value;
    }
}

bool GetBool(boost::json::object object, const std::string key, bool default_value) {
    boost::json::value value = JsonGetValue(object, key);
    if(value != nullptr) {
        return boost::json::value_to<bool>(value);
    } else {
        return default_value;
    }
}

double GetDouble(boost::json::object object, const std::string key, double default_value) {
    boost::json::value value = JsonGetValue(object, key);
    if(value != nullptr) {
        return boost::json::value_to<double>(value);
    } else {
        return default_value;
    }
}

const boost::json::value JsonGetValue(boost::json::object object, 
                                      const std::string& key) {
    // key can be a set of keys delimited by `|` so traverse
    // the JSON structure through the list of keys
    // boost::json support using `/` as a delimiter though
    std::vector<std::string> key_sections;
    boost::split(key_sections, key, boost::is_any_of("|"));


    for(const std::string& key_section : key_sections) {
        if (object.if_contains(key_section)) {
            boost::json::value json_value = object.at(key_section);

            if(key_sections.back() == key_section) {
                return json_value;
            } else {
                object = json_value.get_object();
            }
        }
    }

    return nullptr;
}

