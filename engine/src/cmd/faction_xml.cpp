/**
 * faction_xml.cpp
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
 * Copyright (C) 2022 Stephen G. Tuggy
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


#include <vector>
#include <string>
#include <gnuhash.h>
#include <expat.h>
#include "vegastrike.h"
#include "xml_support.h"
#include <assert.h>
#include "ai/communication.h"
#include "cmd/music.h"
#include "faction_generic.h"
#include "faction_factory.h"

using namespace XMLSupport;

static int unitlevel;

static vector<std::string> contrabandlists;

void Faction::LoadXML(const char *filename, char *xmlbuffer, int buflength) {
    using namespace VSFileSystem;
    using std::endl;
    using std::pair;
    unitlevel = 0;

    FactionFactory factory(filename);
    ParseAllAllies();

    // Results are cached to avoid looking for too many files
    typedef vsUMap<string, boost::shared_ptr<FSM> > Cache;
    Cache cache;

    std::string fileSuffix(".xml");
    std::string neutralName("neutral");
    boost::shared_ptr<FSM> neutralComm;
    neutralComm.reset(new FSM(neutralName + fileSuffix));
    cache.insert(Cache::value_type(neutralName, neutralComm));

    for (unsigned int i = 0; i < factions.size(); i++) {
        boost::shared_ptr<Faction> fact = factions[i];
        std::string myCommFile = fact->factionname + fileSuffix;
        boost::shared_ptr<FSM> myComm;
        Cache::iterator it = cache.find(myCommFile);
        if (it != cache.end()) {
            myComm = it->second;
        } else {
            if (VSFileSystem::LookForFile(myCommFile, CommFile) <= Ok) {
                myComm.reset(new FSM(myCommFile));
            } else {
                myComm.reset();
            }
            cache.insert(Cache::value_type(myCommFile, myComm));
        }

        for (unsigned int j = 0; j < factions[i]->faction.size(); j++) {
            std::string jointCommFile = fact->factionname
                    + std::string("to")
                    + factions[j]->factionname
                    + fileSuffix;
            boost::shared_ptr<FSM> jointComm;
            if (!fact->faction[j].conversation) {
                it = cache.find(jointCommFile);
                if (it != cache.end()) {
                    jointComm = it->second;
                } else {
                    if (VSFileSystem::LookForFile(jointCommFile, CommFile) <= Ok) {
                        jointComm.reset(new FSM(jointCommFile));
                    } else {
                        jointComm.reset();
                    }
                    cache.insert(Cache::value_type(jointCommFile, jointComm));
                }
                factions[i]->faction[j].conversation = jointComm
                        ? jointComm
                        : (myComm ? myComm : neutralComm);
            }
        }
    }
    char *munull = NULL;
    cache.clear();
    FactionUtil::LoadSerializedFaction(munull);
}

void FactionUtil::LoadContrabandLists() {
    for (unsigned int i = 0; i < factions.size() && i < contrabandlists.size(); i++) {
        if (contrabandlists[i].length() > 0) {
            factions[i]->contraband.reset(new Unit(contrabandlists[i].c_str(), true, i));
        }
    }
    contrabandlists.clear();
}

