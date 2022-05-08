/*
 * game_config.cpp
 *
 * Copyright (C) 2020-2022 Daniel Horn, Roy Falk, Stephen G. Tuggy, David Wales,
 * and other Vega Strike Contributors
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


#include "configuration/game_config.h"

// This is probably unique enough to ensure no collision
std::string GameConfig::DEFAULT_ERROR_VALUE = "GameConfig::GetVar DEFAULT_ERROR_VALUE";

void GameConfig::LoadGameConfig(const std::string &filename) {
    ptree tree;
    pt::read_xml(filename, tree);

    std::string const xpath = "vegaconfig.variables.";
    for (const auto &section_iterator : tree.get_child(xpath)) {
        std::string section_name = section_iterator.second.get<std::string>("<xmlattr>.name", "");
        if (section_name == "") {
            continue;
        }

        ptree inner_tree = section_iterator.second;
        for (const auto &variable_iterator : inner_tree) {
            std::string name = variable_iterator.second.get<std::string>("<xmlattr>.name", "");
            std::string value = variable_iterator.second.get<std::string>("<xmlattr>.value", "");
            if (name == "") {
                continue;
            }

            std::string const key = section_name + "." + name;
            (*variables())[key] = value;
        }
    }
}

std::string GameConfig::GetEscapedString(const std::string &section,
        const std::string &name,
        const std::string &default_value) {
    std::string temp = GetVar(section, name);
    if (temp == DEFAULT_ERROR_VALUE) {
        return default_value;
    }
    return EscapedString(temp);
}

std::string GameConfig::GetEscapedString(const std::string &section,
        const std::string &sub_section,
        const std::string &name,
        const std::string &default_value) {
    std::string temp = GetVar(section, sub_section, name);
    if (temp == DEFAULT_ERROR_VALUE) {
        return default_value;
    }
    return EscapedString(temp);
}

std::string GameConfig::EscapedString(const std::string &input) {
    std::string rv;
    std::string::size_type rp = 0;
    std::string::size_type ip = 0;
    std::string::size_type n = input.length();
    for (; rp < n; ++rp) {
        switch (input.at(rp)) {
            case '\\':
                if ((rp > 0) && (input.at(rp - 1) == '\\')) {
                    rv[ip++] = '\\';
                }
                break;
            case 'n':
                if ((rp > 0) && (input.at(rp - 1) == '\\')) {
                    rv[ip++] = '\n';
                } else {
                    rv[ip++] = 'n';
                }
                break;
            case 'r':
                if ((rp > 0) && (input.at(rp - 1) == '\\')) {
                    rv[ip++] = '\r';
                } else {
                    rv[ip++] = 'r';
                }
                break;
            case 't':
                if ((rp > 0) && (input.at(rp - 1) == '\\')) {
                    rv[ip++] = '\t';
                } else {
                    rv[ip++] = 't';
                }
                break;
            default:
                rv[ip++] = input.at(rp);
                break;
        }
    }
    return rv;
}

std::shared_ptr<std::map<std::string, std::string>> GameConfig::variables() {
    static const std::shared_ptr<std::map<std::string, std::string>> VARIABLES_MAP = std::make_shared<std::map<std::string, std::string>>();
    return VARIABLES_MAP;
}
