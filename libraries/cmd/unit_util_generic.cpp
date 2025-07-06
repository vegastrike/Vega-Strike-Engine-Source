/*
 * unit_util_generic.cpp
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


#define PY_SSIZE_T_CLEAN
#include <boost/python.hpp>
#include <string>
#include "cmd/unit_generic.h"
#include "cmd/unit_util.h"
#include "root_generic/configxml.h"
#include "root_generic/vs_globals.h"
#include "gfx_generic/cockpit_generic.h"
#include "src/universe_util.h"
#include "cmd/ai/docking.h"
#include "root_generic/savegame.h"
#include "cmd/planet.h"
#include "root_generic/faction_generic.h"
#include "cmd/ai/fire.h"
#include "gfx_generic/cockpit_generic.h"
#include "cmd/role_bitmask.h"
#include "root_generic/lin_time.h"
#include "cmd/planetary_orbit.h"
#include "src/universe.h"
#include "src/vs_logging.h"
#include "cmd/weapon_info.h"
#include "cmd/unit_csv_factory.h"

#include <boost/algorithm/string/predicate.hpp>

#include "cmd/script/pythonmission.h"
#ifndef NO_GFX
#include "gfx/cockpit.h"
#endif
const Unit *makeTemplateUpgrade(string name, int faction); //for percentoperational
const Unit *getUnitFromUpgradeName(const string &upgradeName, int myUnitFaction = 0); //for percentoperational
extern const char *DamagedCategory;  //for percentoperational
using std::string;
extern Unit *getTopLevelOwner();

static bool nameIsAsteroid(std::string name) {
    if (name.length() < 8) {
        return false;
    }
    return (name[0] == 'A' || name[0] == 'a')
            && name[1] == 's'
            && name[2] == 't'
            && name[3] == 'e'
            && name[4] == 'r'
            && name[5] == 'o'
            && name[6] == 'i'
            && name[7] == 'd';
}

namespace UnitUtil {
//

template<typename T>
static inline T mymin(T a, T b) {
    return (a < b) ? a : b;
}

template<typename T>
static inline T mymax(T a, T b) {
    return (a > b) ? a : b;
}

static const string &getFgDirectiveCR(const Unit *my_unit);

bool isAsteroid(const Unit *my_unit) {
    if (!my_unit) {
        return false;
    }
    return my_unit->getUnitType() == Vega_UnitType::asteroid || nameIsAsteroid(my_unit->name);
}

bool isCapitalShip(const Unit *my_unit) {
    if (!my_unit) {
        return false;
    }
    static unsigned int capitaltypes = ROLES::getCapitalRoles();
    return ((1 << (unsigned int) my_unit->getUnitRoleChar()) & capitaltypes) != 0;
}

bool hasDockingUnits(const Unit *my_unit) {
    if (!my_unit) {
        return false;
    }
    return (my_unit->DockedOrDocking() & Unit::DOCKING_UNITS)
            || (my_unit->hasPendingClearanceRequests());
}

int getPhysicsPriority(Unit *un) {
    static bool FORCE_TOP_PRIORITY = XMLSupport::parse_bool(
            vs_config->getVariable("physics", "priorities", "force_top_priority", "false"));
    if (FORCE_TOP_PRIORITY) {
        return 1;
    }
    //Some other comment mentions these need special treatment for subunit scheduling
    static int PLAYER_PRIORITY = XMLSupport::parse_int(
            vs_config->getVariable("physics", "priorities", "player", "1"));
    static int MISSILE_PRIORITY = XMLSupport::parse_int(
            vs_config->getVariable("physics", "priorities", "missile", "1"));
    static int DOCKABLE_PRIORITY = XMLSupport::parse_int(
            vs_config->getVariable("physics", "priorities", "dockable", "1"));

    float rad = un->rSize();
    Vega_UnitType untype = un->getUnitType();
    float cpdist = FLT_MAX;
    float tooclose = 0;
    unsigned int np = _Universe->numPlayers();
    Cockpit *cockpit = _Universe->AccessCockpit();
    //static float     fixed_system_orbit_priorities =
    //    XMLSupport::parse_float( vs_config->getVariable( "physics", "fixed_system_priority_velocity_cutoff", "3" ) );
    static int SYSTEM_INSTALLATION_PRIORITY = XMLSupport::parse_int(
            vs_config->getVariable("physics", "priorities", "system_installation", "3"));
    bool system_installation = un->owner == getTopLevelOwner();
    bool force_system_installation_priority = true;
    //if (system_installation && un->Velocity.MagnitudeSquared() > fixed_system_orbit_priorities*fixed_system_orbit_priorities)
    //    force_system_installation_priority = true;
    for (unsigned int i = 0; i < np; ++i) {
        Unit *player = _Universe->AccessCockpit(i)->GetParent();
        if (player) {
            if (force_system_installation_priority && player->activeStarSystem == un->activeStarSystem) {
                return SYSTEM_INSTALLATION_PRIORITY;
            }
            if (un == player->Target()) {
                return PLAYER_PRIORITY;
            }
            float tmpdist = UnitUtil::getDistance(un, player);
            if (tmpdist < cpdist) {
                QVector relvel = un->GetVelocity() - player->GetVelocity();
                QVector relpos = un->Position() - player->Position();
                cockpit = _Universe->AccessCockpit(i);
                cpdist = tmpdist;
                float lowest_priority_time = SIM_QUEUE_SIZE * SIMULATION_ATOM;
                if (relpos.Dot(relvel) >= 0) {
                    //No need to be wary if they're getting away
                    tooclose =
                            2 * (un->radial_size + player->radial_size)
                                    + relvel.Magnitude() * lowest_priority_time;
                }
            }
        }
#ifndef NO_GFX
        Camera *cam = _Universe->AccessCockpit(i)->AccessCamera();
        if (cam) {
            QVector relvel = un->GetVelocity() - cam->GetVelocity();
            QVector relpos = un->Position() - cam->GetPosition();
            double dist = relpos.Magnitude() - rad;
            if (dist < cpdist) {
                cpdist = dist;
                Unit *parent = _Universe->AccessCockpit(i)->GetParent();
                float lowest_priority_time = SIM_QUEUE_SIZE * SIMULATION_ATOM;
                if (relpos.Dot(relvel) >= 0) {
                    //No need to be wary if they're getting away
                    tooclose =
                            2 * (un->radial_size + (parent ? parent->radial_size : 0))
                                    + relvel.Magnitude() * lowest_priority_time;
                }
            }
        }
#endif
        if (player == un
                || (0 && _Universe->AccessCockpit(i)->GetSaveParent() == un)) {
            return PLAYER_PRIORITY;
        }
    }
    if (untype == Vega_UnitType::missile) {
        return MISSILE_PRIORITY;
    }
    if (hasDockingUnits(un)) {
        return DOCKABLE_PRIORITY;
    }
    static int ASTEROID_PARENT_PRIORITY = XMLSupport::parse_int(
            vs_config->getVariable("physics", "priorities", "asteroid_parent", "1"));
    static int ASTEROID_HIGH_PRIORITY = XMLSupport::parse_int(
            vs_config->getVariable("physics", "priorities", "asteroid_high", "2"));
    //static int   ASTEROID_LOW_PRIORITY    = XMLSupport::parse_int(
    //    vs_config->getVariable( "physics", "priorities", "asteroid.low", "32" ) );
    static int HIGH_PRIORITY = XMLSupport::parse_int(
            vs_config->getVariable("physics", "priorities", "high", "2"));
    static int MEDIUMHIGH_PRIORITY = XMLSupport::parse_int(
            vs_config->getVariable("physics", "priorities", "mediumhigh", "4"));
    static int MEDIUM_PRIORITY = XMLSupport::parse_int(
            vs_config->getVariable("physics", "priorities", "medium", "8"));
    static int LOW_PRIORITY = XMLSupport::parse_int(
            vs_config->getVariable("physics", "priorities", "low", "32"));
    static int NOT_VISIBLE_COMBAT_HIGH = XMLSupport::parse_int(
            vs_config->getVariable("physics", "priorities", "notvisible_combat_high", "10"));
    static int NOT_VISIBLE_COMBAT_MEDIUM = XMLSupport::parse_int(
            vs_config->getVariable("physics", "priorities", "notvisible_combat_medium", "20"));
    static int NOT_VISIBLE_COMBAT_LOW = XMLSupport::parse_int(
            vs_config->getVariable("physics", "priorities", "notvisible_combat_low", "40"));
    static int NO_ENEMIES = XMLSupport::parse_int(
            vs_config->getVariable("physics", "priorities", "no_enemies", "64"));
    static int INERT_PRIORITY = XMLSupport::parse_int(
            vs_config->getVariable("physics", "priorities", "inert", "64"));
    static double _PLAYERTHREAT_DISTANCE_FACTOR = XMLSupport::parse_float(
            vs_config->getVariable("physics", "priorities", "playerthreat_distance_factor", "2"));
    static double _THREAT_DISTANCE_FACTOR = XMLSupport::parse_float(
            vs_config->getVariable("physics", "priorities", "threat_distance_factor", "1"));
    static double DYNAMIC_THROTTLE_MINFACTOR = XMLSupport::parse_float(
            vs_config->getVariable("physics", "priorities", "dynamic_throttle.mindistancefactor", "0.25"));
    static double DYNAMIC_THROTTLE_MAXFACTOR = XMLSupport::parse_float(
            vs_config->getVariable("physics", "priorities", "dynamic_throttle.maxdistancefactor", "4"));
    static double DYNAMIC_THROTTLE_TARGETFPS = XMLSupport::parse_float(
            vs_config->getVariable("physics", "priorities", "dynamic_throttle.targetfps", "30"));
    static double DYNAMIC_THROTTLE_TARGETELAPSEDTIME = 1.0 / DYNAMIC_THROTTLE_TARGETFPS;
    static double DYNAMIC_THROTTLE_FACTOR = 1.0;
    static double lastThrottleAdjust = 0.0;
    if (UniverseUtil::GetGameTime() != lastThrottleAdjust) {
        lastThrottleAdjust = UniverseUtil::GetGameTime();
        double newfactor = DYNAMIC_THROTTLE_FACTOR * DYNAMIC_THROTTLE_TARGETELAPSEDTIME / GetElapsedTime();
        newfactor = mymax(DYNAMIC_THROTTLE_MINFACTOR, mymin(DYNAMIC_THROTTLE_MAXFACTOR, newfactor));
        DYNAMIC_THROTTLE_FACTOR = (newfactor * GetElapsedTime() + DYNAMIC_THROTTLE_FACTOR) / (1.0 + GetElapsedTime());
    }
    static double PLAYERTHREAT_DISTANCE_FACTOR = _PLAYERTHREAT_DISTANCE_FACTOR * DYNAMIC_THROTTLE_FACTOR;
    static double THREAT_DISTANCE_FACTOR = _THREAT_DISTANCE_FACTOR * DYNAMIC_THROTTLE_FACTOR;
    Unit *parent = cockpit->GetParent();
    float gun_range = 0;
    float missile_range = 0;
    float dist = cpdist;
    if (parent) {
        float speed = 0;
        parent->getAverageGunSpeed(speed, gun_range, missile_range);
    }
    static int cargofac = FactionUtil::GetFactionIndex("cargo");
    int upfac = FactionUtil::GetUpgradeFaction();
    int neutral = FactionUtil::GetNeutralFaction();
    if (un->schedule_priority != Unit::scheduleDefault) {
        //Asteroids do scheduling themselves within subunits, so...
        //...only one caveat: units with their own internal scheduling
        //must have constant priority... otherwise, big mess when changing
        //priorities. I'll think of one way to overcome this...
        switch (un->schedule_priority) {
            case Unit::scheduleDefault:
                break;
            case Unit::scheduleAField:
                return ASTEROID_PARENT_PRIORITY;

            case Unit::scheduleRoid:
                //if (dist < tooclose)
                return ASTEROID_HIGH_PRIORITY;
                //else
                //return ASTEROID_LOW_PRIORITY;
        }
    }
    if (UnitUtil::isAsteroid(un)) {
        //some mods don't do the scheduling--still want correctness
        static std::string blah = vs_config->getVariable("physics", "priorities", "min_asteroid_distance", "none");
        //static float too_close_asteroid = (blah == "none") ? tooclose : XMLSupport::parse_float( blah );
        //if (dist < too_close_asteroid)
        return ASTEROID_HIGH_PRIORITY;
        //else
        //return ASTEROID_LOW_PRIORITY;
    }
    Unit *targ = un->Target();
    if (_Universe->isPlayerStarship(targ)) {
        if (un->isJumppoint()) {
            return PLAYER_PRIORITY;
        } else if (UnitUtil::getDistance(targ, un) <= PLAYERTHREAT_DISTANCE_FACTOR * mymax(gun_range, missile_range)) {
            return HIGH_PRIORITY;
        } else { //Needs to accurately collide with it...
            return MEDIUMHIGH_PRIORITY;
        }
    }
    if (un->graphicOptions.WarpRamping || un->graphicOptions.RampCounter != 0) {
        static float compwarprampuptime =
                XMLSupport::parse_float(vs_config->getVariable("physics",
                        "computerwarprampuptime",
                        "10")); //for the heck of it.  NOTE, variable also in unit_generic.cpp
        static float warprampdowntime =
                XMLSupport::parse_float(vs_config->getVariable("physics", "warprampdowntime", "0.5"));
        float lowest_priority_time = SIM_QUEUE_SIZE * SIMULATION_ATOM;

        float time_ramped = compwarprampuptime - un->graphicOptions.RampCounter;
        if (!un->ftl_drive.Enabled()) {
            time_ramped = warprampdowntime - un->graphicOptions.RampCounter;
        }
        if (un->graphicOptions.WarpRamping || time_ramped < lowest_priority_time) {
            return MEDIUM_PRIORITY;
        }
        if (dist < gun_range) {
            return MEDIUM_PRIORITY;
        }
        if (time_ramped < lowest_priority_time * 2) {
            return LOW_PRIORITY;
        }
        //else defer decision
    }
    if (system_installation || un->faction == cargofac || un->faction == upfac || un->faction == neutral) {
        if (dist < tooclose) {
            return MEDIUM_PRIORITY;
        } else {
            if (system_installation) {
                return SIM_QUEUE_SIZE / ORBIT_PRIORITY;
            } //so that the averaging can still keep track of parent jumps
            return INERT_PRIORITY;
        }
    }
    const string &obj = UnitUtil::getFgDirective(un);
    if (!(obj.length() == 0 || (obj.length() >= 1 && obj[0] == 'b'))) {
        return MEDIUM_PRIORITY;
    }
    if (dist < 2 * PLAYERTHREAT_DISTANCE_FACTOR * mymax(missile_range, gun_range)) {
        if (dist < tooclose * PLAYERTHREAT_DISTANCE_FACTOR) {
            return MEDIUMHIGH_PRIORITY;
        } else if (dist < 2 * gun_range * PLAYERTHREAT_DISTANCE_FACTOR) {
            return MEDIUM_PRIORITY;
        } else {
            return LOW_PRIORITY;
        }
    }
    if (targ) {
        float speed;
        un->getAverageGunSpeed(speed, gun_range, missile_range);
        double distance = UnitUtil::getDistance(un, targ);
        if (distance <= 2.0 * static_cast<double>(gun_range) * THREAT_DISTANCE_FACTOR) {
            return NOT_VISIBLE_COMBAT_HIGH;
        }
        if (distance < 2.0 * static_cast<double>(missile_range) * THREAT_DISTANCE_FACTOR) {
            return NOT_VISIBLE_COMBAT_MEDIUM;
        }
        return NOT_VISIBLE_COMBAT_LOW;
    }
    if (dist < tooclose * THREAT_DISTANCE_FACTOR) {
        return MEDIUMHIGH_PRIORITY;
    } else { //May not have weapons (hence missile_range|gun_range == 0)
        return NO_ENEMIES;
    }
}

void orbit(Unit *my_unit, Unit *orbitee, float speed, QVector R, QVector S, QVector center) {
    if (my_unit) {
        my_unit->PrimeOrders(new PlanetaryOrbit(my_unit,
                speed / (3.1415926536 * (S.Magnitude() + R.Magnitude())),
                0,
                R,
                S,
                center,
                orbitee));
        if (orbitee) {
            if (orbitee->getUnitType() == Vega_UnitType::planet) {
                ((Planet *) orbitee)->AddSatellite(my_unit);
            }
        }
        if (my_unit->faction != FactionUtil::GetFactionIndex("neutral")) {
            Order *tmp = new Orders::FireAt(configuration()->ai.firing.aggressivity);
            my_unit->EnqueueAI(tmp);
            my_unit->SetTurretAI();
        }
        my_unit->SetOwner(getTopLevelOwner());
    }
}

string getFactionName(const Unit *my_unit) {
    if (!my_unit) {
        return "";
    }
    return FactionUtil::GetFaction(my_unit->faction);
}

int getFactionIndex(const Unit *my_unit) {
    if (!my_unit) {
        return 0;
    }
    return my_unit->faction;
}

void setFactionIndex(Unit *my_unit, int factionname) {
    if (!my_unit) {
        return;
    }
    my_unit->SetFaction(factionname);
}

void setFactionName(Unit *my_unit, string factionname) {
    if (!my_unit) {
        return;
    }
    my_unit->SetFaction(FactionUtil::GetFactionIndex(factionname));
}

float getFactionRelation(const Unit *my_unit, const Unit *their_unit) {
    if ((my_unit == nullptr) || (their_unit == nullptr)) {
        VS_LOG(warning, "getFactionRelation: null unit encountered!");
        return 0.0f;
    }
    float relation = FactionUtil::GetIntRelation(my_unit->faction, their_unit->faction);
    int my_cp = _Universe->whichPlayerStarship(my_unit);
    int their_cp = _Universe->whichPlayerStarship(their_unit);
    if (my_cp != -1) {
        relation += UniverseUtil::getRelationModifierInt(my_cp, their_unit->faction);
    } else if (their_cp != -1) {             /* The question is: use an else? */
        relation += UniverseUtil::getRelationModifierInt(their_cp, my_unit->faction);
    }
    return relation;
}

