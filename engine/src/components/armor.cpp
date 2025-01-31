/*
 * armor.cpp
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

#include "armor.h"
#include "damage.h"
#include "unit_csv_factory.h"

int Armor::front = 0;
int Armor::back = 1;
int Armor::left = 2;
int Armor::right = 3;

static const Damage normal_and_phase_damage = Damage(1.0,1.0);

Armor::Armor() : 
    Component(), 
    DamageableLayer(1, FacetConfiguration::four, 1.0, normal_and_phase_damage, false) {
    type = ComponentType::Armor;
}


// Component Methods
void Armor::Load(std::string unit_key) {
    Component::Load(unit_key);

    facets.clear();
    
    // Armor
    // We support 3 options:
    // 1. Minimized armor = x (single value). 
    // 2. New detailed armor (Front, back, left, right).
    // 3. Old detailed (Front-left-top, ...). 8 facets converted to 4. 
    const std::string armor_single_value_string = UnitCSVFactory::GetVariable(unit_key, "armor", std::string());

    if(armor_single_value_string != "") {
        // Minimized
        const double armor_single_value = std::stod(armor_single_value_string, 0);
        
        for (unsigned int i = 0; i < number_of_facets; i++) {
            facets.push_back(Resource<double>(armor_single_value,0,armor_single_value));
        }
    } else {
        // Try new
        std::string armor_keys[] = {"armor_front", "armor_back",
            "armor_left", "armor_right"};
        bool new_form = true;
        for (int i = 0; i < 4; i++) {
            const std::string armor_string_value = UnitCSVFactory::GetVariable(unit_key, armor_keys[i], std::string());
            if(armor_string_value.empty()) {
                new_form = false;
                break;
            }

            double armor_value = std::stod(armor_string_value);
            facets.push_back(Resource<double>(armor_value,0,armor_value));
        }

        // Fallback to old
        if(!new_form) {
            const std::string armor_keys[] = {"Armor_Front_Top_Left", 
                                              "Armor_Front_Top_Right",
                                              "Armor_Front_Bottom_Left", 
                                              "Armor_Front_Bottom_Right",
                                              "Armor_Back_Top_Left", 
                                              "Armor_Back_Top_Right",
                                              "Armor_Back_Bottom_Left", 
                                              "Armor_Back_Bottom_Right"};

            double old_armor_values[8];
            for (int i = 0; i < 8; i++) {
                old_armor_values[i] = UnitCSVFactory::GetVariable(unit_key, armor_keys[i], 0.0);
            }

            // Conversion is tricky because new values are a square and old values are
            // a two layered diamond.
            int index_combinations[4][4] = {{0,1,2,3}, {4,5,6,7}, {0,2,4,6}, {1,3,5,7}};
            for(const auto& facet_indices :  index_combinations) {
                double facet_strength = 0.0;
                for(const auto& facet_index :  facet_indices) {
                    facet_strength += old_armor_values[facet_index];
                }
                facet_strength /= 2;
                facets.push_back(Resource<double>(facet_strength, 0, facet_strength));
            }
        }
    }
}      

void Armor::SaveToCSV(std::map<std::string, std::string>& unit) const {
    unit["armor_front"] = facets[0].Serialize();
    unit["armor_back"] = facets[1].Serialize();
    unit["armor_left"] = facets[2].Serialize();
    unit["armor_right"] = facets[3].Serialize();
}

bool Armor::CanDowngrade() const {
    return installed && !integral;
}

bool Armor::Downgrade() {
    if(!CanDowngrade()) {
        return false;
    }

    // Component
    Component::Downgrade();
    
    facets.clear();
    return true;
}

bool Armor::CanUpgrade(const std::string upgrade_key) const {
    return !integral;
}

bool Armor::Upgrade(const std::string upgrade_key) {
    Component::Upgrade(upgrade_key);

    // We only support single value armor
    facets.clear();
    const double armor_single_value = UnitCSVFactory::GetVariable(upgrade_key, "armor", 0.0);

    if(armor_single_value > 0.0) {
        for (unsigned int i = 0; i < number_of_facets; i++) {
            facets.push_back(Resource<double>(armor_single_value,0,armor_single_value));
        }
    }

    return true;
}