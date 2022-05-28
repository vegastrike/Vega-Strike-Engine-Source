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

namespace vega_config {
// Config Structs Declaration

struct GeneralConfig {
    float pitch{};
    float yaw{};
    float roll{};
    bool force_anonymous_mission_names{};

    GeneralConfig() = default;
};

struct AIConfig {
    bool always_obedient{};
    bool assist_friend_in_need{};
    float ease_to_anger{};
    float ease_to_appease{};
    int32_t hull_damage_anger{};
    float hull_percent_for_comm{};
    float lowest_negative_comm_choice{};
    float lowest_positive_comm_choice{};
    float mood_swing_level{};
    float random_response_range{};
    int32_t shield_damage_anger{};
    bool jump_without_energy{};
    float friend_factor{};
    float kill_factor{};

    AIConfig() = default;
};

struct ShuffleSongsConfig {
    bool clear_history_on_list_change{};

    ShuffleSongsConfig() = default;
};

struct AudioConfig {
    bool every_other_mount{};
    ShuffleSongsConfig shuffle_songs;

    AudioConfig() = default;
};

// These are settings that are relevant to collision.cpp and are marked as hacks
// or their comments are unclear or they are unclear
struct CollisionHacks {
    float collision_hack_distance;
    bool collision_damage_to_ai;
    bool crash_dock_hangar;
    bool crash_dock_unit;
    double front_collision_hack_angle;
    float front_collision_hack_distance;
    bool cargo_deals_collide_damage;
    //int upgrade_faction = (cargo_deals_collide_damage ? -1 : FactionUtil::GetUpgradeFaction())

    CollisionHacks();
};

// Also radar
struct ComputerConfig {
    // Note: below is probably a stale comment
    // DO NOT CHANGE see unit_customize.cpp
    float default_lock_cone;
    float default_max_range;
    float default_tracking_cone;

    ComputerConfig();
};

struct DataConfig {
    std::string master_part_list{};
    bool using_templates{};

    DataConfig() = default;
};

struct EjectConfig {
    float eject_cargo_percent{};
    uint32_t max_dumped_cargo{};
    float hull_damage_to_eject{};
    float auto_eject_percent{};
    bool player_auto_eject{};

    EjectConfig() = default;
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
    float fmec_exit_velocity_inverse{};

    /* This used to be Lithium6constant.
     * There's some relevant context that's been removed from the original name of this variable "Lithium6constant" --
     * a better name would ultimately be "FuelToEnergyConversionRelativeToLithium6DeuterideFusion" -
     * that fully encodes what the efficiency is relative to. */
    float fuel_efficiency;
    bool fuel_equals_warp;
    float normal_fuel_usage;
    bool reactor_uses_fuel;
    float vsd_mj_yield{};
    float no_fuel_thrust{};
    float no_fuel_afterburn{};
    bool variable_fuel_consumption{};
    float deuterium_relative_efficiency_lithium{};
    float fmec_factor{};
    float reactor_idle_efficiency{};
    float min_reactor_efficiency{};
    float ecm_energy_cost{};
    float fuel_conversion{};

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

    HudConfig() = default;
};

struct GlowFlickerConfig {
    float flicker_time{};
    float flicker_off_time{};
    float min_flicker_cycle{};
    float num_times_per_second_on{};
    float hull_for_total_dark{};

    GlowFlickerConfig() = default;
};

struct GraphicsConfig {
    HudConfig hud;
    GlowFlickerConfig glow_flicker;
    std::string automatic_landing_zone_warning{};
    std::string automatic_landing_zone_warning1{};
    std::string automatic_landing_zone_warning2{};
    std::string automatic_landing_zone_warning_text{};
    bool draw_heading_marker{};
    float missile_explosion_radius_mult{};
    bool missile_sparkle{};
    float in_system_jump_ani_second_ahead{};
    float in_system_jump_ani_second_ahead_end{};
    std::string in_system_jump_animation{};
    float atmosphere_emmissive{};
    float atmosphere_diffuse{};
    float default_engine_activation{};
    std::string explosion_animation{};
    int32_t shield_detail{};
    std::string shield_texture{};
    std::string shield_technique{};
    float fov{};
    float reduced_vdus_width{};
    float reduced_vdus_height{};
    float star_body_radius{};
    bool draw_star_glow{};
    bool draw_star_body{};
    float star_glow_radius{};
    bool glow_ambient_star_light{};
    bool glow_diffuse_star_light{};
    int32_t planet_detail_stacks{};
    std::string wormhole_unit{};
    float city_light_strength{};
    float day_city_light_strength{};
    int32_t num_times_to_draw_shine{};