float getRelationToFaction(const Unit *my_unit, int other_faction) {
    float relation = FactionUtil::GetIntRelation(my_unit->faction, other_faction);
    int my_cp = _Universe->whichPlayerStarship(my_unit);
    if (my_cp != -1) {
        relation += UniverseUtil::getRelationModifierInt(my_cp, other_faction);
    }
    return relation;
}

float getRelationFromFaction(const Unit *their_unit, int my_faction) {
    float relation = FactionUtil::GetIntRelation(my_faction, their_unit->faction);
    int their_cp = _Universe->whichPlayerStarship(their_unit);
    if (their_cp != -1) {
        relation += UniverseUtil::getRelationModifierInt(their_cp, my_faction);
    }
    return relation;
}

string getName(const Unit *my_unit) {
    if (!my_unit) {
        return "";
    }
    return my_unit->name;
}

void setName(Unit *my_unit, string name) {
    if (!my_unit) {
        return;
    }
    my_unit->name = name;
}

void SetHull(Unit *my_unit, float newhull) {
    if (!my_unit) {
        return;
    }

    // Don't set hull to 0 (kill) when invulnerable
    if(my_unit->invulnerable && newhull <=0) {
        return;
    }

    my_unit->hull.Set(newhull);
}

float getCredits(const Unit *my_unit) {
    if (!my_unit) {
        return 0;
    }
    Cockpit *tmp;
    float viret = 0;
    if ((tmp = _Universe->isPlayerStarship(my_unit))) {
        viret = tmp->credits;
    }
    return viret;
}

