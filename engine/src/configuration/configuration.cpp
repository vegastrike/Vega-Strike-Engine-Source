#include "configuration.h"
#include "game_config.h"

#include <math.h>

// Global Variable Definition
Configuration configuration;

Configuration::Configuration()
{
    //logging.verbose_debug = GameConfig::GetVariable("data", "verbose_debug", false);
}

vegastrike_configuration::CollisionHacks::CollisionHacks():
    collision_hack_distance(GameConfig::GetVariable( "physics", "collision_avoidance_hack_distance", 10000)),
    collision_damage_to_ai(GameConfig::GetVariable( "physics", "collisionDamageToAI", false)),
    crash_dock_hangar(GameConfig::GetVariable( "physics", "only_hangar_collision_docks", false)),
    crash_dock_unit(GameConfig::GetVariable( "physics", "unit_collision_docks", false)),
    front_collision_hack_angle(std::cos( M_PI * GameConfig::GetVariable( "physics", "front_collision_avoidance_hack_angle", 40)/180.0f )),
    front_collision_hack_distance(GameConfig::GetVariable( "physics", "front_collision_avoidance_hack_distance", 200000)),
    cargo_deals_collide_damage(GameConfig::GetVariable( "physics", "cargo_deals_collide_damage",
                                             false)) {}


vegastrike_configuration::Computer::Computer():
    default_lock_cone(GameConfig::GetVariable( "physics", "lock_cone", 0.8 )),
    default_max_range(GameConfig::GetVariable( "graphics", "hud", "radarRange", 20000 )),
    default_tracking_cone(GameConfig::GetVariable( "physics", "autotracking", 0.93 )) {}

vegastrike_configuration::Fuel::Fuel():
    afterburner_fuel_usage(GameConfig::GetVariable( "physics", "AfterburnerFuelUsage", 4.0f )),
    fmec_exit_velocity_inverse(GameConfig::GetVariable( "physics", "FMEC_exit_vel", 0.0000002 )),
    fuel_efficiency(GameConfig::GetVariable( "physics", "LithiumRelativeEfficiency_Lithium", 1.0 )),
    fuel_equals_warp(GameConfig::GetVariable( "physics", "fuel_equals_warp", false )),
    normal_fuel_usage(GameConfig::GetVariable( "physics", "FuelUsage", 1.0f )),
    reactor_uses_fuel(GameConfig::GetVariable( "physics", "reactor_uses_fuel", false )) {}

vegastrike_configuration::Logging::Logging():
    verbose_debug(GameConfig::GetVariable("data", "verbose_debug", false)) {}

vegastrike_configuration::Physics::Physics():

    bleed_factor(GameConfig::GetVariable( "physics", "warpbleed", 2.0f )),
    collision_scale_factor(GameConfig::GetVariable( "physics", "collision_damage_scale", 1.0f )),
    inelastic_scale(GameConfig::GetVariable( "physics", "inelastic_scale", 0.8f)),
    kilojoules_per_damage( GameConfig::GetVariable( "physics", "kilojoules_per_unit_damage", 5400 ) ),
    max_force_multiplier( GameConfig::GetVariable( "physics", "maxCollisionForceMultiplier", 5) ),
    max_shield_lowers_capacitance(GameConfig::GetVariable( "physics", "max_shield_lowers_capacitance", false )),
    max_torque_multiplier( GameConfig::GetVariable( "physics", "maxCollisionTorqueMultiplier", 0.67f) ),
    minimum_mass( 1e-6f ),
    minimum_time( GameConfig::GetVariable( "physics", "minimum_time_between_recorded_player_collisions", 0.1f )) {}

vegastrike_configuration::Warp::Warp():
    insystem_jump_cost(GameConfig::GetVariable( "physics", "insystem_jump_cost", 0.1 )) {}

vegastrike_configuration::Weapons::Weapons():
    can_fire_in_cloak( GameConfig::GetVariable( "physics", "can_fire_in_cloak", false)),
    can_fire_in_spec(GameConfig::GetVariable( "physics", "can_fire_in_spec", false)) {}
