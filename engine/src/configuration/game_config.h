/*
 * game_config.h
 *
 * Copyright (C) 2020-2022 Daniel Horn, Roy Falk, Stephen G. Tuggy,
 * David Wales, and other Vega Strike contributors
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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


#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

#include <string>
#include <set>
#include <map>
#include <exception>
#include <iostream>
#include <cstdint>
#include <type_traits>

#include "vs_logging.h"

namespace pt = boost::property_tree;

class GameConfig {
private:
    // This is probably unique enough to ensure no collision
//    constexpr static const char DEFAULT_ERROR_VALUE[] {"GameConfig::GetVar DEFAULT_ERROR_VALUE"};

    template<class T>
    static inline T GetVar(std::string const & path, T default_value) {
        return variables_()->get(path, default_value);
    }
    static std::string EscapedString(std::string const & input);
    static boost::shared_ptr<pt::iptree> variables_();

public:
    static void LoadGameConfig(const std::string &filename);

    template<class T>
    static inline T GetVariable(std::string const & path, T default_value) {
        return GetVar(path, default_value);
    }

    static inline std::string GetEscapedString(std::string const & path, std::string const & default_value) {
        return EscapedString(GetVar(path, default_value));
    }

    static inline std::string GetString(std::string const & path, std::string const & default_value) {
        return GetVar(path, default_value);
    }
};

#endif // GAME_CONFIG_H
