/*
 * player_ship.h
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

#ifndef VEGA_STRIKE_ENGINE_COMPONENTS_PLAYER_SHIP_H
#define VEGA_STRIKE_ENGINE_COMPONENTS_PLAYER_SHIP_H

#include <string>
#include <vector>

#include "resource/cargo.h"

class ComponentsManager;

struct PlayerShip {
    bool active;        // The ship we're flying
    
    ComponentsManager* unit; // Pointer to the unit
    Cargo cargo;            // The cargo representation of the unit, for display by the ship dealer
    std::string system;     // The system the ship is in
    std::string base;       // The planet/station the ship is docked at
    double transfer_price;

    PlayerShip(bool active,
               ComponentsManager* unit,
               const Cargo& cargo,
               const std::string& system = "", 
               const std::string& base = "");

    static PlayerShip& GetActiveShip();
    std::string GetName();
    static PlayerShip& GetShipFromIndex(int index);
    // Caution! Will return first ship to match ship_name
    static PlayerShip& GetShipByName(const std::string ship_name);

    bool IsShipInSameBase(const std::string& destination_system, 
                          const std::string& destination_base);

    static Cargo RemoveShip(int index);
    static void SwitchShips(int index);

    void UpdateLocation(const std::string& system, 
                        const std::string& base);

    void UpdateTransportPrice(const std::string& destination_system, 
                             const std::string& destination_base,
                             const int jumps);
};

extern std::vector<PlayerShip> player_fleet;

#endif // VEGA_STRIKE_ENGINE_COMPONENTS_PLAYER_SHIP_H
