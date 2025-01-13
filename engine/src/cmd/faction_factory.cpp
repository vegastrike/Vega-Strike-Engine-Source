/*
 * faction_factory.cpp
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


#include "faction_factory.h"
#include "faction_generic.h"
#include "gfx/animation.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/optional/optional.hpp>

namespace pt = boost::property_tree;
namespace alg = boost::algorithm;

using boost::property_tree::ptree;
using boost::optional;

using std::string;

extern std::vector<boost::shared_ptr<Faction> > factions;   //the factions

FactionFactory::FactionFactory(std::string filename) {
    pt::ptree tree;
    pt::read_xml(filename, tree);

    // Iterate over root
    for (const auto &iterator : tree) {
        parse(iterator.second);

        // There should be only one root. Exiting
        break;
    }
}

void FactionFactory::parse(ptree tree) {
    int counter = 0;
    for (const auto &iterator : tree) {
        if (iterator.first != "Faction") {
            continue;
        }

        ptree inner = iterator.second;
        boost::shared_ptr<Faction> faction(new Faction());

        string tmp_name = inner.get("<xmlattr>.name", "Unknown");

        // TODO: make factioname an std::string
        faction->factionname = new char[tmp_name.size() + 1];
        strcpy(faction->factionname, tmp_name.c_str());

        faction->citizen = inner.get("<xmlattr>.citizen", false);
        string logo_rgb = inner.get("<xmlattr>.logoRGB", "");
        string logo_sec_rgb = inner.get("<xmlattr>.secLogoRGB", "");
        string contraband = inner.get("<xmlattr>.contraband", "");

        // Parse spark colors
        std::string colors[] = {"Red", "Green", "Blue", "Alpha"};
        int i = 0;
        for (const auto &color : colors) {
            if (boost::optional<float> spark = inner.get_optional<float>("<xmlattr>.Spark" + color)) {
                faction->sparkcolor[i++] = *spark;
            }
        }

        string explosion_name = inner.get("Explosion.<xmlattr>.name", "");
        if (!explosion_name.empty()) {
            boost::shared_ptr<Animation> explosion(FactionUtil::createAnimation(explosion_name.c_str()));
            faction->explosion_name.push_back(explosion_name);
            faction->explosion.push_back(explosion);
        }

        for (const auto &iterator : inner) {
            string type = iterator.first;
            ptree inner = iterator.second;

            // We skip attributes and explosions as we processed them above.
            if (type == "<xmlattr>" || type == "Explosion") {
                continue;
            }

            if (type == "stats") {
                string name = inner.get("<xmlattr>.name", "Unknown");
                int relation = inner.get("<xmlattr>.relation", 0);
                faction->ship_relation_modifier[name] = relation;
            }
        }

        // Set index of netural, planets and upgrades factions
        if (tmp_name == "neutral") {
            FactionUtil::neutralfac = factions.size();
        } else if (tmp_name == "planets") {
            FactionUtil::planetfac = factions.size();
        } else if (tmp_name == "upgrades") {
            FactionUtil::upgradefac = factions.size();
        }

        factions.push_back(faction);
    }
}
