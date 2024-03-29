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

const std::string SHIELD_RECHARGE = "Shield_Recharge";

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
Shield::Shield(): 
               Component("", 0.0, 0.0, false),
               DamageableLayer(2, FacetConfiguration::zero, 
                               Health(2, 0, 0), false),
               regeneration(0,0,0),
               power(1.0,0.0,1.0) {}



void Shield::Load(std::string upgrade_key, std::string unit_key, 
                      Unit *unit) {
    //this->upgrade_key = upgrade_key;
    //upgrade_name = UnitCSVFactory::GetVariable(upgrade_key, "Name", std::string());
    //int num_facets = UnitCSVFactory::GetVariable(upgrade_key, "Facets", 0);
    
    // Regeneration
    const double regeneration = UnitCSVFactory::GetVariable(unit_key, SHIELD_RECHARGE, 0.0);

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

    UpdateFacets(shield_values);
    this->regeneration.SetMaxValue(regeneration);

    // TODO: shield leakage & efficiency
}


void Shield::SaveToCSV(std::map<std::string, std::string>& unit) const {
    // TODO: lib_damage figure out if this is correctly assigned
    int number_of_shield_emitters = number_of_facets;

    // TODO: This won't record damage to regeneration or shield facets
    unit[SHIELD_RECHARGE] = std::to_string(regeneration.MaxValue());

    for(int i=0;i<8;i++) {
        unit[shield_facets_eight[i]] = "";
    }

    switch (number_of_shield_emitters) {
        case 8:
            for(int i=0;i<8;i++) {
                unit[shield_facets_eight[i]] = std::to_string(facets[i].health.MaxValue());
            }
            
            break;
        case 4:
            for(int i=0;i<4;i++) {
                unit[shield_facets_four[i]] = std::to_string(facets[i].health.MaxValue());
            }

            break;
        case 2:
            unit[shield_facets_two[0]] = std::to_string(facets[0].health.MaxValue());
            unit[shield_facets_two[1]] = std::to_string(facets[1].health.MaxValue());
            break;

        case 0:
            // No shields
            break;

        default:
            // This should not happen
            std::cout << number_of_shield_emitters << "\n";
            assert(0);
    }

    //TODO: lib_damage shield leak and efficiency
    unit["Shield_Leak"] = std::to_string(0); //tos( shield.leak/100.0 );
    unit["Shield_Efficiency"] = std::to_string(1); //tos( shield.efficiency );
}

std::string Shield::Describe() const {
    return std::string();
}

bool Shield::CanDowngrade() const {
    return !Damaged();
}

bool Shield::CanUpgrade(const std::string upgrade_name) const {
    return !Damaged();
}

bool Shield::Downgrade() {
    if(!CanDowngrade()) {
        return false;
    }
    
    regeneration.SetMaxValue(0.0);
    power.SetMaxValue(0.0);

    std::vector<double> empty_vector;
    UpdateFacets(empty_vector);

    return false;
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

    // Regeneration
    regeneration.SetMaxValue(UnitCSVFactory::GetVariable(upgrade_key, SHIELD_RECHARGE, 0.0));
    
    std::vector<double> shield_values;
    if(num_facets == 2) {
        for (int i = 0; i < 2; i++) {
            shield_values.push_back(UnitCSVFactory::GetVariable(upgrade_key, shield_facets_two[i], 0.0));
        }
    } else if(num_facets == 4) {
        for (int i = 0; i < 4; i++) {
            shield_values.push_back(UnitCSVFactory::GetVariable(upgrade_key, shield_facets_four[i], 0.0));
        }
    } else {
        return false;
    }
    
    UpdateFacets(shield_values);

    // TODO: shield leakage

    return true;
}



void Shield::Damage() {
    for(Health& facet : facets) {
        facet.health.RandomDamage();
    }

    regeneration.RandomDamage();

    // This works fine as long as opacity is originally defined correctly.
    // For crappy shields, need opacity.max_value_ to be <1.0.
    // TODO: opacity.RandomDamage();     
} 

void Shield::Repair() {
    for(Health& facet : facets) {
        facet.health.RepairFully();
    }

    regeneration.RepairFully();
    // TODO: opacity.RepairFully();
}

bool Shield::Damaged() const {
    for(const Health& facet : facets) {
        if(facet.health.Damaged()) {
            return true;
        }
    }

    return regeneration.Damaged();
}

bool Shield::Installed() const {
    return regeneration.MaxValue() > 0;
}


void Shield::AdjustPower(const double &percent) {
    power.Set(percent);
}


void Shield::Disable() {
    power.Set(0.0);
}

// Zeros out shields but can immediately start recharging
// Used for things like jump effects
void Shield::Discharge() {
    for (Health &facet : facets) {
        facet.health.Set(0.0);
    }
}

void Shield::Enable() {
    power.Set(1.0);
}

bool Shield::Enabled() const {
    return power.Value() > 0.0;
}



/** Enhance adds some oomph to shields. 
 * Originally, I thought to just make them 150% one time.
 * However, this isn't really significant and it's hard to implement
 * with the underlying Resource class, which checks for max values.
 * Instead, this will upgrade the Max value of shields and repair them.
 */
// TODO: test, this functionality works, assuming it's actually supported.
void Shield::Enhance() {
    // Boost shields to 150%
    double enhancement_factor = 1.5;

    for(Health& facet : facets) {
        facet.health.SetMaxValue(facet.health.MaxValue() * enhancement_factor);
    }

    regeneration.SetMaxValue(regeneration.MaxValue() * enhancement_factor);
}


double Shield::GetPower() const {
    return power.Value();
}

double Shield::GetPowerCap() const {
    return power.AdjustedValue();
}
    


/*  This is a bit kludgy. Set power via keyboard only works when not suppressed.
*   If ship is in SPEC, power will be continuously set to 0.
*   Therefore, if you set power to 1/3, go to SPEC and out again, power will be
*   set to full again.
*/
void Shield::SetPower(const double power) {
    this->power = power;
}


void Shield::SetPowerCap(const double power) {
    this->power.SetAdjustedMaxValue(power);
    // We need this as well, otherwise power will still be 0.
    this->power.Set(power);
}



double Shield::GetRegeneration() const {
    return regeneration.Value();
}

void Shield::Regenerate() {
    for(Health& facet : facets) {
        if(facet.health.Percent() < power) {
            // If shield generator is damaged, regenerate less
            facet.health += regeneration.Value(); 
        } else if(facet.health.Percent() > power) {
            // If in SPEC or cloaked, decrease shields as fast as possible
            // to prevent a scenario where damaged shields work in SPEC.
            facet.health -= regeneration.MaxValue();
        }
    }
}