void addCredits(const Unit *my_unit, float credits) {
    if (!my_unit) {
        return;
    }
    Cockpit *tmp;
    if ((tmp = _Universe->isPlayerStarship(my_unit))) {
        tmp->credits += credits;
    }
}

const string &getFlightgroupNameCR(const Unit *my_unit) {
    static const string empty_string("");
    if (!my_unit) {
        return empty_string;
    }
    Flightgroup *fg = my_unit->getFlightgroup();
    if (fg) {
        return fg->name;
    } else {
        return empty_string;
    }
}

string getFlightgroupName(const Unit *my_unit) {
    return getFlightgroupNameCR(my_unit);
}

Unit *getFlightgroupLeader(Unit *my_unit) {
    if (!my_unit) {
        return 0;
    }
    class Flightgroup *fg = my_unit->getFlightgroup();
    Unit *ret_unit = fg ? fg->leader.GetUnit() : my_unit;
    if (!ret_unit) {
        ret_unit = 0;
    }
    return ret_unit;
}

bool setFlightgroupLeader(Unit *my_unit, Unit *un) {
    if (!my_unit || !un) {
        return false;
    }
    if (my_unit->getFlightgroup()) {
        my_unit->getFlightgroup()->leader.SetUnit(un);
        return true;
    } else {
        return false;
    }
}