    GraphicsConfig() = default;
};

struct Logging {
    bool verbose_debug;
    Logging();
};

struct PhysicsConfig {
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
    bool system_damage_on_armor{};
    float indiscriminate_system_destruction{};
    bool separate_system_flakiness_component{};
    float shield_energy_capacitance{};
    bool use_max_shield_energy_usage{};
    bool shields_in_spec{};
    float shield_maintenance_charge{};
    uintmax_t max_ecm{};
    float max_lost_target_live_time{};
    float percent_missile_match_target_velocity{};
    float game_speed{};
    float game_accel{};
    float velocity_max{};
    float max_player_rotation_rate{};
    float max_non_player_rotation_rate{};
    bool unit_table{};
    float capship_size{};
    float near_autotrack_cone{};
    float close_enough_to_autotrack{};
    float distance_to_warp{};
    float target_distance_to_warp_bonus{};
    bool no_spec_jump{};
    float difficulty_speed_exponent{};
    float min_damage{};
    float max_damage{};
    float max_radar_cone_damage{};
    float max_radar_lock_cone_damage{};
    float max_radar_track_cone_damage{};
    float thruster_hit_chance{};
    float friendly_auto_radius{};
    float neutral_auto_radius{};
    float hostile_auto_radius{};
    float min_asteroid_distance{};
    bool steady_itts{};
    bool no_unit_collisions{};
    bool difficulty_based_shield_recharge{};
    bool engine_energy_takes_priority{};
    float density_of_rock{};
    float density_of_jump_point{};
    float planet_dock_port_size{};
    float planet_dock_port_min_size{};
    float jump_mesh_radius_scale{};
    bool planets_can_have_subunits{};
    bool planets_always_neutral{};
    bool no_damage_to_docked_ships{};
    bool difficulty_based_enemy_damage{};
    float speeding_discharge{};
    float min_shield_speeding_discharge{};
    float nebula_shield_recharge{};

    PhysicsConfig();
};

struct UnitConfig {
    UnitConfig() = default;

    float default_aggressivity{2.01F};
};

// Covers both SPEC and jumps
struct WarpConfig {
    float insystem_jump_cost{};
    float max_effective_velocity{};
    float bleed_factor{};
    float warp_energy_multiplier{};
    float player_warp_energy_multiplier{};
    float warp_ramp_up_time{};
    float computer_warp_ramp_up_time{};
    float warp_ramp_down_time{};
    float warp_memory_effect{};
    float warp_stretch_cutoff{};
    float warp_stretch_decel_cutoff{};
    float warp_multiplier_min{};
    float warp_multiplier_max{};
    bool use_warp_energy_for_cloak{};

    WarpConfig();
};

struct WeaponsConfig {
    bool can_fire_in_cloak{};
    bool can_fire_in_spec{};
    WeaponsConfig();
};

}

// not using namespace vega_config, because ComputerConfig would be ambiguous
// UPDATE 2022-05-19 stephengtuggy -- Computer renamed to ComputerConfig, so no longer ambiguous

class Configuration {
public:
    Configuration();
    void OverrideDefaultsWithUserConfiguration();
    vega_config::GeneralConfig general_config_;
    vega_config::DataConfig data_config_;
    vega_config::AIConfig ai;
    vega_config::AudioConfig audio_config_;
    vega_config::CollisionHacks collision_hacks;
    vega_config::ComputerConfig computer_config_;
    vega_config::EjectConfig eject_config_;
    vega_config::Fuel fuel;
    vega_config::GraphicsConfig graphics_config_;
    vega_config::Logging logging;
    vega_config::PhysicsConfig physics_config_;
    vega_config::UnitConfig unit_config_;
    vega_config::WarpConfig warp_config_;
    vega_config::WeaponsConfig weapons;
};

extern std::shared_ptr<Configuration> configuration();

#endif // CONFIGURATION_H
