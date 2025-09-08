/*
 * reload_utils.cpp
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

 // -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#include "reload_utils.h"

#include "cmd/unit_generic.h"

#include <vector>

// This generate an upgrade unit from the name and faction.
// An upgrade unit is the unit representation of a specific upgrade.
// e.g. Micro_Driver_ammo__upgrades in units.json
const Unit *getUnitFromUpgradeName(const string &upgradeName, int myUnitFaction = 0);


std::vector<Mount*> GetMountsForName(Unit *unit, const std::string& weapon_unit_name) {
    std::vector<Mount*> mounts;

    // We need to extract the weapon name (from weapons.json) 
    // from the parameter weapon_unit_name (from units.json)
    
    // First step is to get a unit instance from the weapon_unit_name
    const Unit *weapon = getUnitFromUpgradeName(weapon_unit_name, FactionUtil::GetUpgradeFaction());

    // Something went wrong
    if(!weapon) {
        return mounts;
    }

    // Now we can extract the weapon's name as it is in weapons.json
    const std::string weapon_name = weapon->mounts[0].type->name;

    // Finally we iterate over the ship mounts in order to generate a partial list
    // of mounts with the weapon's name
    for(Mount& mount : unit->mounts) {
        if(weapon_name == mount.type->name && mount.ammo != -1) {
            mounts.push_back(&mount);
        }
    }

    return mounts;
} 

int getMaxAmmo(const std::string& weapon_unit_name) {
    // First step is to get a unit instance from the weapon_unit_name
    const Unit *weapon = getUnitFromUpgradeName(weapon_unit_name, FactionUtil::GetUpgradeFaction());

    // Something went wrong
    if(!weapon) {
        return -1;
    }

    // Now we get the first mount, which always holds the actual weapon from weapons.json
    Mount weapon_mount = weapon->mounts[0];

    // Now we can extract the weapon's max ammo
    return weapon_mount.ammo;
}

std::vector<int> getAmmoPerGun(Unit *unit, const std::string& weapon_unit_name) {
    std::vector<int> ammo_vector;

    for(Mount* mount : GetMountsForName(unit, weapon_unit_name)) {
        ammo_vector.push_back(mount->ammo);
    }

    return ammo_vector;
}

double getReloadCost(const int ammo, const int max_ammo, const double price, const double modifier = 0.05) {
    double ammo_percent = 1 - static_cast<double>(ammo)/static_cast<double>(max_ammo);
    return ammo_percent * price * modifier;
}

std::string getReloadDescription(Unit *unit, const double price, const std::string& weapon_unit_name) {
    const int max_ammo = getMaxAmmo(weapon_unit_name);
    std::string description = "#b#Reload cost:#-b#n1.5#";

    int i=1;


    for(Mount* mount : GetMountsForName(unit, weapon_unit_name)) {
        int ammo = mount->ammo;

        // TODO: make modifier configurable in configuration
        double cost = getReloadCost(ammo, max_ammo, price);
        double ammo_percent = static_cast<double>(ammo)/static_cast<double>(max_ammo) * 100;
        description += (boost::format("Gun %1% - ammo: %2%/%3% (%4%%%) cost: %5%#n#")
                            % i % ammo % max_ammo % ammo_percent % cost).str();
        i++;
    }

    return description;
}

bool canReload(Unit *unit, const std::string& weapon_unit_name) {
    const int max_ammo = getMaxAmmo(weapon_unit_name);
    if(max_ammo == -1) {
        return false;
    }

    for(Mount* mount : GetMountsForName(unit, weapon_unit_name)) {
        if(mount->ammo < max_ammo) {
            return true;
        }
    }

    return false;
}