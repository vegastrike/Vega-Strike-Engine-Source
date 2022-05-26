/**
 * energetic.cpp
 *
 * Copyright (C) 2020-2022 Daniel Horn, Roy Falk, Stephen G. Tuggy, and
 * other Vega Strike contributors
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

#include "energetic.h"

#include "configuration/game_config.h"
#include "configuration/configuration.h"
#include "damageable.h"
#include "vegastrike.h"
#include "unit_generic.h"
#include "universe.h"
#include "resource/resource.h"

#include <algorithm>

/* This class provides all energy generation methods to unit types -
 * ships, space installations, missiles, drones, etc. */


Energetic::Energetic() : energy(0, 0),
        recharge(0),
        maxwarpenergy(0),
        warpenergy(0),
        constrained_charge_to_shields(0.0f),
        sufficient_energy_to_recharge_shields(true),
        fuel(0),
        afterburnenergy(0),
        afterburntype(0) {
    jump.warpDriveRating = 0;
    jump.energy = 100;
    jump.insysenergy = configuration()->warp_config_.insystem_jump_cost * jump.energy;
    jump.drive = -2;
    jump.delay = 5;
    jump.damage = 0;
}

void Energetic::decreaseWarpEnergy(bool insys, float time) {
    if (configuration()->fuel.fuel_equals_warp) {
        this->warpenergy = this->fuel;
    }
    this->warpenergy -= (insys ? jump.insysenergy / configuration()->physics_config_.bleed_factor : jump.energy) * time;
    if (this->warpenergy < 0) {
        this->warpenergy = 0;
    }
    if (configuration()->fuel.fuel_equals_warp) {
        this->fuel = this->warpenergy;
    }
}

void Energetic::DecreaseWarpEnergyInWarp() {
    Unit *unit = static_cast<Unit *>(this);

    const bool in_warp = unit->graphicOptions.InWarp;

    if (!in_warp) {
        return;
    }

    //FIXME FIXME FIXME
    // Roy Falk - fix what?
    float bleed = jump.insysenergy / configuration()->physics_config_.bleed_factor * simulation_atom_var;
    if (warpenergy > bleed) {
        warpenergy -= bleed;
    } else {
        unit->graphicOptions.InWarp = 0;
        unit->graphicOptions.WarpRamping = 1;
    }
}

float Energetic::energyData() const {
    float capacitance = const_cast<Energetic *>(this)->totalShieldEnergyCapacitance();

    if (configuration()->physics_config_.max_shield_lowers_capacitance) {
        if (energy.MaxValue() <= capacitance) {
            return 0;
        }
        return (energy) / (energy.MaxValue() - capacitance);
    } else {
        return energy.Percent();
    }
}

float Energetic::energyRechargeData() const {
    return recharge;
}

float Energetic::fuelData() const {
    return fuel;
}

float Energetic::getFuelUsage(bool afterburner) {
    if (afterburner) {
        return configuration()->fuel.afterburner_fuel_usage;
    }
    return configuration()->fuel.normal_fuel_usage;
}

/**
 * @brief Energetic::WCWarpIsFuelHack - in Wing Commander, warp and fuel are the same variable.
 * Therefore, we need to transfer from one to the other to maintain equality
 * @param transfer_warp_to_fuel - true means fuel = warpenergy
 */
// TODO: this is still an ugly hack
void Energetic::WCWarpIsFuelHack(bool transfer_warp_to_fuel) {
    if (!configuration()->fuel.fuel_equals_warp) {
        return;
    }

    if (transfer_warp_to_fuel) {
        fuel = warpenergy;
    } else {
        warpenergy = fuel;
    }
}

float Energetic::ExpendMomentaryFuelUsage(float magnitude) {
    // TODO: have this make some kind of sense to someone other than the person who wrote the comment below.
    //HACK this forces the reaction to be Li-6+D fusion with efficiency governed by the getFuelUsage function
    float quantity = Energetic::getFuelUsage(false) * simulation_atom_var * magnitude *
            configuration()->fuel.fmec_exit_velocity_inverse / configuration()->fuel.fuel_efficiency;

    return ExpendFuel(quantity);
}

