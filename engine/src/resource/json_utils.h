/*
 * json_utils.h
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

#ifndef VEGA_STRIKE_ENGINE_RESOURCE_JSON_UTILS_H
#define VEGA_STRIKE_ENGINE_RESOURCE_JSON_UTILS_H

#include <string>
#include <boost/json.hpp>

// Use when default value already defined
template<typename T>
void ConditionalJsonGet(boost::json::object& object, 
                          T& value, 
                          const std::string& key) {
    if (object.if_contains(key)) {
        value = boost::json::value_to<T>(object.at(key));
    }
}

// Use when default value is not defined
template<typename T>
const T JsonGetWithDefault(boost::json::object object, 
                              const std::string& key, 
                              const T default_value) {
    if (object.if_contains(key)) {
        return boost::json::value_to<T>(object.at(key));
    }

    return default_value;
}



// This function supports multi-level key
const std::string JsonGetStringWithDefault(boost::json::object object, 
                                           const std::string& key, 
                                           const char* default_value);

// This function supports multi-level key
const boost::json::value JsonGetValue(boost::json::object object, 
                                      const std::string& key);

bool GetBool(boost::json::object object, const std::string key, bool default_value);
double GetDouble(boost::json::object object, const std::string key, double default_value);

#endif //VEGA_STRIKE_ENGINE_RESOURCE_JSON_UTILS_H