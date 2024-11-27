/*
 * component.cpp
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

#include "component.h"
#include "unit_csv_factory.h"
#include "resource/cargo.h"
#include "resource/manifest.h"



const std::string NAME = "Name";
const std::string MASS = "Mass";
const std::string VOLUME = "Volume";

Component::Component(double mass, double volume, bool installed, bool integral):
                     unit_key(""),
                     upgrade_name(""),
                     description(""),
                     price(0.0),
                     mass(mass), volume(volume),
                     operational(Resource<double>(100.0,0.0,100.0)),
                     installed(installed),
                     integral(integral) {}


void Component::Load(std::string upgrade_key, std::string unit_key) {
    this->unit_key = unit_key;
    this->upgrade_key = upgrade_key;

    // Integrated components have no name, mass, price, volume or description
    if(!integral) {
        upgrade_name = UnitCSVFactory::GetVariable(upgrade_key, "Name", std::string());
        mass = UnitCSVFactory::GetVariable(upgrade_key, "Mass", 0.0);

        // Get volume and description from MasterPartList.
        // We need try/catch for unit tests where MPL isn't loaded.
        const Cargo cargo = Manifest::MPL().GetCargoByName(upgrade_key);
        if(!cargo.GetName().empty()) {
            price = cargo.GetPrice();
            volume = cargo.GetVolume();
            description = cargo.GetDescription();
        }
    }

    // TODO: bool integral = false;
}



// TODO: convert to std::pair<bool, double>
bool Component::CanWillUpDowngrade(const std::string upgrade_key,
                                   bool upgrade, bool apply) {
    if(upgrade) {
        if(apply) {
            return Upgrade(upgrade_key);
        } else {
            return CanUpgrade(upgrade_key);
        }
    } else {
        if(apply) {
            return Downgrade();
        } else {
            return CanDowngrade();
        }
    }                                        
}

bool Component::Downgrade() {
    upgrade_name = std::string();
    upgrade_key = std::string();
    
    mass = 0.0;
    volume = 0.0;
    installed = false;
    return true;
}

bool Component::Upgrade(const std::string upgrade_key) {
    this->upgrade_key = upgrade_key;
    upgrade_name = UnitCSVFactory::GetVariable(upgrade_key, NAME, std::string());
    
    mass = UnitCSVFactory::GetVariable(upgrade_key, MASS, 0.0);
    // TODO: volume currently not in units.json. need to merge with items list
    volume = UnitCSVFactory::GetVariable(upgrade_key, VOLUME, 0.0);
    installed = true;
    return true;
}

void Component::Damage() {
    operational.RandomDamage();
}

void Component::DamageByPercent(double percent) {
    operational.DamageByPercent(percent);
}

void Component::Repair() {
    operational.RepairFully();
}

bool Component::Damaged() const {
    return operational.Damaged();
}

bool Component::Destroyed() const {
    return operational.Destroyed();
}

bool Component::Installed() const {
    return installed;
}

bool Component::Operational() const {
    return Installed() && !Destroyed();
}

double Component::Percent() const {
    return operational.Percent();
}

void Component::SetIntegral(bool integral) {
    this->integral = integral;
}

// Getters
const std::string Component::GetUnitKey() const { return unit_key; }
const std::string Component::GetUpgradeName() const { return upgrade_name; }
const std::string Component::GetUpgradeKey() const { return upgrade_key; }
const std::string Component::GetDescription() const { return description; }

const double Component::GetPrice() const { return price; }
const double Component::GetMass() const { return mass; }
const double Component::GetVolume() const { return volume; }

const double Component::GetOperational() const { return operational.Value(); }

const bool Component::GetInstalled() const { return installed; }
const bool Component::GetIntegral() const { return integral; }