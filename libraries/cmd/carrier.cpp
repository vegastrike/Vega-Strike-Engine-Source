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



// TODO: probably replace with a lambda expression
class CatCompare {
public:
    bool operator()(const Cargo &a, const Cargo &b) {
        std::string::const_iterator aiter = a.GetCategory().begin();
        std::string::const_iterator aend = a.GetCategory().end();
        std::string::const_iterator biter = b.GetCategory().begin();
        std::string::const_iterator bend = b.GetCategory().end();
        for (; aiter != aend && biter != bend; ++aiter, ++biter) {
            char achar = *aiter;
            char bchar = *biter;
            if (achar < bchar) {
                return true;
            }
            if (achar > bchar) {
                return false;
            }
        }
        return false;
    }
};

// TODO: move these two functions to vector and make into single constructor
inline float uniformrand(float min, float max) {
    return ((float) (rand()) / RAND_MAX) * (max - min) + min;
}

inline QVector randVector(float min, float max) {
    return QVector(uniformrand(min, max),
            uniformrand(min, max),
            uniformrand(min, max));
}

std::string CargoToString(const Cargo &cargo) {
    string missioncargo;
    if (cargo.IsMissionFlag()) {
        missioncargo = string("\" missioncargo=\"") + XMLSupport::tostring(cargo.IsMissionFlag());
    }
    return string("\t\t\t<Cargo mass=\"") + XMLSupport::tostring((float) cargo.GetMass()) + string("\" price=\"")
            + XMLSupport::tostring((float) cargo.GetPrice()) + string("\" volume=\"")
            + XMLSupport::tostring((float) cargo.GetVolume())
            + string(
                    "\" quantity=\"") + XMLSupport::tostring((int) cargo.GetQuantity()) + string("\" file=\"")
            + cargo.GetName()
            + missioncargo
            + string("\"/>\n");
}

Carrier::Carrier() {

}

void Carrier::SortCargo() {
    // TODO: better cast
    Unit *un = (Unit *) this;
    std::sort(un->cargo.begin(), un->cargo.end());
    for (unsigned int i = 0; i + 1 < un->cargo.size(); ++i) {
        if (un->cargo[i].name == un->cargo[i + 1].name) {
            float tmpmass = un->cargo[i].quantity * un->cargo[i].GetMass()
                    + un->cargo[i + 1].quantity * un->cargo[i + 1].GetMass();
            float tmpvolume = un->cargo[i].quantity * un->cargo[i].GetVolume()
                    + un->cargo[i + 1].quantity * un->cargo[i + 1].GetVolume();
            un->cargo[i].quantity += un->cargo[i + 1].quantity;
            if (un->cargo[i].quantity) {
                tmpmass /= un->cargo[i].quantity;
                tmpvolume /= un->cargo[i].quantity;
            }
            un->cargo[i].SetVolume(tmpvolume);
            un->cargo[i].SetMissionFlag((un->cargo[i].IsMissionFlag() || un->cargo[i + 1].IsMissionFlag()));
            un->cargo[i].SetMass(tmpmass);
            //group up similar ones
            un->cargo.erase(un->cargo.begin() + (i + 1));
            i--;
        }
    }
}

