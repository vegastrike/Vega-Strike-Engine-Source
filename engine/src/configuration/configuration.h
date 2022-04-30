/**
 * configuration.h
 *
 * Copyright (C) 2021 Roy Falk, ministerofinformation, and David Wales
 * Copyright (C) 2022 Stephen G. Tuggy
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


#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <memory>

namespace vegastrike_configuration {
// Config Structs Declaration

// These are settings that are relevant to collision.cpp and are marked as hacks
// or their comments are unclear or they are unclear
struct CollisionHacks {
    float collision_hack_distance;
    bool collision_damage_to_ai;
    bool crash_dock_hangar;
    bool crash_dock_unit;
    float front_collision_hack_angle;
    float front_collision_hack_distance;
    bool cargo_deals_collide_damage;
    //int upgrade_faction = (cargo_deals_collide_damage ? -1 : FactionUtil::GetUpgradeFaction())

    CollisionHacks();
};

// Also radar
struct Computer {
    // Note: below is probably a stale comment
    // DO NOT CHANGE see unit_customize.cpp
    float default_lock_cone;
    float default_max_range;
    float default_tracking_cone;

    Computer();
};

struct Fuel {
    float afterburner_fuel_usage;
    /* There are a pair of "FMEC" variables - they both involve "Fuel Mass to Energy Conversion" -
     * this one happens to specify the inverse (it's only ever used as 1/Value, so just encode 1/Value, not Value)
     * of the assumed exit velocity of the mass ejected as thrust, calculated based on energy-possible
     * (if not necessarily plausible) outcomes from a Li-6 + Deuterium fusion reaction.
     * The other variable (not present here) FMEC_factor, is used in reactor --> energy production.
     * As the comment in the code next to the variable init says, it specifies how many metric tons (1 VS mass unit)
     * of fuel are used to produce 100MJ (one units.csv energy recharge unit) of recharge.
     * At some point, it wouldn't kill us to renormalize the engine and dataset to both just use SI units, but that's not a priority.
     */
    float fmec_exit_velocity_inverse;

    /* This used to be Lithium6constant.
     * There's some relevant context that's been removed from the original name of this variable "Lithium6constant" --
     * a better name would ultimately be "FuelToEnergyConversionRelativeToLithium6DeuterideFusion" -
     * that fully encodes what the efficiency is relative to. */
    float fuel_efficiency;
    bool fuel_equals_warp;
    float normal_fuel_usage;
    bool reactor_uses_fuel;

    Fuel();
};

struct Logging {
    bool verbose_debug;
    Logging();
};

struct Physics {
    float bleed_factor;
    float collision_scale_factor;
    float inelastic_scale;
    float kilojoules_per_damage;
    float max_force_multiplier;
    bool max_shield_lowers_capacitance;
    float max_torque_multiplier;
    float minimum_mass;
    float minimum_time;             // between recorded player collisions


    // Disabled bouncing missile option. Missiles always explode when colliding with something.
    //static bool does_missile_bounce = GameConfig::GetVariable( "physics", "missile_bounce", false);

    Physics();
};

// Covers both SPEC and jumps
struct Warp {
    float insystem_jump_cost;

    Warp();
};

struct Weapons {
    bool can_fire_in_cloak;
    bool can_fire_in_spec;
    Weapons();
};

}

// not using namespace vegastrike_configuration, because Computer would be ambiguous

class Configuration {
public:
    Configuration();
    void OverrideDefaultsWithUserConfiguration();
    vegastrike_configuration::CollisionHacks collision_hacks;
    vegastrike_configuration::Computer computer;
    vegastrike_configuration::Fuel fuel;
    vegastrike_configuration::Logging logging;
    vegastrike_configuration::Physics physics;
    vegastrike_configuration::Warp warp;
    vegastrike_configuration::Weapons weapons;
};

extern std::shared_ptr<Configuration> configuration();

#endif // CONFIGURATION_H
