/*
 * faction_generic.cpp
 *
 * Copyright (C) 2001-2025 Daniel Horn, pyramid3d, Stephen G. Tuggy,
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */

#include "root_generic/faction_generic.h"
#include "cmd/unit_generic.h"
#include <cstring>

using namespace FactionUtil;

vector<boost::shared_ptr<Faction> > factions;  //the factions

void Faction::ParseAllAllies() {
    //MSVC has a bug where it won't let you reuse variables initialized in the paramater list of the for loop
    //the work around is A.) make the scope of the variable in the function level or not to re-define it in subsequent loops
    //Is this bug still valid? We are getting an GCC compile warning.
    unsigned int i = 0;
    for (i = 0; i < factions.size(); i++) {
        factions[i]->ParseAllies(i);
    }
    for (i = 0; i < factions.size(); i++) {
        factions[i]->faction[i].relationship = 1;
    }
}

void Faction::ParseAllies(unsigned int thisfaction) {
    unsigned int i, j;
    vector<faction_stuff> tempvec;
    for (i = 0; i < faction.size(); i++) {
        for (j = 0; j < factions.size(); j++) {
            if (strcmp(faction[i].stats.name, factions[j]->factionname) == 0) {
                delete[] faction[i].stats.name;
                faction[i].stats.index = j;
                break;
            }
        }
    }
    for (i = 0; i < factions.size(); i++) {
        tempvec.push_back(faction_stuff());
        tempvec[i].stats.index = i;
        tempvec[i].relationship = ((i == thisfaction) ? 1 : 0);
    }
    for (i = 0; i < faction.size(); i++) {
        faction_stuff::faction_name tmp = tempvec[faction[i].stats.index].stats;
        tempvec[faction[i].stats.index] = faction[i];
        tempvec[faction[i].stats.index].stats = tmp;
    }
    faction.swap(tempvec);

}


const std::map<std::string, std::string> FactionUtil::GetRelationsMap(const int privateer_faction) {
    std::map<std::string, std::string> relations;

    for (int i = 0; i < (int)FactionUtil::GetNumFactions(); i++) {
        float relation = FactionUtil::GetIntRelation(i, privateer_faction);
        string faction_name = FactionUtil::GetFactionName(i);
        const int percent = (int) (relation * 100.0);
        relations.insert(std::pair<std::string, std::string>(faction_name, std::to_string(percent)));
    }
    return relations;
}

const std::map<std::string, std::string> FactionUtil::GetKillsMap(const std::vector<float> *kill_list) {
    //Number of kills for each faction.
    std::map<std::string, std::string> kills;
    for (int i = 0; i < (int)FactionUtil::GetNumFactions(); i++) {
        string faction_name = FactionUtil::GetFactionName(i);
        int kills_for_faction = 0;
        size_t upgrades = FactionUtil::GetUpgradeFaction();
        size_t planets = FactionUtil::GetPlanetFaction();
        static size_t privateer = FactionUtil::GetFactionIndex("privateer");
        size_t neutral = FactionUtil::GetNeutralFaction();
        if (i < kill_list->size() && i != upgrades && i != planets && i != neutral && i != privateer) {
            kills_for_faction = (int) (*kill_list)[i];
        }

        kills.insert(std::pair<std::string, std::string>(faction_name, std::to_string(kills_for_faction)));
    }

    return kills;
}

const std::vector<std::string> FactionUtil::GetFactionNames() {
    std::vector<std::string> names;

    for (int i = 0; i < factions.size(); i++) {
        string faction_name = std::string(factions[i]->factionname);
        names.push_back(faction_name);
    }

    return names;
}

const std::vector<std::string> FactionUtil::GetFactionRelations() {
    static const int privateer_faction = FactionUtil::GetFactionIndex("privateer");

    std::vector<std::string> relations;

    for (int i = 0; i < factions.size(); i++) {
        double relation = FactionUtil::GetIntRelation(i, privateer_faction);
        relations.push_back(std::to_string(relation));
    }

    return relations;
}

const std::vector<std::string> FactionUtil::GetFactionKills(const std::vector<float> *kill_list) {
    std::vector<std::string> kills;

    for (int i = 0; i < factions.size(); i++) {
        int kills_for_faction = 0;
        if (i < (int)kill_list->size()) {
            kills_for_faction = (int) (*kill_list)[i];
        }

        kills.push_back(std::to_string(kills_for_faction));
    }

    return kills;
}