/**
 * @brief expendFuel - reduce fuel by burning it
 * @param quantity - requested quantity to use
 * @return - actual quantity used
 */
float Energetic::ExpendFuel(float quantity) {
    fuel -= configuration()->fuel.normal_fuel_usage * quantity;

    if (fuel < 0) {
        quantity += fuel;
        fuel = 0;
    }

    return quantity;
}

float Energetic::getWarpEnergy() const {
    return warpenergy;
}

void Energetic::increaseWarpEnergy(bool insys, float time) {
    if (configuration()->fuel.fuel_equals_warp) {
        this->warpenergy = this->fuel;
    }
    this->warpenergy += (insys ? jump.insysenergy : jump.energy) * time;
    if (this->warpenergy > this->maxwarpenergy) {
        this->warpenergy = this->maxwarpenergy;
    }
    if (configuration()->fuel.fuel_equals_warp) {
        this->fuel = this->warpenergy;
    }
}

float Energetic::maxEnergyData() const {
    return energy.MaxValue();
}

void Energetic::rechargeEnergy() {
    if ((!configuration()->fuel.reactor_uses_fuel) || (fuel > 0)) {
        energy += recharge * simulation_atom_var;
    }
}

bool Energetic::refillWarpEnergy() {
    if (configuration()->fuel.fuel_equals_warp) {
        this->warpenergy = this->fuel;
    }
    float tmp = this->maxwarpenergy;
    if (tmp < this->jump.energy) {
        tmp = this->jump.energy;
    }
    if (tmp > this->warpenergy) {
        this->warpenergy = tmp;
        if (configuration()->fuel.fuel_equals_warp) {
            this->fuel = this->warpenergy;
        }
        return true;
    }
    return false;
}

void Energetic::setAfterburnerEnergy(float aft) {
    afterburnenergy = aft;
}

void Energetic::setEnergyRecharge(float enrech) {
    recharge = enrech;
}

void Energetic::setFuel(float f) {
    fuel = f;
}

float Energetic::warpCapData() const {
    return maxwarpenergy;
}

float Energetic::warpEnergyData() const {
    if (maxwarpenergy > 0) {
        return ((float) warpenergy) / ((float) maxwarpenergy);
    }
    if (jump.energy > 0) {
        return ((float) warpenergy) / ((float) jump.energy);
    }
    return 0.0f;
}

// Basically max or current shield x 0.2
float Energetic::totalShieldEnergyCapacitance() {
    Unit *unit = static_cast<Unit *>(this);
    DamageableLayer *shield = unit->shield;

    float total_max_shield_value = shield->TotalMaxLayerValue();
    float total_current_shield_value = shield->TotalLayerValue();

    return configuration()->physics_config_.shield_energy_capacitance * (configuration()->physics_config_.use_max_shield_energy_usage ? total_max_shield_value : total_current_shield_value);
}

// The original code was in unit_generic:5476 RegenShields and was simply
// incomprehensible. After several days, I've written a similar version.
// However, someone who understands the previous code can refactor this easily
// or better yet, write plugable consumption models.
//GAHHH reactor in units of 100MJ, shields in units of VSD=5.4MJ to make 1MJ of shield use 1/shieldenergycap MJ
void Energetic::ExpendEnergy(const bool player_ship) {
    // TODO: if we run out of fuel or energy, we die from lack of air

    MaintainShields();
    ExpendEnergyToRechargeShields();
    MaintainECM();
    DecreaseWarpEnergyInWarp();

    RechargeWarpCapacitors(player_ship);

    ExpendFuel();
}

void Energetic::ExpendEnergy(float usage) {
    // Operator overloaded to prevent negative usage
    energy -= usage;
}

// The original code was a continuation of the comment above and simply unclear.
// I replaced it with a very simple model.
void Energetic::ExpendFuel() {
    if (!configuration()->fuel.reactor_uses_fuel) {
        return;
    }

    const float fuel_usage = configuration()->fuel.fmec_exit_velocity_inverse * recharge * simulation_atom_var;
    fuel = std::max(0.0f, fuel - fuel_usage);

    if (!FINITE(fuel)) {
        VS_LOG(error, "Fuel is nan C");
        fuel = 0;
    }
}

