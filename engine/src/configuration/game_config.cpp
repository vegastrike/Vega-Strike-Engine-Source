/*
 * game_config.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Creator: Daniel Horn
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


#include "configuration/game_config.h"

std::string vega_config::GameConfig::EscapedString(const std::string &input) {
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

boost::shared_ptr<pt::iptree> vega_config::GameConfig::variables_() {
    static boost::shared_ptr<pt::iptree> variables_tree = boost::make_shared<pt::iptree>();
    return variables_tree;
}

void vega_config::GameConfig::LoadGameConfig(const std::string &filename) {
    pt::ptree temp_ptree;
    if (boost::filesystem::exists(filename)) {
        VS_LOG(debug, (boost::format("%1%: Found game config at '%2%'") % __func__ % filename));
    } else {
        VS_LOG_FLUSH_EXIT(fatal, (boost::format("%1%: Could not find game config at '%2%'") % __func__ % filename), 404);
    }
    pt::read_xml(filename, temp_ptree, boost::property_tree::xml_parser::no_comments);
//    pt::write_xml(filename + ".out.xml", temp_ptree);
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
//                    VS_LOG(debug, (boost::format("%1%: putting value %2% in the tree at %3%") % __func__ % variable_value % (section_name + "." + variable_name)));
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
//    pt::write_xml(filename + ".variables_.out.xml", variables_()->);
}

vega_config::GameConfig &vega_config::GetGameConfig() {
    static GameConfig kSingleton{};
    return kSingleton;
}