string getFgDirective(const Unit *my_unit) {
    return getFgDirectiveCR(my_unit);
}

static const string &getFgDirectiveCR(const Unit *my_unit) {
    static string emptystr;
    static string fgdirdef("b");
    if (!my_unit) {
        return emptystr;
    }
    if (my_unit->getFlightgroup()) {
        return my_unit->getFlightgroup()->directive;
    } else {
        return fgdirdef;
    }
}

bool setFgDirective(Unit *my_unit, string inp) {
    if (!my_unit) {
        return false;
    }
    if (my_unit->getFlightgroup() != NULL) {
        my_unit->getFlightgroup()->directive = inp;
        return true;
    }
    return false;
}

int getFgSubnumber(Unit *my_unit) {
    if (!my_unit) {
        return -1;
    }
    return my_unit->getFgSubnumber();
}

// erasezero not used anymore, but keeping it for compatibility
int removeCargo(Unit *my_unit, string s, int quantity, bool erasezero) {
    if (!my_unit) {
        return 0;
    }

    Cargo c = my_unit->cargo_hold.RemoveCargo(my_unit, s, quantity);
    return c.GetQuantity();
}

// TODO: I'm almost certain this is no longer relevant.
// Still need to investigate turrets and weapons.
void RecomputeUnitUpgrades(Unit *un) {
    if (un == NULL) {
        return;
    }
    un->ReduceToTemplate();

    for (Cargo& c : un->upgrade_space.getItems()) {
        assert(c.IsComponent());
        
        if(c.IsIntegral()) {
            continue;
        }
        
        un->Upgrade(c.GetName(), 0, 0, true, false);
    }
}

