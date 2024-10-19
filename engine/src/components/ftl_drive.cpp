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
#include "configuration/configuration.h"

FtlDrive::FtlDrive() : 
    Component(),
    EnergyConsumer(nullptr, 0, false) {
    type = ComponentType::FtlDrive;
}

FtlDrive::FtlDrive(EnergyContainer *source):
    Component(0.0, 0.0, true),
    EnergyConsumer(source, false) {
    type = ComponentType::FtlDrive;
}


bool FtlDrive::Enabled() const {
    return Installed() && Operational();
}


// Component Methods
void FtlDrive::Load(std::string upgrade_key, 
                    std::string unit_key) {
    Component::Load(upgrade_key, unit_key);

    const double ftl_drive_factor = configuration()->fuel.ftl_drive_factor;

    // Consumer
    double energy = UnitCSVFactory::GetVariable(unit_key, "Warp_Usage_Cost", 0.0f);
    SetConsumption(energy * ftl_drive_factor);

    // FTL Drive
}      

void FtlDrive::SaveToCSV(std::map<std::string, std::string>& unit) const {
    unit["Warp_Usage_Cost"] = std::to_string(consumption);
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

