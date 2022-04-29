/**
 * game_config.cpp
 *
 * Copyright (C) 2020-2022 Roy Falk, Stephen G. Tuggy, David Wales,
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
string GameConfig::DEFAULT_ERROR_VALUE = "GameConfig::GetVar DEFAULT_ERROR_VALUE";

void GameConfig::LoadGameConfig(const string &filename) {
    ptree tree;
    pt::read_xml(filename, tree);

    string const xpath = "vegaconfig.variables.";
    for (const auto &section_iterator : tree.get_child(xpath)) {
        string section_name = section_iterator.second.get<string>("<xmlattr>.name", "");
        if (section_name == "") {
            continue;
        }

        ptree inner_tree = section_iterator.second;
        for (const auto &variable_iterator : inner_tree) {
            string name = variable_iterator.second.get<string>("<xmlattr>.name", "");
            string value = variable_iterator.second.get<string>("<xmlattr>.value", "");
            if (name == "") {
                continue;
            }

            string const key = section_name + "." + name;
            (*variables())[key] = value;
        }
    }
}

std::shared_ptr<std::map<std::string, std::string>> GameConfig::variables() {
    static const std::shared_ptr<std::map<std::string, std::string>> kVariablesMap = std::make_shared<std::map<std::string, std::string>>();
    return kVariablesMap;
}