bool repair(Unit *my_unit) {
    if (!my_unit) {
        return false;
    }
    return my_unit->RepairUpgrade();
}

float upgrade(Unit *my_unit, string file, int mountoffset, int subunitoffset, bool force, bool loop_through_mounts) {
    if (!my_unit) {
        return 0;
    }
    double percentage = 0;
    percentage = my_unit->Upgrade(file, mountoffset, subunitoffset, force, loop_through_mounts);
    my_unit->SetTurretAI();
    return percentage;
}

int removeWeapon(Unit *my_unit, string name, int mountoffset, bool loop) {
    if (!my_unit) {
        return -1;
    }
    int maxmount = my_unit->mounts.size();
    int max = maxmount + mountoffset;
    for (int loopi = mountoffset; loopi < max; ++loopi) {
        int i = loopi % maxmount;
        if (my_unit->mounts[i].type->name == name
                && (my_unit->mounts[i].status == Mount::ACTIVE || my_unit->mounts[i].status == Mount::INACTIVE)) {
            my_unit->mounts[i].status = Mount::UNCHOSEN;
            return i;
        }
    }
    return -1;
}

int addCargo(Unit *my_unit, Cargo carg) {
    if (!my_unit) {
        return 0;
    }
 
    for (int i = carg.GetQuantity(); i > 0; i--) {
        carg.SetQuantity(i);
        if(my_unit->cargo_hold.CanAddCargo(carg)) {
            my_unit->cargo_hold.AddCargo(my_unit, carg);
            break;
        } 
    }

    return carg.GetQuantity();
}

