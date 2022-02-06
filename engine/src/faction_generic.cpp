/*
 * Copyright (C) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */

#include "faction_generic.h"
#include "cmd/unit_generic.h"
#include "string.h"

using namespace FactionUtil;

vector<boost::shared_ptr<Faction> > factions;  //the factions

void Faction::ParseAllAllies()
{
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

void Faction::ParseAllies(unsigned int thisfaction)
{
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