void Energetic::MaintainECM() {
    Unit *unit = static_cast<Unit *>(this);

    if (!unit->computer.ecmactive) {
        return;
    }

    float sim_atom_ecm = configuration()->fuel.ecm_energy_cost * unit->ecm * simulation_atom_var;
    ExpendEnergy(sim_atom_ecm);
}

void Energetic::MaintainShields() {
    Unit *unit = static_cast<Unit *>(this);

    const bool in_warp = unit->graphicOptions.InWarp;
    const int shield_facets = unit->shield->number_of_facets;

    if (in_warp && !configuration()->physics_config_.shields_in_spec) {
        return;
    }

    if (unit->shield->TotalMaxLayerValue() == 0) {
        return;
    }

    // TODO: lib_damage restore efficiency by replacing with shield->efficiency
    const float efficiency = 1;

    const float shield_maintenance = unit->shield->GetRegeneration() * VSDPercent() *
            efficiency / configuration()->physics_config_.shield_energy_capacitance * shield_facets *
            configuration()->physics_config_.shield_maintenance_charge * simulation_atom_var;

    sufficient_energy_to_recharge_shields = shield_maintenance > energy;

    ExpendEnergy(shield_maintenance);
}

void Energetic::ExpendEnergyToRechargeShields() {
    Unit *unit = static_cast<Unit *>(this);

    const bool in_warp = unit->graphicOptions.InWarp;

    // TODO: add has_shields function instead of check below
    if (unit->shield->TotalMaxLayerValue() == 0) {
        return;
    }

    if (in_warp && !configuration()->physics_config_.shields_in_spec) {
        return;
    }

    float current_shield_value = unit->shield->TotalLayerValue();
    float max_shield_value = unit->shield->TotalMaxLayerValue();
    float regeneration = unit->shield->GetRegeneration();
    float maximum_charge = std::min(max_shield_value - current_shield_value, regeneration);

    // Here we store the actual charge we'll use in RegenShields
    constrained_charge_to_shields = maximum_charge;
    sufficient_energy_to_recharge_shields = (constrained_charge_to_shields > 0);
    float actual_charge = std::min(maximum_charge, energy.Value());
    float energy_required_to_charge = actual_charge * VSDPercent() *
            simulation_atom_var;
    ExpendEnergy(energy_required_to_charge);
}

void Energetic::RechargeWarpCapacitors(const bool player_ship) {
    // Will try to keep the percentage of warp and normal capacitors equal
    const float transfer_capacity = 0.005f;
    const float capacitor_percent = energy / energy.MaxValue();
    const float warp_capacitor_percent = warpenergy / maxwarpenergy;
    const float warp_multiplier = WarpEnergyMultiplier(player_ship);

    if (warp_capacitor_percent >= 1.0f ||
            warp_capacitor_percent > capacitor_percent ||
            capacitor_percent < 0.10f) {
        return;
    }

    const float previous_energy = energy.Value();
    ExpendEnergy(energy.MaxValue() * transfer_capacity);

    const float actual_energy = previous_energy - energy.Value();
    warpenergy = std::min(maxwarpenergy, warpenergy + actual_energy * warp_multiplier);
}

float Energetic::WarpEnergyMultiplier(const bool player_ship) {
    Unit *unit = static_cast<Unit *>(this);
    bool player = player_ship;

    // We also apply player multiplier to wing members
    Flightgroup *flight_group = unit->getFlightgroup();
    if (flight_group && !player_ship) {
        player = _Universe->isPlayerStarship(flight_group->leader.GetUnit()) != nullptr;
    }
    return player ? configuration()->physics_config_.player_warp_energy_multiplier : configuration()->physics_config_.warp_energy_multiplier;
}

float Energetic::VSDPercent() {
    return configuration()->fuel.vsd_mj_yield / 100;
}