int forceAddCargo(Unit *my_unit, Cargo carg) {
    if (!my_unit) {
        return 0;
    }

    // Force add cargo even if it exceeds the cargo hold capacity
    // This is used for missions and other special cases
    my_unit->cargo_hold.AddCargo(my_unit, carg);
    return carg.GetQuantity();
}

int hasCargo(const Unit *my_unit, string mycarg) {
    if (!my_unit) {
        return 0;
    }
    
    if(!my_unit->cargo_hold.HasCargo(mycarg)) {
        return 0;
    }

    int index = my_unit->cargo_hold.GetIndex(mycarg);
    if(index == -1) {
        return 0;
    }

    Cargo c = my_unit->cargo_hold.GetCargo(index);
    return c.GetQuantity();
}

bool JumpTo(Unit *unit, string system) {
    if (unit != NULL) {
        return unit->getStarSystem()->JumpTo(unit, NULL, system);
    } else {
        return false;
    }
}

string getUnitSystemFile(const Unit *un) {
    if (!un) {
        return _Universe->activeStarSystem()->getFileName();
    }
    const StarSystem *ss = un->getStarSystem();
    return ss->getFileName();
}



Cargo GetCargoIndex(const Unit *my_unit, int index) {
    if (!my_unit) {
        return Cargo();
    }

    if (index < 0 || index >= my_unit->cargo_hold.size()) {
        return Cargo();
    }
    return my_unit->cargo_hold.GetCargo(index);
}

