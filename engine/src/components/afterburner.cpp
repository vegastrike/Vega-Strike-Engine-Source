/*
 * afterburner.cpp
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

#include "afterburner.h"
#include "unit_csv_factory.h"





// Note that usage_factor should be closely related to normal drive's usage factor.
// In most cases it should be 1.0 and the difference should be modelled using the afterburner_level.

Afterburner::Afterburner(EnergyContainer *source, double usage_factor):
                         Component("", 0.0, 0.0, false),
                         EnergyConsumer(source, false),
                         usage_factor(usage_factor) {}
    

// Component Methods
void Afterburner::Load(std::string upgrade_key, std::string unit_key) {
    type = UnitCSVFactory::GetVariable(unit_key, "Afterburner_Type", 0);
    usage_cost = UnitCSVFactory::GetVariable(unit_key, "Afterburner_Usage_Cost", 3.0);
    acceleration = UnitCSVFactory::GetVariable(unit_key, "Afterburner_Accel", 1.0);
    speed_governor = UnitCSVFactory::GetVariable(unit_key, "Afterburner_Speed_Governor", 1.0);
}      

void Afterburner::SaveToCSV(std::map<std::string, std::string>& unit) const {
    unit["Afterburner_Type"] = std::to_string(type);
    unit["Afterburner_Usage_Cost"] = std::to_string(usage_cost);
    unit["Afterburner_Accel"] = std::to_string(acceleration);
    unit["Afterburner_Speed_Governor"] = std::to_string(speed_governor);
}

std::string Afterburner::Describe() const {
    return std::string();
} 

bool Afterburner::CanDowngrade() const {
    return !Damaged();
}

bool Afterburner::Downgrade() {
    type = 1;
    usage_cost = 1.0;
    acceleration = 1.0;
    speed_governor  = 1.0;
}

bool Afterburner::CanUpgrade(const std::string upgrade_name) const {
    return !Damaged();
}

bool Afterburner::Upgrade(const std::string upgrade_name) {
    if(!CanUpgrade(upgrade_key)) {
        return false;
    }

    type = UnitCSVFactory::GetVariable(upgrade_name, "Afterburner_Type", 1);
    usage_cost = UnitCSVFactory::GetVariable(upgrade_name, "Afterburner_Usage_Cost", 1.0);
    acceleration = UnitCSVFactory::GetVariable(upgrade_name, "Afterburner_Accel", 1.0);
    speed_governor = UnitCSVFactory::GetVariable(upgrade_name, "Afterburner_Speed_Governor", 1.0);
}

void Afterburner::Damage() {
    // Can't damage the afterburners. 
    // Instead, the drive will be damaged.
    return;
}
 
void Afterburner::Repair() {

}

bool Afterburner::Damaged() const {
    return false;
}

bool Afterburner::Installed() const {
    return acceleration > 1.0;
}