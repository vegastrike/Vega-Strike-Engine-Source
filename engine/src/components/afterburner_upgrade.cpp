/*
 * afterburner_upgrade.cpp
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

#include "components/afterburner_upgrade.h"

#include "components/afterburner.h"
#include "cmd/unit_csv_factory.h"

#include <boost/format.hpp>


AfterburnerUpgrade::AfterburnerUpgrade(Afterburner *afterburner):
    Component(), afterburner(afterburner), thrust(1.0),
    speed(1.0), consumption(1.0) {
    type = ComponentType::AfterburnerUpgrade;
}


// Component Methods
void AfterburnerUpgrade::Load(std::string unit_key) {
    // Not supported with the current file format
    // Instead, we save the modified Afterburner stats and load the upgrade above.
    // Then, if we sell the upgrade, we get the original.
}

void AfterburnerUpgrade::SaveToCSV(std::map<std::string, std::string>& unit) const {
    // Not supported with the current file format
    // Instead, we save the modified Afterburner stats and load the upgrade above.
    // Then, if we sell the upgrade, we get the original.
}


// Can only upgrade/downgrade if Afterburner is undamaged.
// Otherwise, there are a lot of edge cases.
bool AfterburnerUpgrade::CanDowngrade() const {
    return !afterburner->Damaged();
}

bool AfterburnerUpgrade::Downgrade() {
    if(!CanDowngrade()) {
        return false;
    }

    // Component
    Component::Downgrade();

    // Remove effects of upgrade on afterburner
    afterburner->thrust.SetMaxValue(afterburner->thrust.MaxValue() / thrust);
    afterburner->speed.SetMaxValue(afterburner->speed.MaxValue() / speed);
    afterburner->SetConsumption(afterburner->GetConsumption() / consumption);

    // Remove modifiers
    thrust = speed = consumption = 1.0;

    return true;
}

bool AfterburnerUpgrade::CanUpgrade(const std::string upgrade_key) const {
    return !afterburner->Damaged();
}

bool AfterburnerUpgrade::Upgrade(const std::string upgrade_key) {
    if(!CanUpgrade(upgrade_key)) {
        return false;
    }

    // Component
    Component::Upgrade(upgrade_key);

    // Load modifiers
    thrust = UnitCSVFactory::GetVariable(upgrade_key, "Afterburner_Accel", 1.0);
    speed = UnitCSVFactory::GetVariable(upgrade_key, "Afterburner_Speed_Governor", 1.0);
    consumption = UnitCSVFactory::GetVariable(upgrade_key, "Afterburner_Usage_Cost", 1.0);

    // Add effects of upgrade on afterburner
    afterburner->thrust.SetMaxValue(afterburner->thrust.MaxValue() * thrust);
    afterburner->speed.SetMaxValue(afterburner->speed.MaxValue() * speed);
    afterburner->SetConsumption(afterburner->GetConsumption() * consumption);

    return true;
}

