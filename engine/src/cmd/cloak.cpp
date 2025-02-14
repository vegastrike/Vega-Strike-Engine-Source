/*
 * cloak.cpp
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

#include "cloak.h"
#include "unit_csv_factory.h"
#include "vegastrike.h"
#include "configuration/configuration.h"

Cloak::Cloak()
{
    status = CloakingStatus::disabled;

    energy = 0;
    rate = 100;
    glass = false;
    current = 0;
    minimum = 0;
}

Cloak::Cloak(std::string unit_key)
{
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

void Cloak::Save(std::map<std::string, std::string>& unit)
{
    unit["Cloak_Min"] = std::to_string(minimum);
    unit["Can_Cloak"] = std::to_string(Capable());
    unit["Cloak_Rate"] = std::to_string(rate);
    unit["Cloak_Energy"] = std::to_string(energy);
    unit["Cloak_Glass"] = std::to_string(glass);
}

void Cloak::Update(Energetic *energetic)
{
    // Unit is not capable of cloaking or damaged or just not cloaking
    if(status == CloakingStatus::disabled ||
       status == CloakingStatus::damaged ||
       status == CloakingStatus::ready) {
        return;
    }

    // Use warp power for cloaking (SPEC capacitor)
    const static bool warp_energy_for_cloak = configuration()->warp_config.use_warp_energy_for_cloak;
    double available_energy = warp_energy_for_cloak ? energetic->warpenergy : energetic->energy.Value();


    // Insufficient energy to cloak ship
    if(available_energy < this->energy) {
        status = CloakingStatus::decloaking;
    } else {
        // Subtract the energy used
        if (warp_energy_for_cloak) {
            energetic->warpenergy -= (simulation_atom_var * energy);
        } else {
            energetic->energy -= (simulation_atom_var * energy);
        }
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
