/*
 * armor.cpp
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

#include "armor.h"

#include "damageable_layer.h"
#include "unit_csv_factory.h"

const std::string armor_facets[] = {
    "Moment_Of_Inertia",
    "Armor_Front_Top_Right",
    "Armor_Front_Top_Left",
    "Armor_Front_Bottom_Right",
    "Armor_Front_Bottom_Left",
    "Armor_Back_Top_Right",
    "Armor_Back_Top_Left",
    "Armor_Back_Bottom_Right",
    "Armor_Back_Bottom_Left"
};

Armor::Armor(): Component("", 0.0, 0.0, false),
                DamageableLayer(1, FacetConfiguration::eight, 
                                Health(0,0), false) {}

void Armor::Load(std::string upgrade_key, std::string unit_key) {
    // Component
    Component::Load(upgrade_key, unit_key);

 
    // Damageable Layer
    std::string upgrade_type_string = UnitCSVFactory::GetVariable(upgrade_key, "Name", std::string());

    std::vector<double> armor_values;

    // Max values from Upgrade
    for(int i = 0;i < 8;i++) {
        double armor_facet_max = UnitCSVFactory::GetVariable(upgrade_key, armor_facets[i], 0.0);
        armor_values.push_back(armor_facet_max);
    }

    UpdateFacets(armor_values);

    // Current values from Save Game
    for(int i = 0;i < 8;i++) {
        double armor_value = UnitCSVFactory::GetVariable(unit_key, 
                                               armor_facets[i], 0.0);
        facets[i].health.Set(armor_value);
    }
}

void Armor::SaveToCSV(std::map<std::string, std::string>& unit) const {
    for(int i=0;i<8;i++) {
        unit[armor_facets[i]] = std::to_string(facets[i].health.Value());
    }
}

std::string Armor::Describe() const {
    return std::string();
}

bool Armor::CanDowngrade() const {
    if(integral) return false;

    // Nothing to downgrade
    if(upgrade_name.empty()) return false;

    // Other considerations - damaged?!

    return true;
}

bool Armor::Downgrade() {
    if(!CanDowngrade()) {
        return false;
    }

    this->upgrade_name.clear();
    //mass = 0;
    // volume = 0;
    for(int i = 0;i < 8;i++) {
        facets[i].health.SetMaxValue(0.0);
    }

    return true;
}

bool Armor::CanUpgrade(const std::string upgrade_name) const {

    if(integral) {
        return false;
    }

    // Will allow swapping upgrades.
    // TODO: make base_computer sell previous upgrade
    
    // Other considerations - damaged?!

    return true;
}

bool Armor::Upgrade(const std::string upgrade_name) {
    if(!CanUpgrade(upgrade_name)) {
        return false;
    }

    if(!UnitCSVFactory::HasUnit(upgrade_name)) {
        return false;
    }

    std::vector<double> armor_values;

    for(int i = 0;i < 8;i++) {
        double armor_value = UnitCSVFactory::GetVariable(upgrade_name, 
                                               armor_facets[i], 0.0);
        armor_values.push_back(armor_value);
    }

    UpdateFacets(armor_values);

    return true;
}

// Handled by LibDamage
// Consider exposing this as API for python
// Currently has DealDamageToHull which serves a similar purpose
void Armor::Damage() {}

void Armor::Repair() {
    for(int i = 0;i < 8;i++) {
        //armor_layer_->facets[i].health = armor_layer_->facets[i].max_health;
    }
}

bool Armor::Damaged() const {
    for(const Health& facet : facets) {
        if(facet.health.Damaged()) {
            return true;
        }
    }

    return false;
}


bool Armor::Installed() const {
    return TotalMaxLayerValue() > 0;
}