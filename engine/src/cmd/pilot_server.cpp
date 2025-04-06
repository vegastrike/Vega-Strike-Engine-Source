/**
 * pilot_server.cpp
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2025 Stephen G. Tuggy, and other Vega Strike Contributors
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


#include "root_generic/faction_generic.h"
#include "cmd/unit_generic.h"
#include "cmd/pilot.h"
#include "cmd/ai/order.h"
#include "src/universe_util.h"
#include "cmd/unit_util.h"

#include <vector>

Pilot::Pilot(int faction) {
    const float reaction = vega_config::config->ai.firing.reaction_time;
    const float ran = vega_config::config->ai.default_rank;
    this->rank = ran;

    this->reaction_time = reaction;
    this->faction = faction;
    comm_face = NULL;
    gender = 0;
}

void Pilot::SetComm(Unit *parent) {
    this->faction = parent->faction;
    //GET BETTER REACTION TIME AND RANK HERE
}

float Pilot::adjustSpecificRelationship(Unit *parent, void *aggressor, float factor, int faction) {
    relationmap::iterator i = effective_relationship.insert(std::pair<const void *, float>(aggressor, 0)).first;
    if (faction != FactionUtil::GetNeutralFaction()) {
        float rel = UnitUtil::getRelationToFaction(parent, faction);         /* What the bloody hell? */
        bool abovezero = (*i).second + rel < 0;
        if (!abovezero) {
            const float slowrel = vega_config::config->ai.slow_diplomacy_for_enemies;
            factor *= slowrel;
        }
        (*i).second += factor;
        if (rel + factor < 0 && parent->Target() == NULL && parent->aistate) {
            parent->aistate->ChooseTarget();
        }
    } else {
        const float lessrel = vega_config::config->ai.unknown_relation_enemy;
        bool abovezero = (*i).second < lessrel;
        if (!abovezero) {
            const float slowrel = vega_config::config->ai.slow_diplomacy_for_enemies;
            factor *= slowrel;
        }
        (*i).second += factor;
        if ((*i).second < lessrel && parent->Target() == NULL && parent->aistate) {
            parent->aistate->ChooseTarget();
        }
    }
    return (*i).second;
}

void Pilot::DoHit(Unit *parent, void *aggressor, int faction) {
    const float hitcost = vega_config::config->ai.unknown_relation_hit_cost;
    if (hitcost) {
        adjustSpecificRelationship(parent, aggressor, hitcost, faction);
        int whichCp = _Universe->whichPlayerStarship(parent);
        if (whichCp != -1 && faction != parent->faction) {
            UniverseUtil::adjustRelationModifierInt(whichCp, faction, hitcost * getRank());
        } else {
            /* Instead use the Aggressor's cockpit? */
            whichCp = _Universe->whichPlayerStarship((const Unit *) aggressor);
            if (whichCp != -1) {
                Flightgroup *fg = parent->getFlightgroup();
                if (parent->faction != faction) {
                    UniverseUtil::adjustRelationModifierInt(whichCp, parent->faction, hitcost * getRank());
                }
                if (fg) {
                    UniverseUtil::adjustFGRelationModifier(whichCp, fg->name, hitcost * getRank());
                }
            }
        }
    }
}

float Pilot::getAnger(const Unit *parent, const Unit *target) const {
    relationmap::const_iterator iter = effective_relationship.find(target);
    float rel = 0;
    if (iter != effective_relationship.end()) {
        rel = iter->second;
    }
    if (_Universe->isPlayerStarship(target)) {
        if (FactionUtil::GetFactionName(faction).find("pirates") != std::string::npos) {
            static unsigned int cachedCargoNum = 0;
            static bool good = true;
            if (cachedCargoNum != target->numCargo()) {
                cachedCargoNum = target->numCargo();
                good = true;
                for (unsigned int i = 0; i < cachedCargoNum; ++i) {
                    Cargo *c = &target->pImage->cargo[i];
                    if (c->quantity != 0 && c->GetCategory().find("upgrades") == string::npos) {
                        good = false;
                        break;
                    }
                }
            }
            if (good) {
                const float goodness_for_nocargo = vega_config::config->ai.pirate_bonus_for_empty_hold;
                rel += goodness_for_nocargo;
            }
        }
    }
    {
        int fac = faction;
        MapStringFloat::iterator mapiter = factions[fac]->ship_relation_modifier.find(target->name);
        if (mapiter != factions[fac]->ship_relation_modifier.end()) {
            rel += (*mapiter).second;
        }
    }
    {
        int parent_cp = _Universe->whichPlayerStarship(parent);
        int target_cp = _Universe->whichPlayerStarship(target);
        if (parent_cp != -1) {
            Flightgroup *fg = target->getFlightgroup();
            if (fg) {
                rel += UniverseUtil::getFGRelationModifier(parent_cp, fg->name);
            }
        }
        if (target_cp != -1) {
            //... do we count it both ways? else?
            Flightgroup *fg = parent->getFlightgroup();
            if (fg) {
                rel += UniverseUtil::getFGRelationModifier(target_cp, fg->name);
            }
        }
    }

    return rel;
}

float Pilot::GetEffectiveRelationship(const Unit *parent, const Unit *target) const {
    return getAnger(parent, target) + UnitUtil::getFactionRelation(parent, target);
}

extern float myroundclamp(float i);

Animation *Pilot::getCommFace(Unit *parent, float mood, unsigned char &sex) {
    vector<Animation *> *ani = getCommFaces(sex);
//this #ifndef hack below by chuck_starchaser, to get around missing faction_util.o in vegaserver make list
    if (ani->size() == 0) {
        return NULL;
    }
    mood += .1;
    mood *= (ani->size()) / .2;
    unsigned int index = (unsigned int) myroundclamp(floor(mood));
    if (index >= ani->size()) {
        index = ani->size() - 1;
    }
    return (*ani)[index];
}

