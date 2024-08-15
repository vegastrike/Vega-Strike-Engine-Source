/*
 * energy_container.cpp
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

#include "energy_container.h"
#include "unit_csv_factory.h"

#include <iostream>

const std::string FUEL_CAPACITY = "Fuel_Capacity";
const std::string CAPACITOR = "Warp_Capacitor";
const std::string FTL_CAPACITOR = "Primary_Capacitor";

EnergyContainer::EnergyContainer(EnergyType type): 
                                 Component(0.0, 0.0, false),
                                 type(type),
                                 level(Resource<double>(0.0,0.0,0.0)) {}


// Return value - any surplus charge
double EnergyContainer::Charge(const double quantity) {
    double old_level = level.Value();
    level += quantity;

    return quantity - level.Value() + old_level;
}

double EnergyContainer::Deplete(bool partial, const double quantity) {
    // Check we have enough energy to fully charge the consumer
    if(!partial && quantity > level.Value()) {
        return 0.0;
    }

    double old_level = level.Value();
    level -= quantity;
    double actual_usage = old_level - level.Value();
    return actual_usage / quantity;
}

bool EnergyContainer::Depleted() const {
    return (level.Value() < 0.0001);
}

void EnergyContainer::SetCapacity(const double capacity, bool refill) {
    if(refill) {
        level = Resource<double>(capacity,0,capacity);
    } else {
        level = Resource<double>(0,0,capacity);
    }
}

double EnergyContainer::Level() const { return level.Value(); }
void EnergyContainer::SetLevel(double new_level) {
    level = new_level;
} 
double EnergyContainer::MaxLevel() const { return level.MaxValue(); }
double EnergyContainer::Percent() const { 
    if(level.MaxValue() == 0.0) {
        return 0.0;
    }
    
    return level.Value()/level.MaxValue(); 
}

void EnergyContainer::Zero() { level = 0; }


void EnergyContainer::Refill() {
    level.SetToMax();
}


// Component Functions
void EnergyContainer::Load(std::string upgrade_key, std::string unit_key) {
    // Component
    upgrade_key = "";

    // TODO: nice to have - ship mass goes down as fuel depleted
    mass = 0; 
    volume = 0;

    double capacity = 0.0;
 
    switch(type) {
        case EnergyType::Fuel:
        upgrade_name = "Fuel";
        capacity = UnitCSVFactory::GetVariable(unit_key, FUEL_CAPACITY, 1.0);
        break;

        case EnergyType::Energy:
        upgrade_name = "Capacitor";
        capacity = UnitCSVFactory::GetVariable(unit_key, CAPACITOR, 1.0);
        break;
        
        case EnergyType::FTL:
        upgrade_name = "FTL_Capacitor";
        capacity = UnitCSVFactory::GetVariable(unit_key, FTL_CAPACITOR, 1.0);
        break;

        case EnergyType::None:
        break;
    }
    
    SetCapacity(capacity);
}

void EnergyContainer::SaveToCSV(std::map<std::string, std::string>& unit) const {
    unit[FUEL_CAPACITY] = std::to_string(MaxLevel());
}

std::string EnergyContainer::Describe() const {
    return std::string();
}

bool EnergyContainer::CanDowngrade() const {
    return !Damaged();
}

bool EnergyContainer::Downgrade() {
    if(!CanDowngrade()) {
        return false;
    }

    level.SetMaxValue(0.0);
    return true;
}

bool EnergyContainer::CanUpgrade(const std::string upgrade_key) const {
    return !Damaged();
}

bool EnergyContainer::Upgrade(const std::string upgrade_key) {
    if(!CanUpgrade(upgrade_key)) {
        return false;
    }

    this->upgrade_key = upgrade_key;
    upgrade_name = UnitCSVFactory::GetVariable(upgrade_key, "Name", std::string());

    double capacity = 0.0;
 
    switch(type) {
        case EnergyType::Fuel:
        capacity = UnitCSVFactory::GetVariable(upgrade_key, FUEL_CAPACITY, 1.0);
        break;

        case EnergyType::Energy:
        capacity = UnitCSVFactory::GetVariable(upgrade_key, CAPACITOR, 1.0);
        break;
        
        case EnergyType::FTL:
        capacity = UnitCSVFactory::GetVariable(upgrade_key, FTL_CAPACITOR, 1.0);
        break;

        case EnergyType::None:
        break;
    }
    
    SetCapacity(capacity);
    return true;
}


void EnergyContainer::Damage() {
    level.RandomDamage();    
}

void EnergyContainer::DamageByPercent(double percent) {
    level.DamageByPercent(percent);    
}

void EnergyContainer::Repair() {
    level.RepairFully();
}

bool EnergyContainer::Damaged() const {
    return level.Damaged();
}


bool EnergyContainer::Installed() const {
    return level > 0.0;
}