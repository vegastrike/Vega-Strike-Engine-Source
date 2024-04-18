/*
 * jump_drive.cpp
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

#include "jump_drive.h"

JumpDrive::JumpDrive(EnergyContainer *source):
    Component("", 0.0, 0.0, true),
    EnergyConsumer(source, false),
    delay(0.0) {}

bool JumpDrive::Ready() { 
    return true;//installed && enabled; 
}

void JumpDrive::SetDestination(int destination) { 
    this->destination = destination; 
}

// Component Methods
void JumpDrive::Load(std::string upgrade_key, std::string unit_key) {
}      

void JumpDrive::SaveToCSV(std::map<std::string, std::string>& unit) const {
}

std::string JumpDrive::Describe() const {
    return std::string();
} 

bool JumpDrive::CanDowngrade() const {
    return !Damaged();
}

bool JumpDrive::Downgrade() {
    if(!CanDowngrade()) {
        return false;
    }
}

bool JumpDrive::CanUpgrade(const std::string upgrade_name) const {
    return !Damaged();
}

bool JumpDrive::Upgrade(const std::string upgrade_name) {
    if(!CanUpgrade(upgrade_name)) {
        return false;
    }
}

void JumpDrive::Damage() {
    return;
}

void JumpDrive::Repair() {
}

bool JumpDrive::Damaged() const {
    return false;
}

bool JumpDrive::Installed() const {
    return false;
}