/*
 * fuel.cpp
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2024 Stephen G. Tuggy, Benjamen R. Meyer, Roy Falk and other Vega Strike Contributors
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

#include "fuel.h"

#include "unit_csv_factory.h"

#include <string>

const std::string FUEL_CAPACITY = "Fuel_Capacity";

Fuel::Fuel(): Component("", 0.0, 0.0, true),
              EnergyContainer(EnergyType::Fuel, 0.0) {}

void Fuel::Load(std::string upgrade_key, std::string unit_key) {
    // Component
    upgrade_name = "Fuel";
    upgrade_key = "";

    // TODO: nice to have - ship mass goes down as fuel depleted
    mass = 0; 
    volume = 0;
 
    double fuel_capacity = UnitCSVFactory::GetVariable(unit_key, FUEL_CAPACITY, 1.0);
    SetCapacity(fuel_capacity);
}

void Fuel::SaveToCSV(std::map<std::string, std::string>& unit) const {
    unit[FUEL_CAPACITY] = std::to_string(MaxLevel());
}

std::string Fuel::Describe() const {
    return std::string();
}

bool Fuel::CanDowngrade() const {
    return false;
}

bool Fuel::Downgrade() {
    return false;
}

bool Fuel::CanUpgrade(const std::string upgrade_name) const {
    return false;
}

bool Fuel::Upgrade(const std::string upgrade_name) {
    return false;
}


void Fuel::Damage() {
    level.RandomDamage();    
}

void Fuel::Repair() {
    level.RepairFully();
}

bool Fuel::Damaged() const {
    return level.Damaged();
}


bool Fuel::Installed() const {
    return true;
}