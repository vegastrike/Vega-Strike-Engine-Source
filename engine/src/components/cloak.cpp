/*
 * cloak.cpp
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

#include "cloak.h"
#include "energy_consumer.h"
#include "unit_csv_factory.h"
#include "vegastrike.h"
#include "configuration/configuration.h"

Cloak::Cloak(): Component(std::string(), 0.0, 0.0, false),
                EnergyConsumer(nullptr, false) {
    status = CloakingStatus::disabled;

    energy = 0;
    rate = 100;
    glass = false;
    current = 0;
    minimum = 0;
}


Cloak::Cloak(EnergyContainer *source) : 
             Component(std::string(), 0.0, 0.0, false),
             EnergyConsumer(source, false) {
    status = CloakingStatus::disabled;

    energy = 0;
    rate = 100;
    glass = false;
    current = 0;
    minimum = 0;
}

void Cloak::Load(std::string upgrade_key, std::string unit_key) {
    // TODO: load undamaged from upgrade_key
    if(UnitCSVFactory::GetVariable(unit_key, "Can_Cloak", false)) {
        status = CloakingStatus::ready;
    } else {
        status = CloakingStatus::disabled;
    }

    glass = UnitCSVFactory::GetVariable(unit_key, "Cloak_Glass", false);
    rate = UnitCSVFactory::GetVariable(unit_key, "Cloak_Rate", 0.0);
    energy = UnitCSVFactory::GetVariable(unit_key, "Cloak_Energy", 0.0);
    minimum = UnitCSVFactory::GetVariable(unit_key, "Cloak_Min", 0.0);
    minimum = std::min(1.0, std::max(0.0, minimum));
    current = 0;
}

void Cloak::SaveToCSV(std::map<std::string, std::string>& unit) const {
    unit["Cloak_Min"] = std::to_string(minimum);
    unit["Can_Cloak"] = std::to_string(Capable());
    unit["Cloak_Rate"] = std::to_string(rate);
    unit["Cloak_Energy"] = std::to_string(energy);
    unit["Cloak_Glass"] = std::to_string(glass);
}


std::string Cloak::Describe() const {
    return std::string();
}

bool Cloak::CanDowngrade() const {
    if(integral) return false;

    // Nothing to downgrade
    if(!Installed()) return false;

    // Other considerations - damaged?!

    return true;
}

bool Cloak::Downgrade() {
    if(!CanDowngrade()) {
        return false;
    }

    this->upgrade_name.clear();
    mass = 0;
    volume = 0;
    
    status = CloakingStatus::disabled;

    energy = 0;
    rate = 100;
    glass = false;
    current = 0;
    minimum = 0;

    return true;
}

bool Cloak::CanUpgrade(const std::string upgrade_name) const {
    if(integral) {
        return false;
    }

    // Will allow swapping upgrades.
    // TODO: make base_computer sell previous upgrade
    
    // Other considerations - damaged?!
}

bool Cloak::Upgrade(const std::string upgrade_name) {
    if(!CanUpgrade(upgrade_name)) {
        return false;
    }

    if(!UnitCSVFactory::HasUnit(upgrade_name)) {
        return false;
    }

    glass = UnitCSVFactory::GetVariable(upgrade_name, "Cloak_Glass", false);
    rate = UnitCSVFactory::GetVariable(upgrade_name, "Cloak_Rate", 0.0);
    energy = UnitCSVFactory::GetVariable(upgrade_name, "Cloak_Energy", 0.0);
    minimum = UnitCSVFactory::GetVariable(upgrade_name, "Cloak_Min", 0.0);
    minimum = std::min(1.0, std::max(0.0, minimum));
    current = 0;

    return true;
}

// TODO: more granular damage
void Cloak::Damage() {
    status = CloakingStatus::damaged;
    current = 0;
}

void Cloak::Repair() {
    if(status == CloakingStatus::damaged) {
        status = CloakingStatus::ready;
        current = 0;
    }
}

bool Cloak::Damaged() const {
    return (status == CloakingStatus::damaged);
}

bool Cloak::Installed() const {

}


void Cloak::Update()
{
    // Unit is not capable of cloaking or damaged or just not cloaking
    if(status == CloakingStatus::disabled ||
       status == CloakingStatus::damaged ||
       status == CloakingStatus::ready) {
        return;
    }

    double power = Consume();

    // Insufficient energy to cloak ship
    if(power < 1.0) {
        std::cerr << "No power to cloak\n";
        status = CloakingStatus::decloaking;
    } 

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
