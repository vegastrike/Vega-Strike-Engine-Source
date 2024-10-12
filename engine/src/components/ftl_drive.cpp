/*
 * ftl_drive.cpp
 *
 * Copyright (C) 2001-2023 Daniel Horn, Benjamen Meyer, Roy Falk, Stephen G. Tuggy,
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

#include "ftl_drive.h"

#include "unit_csv_factory.h"

FtlDrive::FtlDrive() : 
    Component(),
    EnergyConsumer(nullptr, 0, false) {}

FtlDrive::FtlDrive(EnergyContainer *source):
    Component(0.0, 0.0, true),
    EnergyConsumer(source, false) {}


bool FtlDrive::Enabled() const {
    return Installed() && Operational();
}


// Component Methods
void FtlDrive::Load(std::string upgrade_key, 
                    std::string unit_key, 
                    double ftl_factor) {
    Component::Load(upgrade_key, unit_key);

    // Consumer
    double energy = UnitCSVFactory::GetVariable(unit_key, "Warp_Usage_Cost", 0.0f);
    SetConsumption(energy / ftl_factor);

    // FTL Drive
}      

void FtlDrive::SaveToCSV(std::map<std::string, std::string>& unit) const {
    unit["Warp_Usage_Cost"] = std::to_string(consumption);
}

std::string FtlDrive::Describe() const {
    return std::string();
} 

// FTL drive is integrated and so cannot be upgraded/downgraded
bool FtlDrive::CanDowngrade() const {
    return false;
}

bool FtlDrive::Downgrade() {
    return false;
}

bool FtlDrive::CanUpgrade(const std::string upgrade_name) const {
    return false;
}

bool FtlDrive::Upgrade(const std::string upgrade_name) {
    return false;
}

