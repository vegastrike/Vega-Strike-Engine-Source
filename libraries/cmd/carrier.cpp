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
#include "resource/random_utils.h"
#include "src/vs_random.h"
#include "src/vs_logging.h"
#include "src/vega_cast_utils.h"


// TODO: find out where this is and maybe refactor
extern int SelectDockPort(Unit *, Unit *parent);
extern void SwitchUnits(Unit *, Unit *);
extern void PlayDockingSound(int dock);

constexpr float M_PI_FLT = M_PI;



// TODO: move these two functions to vector and make into single constructor
inline double uniformrand(double min, double max) {
    return (static_cast<double>(rand()) / RAND_MAX) * (max - min) + min;
}

inline QVector randVector(double min, double max) {
    return QVector(uniformrand(min, max),
            uniformrand(min, max),
            uniformrand(min, max));
}

Carrier::Carrier() = default;


//index in here is unsigned, UINT_MAX and UINT_MAX-1 seem to be
//special states.  This means the total amount of cargo any ship can have
//is UINT_MAX -3   which is 65532 for 32bit machines.
void Carrier::EjectCargo(unsigned int index) {
    Unit *unit = vega_dynamic_cast_ptr<Unit>(this);
    Cargo *tmp = nullptr;
    Cargo ejectedPilot;
    Cargo dockedPilot;
    string name;
    bool isplayer = unit->IsPlayerShip();
    //if (index==((unsigned int)-1)) { is ejecting normally
    //if (index==((unsigned int)-2)) { is ejecting for eject-dock

    Cockpit *cp = nullptr;
    if (index == (UINT_MAX - 1)) {
//        _Universe->CurrentCockpit();
        //this calls the unit's existence, by the way.
        name = "return_to_cockpit";
        
        //we will have to check for this on undock to return to the parent unit!
        dockedPilot.SetName("return_to_cockpit");
        dockedPilot.SetMass(.1);
        dockedPilot.SetVolume(1);
        tmp = &dockedPilot;
    }
    if (index == UINT_MAX) {
        int pilotnum = _Universe->CurrentCockpit();
        name = "Pilot";
        if (nullptr != (cp = _Universe->isPlayerStarship(unit))) {
            string playernum = string("player") + ((pilotnum == 0) ? string("") : XMLSupport::tostring(pilotnum));
            isplayer = true;
        }
        ejectedPilot.SetName("eject");
        ejectedPilot.SetMass(.1);
        ejectedPilot.SetVolume(1);
        tmp = &ejectedPilot;
    }
    if (index < unit->cargo_hold.Size()) {
        tmp = &unit->cargo_hold.GetCargo(index);
    }

    // Some sanity checks for tmp
    // Can't eject an upgrade, unless ship is destroyed
    if (tmp && tmp->IsInstalled() && !unit->hull.Destroyed()) {
        return;
    }

    // Can't eject cargo from the hidden hold unless ship is destroyed.
    // TODO: implement

    // Make sure ejected mass isn't 0. This causes game to mishandle
    if (tmp && tmp->GetMass() == 0) {
        tmp->SetMass(0.01);
    }

    const float cargotime = configuration().physics.cargo_live_time_flt;
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
            Unit *cargo = nullptr;
            if (tmp->GetCategory().length() >= static_cast<unsigned int>(sslen)) {
                if ((!tmp->IsMissionFlag()) && tmp->GetCategory() == "starships") {
                    string ans = tmpcontent;
                    string::size_type blank = ans.find(".blank");
                    if (blank != string::npos) {
                        ans = ans.substr(0, blank);
                    }
                    Flightgroup *fg = unit->getFlightgroup();
                    int fgsnumber = 0;
                    if (fg != nullptr) {
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
            const float grot = configuration().graphics.generic_cargo_rotation_speed_flt * M_PI_FLT / 180.0F;
            if (!cargo) {
                const float crot = configuration().graphics.cargo_rotation_speed_flt * M_PI_FLT / 180.0F;
                const float erot = configuration().graphics.eject_rotation_speed_flt * M_PI_FLT / 180.0F;
                if (tmpcontent == "eject") {
                    if (isplayer) {
                        Flightgroup *fg = unit->getFlightgroup();
                        int fgsnumber = 0;
                        if (fg != nullptr) {
                            fgsnumber = fg->nr_ships;
                            ++(fg->nr_ships);
                            ++(fg->nr_ships_left);
                        }
                        cargo = new Unit("eject", false, unit->faction, "", fg, fgsnumber);
                    } else {
                        int fac = FactionUtil::GetUpgradeFaction();
                        cargo = new Unit("eject", false, fac, "", nullptr, 0);
                    }
                    if (unit->owner) {
                        cargo->owner = unit->owner;
                    } else {
                        cargo->owner = unit;
                    }
                    arot = erot;
                    const bool eject_attacks = configuration().ai.eject_attacks;
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
                        if (fg != nullptr) {
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
                        const float ejectcargotime = configuration().physics.eject_live_time_flt;
                        if (cargotime == 0.0) {
                            cargo = new Unit("eject", false, fac, "", nullptr, 0);
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
                    cargo->aistate = nullptr;
                } else {
                    string tmpnam = tmpcontent + ".cargo";
                    static std::string nam("Name");
                    float rot = crot;
                    if (UniverseUtil::LookupUnitStat(tmpnam, "upgrades", nam).empty()) {
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
            const bool all_rotate_same = configuration().graphics.cargo_rotates_at_same_speed;
            if (all_rotate_same && arot != 0) {
                float tmp = rotation.Magnitude();
                if (tmp > .001) {
                    // TODO: Use result of this expression, or remove the statement
                    rotation.Scale(1 / tmp);
                    rotation *= arot;
                }
            }
            {
                Vector tmpvel = -unit->Velocity;
                if (tmpvel.MagnitudeSquared() < .00001) {
                    tmpvel = randVector(-unit->rSize(), unit->rSize()).Cast();
                    if (tmpvel.MagnitudeSquared() < .00001) {
                        tmpvel = Vector(1, 1, 1);
                    }
                }
                tmpvel.Normalize();
                if ((SelectDockPort(unit, unit) > -1)) {
                    const float eject_cargo_offset = configuration().physics.eject_distance_flt;
                    QVector loc(Transform(unit->GetTransformation(),
                            unit->DockingPortLocations()[0].GetPosition().Cast()));
                    //index is always > -1 because it's unsigned.  Let's use the correct terms, -1 in Uint is UINT_MAX
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
                const float velmul = configuration().physics.eject_cargo_speed_flt;
                cargo->SetOwner(unit);
                cargo->SetVelocity(unit->Velocity * velmul + randVector(-.25, .25).Cast());
                cargo->SetMass(tmp->GetMass());
                if (!name.empty()) {
                    cargo->name = name;
                } else if (tmp) {
                    cargo->name = tmpcontent;
                }
                if (cp && _Universe->numPlayers() == 1) {
                    cargo->SetOwner(nullptr);
                    unit->PrimeOrders();
                    cargo->SetTurretAI();
                    cargo->faction = unit->faction;
                    //changes control to that cockpit
                    cp->SetParent(cargo, unit->Position());
                    if (tmpcontent == "return_to_cockpit") {
                        const bool simulate_while_at_base = configuration().physics.simulate_while_docked;
                        if ((simulate_while_at_base) || (_Universe->numPlayers() > 1)) {
                            unit->TurretFAW();
                        }
                        //make unit a sitting duck in the meantime
                        SwitchUnits(nullptr, unit);
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
                        SwitchUnits(nullptr, cargo);
                        if (unit->owner) {
                            cargo->owner = unit->owner;
                        } else {
                            cargo->owner = unit;
                        }
                    }                                            //switching NULL gives "dead" AI to the unit I ejected from, by the way.
                }
                _Universe->activeStarSystem()->AddUnit(cargo);
                if (index != -1U && index != -2U) {
                    if (index < unit->cargo_hold.Size()) {
                        unit->cargo_hold.RemoveCargo(unit, index, 1);
                    }
                }
            }
        }
    }
}

unsigned int Carrier::numCargo() const {
    const Unit *unit = vega_dynamic_cast_ptr<const Unit>(this);
    return unit->cargo_hold.Size();
}

std::string Carrier::GetManifest(unsigned int i, Unit *scanningUnit, const Vector &oldspd) const {
    const Unit *unit = vega_dynamic_cast_ptr<const Unit>(this);

    ///FIXME somehow mangle string
    string mangled = unit->cargo_hold.GetCargo(i).GetName();
    const float scramblingmanifest = configuration().general.percentage_speed_change_to_fault_search_flt;
    {
        //Keep inside subblock, otherwise MSVC will throw an error while redefining 'i'
        bool last = true;
        for (char & i : mangled) {
            if (last) {
                i = toupper(i);
            }
            last = (i == ' ' || i == '_');
        }
    }
    if (unit->CourseDeviation(oldspd, unit->GetVelocity()) > scramblingmanifest) {
        for (char & i : mangled) {
            i += static_cast<char>(randomInt(2) - 1);
        }
    }
    return mangled;
}

