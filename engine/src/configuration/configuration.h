/*
 * configuration.h
 *
 * Copyright (C) 2021-2023 Daniel Horn, Roy Falk, ministerofinformation,
 * David Wales, Stephen G. Tuggy, Benjamen R. Meyer, and other Vega Strike contributors
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


#ifndef VEGA_STRIKE_ENGINE_CONFIG_CONFIGURATION_H
#define VEGA_STRIKE_ENGINE_CONFIG_CONFIGURATION_H

#include <cstdint>
#include <memory>
#include <string>
#include <boost/json.hpp>

#include "components/energy_consumer.h"

#include "graphics_config.h"

namespace vega_config {
// Config Structs Declaration

struct GeneralConfig {
    GeneralConfig() = default;

    int screen{0};  // TODO: move to a dedicated section along with other such paramters.
                    // e.g. screen width and height, resolution, color, etc.

    float pitch{0.0F};
    float yaw{0.0F};
    float roll{0.0F};
    bool force_anonymous_mission_names{true};
    bool write_savegame_on_exit{true};
    int32_t times_to_show_help_screen{3};
    bool remember_savegame{true};
    std::string new_game_save_name{"New_Game"};
    size_t quick_savegame_summaries_buffer{16384U};
    std::string empty_mission{"internal.mission"};
    std::string custom_python{"import custom;custom.processMessage"};
    bool quick_savegame_summaries{true};
    int32_t garbage_collect_frequency{20};
    uint32_t num_old_systems{6U};
    bool delete_old_systems{true};
    bool while_loading_star_system{false};
};

struct AIFiringConfig {
    AIFiringConfig() = default;

    float missile_probability{0.01F};
    float aggressivity{15.0F};
};

struct AITargetingConfig {
    AITargetingConfig() = default;

    float escort_distance{10.0F};
    float turn_leader_distance{5.0F};
    float time_to_recommand_wing{100.0F};
    float min_time_to_switch_targets{3.0F};
};

struct AIConfig {
    AIConfig() = default;

    bool always_obedient{true};
    bool assist_friend_in_need{true};
    float ease_to_anger{-0.5F};
    float ease_to_appease{0.5F};
    int32_t hull_damage_anger{10};
    float hull_percent_for_comm{0.75F};
    float lowest_negative_comm_choice{-0.00001F};
    float lowest_positive_comm_choice{0.0F};
    float mood_swing_level{0.2F};
    float random_response_range{0.8F};
    int32_t shield_damage_anger{1};
    bool jump_without_energy{false};
    float friend_factor{0.1F};
    float kill_factor{0.2F};
    double min_relationship{-20.0};

    AIFiringConfig firing_config;
    AITargetingConfig targeting_config;
};

struct ShuffleSongsConfig {
    bool clear_history_on_list_change{false};

    ShuffleSongsConfig() = default;
};

struct AudioConfig {
    bool every_other_mount{false};
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
    std::string master_part_list{"master_part_list"};
    bool using_templates{true};

    DataConfig() = default;
};

struct EjectConfig {
    float eject_cargo_percent{1.0F};
    uint32_t max_dumped_cargo{15U};
    float hull_damage_to_eject{100.0F}; // TODO: Make use of this setting in actual code
    float auto_eject_percent{0.5F};
    bool player_auto_eject{true};

    EjectConfig() = default;
};

struct Fuel {
    float afterburner_fuel_usage;
    

    /* This used to be Lithium6constant.
     * There's some relevant context that's been removed from the original name of this variable "Lithium6constant" --
     * a better name would ultimately be "FuelToEnergyConversionRelativeToLithium6DeuterideFusion" -
     * that fully encodes what the efficiency is relative to. */
    double fuel_efficiency{1.0};
    bool fuel_equals_warp;
    float normal_fuel_usage;
    bool reactor_uses_fuel;
    float vsd_mj_yield{5.4F};
    float no_fuel_thrust{0.4F};
    float no_fuel_afterburn{0.1F};
    bool variable_fuel_consumption{false};
    float deuterium_relative_efficiency_lithium{1.0F};
    float fmec_factor{0.000000008F};
    float reactor_idle_efficiency{0.98F};
    float min_reactor_efficiency{0.00001F};
    float ecm_energy_cost{0.05F};
    
    double megajoules_factor{100};
    double fuel_factor{60.0};   // Multiply fuel by this to get fuel by minutes
    double energy_factor{1.0};
    double ftl_energy_factor{1.0};

    double reactor_factor{1.0};

    double ftl_drive_factor{0.1};
    double jump_drive_factor{1.0};

    double shield_maintenance_factor{0.01};
    double shield_regeneration_factor{0.1};

    // 0 infinite, 1 fuel, 2 energy, 3 ftl_energy, 4 disabled
    EnergyConsumerSource drive_source{EnergyConsumerSource::Fuel}; 
    EnergyConsumerSource reactor_source{EnergyConsumerSource::Fuel};
    EnergyConsumerSource afterburner_source{EnergyConsumerSource::Fuel};
    EnergyConsumerSource jump_drive_source{EnergyConsumerSource::FTLEnergy};
    EnergyConsumerSource cloak_source{EnergyConsumerSource::Energy};

    double minimum_drive{0.15};

    Fuel();
    Fuel(boost::json::object object);
};

