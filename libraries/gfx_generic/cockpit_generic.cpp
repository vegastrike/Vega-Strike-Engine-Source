/*
 * cockpit_generic.cpp
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


#include "src/in.h"
#include "vegadisk/vsfilesystem.h"
#include "root_generic/vs_globals.h"
#include "src/vegastrike.h"
#include "gfx_generic/cockpit_generic.h"
#include "cmd/unit_generic.h"
#include "cmd/unit_util.h"
#include "cmd/collection.h"
#include "root_generic/lin_time.h" //for fps
#include "root_generic/configxml.h"
#include "cmd/images.h"
#include "cmd/script/mission.h"
#include "cmd/ai/aggressive.h"
#include <assert.h>     //needed for assert() calls
#include "vegadisk/savegame.h"
#include "src/universe_util.h"
#include "cmd/ai/fire.h"
#include "gfx/background.h"
#include "src/save_util.h"
#include "src/star_system.h"
#include "src/universe.h"
#include "cmd/mount_size.h"
#include "cmd/weapon_info.h"
#include "components/player_ship.h"
#include "resource/manifest.h"

#include <algorithm>

#define SWITCH_CONST (0.9)

vector<int> respawnunit;
vector<int> switchunit;

void Cockpit::beginElement(void *userData, const XML_Char *name, const XML_Char **atts) {
    ((Cockpit *) userData)->beginElement(name, AttributeList(atts));
}

void Cockpit::endElement(void *userData, const XML_Char *name) {
    ((Cockpit *) userData)->endElement(name);
}

float Unit::computeLockingPercent() {
    float most = -1024;
    for (int i = 0; i < getNumMounts(); i++) {
        if (mounts[i].type->type == WEAPON_TYPE::PROJECTILE
                || (as_integer(mounts[i].type->size)
                        & (as_integer(MOUNT_SIZE::SPECIALMISSILE) |
                                as_integer(MOUNT_SIZE::LIGHTMISSILE) |
                                as_integer(MOUNT_SIZE::MEDIUMMISSILE) |
                                as_integer(MOUNT_SIZE::HEAVYMISSILE) |
                                as_integer(MOUNT_SIZE::CAPSHIPLIGHTMISSILE) |
                                as_integer(MOUNT_SIZE::CAPSHIPHEAVYMISSILE) |
                                as_integer(MOUNT_SIZE::SPECIAL)))) {
            if (mounts[i].status == Mount::ACTIVE && mounts[i].type->lock_time > 0) {
                float rat = mounts[i].time_to_lock / mounts[i].type->lock_time;
                if (rat < .99) {
                    if (rat > most) {
                        most = rat;
                    }
                }
            }
        }
    }
    return (most == -1024) ? 1 : most;
}

void Cockpit::Eject() {
    ejecting = true;
    going_to_dock_screen = false;
}

void Cockpit::EjectDock() {
    ejecting = true;
    going_to_dock_screen = true;
}

using namespace VSFileSystem;

void Cockpit::Init(const char *file, bool isDisabled) {
    retry_dock = 0;
    shakin = 0;
    autopilot_time = 0;
    bool f404 = false;
    if (file == NULL || strlen(file) == 0) {
        if (isDisabled) {
            file = "cockpit.cpt";
        } else {
            file = "disabled-cockpit.cpt";
        }
        f404 = true;
    }
    if (isDisabled == true) {
        std::string disname = std::string("disabled-") + file;
        Init(disname.c_str());
        return;
    }
    VSFile f;
    VSError err = f.OpenReadOnly(file, CockpitFile);
    if (err > Ok) {
        //File not found...
        if (isDisabled == false && (string(file).find("disabled-") == string::npos)) {
            Init(file, true);
        } else if (!f404) {
            Init(NULL);
        }
        return;
    }
    Delete();
    LoadXML(f);
    f.Close();
}

// A bad function name. 
// This function potentially returns the current subunit, so its target can be extracted.
// See star_system.cpp
Unit *Cockpit::GetSaveParent() {
    return current_unit;
}

void Cockpit::SetParent(Unit *unit, const QVector &position) {
    VS_LOG(debug, (boost::format("SetParent: full name: %1% filename: %2%")
                        % unit->getFullname() % unit->getFilename() ));
    
    if (unit->getFlightgroup() != nullptr) {
        fg = unit->getFlightgroup();
    }

    activeStarSystem = _Universe->activeStarSystem();          //cannot switch to units in other star systems.
    parent.SetUnit(unit);
    unit->SetPlayerShip(); // This is the place to set it.
    savegame->SetPlayerLocation(position);
    
    if (unit) {
        this->unitfaction = unit->faction;
    }
}

void Cockpit::Delete() {
    //int i;
    viewport_offset = cockpit_offset = 0;
}

void Cockpit::RestoreGodliness() {
    const float maxgodliness = configuration().physics.player_godliness_flt;
    godliness = maxgodliness;
    if (godliness > maxgodliness) {
        godliness = maxgodliness;
    }
}

void Cockpit::InitStatic() {
    radar_time = 0;
    cockpit_time = 0;
}

bool Cockpit::unitInAutoRegion(Unit *un) {
    const float autopilot_term_distance = configuration().physics.auto_pilot_termination_distance_flt;
    Unit *targ = autopilot_target.GetUnit();
    if (targ) {
        return UnitUtil::getSignificantDistance(un, targ)
                < autopilot_term_distance * 2.5;          //if both guys just auto'd in.
    } else {
        return false;
    }
}

static float getInitialZoomFactor() {
    const float inizoom = configuration().graphics.initial_zoom_factor_flt;
    return inizoom;
}

Cockpit::Cockpit(const char *file, Unit *parent, const std::string &pilot_name)
        : view(CP_FRONT),
        parent(parent),
        cockpit_offset(0),
        viewport_offset(0),
        zoomfactor(getInitialZoomFactor()),
        savegame(new SaveGame(pilot_name)) {
    partial_number_of_attackers = -1;
    number_of_attackers = 0;
    fg = NULL;
    jumpok = 0;
    TimeOfLastCollision = -200;
    activeStarSystem = NULL;
    InitStatic();
    //mesh=NULL;
    ejecting = false;
    currentcamera = 0;
    going_to_dock_screen = false;
    RestoreGodliness();

    Init(file);
}


void Cockpit::recreate(const std::string &pilot_name) {
    savegame->SetCallsign(pilot_name);
    Init("");
}

static void FaceTarget(Unit *un) {
    Unit *targ = un->Target();
    if (targ) {
        QVector dir(targ->Position() - un->Position());
        dir.Normalize();
        Vector p, q, r;
        un->GetOrientation(p, q, r);
        QVector qq(q.Cast());
        qq = qq + QVector(.001, .001, .001);
        un->SetOrientation(qq, dir);
    }
}

int Cockpit::Autopilot(Unit *target) {
    int retauto = 0;
    if (target) {
        Unit *un = NULL;
        if ((un = GetParent())) {
            if ((retauto = un->AutoPilotTo(un, false))) {
                //can he even start to autopilot
                //SetView (CP_PAN);
                un->AutoPilotTo(target, false);
                const bool face_target_on_auto = configuration().physics.face_target_on_auto;
                if (face_target_on_auto) {
                    FaceTarget(un);
                }
                static double averagetime = GetElapsedTime() / getTimeCompression();
                static double numave = 1.0;
                averagetime += GetElapsedTime() / getTimeCompression();
                //const float autospeed = configuration().physics.autospeed;//10 seconds for auto to kick in;
                numave++;
                /*
                 *  AccessCamera(CP_PAN)->myPhysics.SetAngularVelocity(Vector(0,0,0));
                 *  AccessCamera(CP_PAN)->myPhysics.ApplyBalancedLocalTorque(_Universe->AccessCamera()->P,
                 *                                                     _Universe->AccessCamera()->R,
                 *                                                     averagetime*autospeed/(numave));
                 */
                const float initialzoom = configuration().graphics.initial_zoom_factor_flt;
                zoomfactor = initialzoom;
                const float autotime = configuration().physics.auto_time_in_seconds_flt;                 //10 seconds for auto to kick in;

                autopilot_time = autotime;
                autopilot_target.SetUnit(target);
            }
        }
    }
    return retauto;
}

