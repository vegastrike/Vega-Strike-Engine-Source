/*
 * components_manager.h
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

#ifndef VEGA_STRIKE_ENGINE_COMPONENTS_MANAGER_COMPONENT_H
#define VEGA_STRIKE_ENGINE_COMPONENTS_MANAGER_COMPONENT_H

#include "energy_container.h"
#include "reactor.h"

#include "afterburner.h"
#include "afterburner_upgrade.h"
#include "drive.h"
#include "drive_upgrade.h"

#include "ftl_drive.h"
#include "jump_drive.h"

#include "armor.h"
#include "hull.h"
#include "shield.h"

#include "computer.h"
#include "radar.h"
#include "cloak.h"
#include "ecm.h"
#include "repair_bot.h"
#include "ship_functions.h"

#include "cargo_hold.h"

#include <vector>

/** A collection of components. This class is really a proto-ship,
 * with mass and serving as a stand-in for the Unit sub-class.
 */
class ComponentsManager {
    bool player_ship = false;
    
    // Here we store hud text so we won't have to generate it every cycle
    // Instead we only do this when something changes
    std::string hud_text;

    std::vector<std::pair<const std::string, const int>> prohibited_upgrades;

    friend class CargoHold;
    friend class Movable;

protected:
    // TODO: make it change with fuel consumption
    double mass;
    double base_mass;
public:
    static Resource<double> credits;

    virtual ~ComponentsManager() = default;

    void Load(std::string unit_key);
    void Serialize(std::map<std::string, std::string>& unit) const;

    double GetMass() const;
    void SetMass(double mass);

    double PriceCargo(const std::string &cargo_name);
    void SetPlayerShip();
    bool IsPlayerShip() const;

// Components
    EnergyContainer fuel = EnergyContainer(ComponentType::Fuel);
    EnergyContainer energy = EnergyContainer(ComponentType::Capacitor);
    EnergyContainer ftl_energy = EnergyContainer(ComponentType::FtlCapacitor);

    // TODO: move this to a single constructor?!
    Reactor reactor = Reactor(&fuel, &energy, &ftl_energy);

    Afterburner afterburner;
    AfterburnerUpgrade afterburner_upgrade = AfterburnerUpgrade(&afterburner);
    Cloak cloak = Cloak();
    Drive drive;
    DriveUpgrade drive_upgrade = DriveUpgrade(&drive);
    FtlDrive ftl_drive = FtlDrive(&ftl_energy);
    JumpDrive jump_drive = JumpDrive(&ftl_energy);
    CRadar radar;

    Armor armor;
    Hull hull;
    Shield shield = Shield(&energy, &ftl_drive, &cloak);

    Computer computer;
    ECM ecm;
    RepairBot repair_bot;
    ShipFunctions ship_functions;

    CargoHold cargo_hold = CargoHold(HoldType::cargo);
    CargoHold hidden_hold = CargoHold(HoldType::hidden);
    CargoHold upgrade_space = CargoHold(HoldType::upgrade);

    bool ShipDamaged() const;
    bool AllowedUpgrade(const Cargo& upgrade) const;
    bool UpgradeAlreadyInstalled(const Cargo& upgrade) const;
    void DamageRandomSystem();
    void GenerateHudText(std::string getDamageColor(double));
    std::string GetHudText();
    std::string GetTitle(bool show_cargo, bool show_star_date, std::string date);

    /** place stuff here for now. maybe move to subclass */
    bool BuyCargo(ComponentsManager *seller, Cargo *item, int quantity);
    bool SellCargo(ComponentsManager *seller, Cargo *item, int quantity);
    bool BuyUpgrade(ComponentsManager *seller, Cargo *item, int quantity);
    bool SellUpgrade(ComponentsManager *seller, Cargo *item, int quantity);

    Component* GetComponentByType(const ComponentType type);
    const Component* GetComponentByType(const ComponentType type) const;
private:
    bool _Buy(CargoHold *hold, ComponentsManager *seller, Cargo *item, int quantity);
    bool _Sell(CargoHold *hold, ComponentsManager *buyer, Cargo *item, int quantity);
};

#endif // VEGA_STRIKE_ENGINE_COMPONENTS_MANAGER_COMPONENT_H