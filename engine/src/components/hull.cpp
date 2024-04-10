/*
 * hull.cpp
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2024 Stephen G. Tuggy, Benjamen R. Meyer, Roy Falk and other Vega Strike Contributors
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

#include "hull.h"

#include "unit_csv_factory.h"

const std::string HULL = "Hull";

Hull::Hull(): Component("", 0.0, 0.0, true),
              DamageableLayer(0, FacetConfiguration::one, 
                              Health(0, 1), true) {}

void Hull::Load(std::string upgrade_key, std::string unit_key, 
                      Unit *unit) {
    // Component
    upgrade_name = "Hull";
    upgrade_key = "";

    mass = UnitCSVFactory::GetVariable(unit_key, "mass", 0.0);
    volume = 0;
 
    // Damageable Layer
    double hull_current = UnitCSVFactory::GetVariable(unit_key, "hull", 1.0);
    double hull_max = UnitCSVFactory::GetVariable(upgrade_key, "hull", 1.0);
    facets[0].health.SetMaxValue(hull_max);
    facets[0].health.Set(hull_current);
}

void Hull::SaveToCSV(std::map<std::string, std::string>& unit) const {
    for(int i=0;i<8;i++) {
        unit[HULL] = std::to_string(facets[0].health.Value());
    }
}

std::string Hull::Describe() const {
    return std::string();
}

bool Hull::CanDowngrade() const {
    return false;
}

bool Hull::Downgrade() {
    return false;
}

bool Hull::CanUpgrade(const std::string upgrade_name) const {
    return false;
}

bool Hull::Upgrade(const std::string upgrade_name) {
    return false;
}

// Handled by LibDamage
// Consider exposing this as API for python
// Currently has DealDamageToHull which serves a similar purpose
void Hull::Damage() {}

void Hull::Repair() {
    facets[0].health.SetToMax();
}

bool Hull::Damaged() const {
    return facets[0].health.Damaged();
}


bool Hull::Installed() const {
    return true;
}