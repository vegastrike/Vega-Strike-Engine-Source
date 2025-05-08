/*
 * galaxy.cpp
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


#include "root_generic/galaxy.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <string>
#include <map>
#include <vector>
#include <iostream>

namespace pt = boost::property_tree;

using std::string;
using std::map;
using std::vector;

Galaxy::Galaxy(string const &galaxy_file) {
    pt::ptree tree;
    pt::read_xml(galaxy_file, tree);

    // Parse planet variables and planets
    string const planets_xpath = "galaxy.planets.";
    for (const auto &planet_iterator : tree.get_child(planets_xpath)) {
        string type = planet_iterator.first;
        string name = planet_iterator.second.get<string>("<xmlattr>.name", "");

        // Parse planet variables
        if (type == "var") {
            string value = planet_iterator.second.get<string>("<xmlattr>.value", "");
            if (value == "") {
                continue;
            }
            variables[name] = value;
        } else if (type == "planet") { // Parse planets
            map<string, string> planet;
            pt::ptree inner_tree = planet_iterator.second;
            for (const auto &variable_iterator : inner_tree) {
                string name = variable_iterator.second.get<string>("<xmlattr>.name", "");
                string value = variable_iterator.second.get<string>("<xmlattr>.value", "");
                if (name == "" || value == "") {
                    continue;
                }
                planet[name] = value;
            }
            planets[name] = planet;
        }
    }

    // Parse systems
    string const systems_xpath = "galaxy.systems.";
    for (const auto &sector_iterator : tree.get_child(systems_xpath)) {
        string type = sector_iterator.first;
        string name = sector_iterator.second.get<string>("<xmlattr>.name", "");
        if (name == "") {
            continue;
        }

        // Parse sectors
        map<string, map<string, string>> sector;
        pt::ptree inner_tree = sector_iterator.second;
        for (const auto &system_iterator : inner_tree) {
            string name = system_iterator.second.get<string>("<xmlattr>.name", "");
            if (name == "") {
                continue;
            }

            map<string, string> system;
            pt::ptree innermost_tree = system_iterator.second;
            for (const auto &variable_iterator : innermost_tree) {
                string name = variable_iterator.second.get<string>("<xmlattr>.name", "");
                string value = variable_iterator.second.get<string>("<xmlattr>.value", "");
                if (name == "" || value == "") {
                    continue;
                }
                system[name] = value;
            }
            sector[name] = system;
        }
        systems[name] = sector;
    }
}
