/*
 * unit_functions_generic.cpp
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


#include "cmd/unit_generic.h"
#include "gfx_generic/vec.h"
#include "gfx_generic/cockpit_generic.h"
#include "root_generic/faction_generic.h"
#include "cmd/ai/communication.h"
#include "root_generic/savegame.h"
#include "root_generic/xml_support.h"
#include "cmd/unit_util.h"
#include "src/universe_util.h"
#include "cmd/unit_const_cache.h"
#include "cmd/pilot.h"
#include "cmd/ai/order.h"
#include "src/universe.h"
#include "cmd/mount_size.h"
#include "cmd/damageable.h"
#include "resource/random_utils.h"
#include "root_generic/configxml.h"

//Various functions that were used in .cpp files that are now included because of
//the temple GameUnit class
//If not separated from those files functions would be defined in multiple places
//Those functions are generic ones

//From unit.cpp
double saved_interpolation_blend_factor;
double interpolation_blend_factor;
bool cam_setup_phase = false;



const Unit *getUnitFromUpgradeName(const string &upgradeName, int myUnitFaction = 0) {
    const char *name = upgradeName.c_str();
    const Unit *partUnit = UnitConstCache::getCachedConst(StringIntKey(name, FactionUtil::GetUpgradeFaction()));
    if (!partUnit) {
        partUnit = UnitConstCache::setCachedConst(StringIntKey(name, FactionUtil::GetUpgradeFaction()),
                new Unit(name, true, FactionUtil::GetUpgradeFaction()));
    }
    if (partUnit->name == "LOAD_FAILED") {
        partUnit = UnitConstCache::getCachedConst(StringIntKey(name, myUnitFaction));
        if (!partUnit) {
            partUnit = UnitConstCache::setCachedConst(StringIntKey(name, myUnitFaction),
                    new Unit(name, true, myUnitFaction));
        }
    }
    return partUnit;
}

int SelectDockPort(Unit *utdw, Unit *parent) {
    const vector<DockingPorts> *dp = &utdw->DockingPortLocations();
    float dist = FLT_MAX;
    int num = -1;
    for (unsigned int i = 0; i < dp->size(); ++i) {
        if (!(*dp)[i].IsOccupied()) {
            Vector rez = Transform(utdw->GetTransformation(), (*dp)[i].GetPosition());
            float wdist = (rez - parent->Position()).MagnitudeSquared();
            if (wdist < dist) {
                num = i;
                dist = wdist;
            }
        }
    }
    return num;
}

//From unit_customize.cpp
Unit *CreateGameTurret(std::string tur, int faction) {
    return new Unit(tur.c_str(), true, faction);
}

//un scored a faction kill
void ScoreKill(Cockpit *cp, Unit *un, Unit *killedUnit) {
    if (un->getUnitType() != Vega_UnitType::unit || killedUnit->getUnitType() != Vega_UnitType::unit) {
        return;
    }
    int killedCp = _Universe->whichPlayerStarship(killedUnit);
    int killerCp = killedCp;
    if (killedCp != -1) {
        UniverseUtil::adjustRelationModifierInt(killedCp, un->faction, configuration()->ai.kill_factor_flt);
    } else {
        killerCp = _Universe->whichPlayerStarship(un);
        if (killerCp != -1) {
            UniverseUtil::adjustRelationModifierInt(killerCp, killedUnit->faction, configuration()->ai.kill_factor_flt);
        }
    }
    int faction = killedUnit->faction;
    for (unsigned int i = 0; i < FactionUtil::GetNumFactions(); i++) {
        float relation;
        if (faction != (int) i && un->faction != (int) i) {
            relation = FactionUtil::GetIntRelation(i, faction);
            if (killedCp != -1) {
                relation += UniverseUtil::getRelationModifierInt(i, faction);
            }
            if (relation != 0.0F) {
                if (killerCp != -1) {
                    UniverseUtil::adjustRelationModifierInt(killerCp, i, configuration()->ai.friend_factor_flt * relation);
                }
            }
        }
    }
    int upgrades = FactionUtil::GetUpgradeFaction();
    int planets = FactionUtil::GetPlanetFaction();
    if (cp != NULL) {
        vector<float> *killlist = &cp->savegame->getMissionData(string("kills"));
        while (killlist->size() <= FactionUtil::GetNumFactions()) {
            killlist->push_back((float) 0.0);
        }
        if ((int) killlist->size() > faction) {
            (*killlist)[faction]++;
        }
        killlist->back()++;
    } else if (UnitUtil::getRelationToFaction(un, faction) < 0 && faction != upgrades && faction != planets) {
        int whichcp = rand() % _Universe->numPlayers();
        Unit *whichrecv = _Universe->AccessCockpit(whichcp)->GetParent();
        if (whichrecv != NULL) {
            if (UnitUtil::getUnitSystemFile(whichrecv) == UnitUtil::getUnitSystemFile(un)) {
                if (un->getAIState() && whichrecv->getAIState()) {
                    unsigned char sex;
                    vector<Animation *> *anim = un->pilot->getCommFaces(sex);
                    CommunicationMessage c(un, whichrecv, anim, sex);
                    c.SetCurrentState(c.fsm->GetScoreKillNode(), anim, sex);
                    whichrecv->getAIState()->Communicate(c);
                }
            }
        }
    }
}

//From unit_physics.cpp


float getAutoRSize(Unit *orig, Unit *un, bool ignore_friend = false) {
    const float friendly_autodist = configuration()->physics.friendly_auto_radius_flt;
    const float neutral_autodist = configuration()->physics.neutral_auto_radius_flt;
    const float hostile_autodist = configuration()->physics.hostile_auto_radius_flt;
    int upgradefaction = FactionUtil::GetUpgradeFaction();
    int neutral = FactionUtil::GetNeutralFaction();
    if (un->getUnitType() == Vega_UnitType::asteroid) {
        return configuration()->physics.min_asteroid_distance_flt;
    }
    if (un->getUnitType() == Vega_UnitType::planet
            || (un->getFlightgroup() == orig->getFlightgroup() && orig->getFlightgroup())) {
        //same flightgroup
        return orig->rSize();
    }
    if (un->faction == upgradefaction) {
        return ignore_friend ? -FLT_MAX : (-orig->rSize() - un->rSize());
    }
    float rel = un->getRelation(orig);
    if (orig == un->Target()) {
        rel -= 1.5;
    }
    if (un->faction == neutral) {
        return neutral_autodist;
    } else if (rel > .1) {
        return ignore_friend ? -FLT_MAX : friendly_autodist;          //min distance apart
    } else if (rel < 0) {
        return hostile_autodist;
    } else {
        return ignore_friend ? -FLT_MAX : neutral_autodist;
    }
}

//From unit_weapon.cpp
bool AdjustMatrix(Matrix &mat, const Vector &vel, Unit *target, float speed, bool lead, float cone) {
    if (target) {
        QVector pos(mat.p);
        Vector R(mat.getR());
        QVector targpos(lead ? target->PositionITTS(pos, vel, speed, false) : target->Position());

        Vector dir = (targpos - pos).Cast();
        dir.Normalize();
        if (dir.Dot(R) >= cone) {
            Vector Q(mat.getQ());
            Vector P;
            ScaledCrossProduct(Q, dir, P);
            ScaledCrossProduct(dir, P, Q);
            VectorAndPositionToMatrix(mat, P, Q, dir, pos);
        } else {
            return false;
        }
        return true;
    }
    return false;
}

// TODO: delete
int parseMountSizes(const char *str) {
    char tmp[13][50];
    int ans = as_integer(MOUNT_SIZE::NOWEAP);
    int num = sscanf(str,
            "%s %s %s %s %s %s %s %s %s %s %s %s %s",
            tmp[0],
            tmp[1],
            tmp[2],
            tmp[3],
            tmp[4],
            tmp[5],
            tmp[6],
            tmp[7],
            tmp[8],
            tmp[9],
            tmp[10],
            tmp[11],
            tmp[12]);
    for (int i = 0; i < num && i < 13; i++) {
        ans |= as_integer(getMountSize(tmp[i]));
    }

    int check = getMountSizes(str);
    assert(check == ans);

    return ans;
}

void DealPossibleJumpDamage(Unit *un) {
    const double jump_damage_multiplier = configuration()->physics.jump_damage_multiplier_dbl;
    const double max_damage = configuration()->physics.max_jump_damage_dbl;

    // Also damage multiplier
    double chance_to_damage = randomDouble() - 0.01;

    // If jump drive is fully operational, there's no chance for damage
    if(un->jump_drive.PercentOperational() >= chance_to_damage) {
        return;
    }

    double speed = un->GetVelocity().Magnitude();
    double mass = un->GetMass();

    double jump_damage = mass * speed * chance_to_damage * jump_damage_multiplier;

    jump_damage = std::min(jump_damage, max_damage);

    Damage damage;
    damage.normal_damage = jump_damage;
    un->ApplyDamage((un->Position() + un->GetVelocity().Cast()),
                     un->GetVelocity(),
                     damage,
                     un,
                     GFXColor(((float) (rand() % 100)) / 100,
                        ((float) (rand() % 100)) / 100,
                        ((float) (rand() % 100)) / 100), nullptr);
}

extern Cargo *GetMasterPartList(const char *input_buffer);