extern void SwitchUnits2(Unit *nw);

void SwitchUnits(Unit *ol, Unit *nw) {
    bool pointingtool = false;
    for (unsigned int i = 0; i < _Universe->numPlayers(); ++i) {
        if (i != _Universe->CurrentCockpit()) {
            if (_Universe->AccessCockpit(i)->GetParent() == ol) {
                pointingtool = true;
            }
        }
    }
    if (ol && (!pointingtool)) {
        Unit *oltarg = ol->Target();
        if (oltarg) {
            if (ol->getRelation(oltarg) >= 0) {
                ol->Target(NULL);
            }
        }
        ol->PrimeOrders();
        ol->SetAI(new Orders::AggressiveAI("default.agg.xml"));
        ol->SetVisible(true);
    }
    SwitchUnits2(nw);
}


Unit *GetFinalTurret(Unit *baseTurret) {
    Unit *un = baseTurret;
    Unit *tur;
    for (un_iter uj = un->getSubUnits(); (tur = *uj); ++uj) {
        SwitchUnits(NULL, tur);
        un = GetFinalTurret(tur);
    }
    return un;
}

void Cockpit::UpdAutoPilot() {
    if (autopilot_time != 0) {
        autopilot_time -= SIMULATION_ATOM;
        if (autopilot_time <= 0) {
            autopilot_time = 0;
            Unit *par = GetParent();
            if (par) {
                Unit *autoun = autopilot_target.GetUnit();
                autopilot_target.SetUnit(NULL);
                if (autoun) {
                    par->AutoPilotTo(autoun, false);
                }
            }
        }
    }
}

