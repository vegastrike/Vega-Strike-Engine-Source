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
#include "vega_cast_utils.h"

#include <algorithm>

/* This class provides all energy generation methods to unit types -
 * ships, space installations, missiles, drones, etc. */


Energetic::Energetic() : 
        constrained_charge_to_shields(0.0f),
        sufficient_energy_to_recharge_shields(true),
        afterburntype(0) {
}


void Energetic::DecreaseWarpEnergyInWarp() {
    Unit *unit = vega_dynamic_cast_ptr<Unit>(this);

    if (!unit->ftl_drive.Enabled()) {
        return;
    }

    if(unit->ftl_drive.CanConsume()) {
        unit->ftl_drive.Consume();
    } else {
        unit->ftl_drive.Disable();
        unit->graphicOptions.WarpRamping = 1;
    }
}



float Energetic::getFuelUsage(bool afterburner) {
    if (afterburner) {
        return configuration()->fuel.afterburner_fuel_usage;
    }
    return configuration()->fuel.normal_fuel_usage;
}







float Energetic::maxEnergyData() const {
    const Unit *unit = vega_dynamic_cast_ptr<const Unit>(this);
    return unit->energy.MaxLevel();
}

void Energetic::rechargeEnergy() {
    Unit *unit = vega_dynamic_cast_ptr<Unit>(this);
    if ((!configuration()->fuel.reactor_uses_fuel) || (!unit->fuel.Depleted())) {
        unit->energy.Charge(unit->reactor.Capacity() * simulation_atom_var);
    }
}


void Energetic::setEnergyRecharge(float enrech) {
    Unit *unit = vega_dynamic_cast_ptr<Unit>(this);
    unit->reactor.SetCapacity(enrech);
}


// Basically max or current shield x 0.2
float Energetic::totalShieldEnergyCapacitance() const {
    const Unit *unit = vega_dynamic_cast_ptr<const Unit>(this);
    DamageableLayer *shield = unit->shield;

    float total_max_shield_value = shield->TotalMaxLayerValue();
    float total_current_shield_value = shield->TotalLayerValue();

    return configuration()->physics_config.shield_energy_capacitance * (configuration()->physics_config.use_max_shield_energy_usage ? total_max_shield_value : total_current_shield_value);
}

// The original code was in unit_generic:5476 RegenShields and was simply
// incomprehensible. After several days, I've written a similar version.
// However, someone who understands the previous code can refactor this easily
// or better yet, write plugable consumption models.
//GAHHH reactor in units of 100MJ, shields in units of VSD=5.4MJ to make 1MJ of shield use 1/shieldenergycap MJ
void Energetic::ExpendEnergy(const bool player_ship) {
    Unit *unit = vega_dynamic_cast_ptr<Unit>(this);

    // TODO: if we run out of fuel or energy, we die from lack of air
 
    MaintainShields();
    ExpendEnergyToRechargeShields();
    DecreaseWarpEnergyInWarp();

    unit->reactor.Generate();

    unit->drive.Consume();
    unit->ecm.Consume();
}

void Energetic::ExpendEnergy(float usage) {
    Unit *unit = vega_dynamic_cast_ptr<Unit>(this);
    // Operator overloaded to prevent negative usage
    unit->energy.Deplete(true, usage);
}



void Energetic::MaintainShields() {
    Unit *unit = vega_dynamic_cast_ptr<Unit>(this);

    const bool in_warp = unit->ftl_drive.Enabled();
    const int shield_facets = unit->shield->number_of_facets;

    if (in_warp && !configuration()->physics_config.shields_in_spec) {
        return;
    }

    if (unit->shield->TotalMaxLayerValue() == 0) {
        return;
    }

    // TODO: lib_damage restore efficiency by replacing with shield->efficiency
    const float efficiency = 1;

    const float shield_maintenance = unit->shield->GetRegeneration() * VSDPercent() *
            efficiency / configuration()->physics_config.shield_energy_capacitance * shield_facets *
            configuration()->physics_config.shield_maintenance_charge * simulation_atom_var;

    sufficient_energy_to_recharge_shields = shield_maintenance > unit->energy.Level();

    ExpendEnergy(shield_maintenance);
}

void Energetic::ExpendEnergyToRechargeShields() {
    Unit *unit = vega_dynamic_cast_ptr<Unit>(this);

    const bool in_warp = unit->ftl_drive.Enabled();

    // TODO: add has_shields function instead of check below
    if (unit->shield->TotalMaxLayerValue() == 0) {
        return;
    }

    if (in_warp && !configuration()->physics_config.shields_in_spec) {
        return;
    }

    double current_shield_value = unit->shield->TotalLayerValue();
    double max_shield_value = unit->shield->TotalMaxLayerValue();
    double regeneration = unit->shield->GetRegeneration();
    double maximum_charge = std::min(max_shield_value - current_shield_value, regeneration);

    // Here we store the actual charge we'll use in RegenShields
    constrained_charge_to_shields = maximum_charge;
    sufficient_energy_to_recharge_shields = (constrained_charge_to_shields > 0);
    double actual_charge = std::min(maximum_charge, unit->energy.Level());
    double energy_required_to_charge = actual_charge * VSDPercent() *
            simulation_atom_var;
    ExpendEnergy((float)energy_required_to_charge);
}

void Energetic::RechargeWarpCapacitors(const bool player_ship) {
    Unit *unit = vega_dynamic_cast_ptr<Unit>(this);

    // Will try to keep the percentage of warp and normal capacitors equal
    const double transfer_capacity = 0.005f;
    const double capacitor_percent = unit->energy.Percent();
    const double warp_capacitor_percent = unit->ftl_energy.Percent();
    const double warp_multiplier = WarpEnergyMultiplier(player_ship);

    if (warp_capacitor_percent >= 1.0f ||
            warp_capacitor_percent > capacitor_percent ||
            capacitor_percent < 0.10f) {
        return;
    }

    const double previous_energy = unit->energy.Level();
    ExpendEnergy((float)(unit->energy.MaxLevel() * transfer_capacity));

    const double actual_energy = previous_energy - unit->energy.Level();
    unit->ftl_energy.SetLevel(std::min(unit->ftl_energy.MaxLevel(), unit->ftl_energy.Level() + actual_energy * warp_multiplier));
}

float Energetic::WarpEnergyMultiplier(const bool player_ship) {
    Unit *unit = static_cast<Unit *>(this);
    bool player = player_ship;

    // We also apply player multiplier to wing members
    Flightgroup *flight_group = unit->getFlightgroup();
    if (flight_group && !player_ship) {
        player = _Universe->isPlayerStarship(flight_group->leader.GetUnit()) != nullptr;
    }
    return player ? configuration()->warp_config.player_warp_energy_multiplier : configuration()->warp_config.warp_energy_multiplier;
}

float Energetic::VSDPercent() {
    return configuration()->fuel.vsd_mj_yield / 100;
}
