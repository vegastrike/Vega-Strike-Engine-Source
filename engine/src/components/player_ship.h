/*
 * player_ship.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
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

class ShipNotFoundException : public std::runtime_error {
public:
    explicit ShipNotFoundException(int index);
    explicit ShipNotFoundException(const std::string& name);
};

class NoActiveShipNotFoundException : public std::runtime_error {
public:
    explicit NoActiveShipNotFoundException()
        : std::runtime_error("No active ship found in player fleet.") {}
};

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
    static int GetActiveShipIndex();
    std::string GetName();
    std::string GetPurchaseHeader();
    /** How much of the ship is damaged, from 0.0 to 1.0 */
    double DamagePercent();
    /** What the dealer pays for this ship: its resale value less a share of the damage it carries,
        and never less than its weight in scrap. */
    double SalePrice();
    static PlayerShip& GetShipByIndex(int index);
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

/** What a thing is worth as scrap: its weight in the cheapest metal, and never more than a share
    of what it cost. The floor a dealer will not go below for anything too damaged to be worth
    putting right. */
double ScrapValue(double price, double mass);

#endif // VEGA_STRIKE_ENGINE_COMPONENTS_PLAYER_SHIP_H
