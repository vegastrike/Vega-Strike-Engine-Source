/*
 * component.cpp
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2023 Stephen G. Tuggy, Benjamen R. Meyer, Roy Falk and other Vega Strike Contributors
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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

// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#include "component.h"
#include "unit_csv_factory.h"

Component::Component(std::string upgrade_name, double mass, double volume,
                     bool integral):
                     upgrade_name(upgrade_name),
                     mass(mass), volume(volume),
                     integral(integral) {}


void Component::Load(std::string upgrade_key, std::string unit_key) {
    upgrade_name = UnitCSVFactory::GetVariable(upgrade_key, "Name", std::string());
    this->upgrade_key = upgrade_key;
    
    mass = UnitCSVFactory::GetVariable(upgrade_key, "Mass", 0.0);
    // TODO: volume = UnitCSVFactory::GetVariable(upgrade_key, "Volume", 0.0);
    // TODO: bool integral = false;
}

// TODO: convert to std::pair<bool, double>
bool Component::CanWillUpDowngrade(const std::string upgrade_key,
                                   bool upgrade, bool apply) {
    if(upgrade) {
        if(apply) {
            return Upgrade(upgrade_key);
        } else {
            return CanUpgrade(upgrade_key);
        }
    } else {
        if(apply) {
            return Downgrade();
        } else {
            return CanDowngrade();
        }
    }                                        
}

bool Component::Downgrade() {
    upgrade_name = std::string();
    upgrade_key = std::string();
    
    mass = 0.0;
    volume = 0.0;
}

void Component::SetIntegral(bool integral) {
    this->integral = integral;
}