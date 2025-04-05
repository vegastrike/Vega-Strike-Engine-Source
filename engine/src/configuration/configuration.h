/*
 * configuration.h
 *
 * Copyright (C) 2021-2025 Daniel Horn, Roy Falk, ministerofinformation,
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

#include <string>
#include <memory>
#include <boost/filesystem/path.hpp>

#include "components/energy_consumer.h"

namespace vega_config {
    struct Config {
        explicit Config(const std::string& json_text);
        explicit Config(const boost::filesystem::path& config_file_path);

        void load_config(const std::string& json_text);
        void load_config(const boost::filesystem::path& config_file_path);


    struct {
        double audio_atom = 0.05555555556;
        bool command_interpreter = false;
        std::string custom_python = "import custom;custom.processMessage";
        bool debug_config = true;
        int debug_fs = 0;
        bool delete_old_systems = true;
        double docking_time = 20.0;
        double docking_fee = 0.0;
        std::string empty_mission = "internal.mission";
        bool force_anonymous_mission_names = true;
        double fuel_docking_fee = 0.0;
        int garbage_collect_frequency = 20;
        double jump_key_delay = 0.125;
        bool load_last_savegame = false;
        std::string new_game_save_name = "New_Game";
        int num_old_systems = 6;
        double percentage_speed_change_to_fault_search = 0.5;
        bool persistent_mission_across_ship_switch = true;
        double pitch = 0.0;
        bool quick_savegame_summaries = true;
        int quick_savegame_summaries_buffer_size = 16384;
        bool remember_savegame = true;
        double remove_downgrades_less_than_percent = 0.9;
        bool remove_impossible_downgrades = true;
        double roll = 0.0;
        int screen = 0;
        double simulation_atom = 0.03;
        int times_to_show_help_screen = 3;
        bool trade_interface_tracks_prices = true;
        int trade_interface_tracks_prices_top_rank = 10;
        int verbose_output = 0;
        int wheel_increment_lines = 3;
        bool while_loading_star_system = false;
        bool write_savegame_on_exit = true;
        double yaw = 0.0;

    } general;

    struct {
        int resolution_x = 2560;
        int resolution_y = 1600;
        int screen = 0;
        double atmosphere_emissive = 1.0;
        double atmosphere_diffuse = 1.0;
        std::string automatic_landing_zone_warning = "comm_docking.ani";
        std::string automatic_landing_zone_warning1 = "comm_docking.ani";
        std::string automatic_landing_zone_warning2 = "comm_docking.ani";
        std::string automatic_landing_zone_warning_text = "Now Entering an \"Automatic Landing Zone\".";
        double city_light_strength = 10.0;
        double day_city_light_strength = 0.0;
        double default_engine_activation = 0.00048828125;
        bool draw_heading_marker = false;
        bool draw_rendered_crosshairs = true;
        bool draw_star_body = true;
        bool draw_star_glow = true;
        std::string explosion_animation = "explosion_orange.ani";
        double fov = 78.0;
        bool glow_ambient_star_light = false;
        bool glow_diffuse_star_light = false;
        double in_system_jump_ani_second_ahead = 4.0;
        double in_system_jump_ani_second_ahead_end = 0.03;
        std::string in_system_jump_animation = "warp.ani";
        double missile_explosion_radius_mult = 1.0;
        bool missile_sparkle = false;
        int num_times_to_draw_shine = 2;
        int planet_detail_level = 24;
        double reduced_vdus_width = 0.0;
        double reduced_vdus_height = 0.0;
        int shield_detail_level = 24;
        std::string shield_technique = "";
        std::string shield_texture = "shield.bmp";
        double star_body_radius = 0.33;
        double star_glow_radius = 1.33;
        std::string wormhole_unit = "wormhole";

        struct {
            double flicker_time = 30.0;
            double flicker_off_time = 2.0;
            double min_flicker_cycle = 2.0;
            double num_times_per_second_on = 0.66;
            double hull_for_total_dark = 0.04;

        } glow_flicker;

        struct {
            std::string already_near_message = "#ff0000Already Near#000000";
            double armor_hull_size = 0.55;
            std::string asteroids_near_message = "#ff0000Asteroids Near#000000";
            bool basename_colon_basename = true;
            double box_line_thickness = 1.0;
            bool completed_objectives_last = true;
            bool crosshairs_on_chase_cam = false;
            bool crosshairs_on_padlock = false;
            std::string damage_report_heading = "#00ff00DAMAGE REPORT\n\n";
            bool debug_position = false;
            double diamond_line_thickness = 1.0;
            double diamond_rotation_speed = 1.0;
            double diamond_size = 2.05;
            bool display_relative_velocity = true;
            bool display_warp_energy_if_no_jump_drive = true;
            bool draw_all_target_boxes = false;
            bool draw_always_itts = false;
            bool draw_arrow_on_chase_cam = true;
            bool draw_arrow_on_pan_cam = false;
            bool draw_arrow_on_pan_target = false;
            bool draw_arrow_to_target = true;
            bool draw_blips_on_both_radar = false;
            bool draw_rendered_crosshairs = true;
            bool draw_damage_sprite = true;
            bool draw_gravitational_objects = true;
            bool draw_line_to_itts = false;
            bool draw_line_to_target = false;
            bool draw_line_to_targets_target = false;
            bool draw_nav_grid = true;
            bool draw_nav_symbol = false;
            bool draw_tactical_target = false;
            bool draw_targetting_boxes = true;
            bool draw_targetting_boxes_inside = true;
            bool draw_significant_blips = true;
            bool draw_star_direction = true;
            bool draw_unit_on_chase_cam = true;
            bool draw_vdu_view_shields = true;
            bool draw_weapon_sprite = false;
            std::string enemy_near_message = "#ff0000Enemy Near#000000";
            int gun_list_columns = 1;
            bool invert_friendly_shields = false;
            bool invert_friendly_sprite = false;
            bool invert_target_shields = false;
            bool invert_target_sprite = false;
            bool invert_view_shields = false;
            bool itts_for_beams = false;
            bool itts_for_lockable = false;
            double itts_line_to_mark_alpha = 0.1;
            bool itts_use_average_gun_speed = true;
            bool lock_center_crosshair = false;
            double lock_confirm_line_length = 1.5;
            std::string manifest_heading = "Manifest\n";
            double max_missile_bracket_size = 2.05;
            std::string message_prefix = "";
            double min_missile_bracket_size = 1.05;
            double min_lock_box_size = 0.001;
            double min_radar_blip_size = 0.0;
            double min_target_box_size = 0.01;
            std::string mounts_list_empty = "";
            double nav_cross_line_thickness = 1.0;
            std::string not_included_in_damage_report = "plasteel_hull tungsten_hull isometal_hull";
            bool out_of_cone_distance = false;
            double padlock_view_lag = 1.5;
            double padlock_view_lag_fix_zone = 0.0872;
            std::string planet_near_message = "#ff0000Planetary Hazard Near#000000";
            bool print_damage_percent = true;
            bool print_faction = true;
            bool print_fg_name = true;
            bool print_fg_sub_id = false;
            bool print_request_docking = true;
            bool print_ship_type = true;
            bool projectile_means_missile = false;
            std::string radar_type = "WC";
            double radar_search_extra_radius = 1000.0;
            bool rotating_bracket_inner = true;
            double rotating_bracket_size = 0.58;
            double rotating_bracket_width = 0.1;
            double scale_relationship_color = 10.0;
            bool shield_vdu_fade = true;
            double shield_vdu_thresh0 = 0.25;
            double shield_vdu_thresh1 = 0.5;
            double shield_vdu_thresh2 = 0.75;
            bool show_negative_blips_as_positive = true;
            bool simple_manifest = false;
            std::string starship_near_message = "#ff0000Starship Near#000000";
            bool switch_to_target_mode_on_key = true;
            bool switch_back_from_comms = true;
            double tac_target_foci = 0.5;
            double tac_target_length = 0.1;
            double tac_target_thickness = 1.0;
            double text_background_alpha = 0.0625;
            bool top_view = false;
            bool untarget_beyond_cone = false;

        } hud;

    } graphics;

    struct {

        struct {
            std::string energy_source = "fuel";

        } afterburner;

        struct {
            std::string energy_source = "energy";

        } cloak;

        struct {
            double default_lock_cone = 0.8;
            double default_max_range = 20000.0;
            double default_tracking_cone = 0.93;

        } computer;

        struct {
            std::string energy_source = "energy";
            int non_combat_mode_multiplier = 1000;
            double minimum_drive = 0.15;

        } drive;

        struct {
            int factor = 1;

        } energy;

        struct {
            double afterburner_fuel_usage = 4.0;
            double deuterium_relative_efficiency_lithium = 1.0;
            double ecm_energy_cost = 0.05;
            double fmec_factor = 8e-09;
            double fuel_efficiency = 1.0;
            bool fuel_equals_warp = false;
            int factor = 600;
            double megajoules_factor = 100.0;
            double min_reactor_efficiency = 1e-05;
            double no_fuel_afterburn = 0.1;
            double no_fuel_thrust = 0.4;
            double normal_fuel_usage = 1.0;
            double reactor_idle_efficiency = 0.98;
            bool reactor_uses_fuel = false;
            bool variable_fuel_consumption = false;
            double vsd_mj_yield = 5.4;

        } fuel;

        struct {
            std::string energy_source = "ftl_energy";
            double factor = 0.1;

        } ftl_drive;

        struct {
            double factor = 1.0;

        } ftl_energy;

        struct {
            std::string energy_source = "ftl_energy";
            double factor = 1.0;

        } jump_drive;

        struct {
            std::string energy_source = "fuel";
            double factor = 1.0;

        } reactor;

        struct {
            std::string energy_source = "energy";
            double maintenance_factor = 0.1;
            double regeneration_factor = 0.1;

        } shield;

    } components;

    struct {
        int megajoules_multiplier = 100;
        int kilo = 1000;
        int kj_per_unit_damage = 5400;

    } constants;

    struct {
        std::string master_part_list = "master_part_list";
        bool using_templates = true;

    } data;

    struct {
        std::string default_mission = "main_menu.mission";
        std::string introduction = "Welcome to Vega Strike!\nUse #8080FFTab#000000 to afterburn (#8080FF+,-#000000 cruise control),\n#8080FFarrows#000000 to steer.\nThe #8080FFt#000000 key targets objects; #8080FFspace#000000 fires at them & #8080FFa#000000 activates the SPEC drive.\nTo go to another star system, buy a jump drive for about 10000 credits,\nfly to a wireframe jump-point and press #8080FFj#000000 to warp to a near star.\nTarget a base or planet;\nWhen you get close a green box will appear. Inside the box, #8080FFd#000000 will land.";
        std::string galaxy = "milky_way.xml";

    } game_start;

    struct {

    } advanced;

    struct {
        bool always_obedient = true;
        bool assist_friend_in_need = true;
        double ease_to_anger = -0.5;
        double ease_to_appease = 0.5;
        double friend_factor = 0.1;
        int hull_damage_anger = 10;
        double hull_percent_for_comm = 0.75;
        double kill_factor = 0.2;
        double lowest_negative_comm_choice = -1e-05;
        double lowest_positive_comm_choice = 0.0;
        double min_relationship = -20.0;
        double mood_swing_level = 0.2;
        double random_response_range = 0.8;
        int shield_damage_anger = 1;
        bool jump_without_energy = false;

        struct {
            double missile_probability = 0.01;
            double aggressivity = 15.0;

        } firing;

        struct {
            double escort_distance = 10.0;
            double turn_leader_distance = 5.0;
            double time_to_recommand_wing = 100.0;
            double min_time_to_switch_targets = 3.0;

        } targeting;

    } ai;

    struct {
        double afterburner_gain = 0.5;
        bool ai_high_quality_weapon = false;
        bool ai_sound = true;
        double audio_max_distance = 1000000.0;
        double audio_ref_distance = 4000.0;
        std::string automatic_docking_zone = "automatic_landing_zone.wav";
        std::string battle_playlist = "battle.m3u";
        double buzzing_distance = 5.0;
        bool buzzing_needs_afterburner = false;
        double buzzing_time = 5.0;
        std::string cache_songs = "../music/land.ogg";
        bool cross_fade_music = true;
        std::string dj_script = "modules/dj.py";
        bool doppler = false;
        double doppler_scale = 1.0;
        bool every_other_mount = false;
        double explosion_closeness = 0.8;
        double exterior_weapon_gain = 0.35;
        int frequency = 48000;
        bool high_quality_weapon = true;
        std::string loading_sound = "../music/loading.ogg";
        std::string loss_playlist = "loss.m3u";
        double loss_relationship = -0.1;
        int max_single_sounds = 8;
        int max_total_sounds = 20;
        double max_range_to_hear_weapon_fire = 100000.0;
        double min_weapon_sound_refire = 0.2;
        std::string mission_victory_song = "../music/victory.ogg";
        bool music = true;
        int music_layers = 1;
        double music_muting_fade_in = 0.5;
        double music_muting_fade_out = 0.2;
        double music_volume = 0.5;
        double music_volume_down_latency = 2.0;
        double music_volume_up_latency = 15.0;
        std::string news_song = "../music/news1.ogg";
        std::string panic_playlist = "panic.m3u";
        std::string peace_playlist = "peace.m3u";
        bool positional = true;
        bool shuffle_songs = true;

        struct {
            bool clear_history_on_list_change = true;
            int history_depth = 5;

        } shuffle_songs_section;
        bool sound = true;
        double sound_gain = 0.5;
        int thread_time = 1;
        double time_between_music = 180.0;
        std::string victory_playlist = "victory.m3u";
        double victory_relationship = 0.5;
        double volume = 100.0;
        double weapon_gain = 0.25;

    } audio;

    struct {
        std::string automatic_landing_zone = "als";
        std::string automatic_landing_zone1 = "als";
        std::string automatic_landing_zone2 = "als";
        std::string autopilot_available = "autopilot_available";
        std::string autopilot_disabled = "autopilot_disabled";
        std::string autopilot_enabled = "autopilot";
        std::string autopilot_unavailable = "autopilot_unavailable";
        std::string comm = "vdu_c";
        bool comm_preload = false;
        std::string compress_change = "compress_burst";
        int compress_interval = 3;
        std::string compress_loop = "compress_loop";
        int compress_max = 3;
        std::string compress_stop = "compress_end";
        std::string docking_complete = "docking_complete";
        std::string docking_denied = "request_denied";
        std::string docking_failed = "docking_failed";
        std::string docking_granted = "request_granted";
        std::string examine = "vdu_b";
        std::string jump_engaged = "jump";
        std::string manifest = "vdu_a";
        std::string missile_switch = "vdu_d";
        std::string objective = "vdu_c";
        std::string overload = "overload";
        std::string overload_stopped = "overload_stopped";
        std::string repair = "vdu_a";
        std::string scanning = "vdu_c";
        std::string shield = "vdu_d";
        std::string sounds_extension_1 = "\n";
        std::string sounds_extension_2 = "\n";
        std::string sounds_extension_3 = "\n";
        std::string sounds_extension_4 = "\n";
        std::string sounds_extension_5 = "\n";
        std::string sounds_extension_6 = "\n";
        std::string sounds_extension_7 = "\n";
        std::string sounds_extension_8 = "\n";
        std::string sounds_extension_9 = "\n";
        std::string target = "vdu_b";
        std::string target_reverse = "vdu_a";
        std::string undocking_complete = "undocking_complete";
        std::string undocking_failed = "undocking_failed";
        std::string vdu_static = "vdu_static";
        std::string view = "vdu_b";
        std::string weapon_switch = "vdu_d";

    } cockpit_audio;

    struct {
        double collision_hack_distance = 10000.0;
        bool collision_damage_to_ai = false;
        bool crash_dock_hangar = false;
        bool crash_dock_unit = false;
        double front_collision_hack_angle = 0.9999257675004788;
        double front_collision_hack_distance = 200000.0;
        bool cargo_deals_collide_damage = false;

    } collision_hacks;

    struct {
        int vsdebug = 0;
        bool verbose_debug = false;

    } logging;

    struct {
        double capship_size = 500.0;
        double close_enough_to_autotrack = 4.0;
        double collision_scale_factor = 1.0;
        double debris_time = 500.0;
        double default_shield_tightness = 0.0;
        double density_of_jump_point = 100000.0;
        double density_of_rock = 3.0;
        bool difficulty_based_enemy_damage = true;
        double difficulty_speed_exponent = 0.2;
        bool difficulty_based_shield_recharge = true;
        double distance_to_warp = 1000000000000.0;
        bool does_missile_bounce = false;
        bool engine_energy_takes_priority = true;
        double friendly_auto_radius = 0.0;
        double game_accel = 1.0;
        double game_speed = 1.0;
        double hostile_auto_radius = 1000.0;
        double indiscriminate_system_destruction = 0.25;
        double inelastic_scale = 0.8;
        double jump_mesh_radius_scale = 0.5;
        double max_damage = 0.999;
        int max_ecm = 4;
        double max_force_multiplier = 5.0;
        double max_lost_target_live_time = 30.0;
        double max_non_player_rotation_rate = 360.0;
        double max_player_rotation_rate = 24.0;
        double max_radar_cone_damage = 0.9;
        double max_radar_lock_cone_damage = 0.95;
        double max_radar_track_cone_damage = 0.98;
        bool max_shield_lowers_capacitance = false;
        double max_torque_multiplier = 0.67;
        double min_asteroid_distance = -100.0;
        double min_damage = 0.001;
        double min_shield_speeding_discharge = 0.1;
        double minimum_mass = 1e-06;
        double minimum_time = 0.1;
        double near_autotrack_cone = 0.9;
        double nebula_shield_recharge = 0.5;
        double neutral_auto_radius = 0.0;
        bool no_damage_to_docked_ships = true;
        bool no_spec_jump = true;
        bool no_unit_collisions = false;
        double percent_missile_match_target_velocity = 1.0;
        double planet_dock_min_port_size = 300.0;
        double planet_dock_port_size = 1.2;
        bool planets_always_neutral = true;
        bool planets_can_have_subunits = false;
        bool separate_system_flakiness_component = false;
        double shield_energy_capacitance = 0.2;
        double shield_maintenance_charge = 0.25;
        bool shields_in_spec = false;
        double speeding_discharge = 0.25;
        bool steady_itts = false;
        bool system_damage_on_armor = false;
        double target_distance_to_warp_bonus = 1.33;
        double thruster_hit_chance = 0.25;
        bool use_max_shield_energy_usage = false;
        double velocity_max = 10000.0;

        struct {
            double auto_eject_percent = 0.5;
            double eject_cargo_percent = 1.0;
            int max_dumped_cargo = 15;
            double hull_damage_to_eject = 100.0;
            bool player_auto_eject = true;

        } ejection;

    } physics;

    struct {
        double default_aggressivity = 2.01;

    } unit;

    struct {
        double bleed_factor = 2.0;
        double computer_warp_ramp_up_time = 10.0;
        double in_system_jump_cost = 0.1;
        double max_effective_velocity = 2960881320.33;
        double player_warp_energy_multiplier = 0.12;
        bool use_warp_energy_for_cloak = true;
        double warp_energy_multiplier = 0.12;
        double warp_memory_effect = 0.9;
        double warp_multiplier_max = 300000000.0;
        double warp_multiplier_min = 9.86968440109;
        double warp_ramp_down_time = 0.5;
        double warp_ramp_up_time = 5.0;
        double warp_stretch_cutoff = 500000.0;
        double warp_stretch_decel_cutoff = 500000.0;

    } warp;

    struct {
        bool can_fire_in_cloak = false;
        bool can_fire_in_spec = false;

    } weapons;

    };

    extern std::shared_ptr<Config> config;
}

#endif //VEGA_STRIKE_ENGINE_CONFIG_CONFIGURATION_H
