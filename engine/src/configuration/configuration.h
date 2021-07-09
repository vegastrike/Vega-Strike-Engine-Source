#ifndef CONFIGURATION_H
#define CONFIGURATION_H

namespace vegastrike_configuration {
// Config Structs Declaration

// These are settings that are relevant to collision.cpp and are marked as hacks
// or their comments are unclear or they are unclear
struct CollisionHacks {
    const float collision_hack_distance;
    const bool collision_damage_to_ai;
    const bool crash_dock_hangar;
    const bool crash_dock_unit;
    const float front_collision_hack_angle;
    const float front_collision_hack_distance;
    const bool cargo_deals_collide_damage;
    //const int upgrade_faction = (cargo_deals_collide_damage ? -1 : FactionUtil::GetUpgradeFaction())

    CollisionHacks();
};

// Also radar
struct Computer {
    // Note: below is probably a stale comment
    // DO NOT CHANGE see unit_customize.cpp
    const float default_lock_cone;
    const float default_max_range;
    const float default_tracking_cone;

    Computer();
};

struct Fuel {
    const float afterburner_fuel_usage;
    /* There are a pair of "FMEC" variables - they both involve "Fuel Mass to Energy Conversion" -
     * this one happens to specify the inverse (it's only ever used as 1/Value, so just encode 1/Value, not Value)
     * of the assumed exit velocity of the mass ejected as thrust, calculated based on energy-possible
     * (if not necessarily plausible) outcomes from a Li-6 + Deuterium fusion reaction.
     * The other variable (not present here) FMEC_factor, is used in reactor --> energy production.
     * As the comment in the code next to the variable init says, it specifies how many metric tons (1 VS mass unit)
     * of fuel are used to produce 100MJ (one units.csv energy recharge unit) of recharge.
     * At some point, it wouldn't kill us to renormalize the engine and dataset to both just use SI units, but that's not a priority.
     */
    const float fmec_exit_velocity_inverse;

    /* This used to be Lithium6constant.
     * There's some relevant context that's been removed from the original name of this variable "Lithium6constant" --
     * a better name would ultimately be "FuelToEnergyConversionRelativeToLithium6DeuterideFusion" -
     * that fully encodes what the efficiency is relative to. */
    const float fuel_efficiency;
    const bool fuel_equals_warp;
    const float normal_fuel_usage;
    const bool reactor_uses_fuel;

    Fuel();
};

struct Logging {
    const bool verbose_debug;
    Logging();
};

struct Physics {
    const float bleed_factor;
    const float collision_scale_factor;
    const float inelastic_scale;
    const float kilojoules_per_damage;
    const float max_force_multiplier;
    const bool max_shield_lowers_capacitance;
    const float max_torque_multiplier;
    const float minimum_mass;
    const float minimum_time;             // between recorded player collisions


    // Disabled bouncing missile option. Missiles always explode when colliding with something.
    //static bool does_missile_bounce = GameConfig::GetVariable( "physics", "missile_bounce", false);

    Physics();
};

// Covers both SPEC and jumps
struct Warp {
    const float insystem_jump_cost;

    Warp();
};

struct Weapons {
    const bool can_fire_in_cloak;
    const bool can_fire_in_spec;
    Weapons();
};

}

// not using namespace vegastrike_configuration, because Computer would be ambiguous

struct Configuration
{
    Configuration();

    vegastrike_configuration::CollisionHacks collision_hacks;
    vegastrike_configuration::Computer computer;
    vegastrike_configuration::Fuel fuel;
    vegastrike_configuration::Logging logging;
    vegastrike_configuration::Physics physics;
    vegastrike_configuration::Warp warp;
    vegastrike_configuration::Weapons weapons;
};


extern Configuration configuration;

#endif // CONFIGURATION_H
