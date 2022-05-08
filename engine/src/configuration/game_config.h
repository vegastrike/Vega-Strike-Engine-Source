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

#include "vs_logging.h"

namespace pt = boost::property_tree;

using pt::ptree;

// TODO: test this functionality, especially the subsection...

class GameConfig {
private:
    static std::string DEFAULT_ERROR_VALUE;

    static inline std::string GetVar(std::string const &section, std::string const &name) {
        std::string const key = section + "." + name;
        if (variables()->count(key) > 0) {
            return variables()->at(key);
        }
        VS_LOG(warning, (boost::format("GameConfig::GetVar: Key '%1%' not found in section '%2%'") % name % section));
        return DEFAULT_ERROR_VALUE;
    }

    static inline std::string GetVar(std::string const &section,
            std::string const &sub_section,
            std::string const &name) {
        std::string const key = section + "." + sub_section + "." + name;
        if (variables()->count(key) > 0) {
            return variables()->at(key);
        }
        VS_LOG(warning, (boost::format("GameConfig::GetVar: Key '%1%' not found in section '%2%', subsection '%3%'") % name % section % sub_section));
        return DEFAULT_ERROR_VALUE;
    }

    static std::string EscapedString(const std::string& input);

    static std::shared_ptr<std::map<std::string, std::string>> variables();

public:
    static void LoadGameConfig(const std::string &filename);
    template<class T>
    static inline T GetVariable(std::string const &section, std::string const &name, T default_value) = delete;

    template<class T>
    static inline T GetVariable(std::string const &section, std::string const &sub_section,
            std::string const &name, T default_value) = delete;

    static std::string GetEscapedString(std::string const &section, std::string const& name, std::string const& default_value);
    static std::string GetEscapedString(std::string const &section, std::string const& sub_section, std::string const& name, std::string const& default_value);

    static inline std::string GetString(std::string const &section, std::string const& name, std::string const& default_value) {
        std::string result = GetVar(section, name);
        if (result == DEFAULT_ERROR_VALUE) {
            return default_value;
        }
        return result;
    }

    static inline std::string GetString(std::string const &section, std::string const& sub_section, std::string const& name, std::string const& default_value) {
        std::string result = GetVar(section, sub_section, name);
        if (result == DEFAULT_ERROR_VALUE) {
            return default_value;
        }
        return result;
    }

};

// Template Specialization
template<>
inline bool GameConfig::GetVariable(std::string const &section, std::string const &name, bool default_value) {
    std::string result = GetVar(section, name);
    if (result == DEFAULT_ERROR_VALUE) {
        return default_value;
    }
    boost::algorithm::to_lower(result);
    return result == "true";
}

template<>
inline float GameConfig::GetVariable(std::string const &section, std::string const &name, float default_value) {
    std::string result = GetVar(section, name);
    if (result == DEFAULT_ERROR_VALUE) {
        return default_value;
    }
    try {
        return std::stof(result);
    } catch (std::invalid_argument& e) {
        VS_LOG(error, (boost::format("GameConfig::GetVariable: stof threw invalid_argument: %1%") % e.what()));
        return default_value;
    } catch (std::out_of_range& e) {
        VS_LOG(error, (boost::format("GameConfig::GetVariable: stof threw out_of_range: %1%") % e.what()));
        return default_value;
    }
}

template<>
inline double GameConfig::GetVariable(std::string const &section, std::string const &name, double default_value) {
    std::string result = GetVar(section, name);
    if (result == DEFAULT_ERROR_VALUE) {
        return default_value;
    }
    try {
        return std::stod(result);
    } catch (std::invalid_argument& e) {
        VS_LOG(error, (boost::format("GameConfig::GetVariable: stod threw invalid_argument: %1%") % e.what()));
        return default_value;
    } catch (std::out_of_range& e) {
        VS_LOG(error, (boost::format("GameConfig::GetVariable: stod threw out_of_range: %1%") % e.what()));
        return default_value;
    }
}

template<>
inline int GameConfig::GetVariable(std::string const &section, std::string const &name, int default_value) {
    std::string result = GetVar(section, name);
    if (result == DEFAULT_ERROR_VALUE) {
        return default_value;
    }
    try {
        return std::stoi(result);
    } catch (std::invalid_argument& e) {
        VS_LOG(error, (boost::format("GameConfig::GetVariable: stoi threw invalid_argument: %1%") % e.what()));
        return default_value;
    } catch (std::out_of_range& e) {
        VS_LOG(error, (boost::format("GameConfig::GetVariable: stoi threw out_of_range: %1%") % e.what()));
        return default_value;
    }
}

// With Subsection
template<>
inline bool GameConfig::GetVariable(std::string const &section, std::string const &sub_section,
        std::string const &name, bool default_value) {
    std::string result = GetVar(section, sub_section, name);
    if (result == DEFAULT_ERROR_VALUE) {
        return default_value;
    }
    boost::algorithm::to_lower(result);
    return result == "true";
}

template<>
inline float GameConfig::GetVariable(std::string const &section, std::string const &sub_section,
        std::string const &name, float default_value) {
    std::string result = GetVar(section, sub_section, name);
    if (result == DEFAULT_ERROR_VALUE) {
        return default_value;
    }
    try {
        return std::stof(result);
    } catch (std::invalid_argument& e) {
        VS_LOG(error, (boost::format("GameConfig::GetVariable: stof threw invalid_argument: %1%") % e.what()));
        return default_value;
    } catch (std::out_of_range& e) {
        VS_LOG(error, (boost::format("GameConfig::GetVariable: stof threw out_of_range: %1%") % e.what()));
        return default_value;
    }
}

template<>
inline double GameConfig::GetVariable(std::string const &section, std::string const &sub_section,
        std::string const &name, double default_value) {
    std::string result = GetVar(section, sub_section, name);
    if (result == DEFAULT_ERROR_VALUE) {
        return default_value;
    }
    try {
        return std::stod(result);
    } catch (std::invalid_argument& e) {
        VS_LOG(error, (boost::format("GameConfig::GetVariable: stod threw invalid_argument: %1%") % e.what()));
        return default_value;
    } catch (std::out_of_range& e) {
        VS_LOG(error, (boost::format("GameConfig::GetVariable: stod threw out_of_range: %1%") % e.what()));
        return default_value;
    }
}

template<>
inline int GameConfig::GetVariable(std::string const &section, std::string const &sub_section,
        std::string const &name, int default_value) {
    std::string result = GetVar(section, sub_section, name);
    if (result == DEFAULT_ERROR_VALUE) {
        return default_value;
    }
    try {
        return std::stoi(result);
    } catch (std::invalid_argument& e) {
        VS_LOG(error, (boost::format("GameConfig::GetVariable: stoi threw invalid_argument: %1%") % e.what()));
        return default_value;
    } catch (std::out_of_range& e) {
        VS_LOG(error, (boost::format("GameConfig::GetVariable: stoi threw out_of_range: %1%") % e.what()));
        return default_value;
    }
}

#endif // GAME_CONFIG_H