std::string Carrier::cargoSerializer(const XMLType &input, void *mythis) {
    Unit *un = (Unit *) mythis;
    if (un->cargo.size() == 0) {
        return string("0");
    }
    un->SortCargo();
    string retval("");
    if (!(un->cargo.empty())) {
        retval = un->cargo[0].GetCategory() + string("\">\n") + CargoToString(un->cargo[0]);
        for (unsigned int kk = 1; kk < un->cargo.size(); ++kk) {
            if (un->cargo[kk].GetCategory() != un->cargo[kk - 1].GetCategory()) {
                retval += string("\t\t</Category>\n\t\t<Category file=\"") + un->cargo[kk].GetCategory() + string(
                        "\">\n");
            }
            retval += CargoToString(un->cargo[kk]);
        }
        retval += string("\t\t</Category>\n\t\t<Category file=\"nothing");
    } else {
        retval = string("nothing");
    }
    return retval;
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
        dockedPilot.name = "return_to_cockpit";
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
        ejectedPilot.name = "eject";
        ejectedPilot.SetMass(.1);
        ejectedPilot.SetVolume(1);
        tmp = &ejectedPilot;
    }
    if (index < numCargo()) {
        tmp = &GetCargo(index);
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

    const float cargotime = configuration()->physics.cargo_live_time;
    if (tmp) {
        string tmpcontent = tmp->name;
        if (tmp->IsMissionFlag()) {
            tmpcontent = "Mission_Cargo";
        }

        //prevents a number of bad things, incl. impossible speeds and people getting rich on broken stuff
        if ((!tmp->IsMissionFlag()) && tmp->IsComponent()) {
            tmpcontent = "Space_Salvage";
        }
        //this happens if it's a ship
        if (tmp->quantity > 0) {
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
                        const float ejectcargotime = configuration()->physics.eject_live_time;
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
                    const float eject_cargo_offset = configuration()->physics.eject_distance;
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
                const float velmul = configuration()->physics.eject_cargo_speed;
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
                        const bool simulate_while_at_base = configuration()->physics.simulate_while_docked;
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
                        RemoveCargo(index, 1, true);
                    }
                }
            }
        }
    }
}

int Carrier::RemoveCargo(unsigned int i, int quantity, bool eraseZero) {
    Unit *unit = static_cast<Unit *>(this);
    if (!(i < unit->cargo.size())) {
        VS_LOG(error, "(previously) FATAL problem...removing cargo that is past the end of array bounds.");
        return 0;
    }
    Cargo *carg = &(unit->cargo[i]);
    if (quantity > carg->quantity) {
        quantity = carg->quantity;
    }

    const bool usemass = configuration()->physics.use_cargo_mass;
    if (usemass) {
        unit->setMass(unit->getMass() - quantity * carg->GetMass());
    }

    carg->quantity -= quantity;
    if (carg->quantity <= 0 && eraseZero) {
        unit->cargo.erase(unit->cargo.begin() + i);
    }
    return quantity;
}

void Carrier::AddCargo(const Cargo &carg, bool sort) {
    Unit *unit = static_cast<Unit *>(this);

    const bool usemass = configuration()->physics.use_cargo_mass;
    if (usemass) {
        unit->setMass(unit->getMass() + carg.quantity.Value() * carg.GetMass());
    }

    bool found = false;

    for(Cargo c: this->cargo) {
        if(c.GetName() == carg.GetName() && c.GetCategory() == carg.GetCategory()) {
            found = true;
            c.Add(carg.GetQuantity());
        }
    }

    if(!found) {
        unit->cargo.push_back(carg);
    }

    if (sort) {
        SortCargo();
    }
}

float Carrier::getHiddenCargoVolume() const {
    const Unit *unit = vega_dynamic_cast_ptr<const Unit>(this);
    return unit->HiddenCargoVolume;
}

bool Carrier::CanAddCargo(const Cargo &carg) const {
    const Unit *unit = vega_dynamic_cast_ptr<const Unit>(this);

    //Always can, in this case (this accounts for some odd precision issues)
    if ((carg.quantity == 0) || (carg.GetVolume() == 0)) {
        return true;
    }
    //Test volume availability
    float total_volume = carg.quantity.Value() * carg.GetVolume() + (carg.IsComponent() ? getUpgradeVolume() : getCargoVolume());
    if (total_volume <= (carg.IsComponent() ? getEmptyUpgradeVolume() : getEmptyCargoVolume())) {
        return true;
    }
    //Hm... not in main unit... perhaps a subunit can take it
    for (un_kiter i = unit->viewSubUnits(); !i.isDone(); ++i) {
        if ((*i)->CanAddCargo(carg)) {
            return true;
        }
    }
    //Bad luck
    return false;
}

