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
#include "cmd/unit_csv_factory.h"
#include "configuration/configuration.h"

#include <iostream>

const std::string FUEL_CAPACITY = "Fuel_Capacity";
const std::string CAPACITOR = "Primary_Capacitor";
const std::string FTL_CAPACITOR = "Warp_Capacitor";

EnergyContainer::EnergyContainer(ComponentType type):
                                 Component(0.0, 0.0, false),
                                 level(Resource<double>(0.0,0.0,0.0)) {
    switch(type) {
        case ComponentType::Fuel:
        case ComponentType::Capacitor:
        case ComponentType::FtlCapacitor:
        this->type = type;
        break;

        default:
        this->type = ComponentType::None;
        break;
    }
}


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
    level.Set(level.AdjustedValue());
}


// Component Functions
void EnergyContainer::Load(std::string unit_key) {
    // Component
    Component::Load(unit_key);

    // TODO: nice to have - ship mass goes down as fuel depleted

    switch(type) {
        case ComponentType::Fuel:
        level = Resource<double>(UnitCSVFactory::GetVariable(unit_key, FUEL_CAPACITY, std::string("0.0")), configuration()->fuel.fuel_factor);
        break;

        case ComponentType::Capacitor:
        level = Resource<double>(UnitCSVFactory::GetVariable(unit_key, CAPACITOR, std::string("0.0")), configuration()->fuel.energy_factor);
        break;

        case ComponentType::FtlCapacitor:
        level = Resource<double>(UnitCSVFactory::GetVariable(unit_key, FTL_CAPACITOR, std::string("0.0")), configuration()->fuel.ftl_energy_factor);
        break;

        default: // This really can't happen
        std::cerr << "Illegal container type in EnergyContainer::Load" << std::flush;
        abort();
    }

    operational = level.AdjustedValue() / level.MaxValue();
}

void EnergyContainer::SaveToCSV(std::map<std::string, std::string>& unit) const {
    switch(type) {
        case ComponentType::Fuel:
        unit[FUEL_CAPACITY] = level.Serialize(configuration()->fuel.fuel_factor);
        break;

        case ComponentType::Capacitor:
        unit[CAPACITOR] = level.Serialize(configuration()->fuel.energy_factor);
        break;

        case ComponentType::FtlCapacitor:
        unit[FTL_CAPACITOR] = level.Serialize(configuration()->fuel.ftl_energy_factor);
        break;

        default: // This really can't happen
        std::cerr << "Illegal container type in EnergyContainer::SaveToCSV" << std::flush;
        abort();
    }
}



bool EnergyContainer::CanDowngrade() const {
    return !Damaged();
}

bool EnergyContainer::Downgrade() {
    if(!CanDowngrade()) {
        return false;
    }

    // Component
    Component::Downgrade();

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

    // Component
    Component::Upgrade(upgrade_key);

    // TODO: nice to have - ship mass goes down as fuel depleted

    switch(type) {
        case ComponentType::Fuel:
        level.SetMaxValue(UnitCSVFactory::GetVariable(upgrade_key, FUEL_CAPACITY, 0.0));
        break;

        case ComponentType::Capacitor:
        level.SetMaxValue(UnitCSVFactory::GetVariable(upgrade_key, CAPACITOR, 0.0));
        break;

        case ComponentType::FtlCapacitor:
        level.SetMaxValue(UnitCSVFactory::GetVariable(upgrade_key, FTL_CAPACITOR, 0.0));
        break;

        default: // This really can't happen
        abort();
    }

    return true;
}


void EnergyContainer::Damage() {
    level.RandomDamage();
    operational = level.AdjustedValue() / level.MaxValue();
}

void EnergyContainer::DamageByPercent(double percent) {
    level.DamageByPercent(percent);
    operational = level.AdjustedValue() / level.MaxValue();
}

void EnergyContainer::Repair() {
    level.RepairFully();
    operational = 1.0;
}

bool EnergyContainer::Damaged() const {
    return level.Damaged();
}


bool EnergyContainer::Installed() const {
    return level > 0.0;
}