struct HudConfig {
    bool draw_rendered_crosshairs{true};
    std::string already_near_message{"#ff0000Already Near#000000"};
    float armor_hull_size{0.55F};
    std::string asteroids_near_message{"#ff0000Asteroids Near#000000"};
    bool basename_colon_basename{true};
    float box_line_thickness{1.0F};
    bool completed_objectives_last{true};
    bool crosshairs_on_chase_cam{false};
    bool crosshairs_on_padlock{false};
    std::string damage_report_heading{"#00ff00DAMAGE REPORT\\n\\n"};
    bool debug_position{false};
    float diamond_line_thickness{1.0F};
    float diamond_rotation_speed{1.0F};
    float diamond_size{2.05F};
    bool display_relative_velocity{true};
    bool display_warp_energy_if_no_jump_drive{true};
    bool draw_all_target_boxes{false};
    bool draw_always_itts{false};
    bool draw_line_to_itts{false};
    bool draw_line_to_target{false};
    bool draw_line_to_targets_target{false};
    bool draw_nav_symbol{false};
    bool draw_tactical_target{false};
    bool draw_targeting_boxes{true};
    bool draw_targeting_boxes_inside{true};
    bool draw_arrow_on_chase_cam{true};
    bool draw_arrow_on_pan_cam{false};
    bool draw_arrow_on_pan_target{false};
    bool draw_arrow_to_target{true};
    bool draw_blips_on_both_radar{false};
    bool draw_damage_sprite{true};
    bool draw_gravitational_objects{true};
    bool draw_nav_grid{true};
    bool draw_significant_blips{true};
    bool draw_star_direction{true};
    bool draw_unit_on_chase_cam{true};
    bool draw_vdu_view_shields{true};
    bool draw_weapon_sprite{false};
    std::string enemy_near_message{"#ff0000Enemy Near#000000"};
    int32_t gun_list_columns{1};
    bool invert_friendly_shields{false};
    bool invert_friendly_sprite{false};
    bool invert_target_shields{false};
    bool invert_target_sprite{false};
    bool invert_view_shields{false};
    bool itts_for_beams{false};
    bool itts_for_lockable{false};
    float itts_line_to_mark_alpha{0.1F};
    bool itts_use_average_gun_speed{true};
    bool lock_center_crosshair{false};
    float lock_confirm_line_length{1.5F};
    std::string manifest_heading{"Manifest\\n"};
    float max_missile_bracket_size{2.05F};
    std::string message_prefix{};
    float min_missile_bracket_size{1.05F};
    float min_lock_box_size{0.001F};
    float min_radar_blip_size{0.0F};
    float min_target_box_size{0.01F};
    std::string mounts_list_empty{};
    float nav_cross_line_thickness{1.0F};
    std::string not_included_in_damage_report{"plasteel_hull tungsten_hull isometal_hull"};
    bool out_of_cone_distance{false};
    float padlock_view_lag{1.5F};
    float padlock_view_lag_fix_zone{0.0872F};
    std::string planet_near_message{"#ff0000Planetary Hazard Near#000000"};
    bool print_damage_percent{true};
    bool print_faction{true};
    bool print_fg_name{true};
    bool print_fg_sub_id{false};
    bool print_request_docking{true};
    bool print_ship_type{true};
    bool projectile_means_missile{false};
//    float radar_range{};  // I believe this has been moved to computer_config.default_max_range -- stephengtuggy 2022-05-28
    std::string radar_type{"WC"};
    float radar_search_extra_radius{1000.0F};
    bool rotating_bracket_inner{true};
    float rotating_bracket_size{0.58F};
    float rotating_bracket_width{0.1F};
    float scale_relationship_color{10.0F};
    bool shield_vdu_fade{true};
    float shield_vdu_thresh[3]{};
    bool show_negative_blips_as_positive{true};
    bool simple_manifest{false};
    std::string starship_near_message{"#ff0000Starship Near#000000"};
    bool switch_to_target_mode_on_key{true};
    bool switch_back_from_comms{true};
    float tac_target_foci{0.5F};
    float tac_target_length{0.1F};
    float tac_target_thickness{1.0F};
    float text_background_alpha{0.0625F};
    bool top_view{false};
    bool untarget_beyond_cone{false};

