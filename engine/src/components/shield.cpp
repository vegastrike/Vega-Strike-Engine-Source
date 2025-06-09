/*
 * shield.cpp
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

#include "shield.h"
#include "ftl_drive.h"
#include "components/cloak.h"
#include "cmd/unit_csv_factory.h"
#include "configuration/configuration.h"
#include "src/vs_logging.h"
#include "damage/damage.h"

#include <boost/format.hpp>

int Shield::front = 0;
int Shield::back = 1;
int Shield::left = 2;
int Shield::right = 3;

static const Damage normal_damage = Damage(1.0,0.0);

Shield::Shield(EnergyContainer *source, FtlDrive *ftl_drive, Cloak *cloak, FacetConfiguration configuration) :
    Component(),
    EnergyConsumer(source, true, 0),
    DamageableLayer(2, configuration, 0.0, normal_damage, false),
    ftl_drive(ftl_drive), cloak(cloak),
    regeneration(Resource<double>(0.0,0.0,0.0)),
    max_power(Resource<double>(1.0,0.0,1.0)) {
    type = ComponentType::Shield;
}

Shield::~Shield()
= default;


// Component Methods
void Shield::Load(std::string unit_key) {
    Component::Load(unit_key);

    // Load shield
    // Some basic shield variables
    // TODO: lib_damage figure out how leak and efficiency work
    //char leak = static_cast<char>(UnitCSVFactory::GetVariable(unit_key, "Shield_Leak", 0.0f) * 100);
    //double efficiency = UnitCSVFactory::GetVariable(unit_key, "Shield_Efficiency", 1.0f );

    const std::string regen_string = UnitCSVFactory::GetVariable(unit_key, "Shield_Recharge", std::string());
    regeneration = Resource<double>(regen_string);

    // We support 3 options:
    // 1. Minimized shield_strength = x (single value).
    // 2. New detailed shield (Front, back, left, right).
    // 3. Old detailed (Front-left-top, ...). 4/2 facets converted to 4/2.

    // Get shield count
    std::map<std::string, float> shield_sections{};
    std::vector<std::string> shield_string_values{};

    const std::string shield_strength_string = UnitCSVFactory::GetVariable(unit_key, "shield", std::string());
    const std::string shield_facets_string = UnitCSVFactory::GetVariable(unit_key, "shield_facets", std::string());

    if(!shield_facets_string.empty()) {
        try {
            number_of_facets = std::stoi(shield_facets_string);
        } catch (std::invalid_argument const& ex) {
            VS_LOG(error, (boost::format("%1%: %2% trying to convert shield_facets_string '%3%' to int") % __FUNCTION__ % ex.what() % shield_facets_string));
            number_of_facets = 1;
        } catch (std::out_of_range const& ex) {
            VS_LOG(error, (boost::format("%1%: %2% trying to convert shield_facets_string '%3%' to int") % __FUNCTION__ % ex.what() % shield_facets_string));
            number_of_facets = 1;
        }
    }

    // Single value (short form)
    if(!shield_strength_string.empty()) {
        try {
            Resource<double> facet_strength = Resource<double>(shield_strength_string);
            facets = std::vector<Resource<double>>(number_of_facets,
                facet_strength);

            CalculatePercentOperational();
            installed = true;
            return;
        } catch (std::invalid_argument const& ex) {
            VS_LOG(error, (boost::format("%1%: %2% trying to convert shield_strength_string '%3%' to int") % __FUNCTION__ % ex.what() % shield_strength_string));
        } catch (std::out_of_range const& ex) {
            VS_LOG(error, (boost::format("%1%: %2% trying to convert shield_strength_string '%3%' to int") % __FUNCTION__ % ex.what() % shield_strength_string));
        }
    }

    // Try new longform
    if(!shield_facets_string.empty() &&
       (number_of_facets == 4 || number_of_facets == 2)) {
        std::vector<Resource<double>> shield_values{};
        std::string shield_keys[] = {"shield_front", "shield_back",
            "shield_left", "shield_right"};

        // Using old form for loop to support 2/4 shields.
        for (int i = 0; i < number_of_facets; ++i) {
            // TODO: this can be taken out to a separate utility function, shared to hull, armor and shield
            // also below in old form.
            // Maybe. If we read numbers instead of strings... no need.
            const std::string shield_string_value = UnitCSVFactory::GetVariable(unit_key, shield_keys[i], std::string());
            if (shield_string_value.empty()) {
                shield_values.push_back(Resource<double>(0.0));
                continue;
            }

            try {
                Resource<double> facet_strength = Resource<double>(shield_string_value);
                shield_values.push_back(facet_strength);
                continue;
            } catch (const std::invalid_argument& ex) {
                VS_LOG(error, (boost::format("%1%: Unable to convert shield value '%2%' to a number: %3%") % __FUNCTION__ % shield_string_value % ex.what()));
            } catch (std::out_of_range const& ex) {
                VS_LOG(error, (boost::format("%1%: Unable to convert shield value '%2%' to a number: %3%") % __FUNCTION__ % shield_string_value % ex.what()));
            }
            shield_values.push_back(Resource<double>(0.0));
        }

        facets = shield_values;
        CalculatePercentOperational();
        installed = true;
        return;
    }

    // Fallback to old shield_keys
    int shield_count = 0;
    std::string shield_keys[] = {"Shield_Front_Top_Right", "Shield_Back_Top_Left", "Shield_Front_Bottom_Right", "Shield_Front_Bottom_Left"};
    std::vector<Resource<double>> shield_values{};

    try {
        for (auto &key : shield_keys) {
            std::string string_value = UnitCSVFactory::GetVariable(unit_key, key, std::string());

            // If no shields present, we should break and go directly to end
            // of function
            if(string_value.empty()) {
                break;
            }

            double value = std::stod(string_value);
            shield_values.push_back(Resource<double>(value, 0.0, value));
            ++shield_count;
        }

        if (shield_count == 4 || shield_count == 2) {
            number_of_facets = shield_count;
            facets = shield_values;
            CalculatePercentOperational();
            installed = true;
            return;
        }
    } catch (std::exception const& ex) {
        VS_LOG(error, (boost::format("%1%: %2% Unable to parse shield facets for %3%") % __FUNCTION__ % ex.what() % unit_key));
    }

    // This should already be set, but good practice to do it anyway.
    number_of_facets = 0;
    facets.clear();
    operational = 0.0;
}



void Shield::SaveToCSV(std::map<std::string, std::string>& unit) const {
    // We always save in long form.
    unit["shield_facets"] = std::to_string(number_of_facets);
    unit["shield_front"] = facets[0].Serialize();
    unit["shield_back"] = facets[1].Serialize();
    unit["shield_left"] = facets[2].Serialize();
    unit["shield_right"] = facets[3].Serialize();

    //TODO: lib_damage shield leak and efficiency
    unit["Shield_Leak"] = std::to_string(0);
    unit["Shield_Efficiency"] = std::to_string(1);
    unit["Shield_Recharge"] = regeneration.Serialize();
}

bool Shield::CanDowngrade() const {
    return installed && !integral;
}

bool Shield::Downgrade() {
    if(!CanDowngrade()) {
        return false;
    }

    // Component
    Component::Downgrade();

    facets.clear();
    return true;
}

bool Shield::CanUpgrade(const std::string upgrade_key) const {
    const int facets = UnitCSVFactory::GetVariable(upgrade_key, "shield_facets", 0.0);
    return facets == number_of_facets && !integral;
}

bool Shield::Upgrade(const std::string upgrade_key) {
    Component::Upgrade(upgrade_key);

    // We only support single value shield
    const double regen = UnitCSVFactory::GetVariable(upgrade_key, "Shield_Recharge", 0.0);
    if(regen > 0) {
        regeneration = Resource<double>(regen, 0.0, regen);
    } else {
        return false;
    }

    facets.clear();
    const double shield_single_value = UnitCSVFactory::GetVariable(upgrade_key, "shield", 0.0);

    if(shield_single_value > 0.0) {
        for (unsigned int i = 0; i < number_of_facets; i++) {
            facets.push_back(Resource<double>(shield_single_value,0,shield_single_value));
        }
    }

    return true;
}

double Shield::PercentOperational() const {
    return operational.Value();
}

void Shield::CalculatePercentOperational() {
    double percent = regeneration.Percent();

    for (Resource<double> &facet : facets) {
        if(facet.MaxValue() == 0.0) {
            continue;
        }

        percent += facet.AdjustedValue() / facet.MaxValue();
    }

    // A simple average of regeneration and facets
    // 4 facet shields assign less importance to regeneration
    operational = percent / (number_of_facets + 1);
}

void Shield::Damage() {
    regeneration.RandomDamage();
    double percent = regeneration.Percent();

    for (Resource<double> &facet : facets) {
        facet.RandomDamage();
        percent += facet.Percent();
    }

    // A simple average of regeneration and facets
    // 4 facet shields assign less importance to regeneration
    operational = percent / (number_of_facets + 1);
}

void Shield::DamageByPercent(double percent) {
    regeneration.DamageByPercent(percent);
    double sum_of_percents = regeneration.Percent();

    for (Resource<double> &facet : facets) {
        facet.DamageByPercent(percent);
        sum_of_percents += facet.Percent();
    }

    // A simple average of regeneration and facets
    // 4 facet shields assign less importance to regeneration
    operational = sum_of_percents / (number_of_facets + 1);
}

void Shield::Repair() {
    regeneration.RepairFully();

    for (Resource<double> &facet : facets) {
        facet.RepairFully();
    }

    operational = 1.0;
}

bool Shield::Damaged() const {
    return operational.Value() < 1;
}


void Shield::Regenerate(const bool player_ship) {
    //const bool apply_difficulty_shields = configuration()->physics.difficulty_based_shield_recharge;

    // Discharge numbers for nicer, gradual discharge
    //const float discharge_per_second = configuration()->physics.speeding_discharge;
    //approx
    //const float discharge_rate = (1 - (1 - discharge_per_second) * simulation_atom_var);
    //const float min_shield_discharge = configuration()->physics.min_shield_speeding_discharge;

    // Some basic sanity checks first
    // No point in all this code if there are no shields.
    if (number_of_facets < 2 || TotalMaxLayerValue() == 0) {
        return;
    }

    // No shields in SPEC
    if (ftl_drive->Enabled() && !configuration()->physics.shields_in_spec) {
        Decrease();
        return;
    }

    // No shields while cloaked
    if (cloak->Active()) {
        Decrease();
        return;
    }

    /* A discussion of consumption
    * First, you set the consumption for 1 second (not atom_var)
    * Then you consume and get back actual consumption
    * Finally, you adjust whatever used it to the value in question
    */

    // Shield Maintenance
    // TODO: lib_damage restore efficiency by replacing with shield->efficiency
    //const double efficiency = 1;

    const double shield_maintenance_cost = TotalMaxLayerValue() * configuration()->components.shield.maintenance_factor;
    SetConsumption(shield_maintenance_cost);
    const double actual_maintenance_percent = Consume();
    if(Percent() > actual_maintenance_percent) {
        Decrease();
        return;
    }

    // Manually throttle shield strength
    if(Percent() > max_power) {
        Decrease();
        return;
    }

    // Shield Regeneration
    if(TotalLayerValue() == TotalMaxLayerValue()) {
        // Fully charged. No need for more action or energy consumption
        return;
    }

    const double shield_regeneration_cost = regeneration.AdjustedValue() * configuration()->components.shield.regeneration_factor;
    SetConsumption(shield_regeneration_cost);
    const double actual_regeneration_percent = Consume();
    double regen = actual_regeneration_percent * regeneration.AdjustedValue() * simulation_atom_var;

    for (Resource<double> &facet : facets) {
        facet += regen;
    }

    // Difficulty settings
    // TODO: enable
    //double difficulty_shields = 1.0;
    // if (apply_difficulty_shields) {
    //     difficulty_shields = g_game.difficulty;
    // }

    /*if (unit->GetNebula() != nullptr) {
        shield_recharge *= nebshields;
    }

    // Adjust other (enemy) ships for difficulty
    if (!player_ship) {
        shield_recharge *= difficulty;
    }*/

    //const float nebshields = configuration()->physics.nebula_shield_recharge;
}

void Shield::AdjustPower(double percent) {
    max_power = percent;
}


void Shield::FullyCharge() {
    for (Resource<double> &facet : facets) {
        facet.Set(facet.AdjustedValue());
    }
}

void Shield::Decrease() {
    for(auto& facet : facets) {
        facet -= regeneration.MaxValue();
    }
}

void Shield::Zero() {
    for(auto& facet : facets) {
        facet = 0;
    }
}

double Shield::Consume()
{
    return EnergyConsumer::Consume();
}
