/*
 * hull.cpp
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

#include "hull.h"

#include "cmd/unit_csv_factory.h"

static const Damage normal_and_phase_damage = Damage(1.0,1.0);


Hull::Hull() :
    Component(0.0, 0.0, true, true),
    DamageableLayer(0, FacetConfiguration::one, 0.0, normal_and_phase_damage, true) {
    type = ComponentType::Hull;
    upgrade_name = "Hull";
    installed = true;
}


// Component Methods
void Hull::Load(std::string unit_key) {
    Component::Load(unit_key);

    Resource<double> temp_hull = Resource<double>(UnitCSVFactory::GetVariable(unit_key, "Hull", std::string()));
    facets.clear();
    facets.push_back(temp_hull);
}

void Hull::SaveToCSV(std::map<std::string, std::string>& unit) const {
    unit["Hull"] = facets[0].Serialize();
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

double Hull::Percent() const {
    return facets[0].Percent();
}

void Hull::Destroy() {
    facets[0] = 0;
}

double Hull::Get() {
    return facets[0].Value();
}

double Hull::GetMax() {
    return facets[0].MaxValue();
}

void Hull::Set(double value) {
    facets[0] = Resource<double>(value, 0, value);
}

Hull::~Hull() = default;

double Hull::PercentOperational() const {
    return Percent();
}

bool Hull::Damaged() const {
    return Percent() < 1;
}

void Hull::Repair() {
    for(auto& facet : facets) {
        facet.RepairFully();
    }
}