extern void DoCockpitKeys();

static float dockingdistance(Unit *port, Unit *un) {
    vector<DockingPorts>::const_iterator i = port->GetImageInformation().dockingports.begin();
    vector<DockingPorts>::const_iterator end = port->GetImageInformation().dockingports.end();
    QVector pos(InvTransform(port->cumulative_transformation_matrix, un->Position()));
    float mag = FLT_MAX;
    for (; i != end; ++i) {
        float tmag = (pos.Cast() - (*i).GetPosition()).Magnitude() - un->rSize() - (*i).GetRadius();
        if (tmag < mag) {
            mag = tmag;
        }
    }
    if (mag == FLT_MAX) {
        return UnitUtil::getDistance(port, un);
    }
    return mag;
}

static bool too_many_attackers = false;

bool Cockpit::tooManyAttackers() {
    return too_many_attackers;
}

void Cockpit::updateAttackers() {
    const int max_attackers = configuration().ai.max_player_attackers;
    if (max_attackers == 0) {
        return;
    }
    static un_iter attack_iterator = _Universe->activeStarSystem()->getUnitList().createIterator();
    static StarSystem *ss = _Universe->activeStarSystem();
    if (ss != _Universe->activeStarSystem()) {
        attack_iterator = _Universe->activeStarSystem()->getUnitList().createIterator();
        ss = _Universe->activeStarSystem();
    }
    bool isDone = attack_iterator.isDone();
    if (_Universe->AccessCockpit(_Universe->numPlayers() - 1) == this) {
        if (!isDone) {
            ++attack_iterator;
        } else {
            attack_iterator = _Universe->activeStarSystem()->getUnitList().createIterator();
        }
        //too_many_attackers=false;
    }
    if (isDone) {
        if (_Universe->AccessCockpit(0) == this) {
            too_many_attackers = false;
        }
        number_of_attackers = partial_number_of_attackers;         //reupdate the count
        partial_number_of_attackers = 0;
        too_many_attackers = max_attackers > 0 && (too_many_attackers || number_of_attackers > max_attackers);
    }
    Unit *un;
    if (attack_iterator.isDone() == false && (un = *attack_iterator) != NULL) {
        Unit *targ = un->Target();
        float speed = 0, range = 0, mmrange = 0;
        if (parent == targ && targ != NULL) {
            un->getAverageGunSpeed(speed, range, mmrange);
            float dist = UnitUtil::getDistance(targ, un);
            if (dist <= range * 2 || dist <= mmrange) {
                partial_number_of_attackers += 1;
            }
        }
    } else {
        //

        //partial_number_of_attackers=-1;
    }
}

