/*
 * repair_bot.cpp
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

#include "repair_bot.h"
#include "cmd/unit_csv_factory.h"

RepairBot::RepairBot() :
    Component() {
    type = ComponentType::RepairBot;
}


// Component Methods
void RepairBot::Load(std::string unit_key) {
    Component::Load(unit_key);

    repair_bot = UnitCSVFactory::GetVariable(unit_key, "repair", 0);
}

void RepairBot::SaveToCSV(std::map<std::string, std::string>& unit) const {
    unit["repair"] = std::to_string(repair_bot);
}

bool RepairBot::CanDowngrade() const {
    return (installed && !integral);
}

bool RepairBot::Downgrade() {
    if(!CanDowngrade()) {
        return false;
    }

    repair_bot = Resource<int>(0, 0, 0);
    installed = false;
    return true;
}

bool RepairBot::CanUpgrade(const std::string upgrade_key) const {
    return !integral;
}

bool RepairBot::Upgrade(const std::string upgrade_key) {
    if(!CanUpgrade(upgrade_key)) {
        return false;
    }

    Component::Load(upgrade_key);
    repair_bot = UnitCSVFactory::GetVariable(upgrade_key, "repair", 0);
    return true;
}

// Bot Methods
int RepairBot::Get() const {
    return repair_bot;
}

RepairBot::~RepairBot() = default;
