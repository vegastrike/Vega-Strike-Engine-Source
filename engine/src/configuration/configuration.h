/*
 * configuration.h
 *
 * Copyright (C) 2021-2022 Daniel Horn, Roy Falk, ministerofinformation,
 * David Wales, Stephen G. Tuggy, and other Vega Strike contributors
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

#include <cstdint>
#include <memory>
#include <string>

namespace vegastrike_configuration {
// Config Structs Declaration

struct AIConfig {
    bool assist_friend_in_need{};
    int32_t hull_damage_anger{};
    float hull_percent_for_comm{};
    int32_t shield_damage_anger{};
};

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

struct HudConfig {
    bool draw_rendered_crosshairs{};
    std::string already_near_message{};
    float armor_hull_size{};
    std::string asteroids_near_message{};
    bool basename_colon_basename{};
    float box_line_thickness{};
    bool completed_objectives_last{};
    bool crosshairs_on_chase_cam{};
    bool crosshairs_on_padlock{};
    std::string damage_report_heading{};
    bool debug_position{};
    float diamond_line_thickness{};
    float diamond_rotation_speed{};
    float diamond_size{};
    bool display_relative_velocity{};
    bool display_warp_energy_if_no_jump_drive{};
    bool draw_all_target_boxes{};
    bool draw_always_itts{};
    bool draw_line_to_itts{};
    bool draw_line_to_target{};
    bool draw_line_to_targets_target{};
    bool draw_nav_symbol{};
    bool draw_tactical_target{};
    bool draw_targeting_boxes{};
    bool draw_targeting_boxes_inside{};
    bool draw_arrow_on_chase_cam{};
    bool draw_arrow_on_pan_cam{};
    bool draw_arrow_on_pan_target{};
    bool draw_arrow_to_target{};
    bool draw_blips_on_both_radar{};
    bool draw_damage_sprite{};
    bool draw_gravitational_objects{};
    bool draw_nav_grid{};
    bool draw_significant_blips{};
    bool draw_star_direction{};
    bool draw_unit_on_chase_cam{};
    bool draw_vdu_view_shields{};
    bool draw_weapon_sprite{};
    std::string enemy_near_message{};
    int32_t gun_list_columns{};
    bool invert_friendly_shields{};
    bool invert_friendly_sprite{};
    bool invert_target_shields{};
    bool invert_target_sprite{};
    bool invert_view_shields{};
    bool itts_for_beams{};
    bool itts_for_lockable{};
    float itts_line_to_mark_alpha{};
    bool itts_use_average_gun_speed{};
    bool lock_center_crosshair{};
    float lock_confirm_line_length{};
    std::string manifest_heading{};
    float max_missile_bracket_size{};
    std::string message_prefix{};
    float min_missile_bracket_size{};
    float min_lock_box_size{};
    float min_radar_blip_size{};
    float min_target_box_size{};
    std::string mounts_list_empty{};
    float nav_cross_line_thickness{};
    std::string not_included_in_damage_report{};
    bool out_of_cone_distance{};
    float padlock_view_lag{};
    float padlock_view_lag_fix_zone{};
    std::string planet_near_message{};
    bool print_damage_percent{};
    bool print_faction{};
    bool print_fg_name{};
    bool print_fg_sub_id{};
    bool print_request_docking{};
    bool print_ship_type{};
    bool projectile_means_missile{};
    float radar_range{};
    std::string radar_type{};
    float radar_search_extra_radius{};
    bool rotating_bracket_inner{};
    float rotating_bracket_size{};
    float rotating_bracket_width{};
    float scale_relationship_color{};
    bool shield_vdu_fade{};
    float shield_vdu_thresh[3]{};
    bool show_negative_blips_as_positive{};
    bool simple_manifest{};
    std::string starship_near_message{};
    bool switch_to_target_mode_on_key{};
    bool switch_back_from_comms{};
    float tac_target_foci{};
    float tac_target_length{};
    float tac_target_thickness{};
    float text_background_alpha{};
    bool top_view{};
    bool untarget_beyond_cone{};
};

struct GraphicsConfig {
    HudConfig hud;
    std::string automatic_landing_zone_warning{};
    std::string automatic_landing_zone_warning1{};
    std::string automatic_landing_zone_warning2{};
    std::string automatic_landing_zone_warning_text{};
    bool draw_heading_marker{};
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

    bool does_missile_bounce{};
    float default_shield_tightness{};

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
    vegastrike_configuration::AIConfig ai;
    vegastrike_configuration::CollisionHacks collision_hacks;
    vegastrike_configuration::Computer computer;
    vegastrike_configuration::Fuel fuel;
    vegastrike_configuration::GraphicsConfig graphics;
    vegastrike_configuration::Logging logging;
    vegastrike_configuration::Physics physics;
    vegastrike_configuration::Warp warp;
    vegastrike_configuration::Weapons weapons;
};

extern std::shared_ptr<Configuration> configuration();

#endif // CONFIGURATION_H