Cargo GetCargo(const Unit *my_unit, std::string cargo_name) {
    if (!my_unit) {
        return Cargo();
    }

    int index = my_unit->cargo_hold.GetIndex(cargo_name);
    if (index == -1) {
        return Cargo();
    }

    return my_unit->cargo_hold.GetCargo(index);
}

bool isDockableUnit(const Unit *my_unit) {
    if (!my_unit) {
        return false;
    }
    return (
            (
                    my_unit->isPlanet()
                            && !isSun(my_unit)
                            && isSignificant(my_unit)
                            && !my_unit->isJumppoint()
            )
                    || (my_unit->getUnitType() == Vega_UnitType::unit)
                    || (getFlightgroupName(my_unit) == "Base")
    )
            && (my_unit->DockingPortLocations().size() > 0);
}

bool isCloseEnoughToDock(const Unit *my_unit, const Unit *un) {
    static bool
            superdock = XMLSupport::parse_bool(vs_config->getVariable("physics", "dock_within_base_shield", "false"));
    float dis =
            (un->getUnitType() == Vega_UnitType::planet || superdock) ? UnitUtil::getSignificantDistance(my_unit, un)
                    : UnitUtil::getDistance(
                    my_unit,
                    un);
    if (dis < un->rSize()) {
        return true;
    }
    return false;
}

float getDistance(const Unit *my_unit, const Unit *un) {
    if (my_unit == NULL || un == NULL) {
        return FLT_MAX;
    }
    return (my_unit->Position() - un->Position()).Magnitude() - my_unit->rSize() - un->rSize();
}

float getSignificantDistance(const Unit *un, const Unit *sig) {
    if (un == NULL || sig == NULL) {
        return FLT_MAX;
    }
    float dist = getDistance(un, sig);

    float planetpct = UniverseUtil::getPlanetRadiusPercent();
    if (sig->isPlanet()) {
        dist = dist - (sig->rSize() * planetpct);
    }
    if (un->isPlanet()) {
        dist = dist - (un->rSize() * planetpct);
    }
    return dist;
}

bool isSun(const Unit *my_unit) {
    if (!my_unit) {
        return false;
    }
    return my_unit->isPlanet() && ((Planet *) my_unit)->hasLights();
}

bool isSignificant(const Unit *my_unit) {
    if (!my_unit) {
        return false;
    }
    bool res = false;
    Vega_UnitType typ = my_unit->getUnitType();
    const string &s = getFlightgroupNameCR(my_unit);
    res = (typ == Vega_UnitType::planet || typ == Vega_UnitType::asteroid || typ == Vega_UnitType::nebula || s == "Base");
    return res && !isSun(my_unit);
}

int isPlayerStarship(const Unit *un) {
    return _Universe->whichPlayerStarship(un);
}

void setSpeed(Unit *my_unit, float speed) {
    if (my_unit) {
        my_unit->computer.set_speed = speed;
    }
}

float maxSpeed(const Unit *my_unit) {
    if (!my_unit) {
        return 0;
    }
    return my_unit->MaxSpeed();
}

float maxAfterburnerSpeed(const Unit *my_unit) {
    if (!my_unit) {
        return 0;
    }
    return my_unit->MaxAfterburnerSpeed();
}

void setECM(Unit *my_unit, int NewECM) {
    //short fix
    if (!my_unit) {
        return;
    }
    my_unit->ecm.Set(NewECM);
}

int getECM(const Unit *my_unit) {
    //short fix
    if (!my_unit) {
        return 0;
    }

    return my_unit->ecm.Get();
}

static bool ishere(const Unit *par, const Unit *look) {
    const Unit *un;
    for (un_kiter uniter = par->viewSubUnits(); (un = *uniter); ++uniter) {
        if (un == look || (un != par && ishere(un, look))) {
            return true;
        }
    }
    return false;
}

Unit *owner(const Unit *un) {
    Unit *tmp;
    for (UniverseUtil::PythonUnitIter uniter = UniverseUtil::getUnitList(); (tmp = *uniter); ++uniter) {
        if (tmp == un || ishere(tmp, un)) {
            return (tmp);
        }
    }
    return (NULL);
}

