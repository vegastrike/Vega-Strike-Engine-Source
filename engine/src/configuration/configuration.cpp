/**
 * configuration.cpp
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


#include "configuration.h"
#include "game_config.h"
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <math.h>

// Global Variable Definition
Configuration configuration;

Configuration::Configuration() {
    //logging.verbose_debug = GameConfig::GetVariable("data", "verbose_debug", false);
}

/* Override the default value(provided by constructor) with the value from the user specified configuration file, if any.
* Behavior is undefined and erroneous if no default is provided in the constructor!
* Descriptions/invariants for each variable are/will be provided in the header file and are not repeated here, except where 
* behavior/range bounding is complicated/necessary-for-stability abd additional validation code is required for checking/adjusting user inputs
*/
void Configuration::OverrideDefaultsWithUserConfiguration() {
    // collision_hacks substruct
    collision_hacks.collision_hack_distance = GameConfig::GetVariable("physics",
            "collision_avoidance_hack_distance",
            collision_hacks.collision_hack_distance);
    collision_hacks.collision_damage_to_ai =
            GameConfig::GetVariable("physics", "collisionDamageToAI", collision_hacks.collision_damage_to_ai);
    collision_hacks.crash_dock_hangar =
            GameConfig::GetVariable("physics", "only_hangar_collision_docks", collision_hacks.crash_dock_hangar);
    collision_hacks.crash_dock_unit =
            GameConfig::GetVariable("physics", "unit_collision_docks", collision_hacks.crash_dock_unit);
    // (STYLE WARNING) front collision_hack_angle appears to expects to equal the cosine of a radian value, but default implies that it reads degrees! Value not specified directly, breaks pattern, should consider normalizing behavior with other config vars
    collision_hacks.front_collision_hack_angle = std::cos(
            M_PI * GameConfig::GetVariable("physics", "front_collision_avoidance_hack_angle", 40)
                    / 180.0f); // uses default value - must be changed in tandem with constructor!
    collision_hacks.front_collision_hack_distance = GameConfig::GetVariable("physics",
            "front_collision_avoidance_hack_distance",
            collision_hacks.front_collision_hack_distance);
    collision_hacks.cargo_deals_collide_damage = GameConfig::GetVariable("physics",
            "cargo_deals_collide_damage",
            collision_hacks.cargo_deals_collide_damage);

    // computer substruct
    computer.default_lock_cone = GameConfig::GetVariable("physics", "lock_cone", computer.default_lock_cone);
    computer.default_max_range = GameConfig::GetVariable("graphics", "hud", "radarRange", computer.default_max_range);
    computer.default_tracking_cone = GameConfig::GetVariable("physics", "autotracking", computer.default_tracking_cone);

    // fuel substruct
    fuel.afterburner_fuel_usage =
            GameConfig::GetVariable("physics", "AfterburnerFuelUsage", fuel.afterburner_fuel_usage);
    fuel.fmec_exit_velocity_inverse =
            GameConfig::GetVariable("physics", "FMEC_exit_vel", fuel.fmec_exit_velocity_inverse);
    fuel.fuel_efficiency =
            GameConfig::GetVariable("physics", "LithiumRelativeEfficiency_Lithium", fuel.fuel_efficiency);
    fuel.fuel_equals_warp = GameConfig::GetVariable("physics", "fuel_equals_warp", fuel.fuel_equals_warp);
    fuel.normal_fuel_usage = GameConfig::GetVariable("physics", "FuelUsage", fuel.normal_fuel_usage);
    fuel.reactor_uses_fuel = GameConfig::GetVariable("physics", "reactor_uses_fuel", fuel.reactor_uses_fuel);

    // logging substruct
    logging.verbose_debug = GameConfig::GetVariable("data", "verbose_debug", logging.verbose_debug);

    // physics substruct
    physics.bleed_factor = GameConfig::GetVariable("physics", "warpbleed", physics.bleed_factor);
    physics.collision_scale_factor =
            GameConfig::GetVariable("physics", "collision_damage_scale", physics.collision_scale_factor);
    physics.inelastic_scale = GameConfig::GetVariable("physics", "inelastic_scale", physics.inelastic_scale);
    physics.kilojoules_per_damage =
            GameConfig::GetVariable("physics", "kilojoules_per_unit_damage", physics.kilojoules_per_damage);
    physics.max_force_multiplier =
            GameConfig::GetVariable("physics", "maxCollisionForceMultiplier", physics.max_force_multiplier);
    physics.max_shield_lowers_capacitance =
            GameConfig::GetVariable("physics", "max_shield_lowers_capacitance", physics.max_shield_lowers_capacitance);
    physics.max_torque_multiplier =
            GameConfig::GetVariable("physics", "maxCollisionTorqueMultiplier", physics.max_torque_multiplier);
    physics.minimum_mass = GameConfig::GetVariable("physics", "minimum_physics_object_mass", physics.minimum_mass);
    physics.minimum_time =
            GameConfig::GetVariable("physics", "minimum_time_between_recorded_player_collisions", physics.minimum_time);

    // warp substruct
    warp.insystem_jump_cost = GameConfig::GetVariable("physics", "insystem_jump_cost", warp.insystem_jump_cost);

    // weapons substruct
    weapons.can_fire_in_cloak = GameConfig::GetVariable("physics", "can_fire_in_cloak", weapons.can_fire_in_cloak);
    weapons.can_fire_in_spec = GameConfig::GetVariable("physics", "can_fire_in_spec", weapons.can_fire_in_spec);
}

// Each of the following constructors MUST specify default values for ALL struct elements - will be subsequently overrident by user specified values, if any

vegastrike_configuration::CollisionHacks::CollisionHacks() :
        collision_hack_distance(10000.0f),
        collision_damage_to_ai(false),
        crash_dock_hangar(false),
        crash_dock_unit(false),
        front_collision_hack_angle(std::cos(M_PI * 40.0
                / 180.0)), /* Note: Does not follow pattern of directly setting itself to user specified config value, must change default value in override as well if changing it here*/
        front_collision_hack_distance(200000.0f),
        cargo_deals_collide_damage(false) {
}

vegastrike_configuration::Computer::Computer() :
        default_lock_cone(0.8f),
        default_max_range(20000.0f),
        default_tracking_cone(0.93f) {
}

vegastrike_configuration::Fuel::Fuel() :
        afterburner_fuel_usage(4.0f),
        fmec_exit_velocity_inverse(0.0000002f),
        fuel_efficiency(1.0f),
        fuel_equals_warp(false),
        normal_fuel_usage(1.0f),
        reactor_uses_fuel(false) {
}

vegastrike_configuration::Logging::Logging() :
        verbose_debug(false) {
}

vegastrike_configuration::Physics::Physics() :
        bleed_factor(2.0f),
        collision_scale_factor(1.0f),
        inelastic_scale(0.8f),
        kilojoules_per_damage(5400.0f),
        max_force_multiplier(5.0f),
        max_shield_lowers_capacitance(false),
        max_torque_multiplier(0.67f),
        minimum_mass(1e-6f), /* this is actually a bit high (1 gram, in the current, non-SI, units that VS uses ), fwiw - we may want to change this some day to, say, a miligram .*/
        minimum_time(0.1f) {
}

vegastrike_configuration::Warp::Warp() :
        insystem_jump_cost(0.1f) {
}

vegastrike_configuration::Weapons::Weapons() :
        can_fire_in_cloak(false),
        can_fire_in_spec(false) {
}