    HudConfig() = default;
};

struct GlowFlickerConfig {
    float flicker_time{30.0F};
    float flicker_off_time{2.0F};
    float min_flicker_cycle{2.0F};
    float num_times_per_second_on{0.66F};
    float hull_for_total_dark{0.04F};

    GlowFlickerConfig() = default;
};

struct GraphicsConfig {
    HudConfig hud;
    GlowFlickerConfig glow_flicker;
    std::string automatic_landing_zone_warning{"comm_docking.ani"};
    std::string automatic_landing_zone_warning1{"comm_docking.ani"};
    std::string automatic_landing_zone_warning2{"comm_docking.ani"};
    std::string automatic_landing_zone_warning_text{"Now Entering an \"Automatic Landing Zone\"."};
    bool draw_heading_marker{false};
    float missile_explosion_radius_mult{1.0F};
    bool missile_sparkle{false};
    float in_system_jump_ani_second_ahead{4.0F};
    float in_system_jump_ani_second_ahead_end{0.03F};
    std::string in_system_jump_animation{"warp.ani"};
    float atmosphere_emmissive{1.0F};
    float atmosphere_diffuse{1.0F};
    float default_engine_activation{0.00048828125F};
    std::string explosion_animation{"explosion_orange.ani"};
    int32_t shield_detail{16};
    std::string shield_texture{"shield.bmp"};
    std::string shield_technique;
    float fov{78.0F};
    float reduced_vdus_width{0.0F};
    float reduced_vdus_height{0.0F};
    float star_body_radius{0.33F};
    bool draw_star_glow{true};
    bool draw_star_body{true};
    float star_glow_radius{1.33F};
    bool glow_ambient_star_light{false};
    bool glow_diffuse_star_light{false};
    int32_t planet_detail_stacks{24};
    std::string wormhole_unit{"wormhole"};
    float city_light_strength{10.0F};
    float day_city_light_strength{0.0F};
    int32_t num_times_to_draw_shine{2};

    GraphicsConfig() = default;
};

struct Logging {
    Logging() = default;

    int8_t vsdebug{0};
    bool verbose_debug{false};
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

