/*
 * components_manager.cpp
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2023 Stephen G. Tuggy, Benjamen R. Meyer, Roy Falk and other Vega Strike Contributors
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

// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#include "components_manager.h"
#include "resource/random_utils.h"

void ComponentsManager::DamageRandomSystem() {
    double percent = 1 - hull.Percent();

    Component* components[] = {&fuel, &energy, &ftl_energy, &reactor,
                               &afterburner, &cloak, &drive, &ftl_drive,
                               &jump_drive, &computer, &radar, &shield,
                               &ecm, &repair_bot, &ship_functions};
    
    for(Component* component : components) {
        double chance_to_damage = randomDouble();
        if (chance_to_damage < percent) {
            component->DamageByPercent(chance_to_damage/10.0);
        }
    }
    
    /*
        Things not handled at the moment:
        1. Mounts
        2. Upgrade volume
        3. Cargo volume
        4. Cargo

        TODO: mounts

        To think about:
        1. Should we damage the cargo? 
            - To be handled by carrier. 
            - More applicable for really large ships
        2. Should we damage the upgrade volume?
            - Probably not. How would that work in real life?!
        3. Should we damage the cargo volume?
            - Probably not. How would that work in real life?!
            - More applicable for really large ships, where you shoot at one of the cargo containers
    */

  

    // TODO: take actual damage into account when damaging components
    /*
    if (degrees >= 20 && degrees < 35) {
        //DAMAGE MOUNT
        if (randnum >= .65 && randnum < .9) {
            ecm.Damage();
        } else if (getNumMounts()) {
            unsigned int whichmount = rand() % getNumMounts();
            if (randnum >= .9) {
                DestroyMount(&mounts[whichmount]);
            } else if (mounts[whichmount].ammo > 0 && randnum >= .75) {
                mounts[whichmount].ammo *= float_to_int(dam);
            } else if (randnum >= .7) {
                mounts[whichmount].time_to_lock += (100 - (100 * dam));
            } else if (randnum >= .2) {
                mounts[whichmount].functionality *= dam;
            } else {
                mounts[whichmount].maxfunctionality *= dam;
            }
        }
        return;
    }
    
            //Do something NASTY to the cargo
            if (cargo.size() > 0) {
                unsigned int i = 0;
                unsigned int cargorand_o = rand();
                unsigned int cargorand;
                do {
                    cargorand = (cargorand_o + i) % cargo.size();
                } while ((cargo[cargorand].GetQuantity() == 0
                        || cargo[cargorand].GetMissionFlag()) && (++i) < cargo.size());
                cargo[cargorand].SetQuantity(cargo[cargorand].GetQuantity() * float_to_int(dam));
            }
    }*/
}