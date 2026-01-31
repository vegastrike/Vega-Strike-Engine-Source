/*
 * player_ship.cpp
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

#include "player_ship.h"
#include "components_manager.h"
#include "configuration/configuration.h"

static const std::string player_fleet_category = "starships/My_Fleet";

// An identifier of the ship in question. This is NOT the index in the player_fleet vector.
// This identifier is added to the ship's cargo.index field.
// TODO: this is not elegant. Figure out something better. 
// Probably make PlayerShip a subclass of Cargo.
int counter = 1;    // We start counting from 1, as 0 is for non-player ship.

// Another hack. We should warn and log everytime this is returned.
static PlayerShip dummy_ship(true, nullptr, "","","dummy_ship",0,0,0);

PlayerShip::PlayerShip(bool active,
                       ComponentsManager* unit, 
                       const std::string& system, 
                       const std::string& base,
                       const std::string& name,
                       const double purchase_price,
                       const double mass,
                       const double volume): 
    active(active),
    unit(unit),
    cargo(Cargo(name, player_fleet_category, 
                0.0, 1, mass, volume)),
    system(system), base(base), purchase_price(purchase_price) {
    cargo.index = counter++;
}


PlayerShip& PlayerShip::GetActiveShip() {
    for(PlayerShip& ship : player_fleet) {
        if(ship.active) {
            return ship;
        }
    }

    return dummy_ship;
}

std::string PlayerShip::GetName() {
    std::string name = cargo.GetName();
    return name;
}

PlayerShip& PlayerShip::GetShipFromIndex(int index) {
    for(PlayerShip& ship : player_fleet) {
        if(ship.cargo.index == index) {
            return ship;
        }
    }

    return dummy_ship;
}

PlayerShip& PlayerShip::GetShipFromName(const std::string ship_name) {
    for(PlayerShip& ship : player_fleet) {
        if(ship_name == ship.cargo.GetName()) {
            return ship;
        }
    }

    return dummy_ship;
}

bool PlayerShip::IsShipInSameBase(const std::string& destination_system, 
                                  const std::string& destination_base) {
    std::cout << destination_system << " " << destination_base << std::endl;
    std::cout << system << " " << base << std::endl;
    return (destination_system == system && destination_base == base);
}

Cargo PlayerShip::RemoveShip(int index) {
    // Note that index is an internal number stored in cargo and is NOT
    // the index of the ship in the std::vector player_fleet!
    int i=0;
    for(PlayerShip& ship : player_fleet) {
        std::cout << i++ << " " << ship.cargo.GetName() << std::endl;
    }

    for(auto it = player_fleet.begin(); it != player_fleet.end(); ++it) {
        if (it->cargo.index == index) {
            std::cerr << "Removing ship "
                  << std::distance(player_fleet.begin(), it)
                  << " out of " << player_fleet.size() << std::endl;

            Cargo cargo = it->cargo;     // copy BEFORE erase

            if (it->unit) {
                delete it->unit;
                it->unit = nullptr;
            }

            player_fleet.erase(it);
            return cargo;
        }
    }

    return dummy_ship.cargo;
}

void PlayerShip::SwitchShips(int index) {
    for(PlayerShip& ship : player_fleet) {
        if(ship.cargo.index == index) {
            ship.active = true;
            continue;
        }

        if(ship.active) {
            ship.active = false;
        }
    }
}

void PlayerShip::UpdateLocation(const std::string& system, 
                                const std::string& base) {
    this->system = system;
    this->base = base;
}

// Right now we feed everything to the function and it's trivial.
// But we could calculate transport price by parsec, risk per system, etc.
void PlayerShip::UpdateTransportPrice(const std::string& destination_system, 
                                     const std::string& destination_base,
                                     const int jumps) {
    const float shipping_cost_base = configuration().economics.shipping_price_base_flt;
    const float shipping_cost_insys = configuration().economics.shipping_price_insys_flt;
    const float shipping_cost_per_jump = configuration().economics.shipping_price_perjump_flt;

    
    if(destination_system == system && destination_base == base) {
        // Ship is in the same base
        cargo.SetPrice(0.0);
    } else if(destination_system == system) {
        // Ship is in the same system
        cargo.SetPrice(shipping_cost_base + shipping_cost_insys);
    } else {
        // Ship is in another system
        cargo.SetPrice(shipping_cost_base + (jumps * shipping_cost_per_jump));
    }
}


std::vector<PlayerShip> player_fleet;