bool Cockpit::Update() {
    if (retry_dock) {
        QVector vec;
        DockToSavedBases(_Universe->CurrentCockpit(), vec);
    }
    if (jumpok) {
        jumpok++;
    }
    if (jumpok > 10) {
        jumpok = 0;
    }
    UpdAutoPilot();
    Unit *par = GetParent();

    const bool autoclear = configuration().ai.auto_dock;
    if (autoclear && par) {
        Unit *targ = par->Target();
        if (targ) {
            const float autopilot_term_distance = configuration().physics.auto_pilot_termination_distance_flt;
            float doubled = dockingdistance(targ, par);
            if (((targ->getUnitType() != Vega_UnitType::planet
                    && doubled < autopilot_term_distance)
                    || (UnitUtil::getSignificantDistance(targ,
                            par) <= 0))
                    && (!(par->IsCleared(targ) || targ->IsCleared(par) || par->isDocked(targ)
                            || targ->isDocked(par))) && (par->getRelation(targ) >= 0)
                    && (targ->getRelation(par) >= 0)) {
                if (targ->getUnitType() != Vega_UnitType::planet || targ->GetDestinations().empty()) {
                    RequestClearence(par, targ, 0);
                }                      //sex is always 0... don't know how to	 get it.
            } else if ((par->IsCleared(targ)
                    || targ->IsCleared(par)) && (!(par->isDocked(targ)) || targ->isDocked(par))
                    && ((targ->getUnitType() == Vega_UnitType::planet && UnitUtil::getSignificantDistance(par, targ) > 0)
                            || ((targ->getUnitType() != Vega_UnitType::planet
                                    && UnitUtil::getSignificantDistance(par, targ) > (targ->rSize() + par->rSize()))
                                    && (doubled >= autopilot_term_distance)))) {
                if (targ->getUnitType() != Vega_UnitType::planet || targ->GetDestinations().empty()) {
                    par->EndRequestClearance(targ);
                    targ->EndRequestClearance(par);
                }
            }
        }
    }
    if (switchunit.size() > _Universe->CurrentCockpit()) {
        if (switchunit[_Universe->CurrentCockpit()]) {
            const float initialzoom = configuration().graphics.initial_zoom_factor_flt;
            zoomfactor = initialzoom;
            static int index = 0;
            switchunit[_Universe->CurrentCockpit()] = 0;
            const bool switch_nonowned_units = configuration().ai.switch_nonowned_units;
//switch_nonowned_units = true;
            //const bool switch_to_fac=configuration().ai.switch_to_whole_faction;

            Unit *un;
            bool found = false;
            int i = 0;
            for (un_iter ui = _Universe->activeStarSystem()->getUnitList().createIterator(); (un = *ui); ++ui) {
                if (un->faction == this->unitfaction) {
//this switches units UNLESS we're an ejected pilot. Instead, if we are an ejected
//pilot, switch only if we're close enough.
//the trigger is to allow switching only between ships that are actually owned by you, this prevents
//stealing a ship from a hired wingman.
                    if ((((par != NULL)
                            && (i++) >= index)
                            || par == NULL) && (!un->IsPlayerShip())
                            && (switch_nonowned_units
                                    || (par != NULL
                                            && un->owner == par->owner)
                                    || (par != NULL
                                            && un == par->owner)
                                    || (par != NULL
                                            && un->owner == par)
                                    || (par == NULL
                                            && un->owner)) && (un->name != "eject") && (un->name != "Pilot")
                            && (un->getUnitType() != Vega_UnitType::missile)) {
                        found = true;
                        ++index;
                        Unit *k = GetParent();
                        bool proceed = true;
                        if (k) {
                            if (k->name == "eject" || k->name == "Pilot" || k->name == "return_to_cockpit") {
                                proceed = false;
                            }
                        }
                        //we are an ejected pilot, so, if we can get close enough to the related unit, jump into it and remove the seat. This said, always allow
                        //switching from the "fake" ejection seat (ejectdock).
                        if (!proceed && k
                                && (k->Position() - un->Position()).Magnitude() < (un->rSize() + 5 * k->rSize())) {
                            if (!(k->name == "return_to_cockpit")) {
                                SwitchUnits(k, un);
                            }
                            //this refers to cockpit
                            if (!(k->name == "return_to_cockpit")) {
                                VS_LOG(debug, "Cockpit::Update() return_to_cockpit 2");
                                this->SetParent(un, savegame->GetPlayerLocation());
                            }
                            if (!(k->name == "return_to_cockpit")) {
                                k->Kill();
                            }
                        }
                        if (proceed) {
                            SwitchUnits(k, un);
                            VS_LOG(debug, "Cockpit::Update() proceed");
                            this->SetParent(un, savegame->GetPlayerLocation());
                        }
                        break;
                    }
                }
            }
            if (!found) {
                index = 0;
            }
        }
    }
    //this causes the physical ejecting. Check going_to_dock_screen in here, also.
    if (ejecting) {
        ejecting = false;
//going_to_dock_screen=true; // NO, clear this only after we've UNDOCKED that way we know we don't have issues.

        Unit *un = GetParent();
        if (un) {
            if (going_to_dock_screen == false) {
                un->EjectCargo((unsigned int) -1);
            }
            if (going_to_dock_screen == true) {
                un->EjectCargo((unsigned int) -2);
                going_to_dock_screen = false;
            }
        }
    }
    if (!par) {
        if (respawnunit.size() > _Universe->CurrentCockpit()) {
            if (respawnunit.at(_Universe->CurrentCockpit())) {
                VS_LOG(debug, "respawnunit.at(_Universe->CurrentCockpit()) is truthy");
                const float initialzoom = configuration().graphics.initial_zoom_factor_flt;
                zoomfactor = initialzoom;

                respawnunit.at(_Universe->CurrentCockpit()) = 0;
                std::string savegamefile = mission->getVariable("savegame", "");
                unsigned int k;
                for (k = 0; k < _Universe->numPlayers(); ++k) {
                    if (_Universe->AccessCockpit(k) == this) {
                        break;
                    }
                }
                if (k == _Universe->numPlayers()) {
                    k = 0;
                }
                if (active_missions.size() > 1) {
                    for (int i = active_missions.size() - 1; i > 0;
                            --i) {                          //don't terminate zeroth mission
                        if (active_missions[i]->player_num == k) {
                            active_missions[i]->terminateMission();
                        }
                    }
                }
                unsigned int whichcp = k;
                string newsystem;
                QVector pos;
                bool setplayerXloc;
                savegame->SetStarSystem("");
                QVector tmpoldpos = savegame->GetPlayerLocation();
                savegame->SetPlayerLocation(QVector(FLT_MAX, FLT_MAX, FLT_MAX));
                vector<string> packedInfo;
                ComponentsManager::credits = savegame->ParseSaveGame(savegamefile,
                        newsystem,
                        newsystem,
                        pos,
                        setplayerXloc,
                        packedInfo,
                        k);
                UnpackUnitInfo(packedInfo);
                if (pos.i == FLT_MAX && pos.j == FLT_MAX && pos.k == FLT_MAX) {
                    pos = tmpoldpos;
                }
                savegame->SetPlayerLocation(pos);
                CopySavedShips(savegame->GetCallsign(), whichcp, packedInfo, true);
                bool actually_have_save = false;
                const bool persistent_on_load = configuration().physics.persistent_on_load;
                if (savegame->GetStarSystem() != "") {
                    actually_have_save = true;
                    newsystem = savegame->GetStarSystem() + ".system";
                } else {
                    newsystem = _Universe->activeStarSystem()->getFileName();
                    if (newsystem.find(".system") == string::npos) {
                        newsystem += ".system";
                    }
                }
                Background::BackgroundClone savedtextures = {
                        {NULL, NULL, NULL, NULL, NULL, NULL, NULL}
                };
                if (persistent_on_load) {
                    _Universe->getActiveStarSystem(0)->SwapOut();
                } else {
                    Background *tmp = _Universe->activeStarSystem()->getBackground();
                    savedtextures = tmp->Cache();
                    _Universe->clearAllSystems();
                }
                StarSystem *ss = _Universe->GenerateStarSystem(newsystem.c_str(), "", Vector(0, 0, 0));
                if (!persistent_on_load) {
                    savedtextures.FreeClone();
                }
                this->activeStarSystem = ss;
                _Universe->pushActiveStarSystem(ss);

                vector<StarSystem *> saved;
                while (_Universe->getNumActiveStarSystem()) {
                    saved.push_back(_Universe->activeStarSystem());
                    _Universe->popActiveStarSystem();
                }
                if (!saved.empty()) {
                    saved.back() = ss;
                }
                unsigned int mysize = saved.size();
                for (unsigned int i = 0; i < mysize; i++) {
                    _Universe->pushActiveStarSystem(saved.back());
                    saved.pop_back();
                }
                ss->SwapIn();
                int fgsnumber = 0;
                if (fg) {
                    fgsnumber = fg->flightgroup_nr++;
                    fg->nr_ships++;
                    fg->nr_ships_left++;
                }

                // This code is suspicious. We are creating a new unit while we already have one in fleet.
                std::string unit_filename = PlayerShip::GetActiveShip().GetName();
                Unit *un = new Unit(unit_filename.c_str(), false, this->unitfaction, "player", fg, fgsnumber);
                un->SetCurPosition(UniverseUtil::SafeEntrancePoint(savegame->GetPlayerLocation()));
                ss->AddUnit(un);

                this->SetParent(un, savegame->GetPlayerLocation());
                SwitchUnits(NULL, un);
                DoCockpitKeys();
                _Universe->popActiveStarSystem();
                _Universe->pushActiveStarSystem(ss);
                savegame->ReloadPickledData();
                savegame->LoadSavedMissions();
                if (actually_have_save) {
                    QVector vec;
                    DockToSavedBases(whichcp, vec);
                }
                UniverseUtil::hideSplashScreen();
                _Universe->popActiveStarSystem();
                if (!persistent_on_load) {
                    _Universe->pushActiveStarSystem(ss);
                }
                return true;

            }
        }
    }
    return false;
}