//The cargo volume of this ship when empty.  Max cargo volume.
float Carrier::getEmptyCargoVolume(void) const {
    const Unit *unit = vega_dynamic_cast_ptr<const Unit>(this);
    return unit->CargoVolume;
}

float Carrier::getEmptyUpgradeVolume(void) const {
    const Unit *unit = vega_dynamic_cast_ptr<const Unit>(this);
    return unit->UpgradeVolume;
}

float Carrier::getCargoVolume(void) const {
    const Unit *unit = vega_dynamic_cast_ptr<const Unit>(this);
    float result = 0.0;
    for (unsigned int i = 0; i < unit->cargo.size(); ++i) {
        if (!(unit->cargo[i].IsComponent())) {
            result += unit->cargo[i].quantity.Value() * unit->cargo[i].GetVolume();
        }
    }
    return result;
}



float Carrier::PriceCargo(const std::string &s) {
    Unit *unit = static_cast<Unit *>(this);

    Cargo tmp;
    tmp.name = s;
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
    price = (*mycargo).price;
    return price;
}

float Carrier::getUpgradeVolume(void) const {
    const Unit *unit = vega_dynamic_cast_ptr<const Unit>(this);
    float result = 0.0;
    for (unsigned int i = 0; i < unit->cargo.size(); ++i) {
        if ((unit->cargo[i].IsComponent())) {
            result += unit->cargo[i].quantity.Value() * unit->cargo[i].GetVolume();
        }
    }
    return result;
}

Cargo &Carrier::GetCargo(unsigned int i) {
    Unit *unit = static_cast<Unit *>(this);
    return unit->cargo[i];
}

const Cargo &Carrier::GetCargo(unsigned int i) const {
    const Unit *unit = vega_dynamic_cast_ptr<const Unit>(this);
    return unit->cargo[i];
}

void Carrier::GetSortedCargoCat(const std::string &cat, size_t &begin, size_t &end) {
    Unit *unit = static_cast<Unit *>(this);
    vector<Cargo>::iterator Begin = unit->cargo.begin();
    vector<Cargo>::iterator End = unit->cargo.end();
    vector<Cargo>::iterator lbound = unit->cargo.end();
    vector<Cargo>::iterator ubound = unit->cargo.end();

    Cargo beginningtype;
    beginningtype.SetCategory(cat);
    CatCompare Comp;
    lbound = std::lower_bound(Begin, End, beginningtype, Comp);
    beginningtype.name = "zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz";
    ubound = std::upper_bound(Begin, End, beginningtype, Comp);
    begin = lbound - Begin;
    end = ubound - Begin;
}

// TODO: I removed a superfluous call via this->GetCargo and got a warning about recursion
// all paths through this function will call itself
// The game also crashed due to endless loop.
// I returned the code and now it works and I don't know why.
Cargo *Carrier::GetCargo(const std::string &s, unsigned int &i) {
    const Unit *unit = vega_dynamic_cast_ptr<const Unit>(this);
    if (unit->GetCargo(s, i)) {
        return &GetCargo(i);
    }
    return NULL;
}

