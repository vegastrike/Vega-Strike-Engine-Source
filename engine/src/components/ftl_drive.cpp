/*
 * ftl_drive.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
 *
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
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "ftl_drive.h"

#include "cmd/unit_csv_factory.h"
#include "configuration/configuration.h"

FtlDrive::FtlDrive() :
    Component(0.0, 0.0, true, true),
    EnergyConsumer(nullptr, false, 0),
    enabled(false) {
    type = ComponentType::FtlDrive;
    upgrade_name = "FTL Drive";
    installed = true;
}

FtlDrive::FtlDrive(EnergyContainer *source):
    Component(0.0, 0.0, true),
    EnergyConsumer(source, false),
    enabled(false) {
    type = ComponentType::FtlDrive;
    upgrade_name = "FTL Drive";
    installed = true;
}

FtlDrive::~FtlDrive()
= default;

void FtlDrive::Enable() {
    enabled = true;
}

void FtlDrive::Disable() {
    enabled = false;
}

void FtlDrive::Toggle() {
    enabled = !enabled;
}

bool FtlDrive::Enabled() const {
    return Installed() && Operational() && enabled;
}


// Component Methods
void FtlDrive::Load(std::string unit_key) {
    Component::Load(unit_key);

    // Consumer
    double energy = UnitCSVFactory::GetVariable(unit_key, "Warp_Usage_Cost", 0.0f);
    SetConsumption(energy * configuration().components.ftl_drive.factor);

    // FTL Drive
}

void FtlDrive::SaveToCSV(std::map<std::string, std::string>& unit) const {
    unit["Warp_Usage_Cost"] = std::to_string(consumption  / configuration().components.ftl_drive.factor);
}

// FTL drive is integrated and so cannot be upgraded/downgraded
bool FtlDrive::CanDowngrade() const {
    return false;
}

bool FtlDrive::Downgrade() {
    return false;
}

bool FtlDrive::CanUpgrade(const std::string upgrade_key) const {
    return false;
}

bool FtlDrive::Upgrade(const std::string upgrade_key) {
    return false;
}

double FtlDrive::Consume()
{
    if (!enabled)
    {
        return 0.0;
    }
    return EnergyConsumer::Consume();
}