void visitSystemHelp(Cockpit *cp, string systemname, float num) {
    string key(string("visited_") + systemname);
    vector<float> *v = &cp->savegame->getMissionData(key);
    if (v->empty()) {
        v->push_back(num);
    } else if ((*v)[0] != 1.0 && num == 1) {
        (*v)[0] = num;
    }
}

void Cockpit::visitSystem(string systemname) {
    visitSystemHelp(this, systemname, 1.0);
    int adj = UniverseUtil::GetNumAdjacentSystems(systemname);
    for (int i = 0; i < adj; ++i) {
        visitSystemHelp(this, UniverseUtil::GetAdjacentSystem(systemname, i), 0.0);
    }
}

Cockpit::~Cockpit() {
    Cockpit::Delete();
    if (savegame != nullptr) {
        delete savegame;
        savegame = nullptr;
    }
}

void Cockpit::SetInsidePanYawSpeed(float) {
    // No-op in generic cockpits
}

void Cockpit::SetInsidePanPitchSpeed(float) {
    // No-op in generic cockpits
}

void Cockpit::PackUnitInfo(vector<std::string> &info) const {
    info.clear();

    // Nothing to pack
    if(player_fleet.size() == 0) {
        return;
    }

    // First entry, current ship
    const std::string active_ship_name = PlayerShip::GetActiveShip().cargo.GetName();
    info.push_back(active_ship_name);

    // Following entries, ship/location pairs
    int i=0;
    for (PlayerShip& ship : player_fleet) {
        // We already wrote the active ship
        if(ship.active) {
            continue;
        }
        i++;

        info.push_back(ship.cargo.GetName());
        info.push_back(ship.system + "@" + ship.base);
    }
}


