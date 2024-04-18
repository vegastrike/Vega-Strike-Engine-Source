/*
 * drive.cpp
 *
 * Copyright (C) 2001-2023 Daniel Horn, Benjaman Meyer, Roy Falk, Stephen G. Tuggy,
 * and other Vega Strike contributors.
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
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */

#include "drive.h"

// Note that usage_factor should be closely related to normal drive's usage factor.
// In most cases it should be 1.0 and the difference should be modelled using the afterburner_level.
Drive::Drive(EnergyContainer *source, 
             double usage_factor):
             Component("", 0.0, 0.0, false),
             EnergyConsumer(source, false),
             usage_factor(usage_factor) {
    consumption = usage_factor * 1.0 * mass * simulation_atom_var;
}


// Component Methods
void Drive::Load(std::string upgrade_key, std::string unit_key) {
}      

void Drive::SaveToCSV(std::map<std::string, std::string>& unit) const {
}

std::string Drive::Describe() const {
    return std::string();
} 

bool Drive::CanDowngrade() const {
    return !Damaged();
}

bool Drive::Downgrade() {
    if(!CanDowngrade()) {
        return false;
    }
}

bool Drive::CanUpgrade(const std::string upgrade_name) const {
    return !Damaged();
}

bool Drive::Upgrade(const std::string upgrade_name) {
    if(!CanUpgrade(upgrade_name)) {
        return false;
    }
}

void Drive::Damage() {
    return;
}

void Drive::Repair() {
}

bool Drive::Damaged() const {
    return false;
}

bool Drive::Installed() const {
    return false;
}