    bool does_missile_bounce{false};     // TODO: Why is this not being used anywhere?
    float default_shield_tightness{0.0F};
    bool system_damage_on_armor{false};
    float indiscriminate_system_destruction{0.25F};
    bool separate_system_flakiness_component{false};
    float shield_energy_capacitance{0.2F};
    bool use_max_shield_energy_usage{false};
    bool shields_in_spec{false};
    float shield_maintenance_charge{0.25F};
    uintmax_t max_ecm{4U};
    float max_lost_target_live_time{30.0F};
    float percent_missile_match_target_velocity{1.0F};
    double game_speed{1.0};
    bool game_speed_lying{false};
    double game_accel{1.0};
    double combat_mode_multiplier{100.0};
    float velocity_max{10000.0F};
    float max_player_rotation_rate{24.0F};
    float max_non_player_rotation_rate{360.0F};
    float capship_size{500.0F};
    float near_autotrack_cone{0.9F};
    float close_enough_to_autotrack{4.0F};
    float distance_to_warp{1000000000000.0F};
    float target_distance_to_warp_bonus{1.33F};
    bool no_spec_jump{true};
    float difficulty_speed_exponent{0.2F};
    float min_damage{0.001F};
    float max_damage{0.999F};
    float max_radar_cone_damage{0.9F};
    float max_radar_lock_cone_damage{0.95F};
    float max_radar_track_cone_damage{0.98F};
    float thruster_hit_chance{0.25F};
    float friendly_auto_radius{0.0F};
    float neutral_auto_radius{0.0F};
    float hostile_auto_radius{1000.0F};
    float min_asteroid_distance{-100.0F};
    bool steady_itts{false};
    bool no_unit_collisions{false};
    bool difficulty_based_shield_recharge{true};
    bool engine_energy_takes_priority{true};
    float density_of_rock{3.0F};
    float density_of_jump_point{100000.0F};
    float jump_mesh_radius_scale{0.5F};
    bool planets_can_have_subunits{false};
    bool planets_always_neutral{true};
    bool no_damage_to_docked_ships{true};
    bool difficulty_based_enemy_damage{true};
    float speeding_discharge{0.25F};
    float min_shield_speeding_discharge{0.1F};
    float nebula_shield_recharge{0.5F};

    PhysicsConfig();
};

struct UnitConfig {
    UnitConfig() = default;

    float default_aggressivity{2.01F};
};

// Covers both SPEC and jumps
struct WarpConfig {
    float insystem_jump_cost{0.1F};
    // Pi^2 * C
    float max_effective_velocity{2960881320.33F};
    float bleed_factor{2.0F};
    float warp_energy_multiplier{0.12F};
    float player_warp_energy_multiplier{0.12F};
    float warp_ramp_up_time{5.0F};
    float computer_warp_ramp_up_time{10.0F};
    float warp_ramp_down_time{0.5F};
    float warp_memory_effect{0.9F};
    float warp_stretch_cutoff{500000.0F};
    float warp_stretch_decel_cutoff{500000.0F};
    // Pi^2
    float warp_multiplier_min{9.86968440109F};
    // C
    float warp_multiplier_max{300000000.0F};
    bool use_warp_energy_for_cloak{true};

    WarpConfig() = default;
};

struct WeaponsConfig {
    bool can_fire_in_cloak{};
    bool can_fire_in_spec{};
    WeaponsConfig();
};

struct GameStart {
    std::string default_mission;
    std::string introduction;

    GameStart();
    GameStart(boost::json::object object);
};

struct Dock {
    float planet_dock_port_size{1.2F};
    float planet_dock_port_min_size{300.0F};
    double dock_planet_radius_percent{1.5};
    bool simple_dock{true};

    Dock() {}
    Dock(boost::json::object object);
};

}

// not using namespace vega_config, because ComputerConfig would be ambiguous
// UPDATE 2022-05-19 stephengtuggy -- Computer renamed to ComputerConfig, so no longer ambiguous

class Configuration {
public:
    Configuration();
    void OverrideDefaultsWithUserConfiguration();
    vega_config::GeneralConfig general_config;
    Graphics2Config graphics2_config;
    vega_config::DataConfig data_config;
    vega_config::AIConfig ai;
    vega_config::AudioConfig audio_config;
    vega_config::CollisionHacks collision_hacks;
    vega_config::ComputerConfig computer_config;
    vega_config::EjectConfig eject_config;
    vega_config::Fuel fuel;
    vega_config::GraphicsConfig graphics_config;
    vega_config::Logging logging;
    vega_config::PhysicsConfig physics_config;
    vega_config::UnitConfig unit_config;
    vega_config::WarpConfig warp_config;
    vega_config::WeaponsConfig weapons;
    vega_config::GameStart game_start;
    vega_config::Dock dock;
};

extern std::shared_ptr<Configuration> configuration();

#endif //VEGA_STRIKE_ENGINE_CONFIG_CONFIGURATION_H
