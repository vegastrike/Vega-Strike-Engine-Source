/*
 * shield.cpp
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

#include "shield.h"
#include "unit_csv_factory.h"
#include "damage/damageable_layer.h"
#include "resource/cout_util.h"

#include <random>

std::string shield_facets_eight[8] = {
    "Shield_Front_Top_Right",
    "Shield_Front_Top_Left",
    "Shield_Back_Top_Right",
    "Shield_Back_Top_Left",
    "Shield_Front_Bottom_Right",
    "Shield_Front_Bottom_Left",
    "Shield_Back_Bottom_Right",
    "Shield_Back_Bottom_Left"
};

std::string shield_facets_four[4] = { 
    "Shield_Front_Top_Right", 
    "Shield_Back_Top_Left",
    "Shield_Front_Bottom_Right", 
    "Shield_Front_Bottom_Left"
};

std::string shield_facets_two[2] = { 
    "Shield_Front_Top_Right", 
    "Shield_Back_Top_Left"
};



// Note that we need to define FacetConfiguration during load
Shield::Shield(DamageableLayer* shield_): Component("", 0.0, 0.0, false),
    shield_(shield_)
    {}



void Shield::Load(std::string upgrade_key, std::string unit_key, 
                      Unit *unit) {
    //this->upgrade_key = upgrade_key;
    //upgrade_name = UnitCSVFactory::GetVariable(upgrade_key, "Name", std::string());
    //int num_facets = UnitCSVFactory::GetVariable(upgrade_key, "Facets", 0);
    
    printPlayerMessage(unit_key, "Old Facets", std::to_string(shield_->number_of_facets));
    printPlayerMessage(unit_key, "Old Regeneration", std::to_string(shield_->GetRegeneration()));

    // Regeneration
    const double regeneration = UnitCSVFactory::GetVariable(unit_key, "Shield_Recharge", 0.0);
    printPlayerMessage(unit_key, "Regeneration", std::to_string(regeneration));

    // Get shield count

    int shield_count = 0;
    std::vector<double> shield_values;
    std::string shield_string_values[4];

    // TODO: this mapping should really go away
    // I love macros, NOT.
    shield_string_values[0] = UnitCSVFactory::GetVariable(unit_key, "Shield_Front_Top_Right", std::string());
    shield_string_values[1] = UnitCSVFactory::GetVariable(unit_key, "Shield_Back_Top_Left", std::string());
    shield_string_values[2] = UnitCSVFactory::GetVariable(unit_key, "Shield_Front_Bottom_Right", std::string());
    shield_string_values[3] = UnitCSVFactory::GetVariable(unit_key, "Shield_Front_Bottom_Left", std::string());

    for (int i = 0; i < 4; i++) {
        if (shield_string_values[i].empty()) {
            continue;
        }

        shield_values.push_back(std::stod(shield_string_values[i]));

        // Should add up to the shield type - quad or dual
        shield_count++;
    }

    /*
     We are making the following assumptions:
     1. The CSV is correct
     2. Dual shields are 0 front and 1 rear
     3. Quad shields are front (0), rear(1), right(2) and left(3)
     4. There is no support for 8 facet shields in the game.
        This has more to do with the cockpit code than anything else
     5. We map the above index to our own
     */

    shield_->number_of_facets = shield_values.size();
    shield_->UpdateFacets(shield_values);
    shield_->UpdateRegeneration(regeneration);


    // TODO: shield leakage & efficiency
}


void Shield::SaveToCSV(std::map<std::string, std::string>& unit) const {
    // TODO: lib_damage figure out if this is correctly assigned
    int number_of_shield_emitters = shield_->number_of_facets;

    for(int i=0;i<8;i++) {
        unit[shield_facets_eight[i]] = "";
    }

    switch (number_of_shield_emitters) {
        case 8:
            for(int i=0;i<8;i++) {
                unit[shield_facets_eight[i]] = std::to_string(shield_->facets[i].health.MaxValue());
            }
            
            break;
        case 4:
            for(int i=0;i<4;i++) {
                unit[shield_facets_four[i]] = std::to_string(shield_->facets[i].health.MaxValue());
            }

            break;
        case 2:
            unit[shield_facets_two[0]] = std::to_string(shield_->facets[0].health.MaxValue());
            unit[shield_facets_two[1]] = std::to_string(shield_->facets[1].health.MaxValue());
            break;

        case 0:
            // No shields
            break;

        default:
            // This should not happen
            std::cout << number_of_shield_emitters << "\n";
            assert(0);
    }
}