void performDockingOperations(Unit *un, Unit *unitToDockWith, int actually_dock) {
    if (un && unitToDockWith) {
        Order *ai = un->aistate;
        un->aistate = NULL;
        un->PrimeOrders(new Orders::DockingOps(unitToDockWith, ai, actually_dock, true));
    }
}

float PercentOperational(const Cargo item, Unit *un, std::string name, std::string category, bool countHullAndArmorAsFull) {
    if (!un) {
        return 0;
    }

    // New Code
    // TODO: Make actually return percent damaged

    // name is unit_key with stripped suffix. Need to add it again
    // TODO: check for prefixes: add_ mult_
    std::string unit_key = name;
    if(!boost::algorithm::ends_with(unit_key, "__upgrades")) {
        unit_key = name + "__upgrades";
    }

    const std::string upgrade_category = UnitCSVFactory::GetVariable(unit_key, "Upgrade_Type", std::string());
    if(upgrade_category == "Hull") {
        return un->hull.PercentOperational();
    }

    if(upgrade_category == "Armor") {
        return un->armor.PercentOperational();
    }

    if(upgrade_category == "Shield") {
        return un->shield.PercentOperational();
    }

    if(upgrade_category == "Afterburner") {
        return un->afterburner.PercentOperational();
    }

    if(upgrade_category == "Drive") {
        return un->drive.PercentOperational();
    }

    if(upgrade_category == "FTL_Drive") {
        return un->ftl_drive.PercentOperational();
    }

    if(upgrade_category == "Reactor") {
        return un->reactor.PercentOperational();
    }

    if(upgrade_category == "Capacitor") {
        return un->energy.PercentOperational();
    }

    if(upgrade_category == "FTL_Capacitor") {
        return un->ftl_energy.PercentOperational();
    }

    if(upgrade_category == "Jump_Drive") {
        return un->jump_drive.PercentOperational();
    }

    if(upgrade_category == "FTL_Drive") {
        return un->ftl_drive.PercentOperational();
    }

    if(upgrade_category == "Cloak") {
        return un->cloak.PercentOperational();
    }

    if(upgrade_category == "Radar") {
        return un->radar.PercentOperational();
    }
    if(upgrade_category == "ECM") {
        return un->ftl_drive.PercentOperational();
    }

    if(upgrade_category == "Repair_Bot") {
        return un->cloak.PercentOperational();
    }


    // Old Code
    if (category.find(DamagedCategory) == 0) {
        return 0.0f;
    }
    const Unit *upgrade = getUnitFromUpgradeName(name, un->faction);
    if (!upgrade) {
        return 1.0f;
    }
    if (item.IsWeapon()) {
        static std::string loadfailed("LOAD_FAILED");
        if (upgrade->getNumMounts()) {
            const Mount *mnt = &upgrade->mounts[0];
            unsigned int nummounts = un->getNumMounts();
            for (unsigned int i = 0; i < nummounts; ++i) {
                if (mnt->type->name == un->mounts[i].type->name) {
                    if (un->mounts[i].status == Mount::DESTROYED) {
                        return 0.0;
                    }
                    if (un->mounts[i].functionality < 1.0f) {
                        return un->mounts[i].functionality;
                    }
                }
            }
        }
    } else if (name.find("add_") != 0 && name.find("mult_") != 0) {
        double percent = 0;
        if (un->canUpgrade(upgrade, -1, -1, 0, true, percent, makeTemplateUpgrade(un->name, un->faction), false)) {
            if (percent > 0 && percent < 1) {
                return percent;
            } else if (percent
                    >= 1) { //FIXME workaround for sensors -- see below comment, not sure why sensors report erroneous functional percentage
                return 1.0;
            } else {
                return .5;
            } //FIXME does not interact well with radar type
        } else if (percent > 0) {
            return percent;
        }
    }
    return 1.0;
}

void setMissionRelevant(Unit *un) {
    if (un && mission->runtime.pymissions) {
        mission->runtime.pymissions->relevant_units.push_back(new UnitContainer(un));
    }
}

void unsetMissionRelevant(Unit *un) {
    if (un && mission->runtime.pymissions) {
        vector<UnitContainer *> *relevant = &mission->runtime.pymissions->relevant_units;
        auto i = relevant->begin();
        while (i != relevant->end()) {
            if (**i == un) {
                i = relevant->erase(i);
            } else {
                ++i;
            }
        }
    }
}

}

