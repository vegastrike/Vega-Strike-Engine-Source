/*
 * jump_drive.cpp
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

#include "components/jump_drive.h"

#include "cmd/unit_csv_factory.h"
#include "configuration/configuration.h"

JumpDrive::JumpDrive() :
    Component(),
    EnergyConsumer(nullptr, false, 0),
    destination(-1),
    delay(0.0) {
    type = ComponentType::JumpDrive;
}

JumpDrive::JumpDrive(EnergyContainer *source):
    Component(0.0, 0.0, true),
    EnergyConsumer(source, false),
    delay(0.0) {
    type = ComponentType::JumpDrive;
}

JumpDrive::~JumpDrive()
= default;


int JumpDrive::Destination() const {
    return destination;
}

bool JumpDrive::IsDestinationSet() const {
    return !Destroyed() && destination != -1;
}

void JumpDrive::SetDestination(int destination) {
    if(!Destroyed()) {
        this->destination = destination;
    }
}

void JumpDrive::UnsetDestination() {
    destination = -1;
}

double JumpDrive::Delay() const {
    return delay;
}

void JumpDrive::SetDelay(double delay) {
    this->delay += delay;
}

bool JumpDrive::Enabled() const {
    return Installed() && !Destroyed();
}


// Component Methods
void JumpDrive::Load(std::string unit_key) {
    Component::Load(unit_key);

    // Consumer
    double energy = UnitCSVFactory::GetVariable(unit_key, "Outsystem_Jump_Cost", 0.0f);
    // Jump drive is unique - consumption and atom_consumption are identical
    atom_consumption = consumption = energy * vega_config::config->components.fuel.jump_drive_factor;

    // Jump Drive
    installed = UnitCSVFactory::GetVariable(unit_key, "Jump_Drive_Present", false);
    delay = UnitCSVFactory::GetVariable(unit_key, "Jump_Drive_Delay", 0);
}

void JumpDrive::SaveToCSV(std::map<std::string, std::string>& unit) const {
    unit["Jump_Drive_Present"] = std::to_string(Installed());
    unit["Jump_Drive_Delay"] = std::to_string(delay);
    unit["Outsystem_Jump_Cost"] = std::to_string(consumption / vega_config::config->components.fuel.jump_drive_factor);
}

bool JumpDrive::CanDowngrade() const {
    return !Damaged();
}

bool JumpDrive::Downgrade() {
    if(!CanDowngrade()) {
        return false;
    }

    Component::Downgrade();

    return true;
}

bool JumpDrive::CanUpgrade(const std::string upgrade_key) const {
    return !Damaged() && !Installed();
}

bool JumpDrive::Upgrade(const std::string upgrade_key) {
    if(!CanUpgrade(upgrade_key)) {
        return false;
    }

    Component::Upgrade(upgrade_key);
    return true;
}

double JumpDrive::Consume()
{
    return 0.0;
    // return EnergyConsumer::Consume();
}



