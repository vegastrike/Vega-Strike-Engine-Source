#ifndef CONFIGURATION_H
#define CONFIGURATION_H


struct Configuration
{
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
        const float fmec_exit_velocity_inverse; // Well, that's very clear
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

    Configuration();

    CollisionHacks collision_hacks;
    Computer computer;
    Fuel fuel;
    Logging logging;
    Physics physics;
    Warp warp;
    Weapons weapons;
};

extern Configuration configuration;

#endif // CONFIGURATION_H