std::string Shield::Describe() const {
    return std::string();
}

bool Shield::CanUpgrade(const std::string upgrade_name) const {
    return !Damaged();
}

bool Shield::CanDowngrade() const {
    return !Damaged();
}

bool Shield::Upgrade(const std::string upgrade_key) {
    if(!CanUpgrade(upgrade_key)) {
        return false;
    }

    int num_facets = UnitCSVFactory::GetVariable(upgrade_key, "Facets", 0);
    //FacetConfiguration new_configuration = GetFacetForIndex(num_facets);
    /*if(facet_configuration != GetFacetForIndex(num_facets)) {
        return false;
    }*/

    this->upgrade_key = upgrade_key;
    upgrade_name = UnitCSVFactory::GetVariable(upgrade_key, "Name", std::string());
    std::cout << upgrade_key << " : " << upgrade_name << " : " << num_facets << std::endl;

    // Regeneration
    double regeneration = UnitCSVFactory::GetVariable(upgrade_key, "Shield_Recharge", 0.0);
    
    std::vector<double> shield_values;
    if(num_facets == 2) {
        for (int i = 0; i < 2; i++) {
            shield_values.push_back(UnitCSVFactory::GetVariable(upgrade_key, shield_facets_two[i], 0.0));
            shield_->facets[i].regeneration.SetMaxValue(regeneration);
        }
    } else if(num_facets == 4) {
        for (int i = 0; i < 4; i++) {
            shield_values.push_back(UnitCSVFactory::GetVariable(upgrade_key, shield_facets_four[i], 0.0));
            shield_->facets[i].regeneration.SetMaxValue(regeneration);
        }
    } else {
        return false;
    }
    
    shield_->UpdateFacets(shield_values);

    // TODO: shield leakage

    return true;
}

bool Shield::Downgrade() {
    return false;
}

void Shield::Damage() {
    /*for(Facet& facet : facets) {
        facet.RandomDamage();
    }

    regeneration.RandomDamage();

    // This works fine as long as opacity is originally defined correctly.
    // For crappy shields, need opacity.max_value_ to be <1.0.
    opacity.RandomDamage();    */ 
} 

void Shield::Repair() {
    /*DamageableLayer::Repair();

    regeneration.RepairFully();
    opacity.RepairFully();*/
}

bool Shield::Damaged() const {
    /*for(const Facet& facet : facets) {
        if(facet.Damaged()) {
            return true;
        }
    }

    return regeneration.Damaged();*/
    return false;
}

bool Shield::Installed() const {
    return true; //regeneration.MaxValue() > 0;
}


void Shield::Disable() {
    /*for (Facet facet : facets) {
        facet.Set(0.0);
    }

    regeneration.Set(0.0);*/
}

void Shield::Discharge() {
    /*for (Facet &facet : facets) {
        facet -= regeneration;
    }*/
}

void Shield::Enable() {
    //regeneration.Set(regeneration.AdjustedValue());
}

bool Shield::Enabled() const {
    return true; // facets[0].Enabled();
}


// This is meant to be used when colliding with an enhancement.
// It enhances the shields.
// Right now, it simply upgrades them forever. Needs further thought.
// TODO: test, this functionality works, assuming it's actually supported.
void Shield::Enhance() {
    // Boost shields to 150%
    /*double enhancement_factor = 1.5;

    for(Facet& facet : facets) {
        facet.SetMaxValue(facet.MaxValue() * enhancement_factor);
    }

    regeneration.SetMaxValue(regeneration.MaxValue() * enhancement_factor);*/
}

void Shield::Regenerate() {
    /*for(Facet& facet : facets) {
        facet += regeneration;
    }*/
}

void Shield::AdjustStrength(const double &percent) {
    //double adjusted_percent = std::max(std::min(percent, 1.0f), 0.0f);

    /*for (Facet facet : facets) {
        // TODO:
        //facet.(adjusted_percent);
    }*/
}
