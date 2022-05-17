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

std::string GameConfig::EscapedString(const std::string &input) {
    std::string rv;
    std::string::size_type rp = 0;
    std::string::size_type n = input.length();
    for (; rp < n; ++rp) {
        switch (input.at(rp)) {
            case '\\':
                if ((rp > 0) && (input.at(rp - 1) == '\\')) {
                    rv += '\\';
                }
                break;
            case 'n':
                if ((rp > 0) && (input.at(rp - 1) == '\\')) {
                    rv += '\n';
                } else {
                    rv += 'n';
                }
                break;
            case 'r':
                if ((rp > 0) && (input.at(rp - 1) == '\\')) {
                    rv += '\r';
                } else {
                    rv += 'r';
                }
                break;
            case 't':
                if ((rp > 0) && (input.at(rp - 1) == '\\')) {
                    rv += '\t';
                } else {
                    rv += 't';
                }
                break;
            default:
                rv += input.at(rp);
                break;
        }
    }
    return rv;
}

boost::shared_ptr<pt::ptree> GameConfig::variables_() {
    static boost::shared_ptr<pt::ptree> VARIABLES_MAP = boost::make_shared<pt::ptree>();
    return VARIABLES_MAP;
}

void GameConfig::LoadGameConfig(const std::string &filename) {
    pt::ptree temp_ptree;
    if (!boost::filesystem::exists(filename)) {
        VS_LOG_AND_FLUSH(fatal, (boost::format("void GameConfig::LoadGameConfig(const std::string &filename): Could not find vegastrike.config at '%1%'") % filename));
//        VSExit
    }
    pt::read_xml(filename, temp_ptree);
    for (const auto& iterator : temp_ptree.get_child("vegaconfig.variables.")) {
        if (boost::iequals(iterator.first, "section")) {
            std::string section_name = iterator.second.get<std::string>("<xmlattr>.name", "");
            if (section_name.empty()) {
                continue;
            }
            pt::ptree inner_tree = iterator.second;
            for (const auto& iterator2 : inner_tree) {
                if (boost::iequals(iterator2.first, "var")) {
                    std::string variable_name = iterator2.second.get<std::string>("<xmlattr>.name", "");
                    if (variable_name.empty()) {
                        continue;
                    }
                    std::string variable_value = iterator2.second.get<std::string>("<xmlattr>.value", "");
                    variables_()->put(section_name + "." + variable_name, variable_value);
                } else if (boost::iequals(iterator2.first, "section")) {
                    std::string subsection_name = iterator2.second.get<std::string>("<xmlattr>.name", "");
                    if (subsection_name.empty()) {
                        continue;
                    }
                    pt::ptree inner_tree2 = iterator2.second;
                    for (const auto& iterator3 : inner_tree2) {
                        if (boost::iequals(iterator3.first, "var")) {
                            std::string variable_name2 = iterator3.second.get<std::string>("<xmlattr>.name", "");
                            if (variable_name2.empty()) {
                                continue;
                            }
                            std::string variable_value2 = iterator3.second.get<std::string>("<xmlattr>.value", "");
                            variables_()->put(section_name + "." + subsection_name + "." + variable_name2, variable_value2);
                        }
                    }
                }
            }
        }
    }
}