static void pushShipToFleet(bool active, 
                            const std::string& filename, 
                            const std::string& system,
                            const std::string& base,
                            const int faction,
                            Flightgroup *flight_group,
                            int fgsnumber) {
    Unit *unit = new Unit(filename.c_str(), false, faction, "player", flight_group, fgsnumber);
    Cargo cargo;
    try {
        cargo = Manifest::MPL().GetCargoByName(filename);
    } catch (const std::exception &e) {
        VS_LOG(warning, (boost::format("Failed to load player ship %1%: %2%.\nGenerating dummy cargo entry.") % filename % e.what()));
        cargo = Cargo(filename, "Spaceship", 1, 1, 1, 1);
    }
    PlayerShip player_ship(active, unit, "", "", filename, 0, 0, 0);
    player_fleet.push_back(player_ship);
}

void Cockpit::UnpackUnitInfo(vector<std::string> &info) {
    player_fleet.clear();

    // Nothing to unpack
    if(info.empty()) {
        return;
    }

    int fgsnumber = 0;
    if (fg) {
        fgsnumber = fg->flightgroup_nr++;
        fg->nr_ships++;
        fg->nr_ships_left++;
    }

    // Parse first
    std::string filename = info[0];
    pushShipToFleet(true, filename, "", "", this->unitfaction, fg, fgsnumber);
    

    // Following entries, ship/location pairs
    for (size_t i = 1, n = info.size(); i < n; i += 2) {
        if (fg) {
            fgsnumber = fg->flightgroup_nr++;
            fg->nr_ships++;
            fg->nr_ships_left++;
        }

        std::string filename = info[i];

        string location = ((i + 1) < n) ? info[i + 1] : "";
        string::size_type atpos = location.find_first_of('@');

        std::string system = location.substr(0, atpos);
        std::string base = ((atpos != string::npos) ? location.substr(atpos + 1) : "");
        std::cout << std::endl << std::endl << filename << " " << system << " " << base << std::endl << std::endl;
        pushShipToFleet(false, filename, system, base, this->unitfaction, fg, fgsnumber);
    }
}


string Cockpit::MakeBaseName(const Unit *base) {
    string name;
    if (base != NULL) {
        if (base->getFlightgroup() != NULL) {
            name = base->getFlightgroup()->name + ':';
        }
        name += base->getFullname();
        if (base->getFgSubnumber() > 0) {
            name += ':' + XMLSupport::tostring(base->getFgSubnumber());
        }
    }

    // remove all whitespace, it breaks savegames
    std::replace(name.begin(), name.end(), ' ', '_');

    return name;
}

SoundContainer *Cockpit::GetSoundForEvent(Cockpit::EVENTID eventId) const {
    if (eventId < sounds.size()) {
        return sounds[eventId];
    } else {
        return NULL;
    }
}

void Cockpit::SetSoundForEvent(Cockpit::EVENTID eventId, const SoundContainer &soundSpecs) {
    while (eventId >= sounds.size()) {
        sounds.push_back(NULL);
    }
    sounds[eventId] = soundImpl(soundSpecs);
}

SoundContainer *Cockpit::soundImpl(const SoundContainer &specs) {
    return new SoundContainer(specs);
}

