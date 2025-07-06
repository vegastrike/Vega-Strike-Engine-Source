/*
 * carrier.cpp
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


#include "cmd/carrier.h"

#include "root_generic/configxml.h"

#include "cmd/unit_generic.h"
#include "cmd/ship.h"
#include "src/universe.h"
#include "src/universe_util.h"

#include "cmd/ai/aggressive.h"
#include "cmd/missile.h"
#include "src/vs_random.h"
#include "src/vs_logging.h"
#include "src/vega_cast_utils.h"


// TODO: find out where this is and maybe refactor
extern int SelectDockPort(Unit *, Unit *parent);
extern void SwitchUnits(Unit *, Unit *);
extern void PlayDockingSound(int dock);





// TODO: move these two functions to vector and make into single constructor
inline double uniformrand(double min, double max) {
    return ((double) (rand()) / RAND_MAX) * (max - min) + min;
}

inline QVector randVector(double min, double max) {
    return QVector(uniformrand(min, max),
            uniformrand(min, max),
            uniformrand(min, max));
}

Carrier::Carrier() {

}


//index in here is unsigned, UINT_MAX and UINT_MAX-1 seem to be
//special states.  This means the total amount of cargo any ship can have
//is UINT_MAX -3   which is 65532 for 32bit machines.
void Carrier::EjectCargo(unsigned int index) {
    Unit *unit = static_cast<Unit *>(this);
    Cargo *tmp = NULL;
    Cargo ejectedPilot;
    Cargo dockedPilot;
    string name;
    bool isplayer = false;
    //if (index==((unsigned int)-1)) { is ejecting normally
    //if (index==((unsigned int)-2)) { is ejecting for eject-dock

    Cockpit *cp = NULL;
    if (index == (UINT_MAX - 1)) {
//        _Universe->CurrentCockpit();
        //this calls the unit's existence, by the way.
        name = "return_to_cockpit";
        if (NULL != (cp = _Universe->isPlayerStarship(unit))) {
            isplayer = true;
        }
        //we will have to check for this on undock to return to the parent unit!
        dockedPilot.SetName("return_to_cockpit");
        dockedPilot.SetMass(.1);
        dockedPilot.SetVolume(1);
        tmp = &dockedPilot;
    }
    if (index == UINT_MAX) {
        int pilotnum = _Universe->CurrentCockpit();
        name = "Pilot";
        if (NULL != (cp = _Universe->isPlayerStarship(unit))) {
            string playernum = string("player") + ((pilotnum == 0) ? string("") : XMLSupport::tostring(pilotnum));
            isplayer = true;
        }
        ejectedPilot.SetName("eject");
        ejectedPilot.SetMass(.1);
        ejectedPilot.SetVolume(1);
        tmp = &ejectedPilot;
    }
    if (index < unit->cargo_hold.size()) {
        tmp = &unit->cargo_hold.GetCargo(index);
    }

    // Some sanity checks for tmp
    // Can't eject an upgrade, unless ship is destroyed
    if(tmp->IsInstalled() && !unit->hull.Destroyed()) {
        return;
    }

    // Can't eject cargo from the hidden hold unless ship is destroyed.
    // TODO: implement

    // Make sure ejected mass isn't 0. This causes game to mishandle
    if(tmp->GetMass() == 0) {
        tmp->SetMass(0.01);
    }

    static float cargotime = XMLSupport::parse_float(vs_config->getVariable("physics", "cargo_live_time", "600"));
    if (tmp) {
        string tmpcontent = tmp->GetName();
        if (tmp->IsMissionFlag()) {
            tmpcontent = "Mission_Cargo";
        }

        //prevents a number of bad things, incl. impossible speeds and people getting rich on broken stuff
        if ((!tmp->IsMissionFlag()) && tmp->IsComponent()) {
            tmpcontent = "Space_Salvage";
        }
        //this happens if it's a ship
        if (tmp->GetQuantity() > 0) {
            const int sslen = strlen("starships");
            Unit *cargo = NULL;
            if (tmp->GetCategory().length() >= (unsigned int) sslen) {
                if ((!tmp->IsMissionFlag()) && tmp->GetCategory() == "starships") {
                    string ans = tmpcontent;
                    string::size_type blank = ans.find(".blank");
                    if (blank != string::npos) {
                        ans = ans.substr(0, blank);
                    }
                    Flightgroup *fg = unit->getFlightgroup();
                    int fgsnumber = 0;
                    if (fg != NULL) {
                        fgsnumber = fg->nr_ships;
                        ++(fg->nr_ships);
                        ++(fg->nr_ships_left);
                    }
                    cargo = new Ship(ans.c_str(), false, unit->faction, "", fg, fgsnumber);
                    cargo->PrimeOrders();
                    cargo->SetAI(new Orders::AggressiveAI("default.agg.xml"));
                    cargo->SetTurretAI();
                    //he's alive!!!!!
                }
            }
            float arot = 0;
            const float grot =
                    configuration()->graphics.generic_cargo_rotation_speed * 3.1415926536 / 180;
            if (!cargo) {
                const float crot =
                        configuration()->graphics.cargo_rotation_speed * 3.1415926536 / 180;
                const float erot =
                        configuration()->graphics.eject_rotation_speed * 3.1415926536 / 180;
                if (tmpcontent == "eject") {
                    if (isplayer) {
                        Flightgroup *fg = unit->getFlightgroup();
                        int fgsnumber = 0;
                        if (fg != NULL) {
                            fgsnumber = fg->nr_ships;
                            ++(fg->nr_ships);
                            ++(fg->nr_ships_left);
                        }
                        cargo = new Unit("eject", false, unit->faction, "", fg, fgsnumber);
                    } else {
                        int fac = FactionUtil::GetUpgradeFaction();
                        cargo = new Unit("eject", false, fac, "", NULL, 0);
                    }
                    if (unit->owner) {
                        cargo->owner = unit->owner;
                    } else {
                        cargo->owner = unit;
                    }
                    arot = erot;
                    static bool eject_attacks =
                            XMLSupport::parse_bool(vs_config->getVariable("AI", "eject_attacks", "false"));
                    if (eject_attacks) {
                        cargo->PrimeOrders();
                        //generally fraidycat AI
                        cargo->SetAI(new Orders::AggressiveAI("default.agg.xml"));
                    }

                    //Meat. Docking should happen here
                } else if (tmpcontent == "return_to_cockpit") {
                    if (isplayer) {
                        Flightgroup *fg = unit->getFlightgroup();
                        int fgsnumber = 0;
                        if (fg != NULL) {
                            fgsnumber = fg->nr_ships;
                            ++(fg->nr_ships);
                            ++(fg->nr_ships_left);
                        }
                        cargo = new Unit("return_to_cockpit", false, unit->faction, "", fg, fgsnumber);
                        if (unit->owner) {
                            cargo->owner = unit->owner;
                        } else {
                            cargo->owner = unit;
                        }
                    } else {
                        int fac = FactionUtil::GetUpgradeFaction();
                        static float ejectcargotime =
                                XMLSupport::parse_float(vs_config->getVariable("physics", "eject_live_time", "0"));
                        if (cargotime == 0.0) {
                            cargo = new Unit("eject", false, fac, "", NULL, 0);
                        } else {
                            cargo = new Missile("eject",
                                    fac, "",
                                    0,
                                    0,
                                    ejectcargotime,
                                    1,
                                    1,
                                    1);
                        }
                    }
                    arot = erot;
                    cargo->PrimeOrders();
                    cargo->aistate = NULL;
                } else {
                    string tmpnam = tmpcontent + ".cargo";
                    static std::string nam("Name");
                    float rot = crot;
                    if (UniverseUtil::LookupUnitStat(tmpnam, "upgrades", nam).length() == 0) {
                        tmpnam = "generic_cargo";
                        rot = grot;
                    }
                    int upgrfac = FactionUtil::GetUpgradeFaction();
                    cargo = new Missile(tmpnam.c_str(),
                            upgrfac,
                            "",
                            0,
                            0,
                            cargotime,
                            1,
                            1,
                            1);
                    arot = rot;
                }
            }
            if (cargo->name == "LOAD_FAILED") {
                cargo->Kill();
                cargo = new Missile("generic_cargo",
                        FactionUtil::GetUpgradeFaction(), "",
                        0,
                        0,
                        cargotime,
                        1,
                        1,
                        1);
                arot = grot;
            }
            Vector rotation
                    (vsrandom.uniformInc(-arot, arot), vsrandom.uniformInc(-arot, arot), vsrandom.uniformInc(-arot,
                            arot));
            const bool all_rotate_same = configuration()->graphics.cargo_rotates_at_same_speed;
            if (all_rotate_same && arot != 0) {
                float tmp = rotation.Magnitude();
                if (tmp > .001) {
                    rotation.Scale(1 / tmp);
                    rotation *= arot;
                }
            }
            if (0 && cargo->rSize() >= unit->rSize()) {
                cargo->Kill();
            } else {
                Vector tmpvel = -unit->Velocity;
                if (tmpvel.MagnitudeSquared() < .00001) {
                    tmpvel = randVector(-unit->rSize(), unit->rSize()).Cast();
                    if (tmpvel.MagnitudeSquared() < .00001) {
                        tmpvel = Vector(1, 1, 1);
                    }
                }
                tmpvel.Normalize();
                if ((SelectDockPort(unit, unit) > -1)) {
                    static float eject_cargo_offset =
                            XMLSupport::parse_float(vs_config->getVariable("physics", "eject_distance", "20"));
                    QVector loc(Transform(unit->GetTransformation(),
                            unit->DockingPortLocations()[0].GetPosition().Cast()));
                    //index is always > -1 because it's unsigned.  Lets use the correct terms, -1 in Uint is UINT_MAX
                    loc += tmpvel * 1.5 * unit->rSize()
                            + randVector(-.5 * unit->rSize() + (index == UINT_MAX ? eject_cargo_offset / 2 : 0),
                                    .5 * unit->rSize() + (index == UINT_MAX ? eject_cargo_offset : 0));
                    cargo->SetPosAndCumPos(loc);
                    Vector p, q, r;
                    unit->GetOrientation(p, q, r);
                    cargo->SetOrientation(p, q, r);
                    if (unit->owner) {
                        cargo->owner = unit->owner;
                    } else {
                        cargo->owner = unit;
                    }
                } else {
                    cargo->SetPosAndCumPos(unit->Position() + tmpvel * 1.5 * unit->rSize()
                            + randVector(-.5 * unit->rSize(), .5 * unit->rSize()));
                    cargo->SetAngularVelocity(rotation);
                }
                static float
                        velmul = XMLSupport::parse_float(vs_config->getVariable("physics", "eject_cargo_speed", "1"));
                cargo->SetOwner(unit);
                cargo->SetVelocity(unit->Velocity * velmul + randVector(-.25, .25).Cast());
                cargo->setMass(tmp->GetMass());
                if (name.length() > 0) {
                    cargo->name = name;
                } else if (tmp) {
                    cargo->name = tmpcontent;
                }
                if (cp && _Universe->numPlayers() == 1) {
                    cargo->SetOwner(NULL);
                    unit->PrimeOrders();
                    cargo->SetTurretAI();
                    cargo->faction = unit->faction;
                    //changes control to that cockpit
                    cp->SetParent(cargo, "", "", unit->Position());
                    if (tmpcontent == "return_to_cockpit") {
                        static bool simulate_while_at_base =
                                XMLSupport::parse_bool(vs_config->getVariable("physics",
                                        "simulate_while_docked",
                                        "false"));
                        if ((simulate_while_at_base) || (_Universe->numPlayers() > 1)) {
                            unit->TurretFAW();
                        }
                        //make unit a sitting duck in the mean time
                        SwitchUnits(NULL, unit);
                        if (unit->owner) {
                            cargo->owner = unit->owner;
                        } else {
                            cargo->owner = unit;
                        }
                        unit->PrimeOrders();
                        cargo->SetOwner(unit);
                        cargo->Position() = unit->Position();
                        cargo->SetPosAndCumPos(unit->Position());
                        //claims to be docked, stops speed and taking damage etc. but doesn't seem to call the base script
                        cargo->ForceDock(unit, 0);
                        PlayDockingSound(3);
                        //actually calls the interface, meow. yay!
                        cargo->UpgradeInterface(unit);
                        if ((simulate_while_at_base) || (_Universe->numPlayers() > 1)) {
                            unit->TurretFAW();
                        }
                    } else {
                        SwitchUnits(NULL, cargo);
                        if (unit->owner) {
                            cargo->owner = unit->owner;
                        } else {
                            cargo->owner = unit;
                        }
                    }                                            //switching NULL gives "dead" ai to the unit I ejected from, by the way.
                }
                _Universe->activeStarSystem()->AddUnit(cargo);
                if ((unsigned int) index != ((unsigned int) -1) && (unsigned int) index != ((unsigned int) -2)) {
                    if (index < unit->cargo.size()) {
                        unit->cargo_hold.RemoveCargo(unit, index, 1);
                    }
                }
            }
        }
    }
}


float Carrier::PriceCargo(const std::string &s) {
    Unit *unit = static_cast<Unit *>(this);

    Cargo tmp;
    tmp.SetName(s);
    vector<Cargo>::iterator mycargo = std::find(unit->cargo.begin(),
            unit->cargo.end(), tmp);
    if (mycargo == unit->cargo.end()) {
        Unit *mpl = getMasterPartList();
        if (this != mpl) {
            return mpl->PriceCargo(s);
        } else {
            const float spacejunk = configuration()->cargo.space_junk_price;
            return spacejunk;
        }
    }
    float price;
    price = (*mycargo).GetPrice();
    return price;
}
unsigned int Carrier::numCargo() const {
    const Unit *unit = vega_dynamic_cast_ptr<const Unit>(this);
    return unit->cargo.size();
}

std::string Carrier::GetManifest(unsigned int i, Unit *scanningUnit, const Vector &oldspd) const {
    const Unit *unit = vega_dynamic_cast_ptr<const Unit>(this);

    ///FIXME somehow mangle string
    string mangled = unit->cargo[i].GetName();
    const float scramblingmanifest = configuration()->general.percentage_speed_change_to_fault_search;
    {
        //Keep inside subblock, otherwise MSVC will throw an error while redefining 'i'
        bool last = true;
        for (string::iterator i = mangled.begin(); i != mangled.end(); ++i) {
            if (last) {
                (*i) = toupper(*i);
            }
            last = (*i == ' ' || *i == '_');
        }
    }
    if (unit->CourseDeviation(oldspd, unit->GetVelocity()) > scramblingmanifest) {
        for (string::iterator i = mangled.begin(); i != mangled.end(); ++i) {
            (*i) += (rand() % 3 - 1);
        }
    }
    return mangled;
}

