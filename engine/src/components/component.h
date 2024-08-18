/*
 * component.h
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

#ifndef COMPONENT_H
#define COMPONENT_H

#include <string>
#include <map>

/**
 * LibComponent is currently tightly coupled to LibDamage and
 * other various libraries in VegaStrike engine.
 * Consider decoupling and subclassing every component in it.
 */

class Unit;

// TODO: add complete list
enum class ComponentType {
    Hull, 
    Armor, 
    Shield,
    Drive
};

class Component
{
protected:
    std::string unit_key;       // Areus.blank
    std::string upgrade_name;   // Isometal Armor
    std::string upgrade_key;    // armor03__upgrades
    
    double mass = 0;
    double volume = 0;

    bool integral = false; // Part of the ship. Can't be upgraded/downgraded
public:
    Component(double mass = 0, 
              double volume = 0, bool integral = false);

    // Load from units dictionary
    virtual void Load(std::string upgrade_key, std::string unit_key);      
    
    virtual void SaveToCSV(std::map<std::string, std::string>& unit) const = 0;

    virtual std::string Describe() const = 0; // Describe component in base_computer 

    // Handle the four cases of CanUpgrade/Upgrade/CanDowngrade/Downgrade
    bool CanWillUpDowngrade(const std::string upgrade_key,
                                           bool upgrade, bool apply);

    virtual bool CanDowngrade() const = 0;

    virtual bool Downgrade();

    virtual bool CanUpgrade(const std::string upgrade_key) const = 0;

    virtual bool Upgrade(const std::string upgrade_key);

    virtual void Damage() = 0;
    virtual void DamageByPercent(double percent) = 0;
    virtual void Repair() = 0;

    virtual bool Damaged() const = 0;
    virtual bool Installed() const = 0;

    void SetIntegral(bool integral);
};
#endif // COMPONENT_H
