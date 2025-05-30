/*
 * component.h
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2025 Stephen G. Tuggy, Benjamen R. Meyer, Roy Falk and other Vega Strike Contributors
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

#ifndef VEGA_STRIKE_ENGINE_COMPONENTS_COMPONENT_H
#define VEGA_STRIKE_ENGINE_COMPONENTS_COMPONENT_H

#include <string>
#include <map>

#include "resource/resource.h"

/**
 * LibComponent is currently tightly coupled to LibDamage and
 * other various libraries in VegaStrike engine.
 * Consider decoupling and subclassing every component in it.
 */

class Unit;

// TODO: add complete list
enum class ComponentType {
    NoComponent,
    Dummy,

    Hull,
    Armor,
    Shield,

    Afterburner,
    AfterburnerUpgrade,
    Drive,
    DriveUpgrade,
    FtlDrive,
    JumpDrive,

    Reactor,
    Capacitor,
    FtlCapacitor,
    Fuel,

    Cloak,
    Radar,
    ECM,
    RepairBot,
    ShipFunctions

    // TODO: all the rest of the upgrades, shady or not...
};

class Component
{
protected:
    std::string upgrade_name;   // Isometal Armor
    std::string upgrade_key;    // armor03__upgrades
    std::string description;    // Long text and picture. Taken from master_parts_list

    double price = 0;
    double mass = 0;
    double volume = 0;

    Resource<double> operational; // Percent operational

    bool installed = false;
    bool integral = false; // Part of the ship. Can't be upgraded/downgraded
public:
    virtual ~Component();
    ComponentType type = ComponentType::NoComponent;

    Component(double mass = 0,
              double volume = 0,
              bool installed = false,
              bool integral = false);

    // Load from units dictionary
    // TODO: we should really switch the two parameters around.
    virtual void Load(std::string unit_key);

    virtual void SaveToCSV(std::map<std::string, std::string>& unit) const = 0;

    // Handle the four cases of CanUpgrade/Upgrade/CanDowngrade/Downgrade
    bool CanWillUpDowngrade(const std::string upgrade_key,
                                           bool upgrade, bool apply);

    virtual bool CanDowngrade() const = 0;

    virtual bool Downgrade();

    virtual bool CanUpgrade(const std::string upgrade_key) const = 0;

    virtual bool Upgrade(const std::string upgrade_key);

    virtual void Damage();
    virtual void DamageByPercent(double percent);
    virtual void Repair();
    virtual void Destroy();

    virtual bool Damaged() const;
    bool Destroyed() const;
    virtual double PercentOperational() const;
    virtual bool Installed() const;
    bool Operational() const;

    void SetIntegral(bool integral);

    // Getters
    const std::string GetUnitKey() const;
    const std::string GetUpgradeName() const;
    const std::string GetUpgradeKey() const;
    const std::string GetDescription() const;

    const double GetPrice() const;
    const double GetMass() const;
    const double GetVolume() const;

    const bool Integral() const;
};
#endif // VEGA_STRIKE_ENGINE_COMPONENTS_COMPONENT_H
