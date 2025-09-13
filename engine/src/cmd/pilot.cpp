/*
 * pilot.cpp
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


#include "root_generic/faction_generic.h"
#include "cmd/unit_generic.h"
#include "cmd/pilot.h"
#include "cmd/ai/order.h"
#include "src/universe_util.h"
#include "cmd/unit_util.h"
#include "root_generic/configxml.h"
#include "src/universe.h"
#include "src/vs_logging.h"
#include <vector>

Pilot::Pilot(int faction) {
    const float reaction = configuration().ai.firing.reaction_time_flt;
    const float ran = configuration().ai.default_rank_flt;
    this->rank = ran;

    this->reaction_time = reaction;
    this->faction = faction;
    comm_face = NULL;
    gender = 0;
}

void Pilot::SetComm(Unit *parent) {
    this->faction = parent->faction;
    //GET BETTER REACTION TIME AND RANK HERE
    comm_face = FactionUtil::GetRandCommAnimation(faction, parent, gender);
}

float Pilot::adjustSpecificRelationship(Unit *parent, void *aggressor, float factor, int faction) {
    relationmap::iterator i = effective_relationship.insert(std::pair<const void *, float>(aggressor, 0)).first;
    if (faction != FactionUtil::GetNeutralFaction()) {
        float rel = UnitUtil::getRelationToFaction(parent, faction);         /* What the bloody hell? */
        bool abovezero = (*i).second + rel < 0;
        if (!abovezero) {
            const float slowrel = configuration().ai.slow_diplomacy_for_enemies_flt;
            factor *= slowrel;
        }
        (*i).second += factor;
        if (rel + factor < 0 && parent->Target() == nullptr && parent->aistate) {
            parent->aistate->ChooseTarget();
        }
    } else {
        const float lessrel = configuration().ai.unknown_relation_enemy_flt;
        bool abovezero = (*i).second < lessrel;
        if (!abovezero) {
            const float slowrel = configuration().ai.slow_diplomacy_for_enemies_flt;
            factor *= slowrel;
        }
        (*i).second += factor;
        if ((*i).second < lessrel && parent->Target() == nullptr && parent->aistate) {
            parent->aistate->ChooseTarget();
        }
    }
    return (*i).second;
}

void Pilot::DoHit(Unit *parent, void *aggressor, int faction) {
    const float hitcost = configuration().ai.unknown_relation_hit_cost_flt;
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
    float rel = 0.0f;
    if (target == nullptr) {
        VS_LOG(warning, "Pilot::getAnger(): target is null");
        return 0.0f;
    }
    relationmap::const_iterator iter = effective_relationship.find(target);
    if (iter != effective_relationship.end()) {
        rel = iter->second;
    }
    if (target->IsPlayerShip()) {
        if (FactionUtil::GetFactionName(faction).find("pirates") != std::string::npos) {
            static unsigned int cachedCargoNum = 0;
            static bool empty_cargo_hold = true;
            if (cachedCargoNum != target->cargo_hold.Size()) {
                cachedCargoNum = target->cargo_hold.Size();

                // Check if pirates 
                if(cachedCargoNum == 0) {
                    empty_cargo_hold = true;
                } else {
                    empty_cargo_hold = false;
                }
            }
            if (empty_cargo_hold) {
                const float goodness_for_nocargo = configuration().ai.pirate_bonus_for_empty_hold_flt;
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
    if (target == nullptr) {
        VS_LOG(warning, "Pilot::GetEffectiveRelationship(): target is null");
        return 0.0f;
    }
    return getAnger(parent, target) + UnitUtil::getFactionRelation(parent, target);
}

extern float myroundclamp(float i);

Animation *Pilot::getCommFace(Unit *parent, float mood, unsigned char &sex) {
    vector<Animation *> *ani = getCommFaces(sex);
    if (ani == NULL) {
        ani = FactionUtil::GetRandCommAnimation(parent->faction, parent, sex);
        if (ani == NULL) {
            return NULL;
        }
    }
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