const Cargo *Carrier::GetCargo(const std::string &s, unsigned int &i) const {
    const Unit *unit = vega_dynamic_cast_ptr<const Unit>(this);

    static Hashtable<string, unsigned int, 2047> index_cache_table;
    Unit *mpl = getMasterPartList();
    if (this == mpl) {
        unsigned int *ind = index_cache_table.Get(s);
        if (ind) {
            if (*ind < unit->cargo.size()) {
                Cargo *guess = const_cast<Cargo *>(&unit->cargo[*ind]);
                if (guess->name == s) {
                    i = *ind;
                    return guess;
                }
            }
        }
        Cargo searchfor;
        searchfor.name = s;

        // TODO: could not deduce the right var type. Resorted to auto
        //vector< Cargo >::iterator tmp = std::find( unit->cargo.begin(),
        //                                           unit->cargo.end(), searchfor );
        auto tmp = std::find(unit->cargo.begin(), unit->cargo.end(), searchfor);
        if (tmp == unit->cargo.end()) {
            return nullptr;
        }
        if ((*tmp).name == searchfor.name) {
            i = (tmp - unit->cargo.begin());
            if (this == mpl) {
                unsigned int *tmp = new unsigned int;
                *tmp = i;
                if (index_cache_table.Get(s)) {
                    index_cache_table.Delete(s);
                }
                //memory leak--should not be reached though, ever
                index_cache_table.Put(s, tmp);
            }
            return &(*tmp);
        }
        return nullptr;
    }
    Cargo searchfor;
    searchfor.name = s;

    // TODO: could not deduce the right var type. Resorted to auto
    auto tmp = (std::find(unit->cargo.begin(), unit->cargo.end(), searchfor));
    if (tmp == unit->cargo.end()) {
        return NULL;
    }
    i = (tmp - unit->cargo.begin());
    return &(*tmp);
}

unsigned int Carrier::numCargo() const {
    const Unit *unit = vega_dynamic_cast_ptr<const Unit>(this);
    return unit->cargo.size();
}

std::string Carrier::GetManifest(unsigned int i, Unit *scanningUnit, const Vector &oldspd) const {
    const Unit *unit = vega_dynamic_cast_ptr<const Unit>(this);

    ///FIXME somehow mangle string
    string mangled = unit->cargo[i].name;
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

bool Carrier::SellCargo(unsigned int i, int quantity, float &creds, Cargo &carg, Unit *buyer) {
    Unit *unit = vega_dynamic_cast_ptr<Unit>(this);

    if (i < 0 || i >= unit->cargo.size() || !buyer->CanAddCargo(unit->cargo[i])
            || unit->getMass() < unit->cargo[i].GetMass()) {
        return false;
    }
    carg = unit->cargo[i];
    if (quantity > unit->cargo[i].quantity.Value()) {
        quantity = unit->cargo[i].quantity.Value();
    }
    carg.price = buyer->PriceCargo(unit->cargo[i].name);
    creds += quantity * carg.price;

    carg.quantity = quantity;
    buyer->AddCargo(carg);

    RemoveCargo(i, quantity);
    return true;
}

bool Carrier::SellCargo(const std::string &s, int quantity, float &creds, Cargo &carg, Unit *buyer) {
    Unit *unit = vega_dynamic_cast_ptr<Unit>(this);

    Cargo tmp;
    tmp.name = s;

    // TODO: could not deduce the right var type. Resorted to auto
    auto mycargo = std::find(unit->cargo.begin(), unit->cargo.end(), tmp);
    if (mycargo == unit->cargo.end()) {
        return false;
    }
    return SellCargo(mycargo - unit->cargo.begin(), quantity, creds, carg, buyer);
}

bool Carrier::BuyCargo(const Cargo &carg, float &creds) {
    if (!CanAddCargo(carg) || creds < carg.quantity.Value() * carg.price) {
        return false;
    }
    AddCargo(carg);
    creds -= carg.quantity.Value() * carg.price;

    return true;
}

bool Carrier::BuyCargo(unsigned int i, unsigned int quantity, Unit *seller, float &creds) {
    Cargo soldcargo = seller->cargo[i];
    if (quantity > (unsigned int) soldcargo.quantity) {
        quantity = soldcargo.quantity;
    }
    if (quantity == 0) {
        return false;
    }
    soldcargo.quantity = quantity;
    if (BuyCargo(soldcargo, creds)) {
        seller->RemoveCargo(i, quantity, false);
        return true;
    }
    return false;
}

bool Carrier::BuyCargo(const std::string &cargo, unsigned int quantity, Unit *seller, float &creds) {
    unsigned int i;
    if (seller->GetCargo(cargo, i)) {
        return BuyCargo(i, quantity, seller, creds);
    }
    return false;
}
