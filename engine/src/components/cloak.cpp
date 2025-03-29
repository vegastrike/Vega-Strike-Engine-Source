/*
 * cloak.cpp
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

#include "components/cloak.h"
#include "cmd/unit_csv_factory.h"
#include "configuration/configuration.h"

Cloak::Cloak() :
    Component(),
    EnergyConsumer(nullptr, false, 0)
{
    type = ComponentType::Cloak;
    downgrade_private();
}

Cloak::~Cloak()
= default;


// Component Overrides
void Cloak::Load(std::string unit_key) {
    // Energy Consumer
    SetConsumption(UnitCSVFactory::GetVariable(unit_key, "Cloak_Energy", 0.0));

    // Cloak
    upgrade_private(unit_key);
}


void Cloak::SaveToCSV(std::map<std::string, std::string>& unit) const {
    unit["Cloak_Min"] = std::to_string(minimum);
    unit["Can_Cloak"] = std::to_string(Capable());
    unit["Cloak_Rate"] = std::to_string(rate);
    unit["Cloak_Energy"] = std::to_string(consumption);
    unit["Cloak_Glass"] = std::to_string(glass);
}


bool Cloak::CanDowngrade() const {
    return true;
}

bool Cloak::Downgrade() {
    Component::Downgrade();
    SetConsumption(0);

    status = CloakingStatus::disabled;

    rate = 100;
    glass = false;
    current = 0;
    minimum = 0;

    return true;
}

bool Cloak::CanUpgrade(const std::string upgrade_key) const {
    return true;
}

bool Cloak::Upgrade(const std::string upgrade_key) {
    upgrade_private(upgrade_key);

    return true;
}


// TODO: more granular damage
// TODO: damageable component
void Cloak::Damage() {
    status = CloakingStatus::damaged;
    current = 0;
    operational  = 0;
}

void Cloak::DamageByPercent(double percent) {
    // TODO: implement
    Damage();
}

void Cloak::Repair() {
    if(status == CloakingStatus::damaged) {
        status = CloakingStatus::ready;
        current = 0;
        operational  = 1;
    }
}

bool Cloak::Damaged() const {
    return status == CloakingStatus::damaged;
}

bool Cloak::Installed() const {
    return status != CloakingStatus::disabled;
}



// Cloak Methods

void Cloak::Update()
{
    // Unit is not capable of cloaking or damaged or just not cloaking
    if(status == CloakingStatus::disabled ||
       status == CloakingStatus::damaged ||
       status == CloakingStatus::ready) {
        return;
    }

    double actual_available_energy = Consume();
    if(actual_available_energy < 1.0) {
        // Insufficient energy to cloak ship
        status = CloakingStatus::decloaking;
    }

    // TODO: Use warp power for cloaking (SPEC capacitor)
    //const static bool warp_energy_for_cloak = configuration()->warp_config.use_warp_energy_for_cloak;




    // TODO: deplete has a more elegant solution for this code.
    // Also use a pointer to which is used instead of ifs


    if(status == CloakingStatus::decloaking) {
        current = std::max(0.0, current - rate * simulation_atom_var);

        if(current == 0) {
            status = CloakingStatus::ready;
        }
    }

    if(status == CloakingStatus::cloaking) {
        current = std::min(1.0, current + rate * simulation_atom_var);

        if(current > minimum) {
            status = CloakingStatus::cloaked;
        }
    }
}

void Cloak::Toggle() {
    // Unit is not capable of cloaking or damaged
    if(status == CloakingStatus::disabled ||
       status == CloakingStatus::damaged) {
        return;
    }

    // If we're ready start cloaking
    if(status == CloakingStatus::ready) {
        status = CloakingStatus::cloaking;
        return;
    }

    // In any other case, start decloaking
    status = CloakingStatus::decloaking;
}

void Cloak::Activate() {
    if(status == CloakingStatus::ready) {
        status = CloakingStatus::cloaking;
    }
}

void Cloak::Deactivate() {
    // Unit is not capable of cloaking or damaged or just not cloaking
    if(status == CloakingStatus::disabled ||
       status == CloakingStatus::damaged ||
       status == CloakingStatus::ready) {
        return;
    }

    // Start decloaking
    status = CloakingStatus::decloaking;
}

double Cloak::Consume()
{
    // Unit is not capable of cloaking or damaged or just not cloaking
    if (status == CloakingStatus::disabled ||
            status == CloakingStatus::damaged ||
            status == CloakingStatus::ready) {
        return 0.0;
    }
    return EnergyConsumer::Consume();
}

void Cloak::downgrade_private() {
    Component::Downgrade();
    SetConsumption(0);

    status = CloakingStatus::disabled;

    rate = 100;
    glass = false;
    current = 0;
    minimum = 0;


}

void Cloak::upgrade_private(const std::string upgrade_key) {
    Component::Upgrade(upgrade_key);

    SetConsumption(UnitCSVFactory::GetVariable(upgrade_key, "Cloak_Energy", 0.0));

    if(UnitCSVFactory::GetVariable(upgrade_key, "Can_Cloak", false)) {
        status = CloakingStatus::ready;
    } else {
        status = CloakingStatus::disabled;
    }

    glass = UnitCSVFactory::GetVariable(upgrade_key, "Cloak_Glass", false);
    rate = UnitCSVFactory::GetVariable(upgrade_key, "Cloak_Rate", 0.0);
    minimum = UnitCSVFactory::GetVariable(upgrade_key, "Cloak_Min", 0.0);
    minimum = std::min(1.0, std::max(0.0, minimum));
    current = 0;
}
