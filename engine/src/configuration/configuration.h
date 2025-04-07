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
        double docking_fee = 0.0;
        std::string empty_mission = "internal.mission";
        bool force_anonymous_mission_names = true;
        double fuel_docking_fee = 0.0;
        int garbage_collect_frequency = 20;
        double jump_key_delay = 0.125;
        bool load_last_savegame = false;
        std::string new_game_save_name = "New_Game";
        int num_old_systems = 6;
        double percentage_speed_change_to_fault_search = 300.0;
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
        int resolution_x = 1920;
        int resolution_y = 1080;
        int screen = 0;
        double afterburner_color_red = 1.0;
        double afterburner_color_green = 0.0;
        double afterburner_color_blue = 0.0;
        bool always_make_smooth_cam = false;
        double anim_far_percent = 0.8;
        std::string armor_flash_animation = "armorflash.ani";
        double aspect = 1.33;
        double atmosphere_emissive = 1.0;
        double atmosphere_diffuse = 1.0;
        std::string automatic_landing_zone_warning = "comm_docking.ani";
        std::string automatic_landing_zone_warning1 = "comm_docking.ani";
        std::string automatic_landing_zone_warning2 = "comm_docking.ani";
        std::string automatic_landing_zone_warning_text = "Now Entering an \"Automatic Landing Zone\".";
        bool background = true;
        double base_faction_color_darkness = 0.75;
        double beam_fadeout_length = 0.8;
        double beam_fadeout_hit_length = 0.95;
        bool bitmap_alphamap = true;
        bool blend_guns = true;
        double bolt_offset = -0.2;
        double bolt_pixel_size = 0.5;
        double camera_pan_speed = 0.0001;
        bool can_target_asteroid = true;
        bool can_target_cargo = false;
        bool can_target_sun = true;
        bool cargo_rotates_at_same_speed = true;
        double cargo_rotation_speed = 60.0;
        double circle_accuracy = 0.1;
        int city_light_filter = 4;
        double city_light_strength = 10.0;
        bool clear_on_startup = true;
        bool cockpit = true;
        int cockpit_z_partitions = 1;
        int color_depth = 32;
        std::string comm_static = "static.ani";
        double day_city_light_strength = 0.0;
        std::string death_menu_script = "";
        double death_scene_time = 4.0;
        std::string default_boot_message = "";
        double default_engine_activation = 0.00048828125;
        double detail_texture_scale = 1.0;
        bool detail_texture_trilinear = true;
        bool disabled_cockpit_allowed = true;
        bool displaylists = false;
        bool draw_docking_boxes = true;
        bool draw_heading_marker = false;
        bool draw_jump_target_nav_symbol = true;
        bool draw_near_stars_in_front_of_planets = false;
        bool draw_rendered_crosshairs = true;
        bool draw_star_body = true;
        bool draw_star_glow = true;
        bool draw_star_glow_halo = false;
        bool draw_target_nav_symbol = true;
        bool draw_weapons = false;
        double eject_rotation_speed = 0.0;
        double engine_color_red = 1.0;
        double engine_color_green = 1.0;
        double engine_color_blue = 1.0;
        double engine_length_scale = 1.25;
        double engine_radii_scale = 0.4;
        std::string explosion_animation = "explosion_orange.ani";
        bool explosion_face_player = true;
        double explosion_force = 0.5;
        double explosion_torque = 0.001;
        bool ext_clamp_to_border = true;
        bool ext_clamp_to_edge = true;
        bool faction_dependent_textures = true;
        double far_stars_sprite_size = 2.0;
        std::string far_stars_sprite_texture = "";
        int fog_detail = 2;
        std::string font = "helvetica12";
        double font_point = 16.0;
        bool force_lighting = true;
        double fov = 37.5;
        bool framerate_changes_shader = false;
        bool full_screen = true;
        std::string gauge_static = "static.ani";
        double generic_cargo_rotation_speed = 1.0;
        bool gl_accelerated_visual = true;
        bool glow_ambient_star_light = false;
        bool glow_diffuse_star_light = false;
        bool glut_stencil = true;
        bool halo_far_draw = false;
        double halo_sparkle_rate = 20.0;
        double halo_sparkle_scale = 6.0;
        double halo_sparkle_speed = 0.5;
        bool halos_by_velocity = false;
        bool high_quality_font = true;
        bool high_quality_font_computer = true;
        std::string hull_flash_animation = "armorflash.ani";
        double in_system_jump_ani_second_ahead = 4.0;
        double in_system_jump_ani_second_ahead_end = 0.03;
        std::string in_system_jump_animation = "warp.ani";
        double in_system_jump_animation_growth = 0.99;
        double in_system_jump_animation_size = 4.0;
        std::string initial_boot_message = "Loading...";
        double initial_zoom_factor = 2.25;
        double jump_animation_shrink = 0.95;
        std::string jump_gate = "blackclear.ani";
        double jump_gate_size = 1.75;
        double light_cutoff = 0.06;
        double light_optimal_intensity = 0.06;
        double light_saturation = 0.95;
        bool lock_significant_target_box = true;
        bool lock_vertex_arrays = false;
        std::string mac_shader_name = "mac";
        bool main_menu = false;
        int max_cubemap_size = 1024;
        int max_movie_dimension = 1024;
        int max_texture_dimension = 65536;
        double mesh_animation_max_frames_skipped = 3.0;
        double mesh_far_percent = 0.8;
        int mipmap_detail = 3;
        double missile_explosion_radius_mult = 1.0;
        bool missile_sparkle = false;
        double model_detail = 1.0;
        double near_stars_sprite_size = 2.0;
        std::string near_stars_sprite_texture = "";
        int num_far_stars = 2000;
        int num_lights = 4;
        int num_near_stars = 1000;
        int num_times_to_draw_shine = 2;
        bool offset_sprites_by_pos = true;
        bool only_stretch_in_warp = true;
        bool open_picker_categories = false;
        bool optimize_vertex_arrays = false;
        double optimize_vertex_condition = 4.0;
        double percent_afterburner_color_change = 0.5;
        double percent_halo_fade_in = 0.5;
        double percent_shockwave = 0.5;
        bool per_pixel_lighting = true;
        int planet_detail_level = 24;
        bool pot_video_textures = false;
        double precull_dist = 500000000.0;
        bool rect_textures = false;
        double reduced_vdus_width = 0.0;
        double reduced_vdus_height = 0.0;
        bool reflection = true;
        double reflectivity = 0.8;
        std::string rgb_pixel_format = "undefined";
        bool s3tc = true;
        bool separate_specular_color = false;
        std::string shader_name = "default";
        bool shared_vertex_arrays = true;
        int shield_detail_level = 24;
        std::string shield_flash_animation = "";
        std::string shield_technique = "";
        std::string shield_texture = "shield.bmp";
        std::string shockwave_animation = "explosion_wave.ani";
        double shockwave_growth = 1.05;
        bool show_stardate = true;
        bool smooth_lines = false;
        bool smooth_points = true;
        bool smooth_shade = true;
        double sparkle_absolute_speed = 0.02;
        double sparkle_engine_size_relative_to_ship = 0.1875;
        double sparkle_rate = 5.0;
        std::string splash_audio = "";
        std::string splash_screen = "vega_splash.ani";
        bool split_dead_subunits = true;
        std::string star_allowable_sectors = "Vega Sol";
        bool star_blend = true;
        double star_body_radius = 0.33;
        double star_glow_radius = 1.33;
        double star_overlap = 1.0;
        std::string star_shine = "shine.ani flare1.ani flare2.ani flare3.ani flare4.ani flare5.ani flare6.ani";
        double star_spreading = 30000.0;
        bool stars_dont_move = true;
        std::string startup_cockpit_view = "front";
        double stretch_bolts = 0.5;
        std::string technique_set = "6_ps4.0";
        double text_delay = 0.05;
        double text_speed = 0.025;
        std::string texture = "supernova.bmp";
        int texture_compression = 0;
        std::string unprintable_faction_extension = "citizen";
        std::string unprintable_factions = "";
        bool update_nav_after_jump = false;
        bool use_animations = true;
        bool use_detail_texture = true;
        bool use_faction_gui_background_color = true;
        bool use_logos = true;
        bool use_planet_atmosphere = true;
        bool use_planet_fog = true;
        bool use_star_coords = true;
        bool use_textures = true;
        bool use_planet_textures = true;
        bool use_ship_textures = true;
        bool use_videos = true;
        bool use_vs_sprites = true;
        bool use_wireframe = false;
        bool vbo = true;
        double warp_stretch_cutoff = 50000.0;
        double warp_stretch_max = 2.0;
        double warp_stretch_max_region0_speed = 240000.0;
        double warp_stretch_max_speed = 300000000.0;
        double warp_stretch_region0_max = 1.0;
        bool warp_trail = true;
        double warp_trail_time = 20.0;
        double weapon_gamma = 1.35;
        double weapon_xyscale = 1.0;
        double weapon_zscale = 1.0;
        std::string wormhole_unit = "wormhole";
        int z_pixel_format = 24;
        double zfar = 100000.0;
        double znear = 1.0;

        struct {
            double alpha_test_cutoff = 0.0;
            bool draw_location_borders = false;
            bool draw_location_text = true;
            bool enable_debug_markers = false;
            bool enable_location_markers = true;
            bool include_base_name_on_dock = true;
            double location_marker_distance = 0.5;
            bool location_marker_draw_always = false;
            std::string location_marker_sprite = "";
            double location_marker_text_color_r = 1.0;
            double location_marker_text_color_g = 1.0;
            double location_marker_text_color_b = 1.0;
            double location_marker_text_offset_x = 0.0;
            double location_marker_text_offset_y = 0.0;
            std::string font = "helvetica12";
            double fov = 60.0;
            int max_width = 1440;
            int max_height = 1080;
            bool print_cargo_volume = true;
            double text_background_alpha = 0.0625;
            bool blur_bases = true;

        } bases;

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

        struct {
            bool modern_mouse_cursor = true;
            std::string mouse_cursor_sprite = "mouse.spr";

        } nav;

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
        std::string ai_directory = "ai";
        std::string animations = "animations";
        std::string capship_roles = "ESCORTCAP CAPITAL CARRIER BASE TROOP";
        std::string cockpits = "cockpits";
        std::string data_dir = "";
        std::string hqtextures = "hqtextures";
        std::string master_part_list = "master_part_list";
        std::string mod_unit_csv = "";
        std::string movies = "movies";
        std::string python_bases = "bases";
        std::string python_modules = "modules";
        std::string sectors = "sectors";
        std::string serialized_xml = "serialized_xml";
        std::string shared_meshes = "meshes";
        std::string shared_sounds = "sounds";
        std::string shared_textures = "textures";
        std::string shared_units = "units";
        std::string sprites = "sprites";
        std::string techniques_base_path = "techniques";
        std::string universe_path = "universe";
        bool using_templates = true;
        std::string volume_format = "pk3";

    } data;

    struct {
        std::string campaigns = "privateer_campaign vegastrike_campaign";
        std::string default_mission = "main_menu.mission";
        std::string introduction = "Welcome to Vega Strike!\nUse #8080FFTab#000000 to afterburn (#8080FF+,-#000000 cruise control),\n#8080FFarrows#000000 to steer.\nThe #8080FFt#000000 key targets objects; #8080FFspace#000000 fires at them & #8080FFa#000000 activates the SPEC drive.\nTo go to another star system, buy a jump drive for about 10000 credits,\nfly to a wireframe jump-point and press #8080FFj#000000 to warp to a near star.\nTarget a base or planet;\nWhen you get close a green box will appear. Inside the box, #8080FFd#000000 will land.";
        std::string galaxy = "milky_way.xml";

    } game_start;

    struct {

    } advanced;

    struct {
        bool afterburn_to_no_enemies = true;
        bool allow_any_speed_reference = false;
        bool allow_civil_war = false;
        bool allow_nonplayer_faction_change = false;
        bool always_fire_autotrackers = false;
        bool always_have_jumpdrive_cheat = false;
        bool always_obedient = true;
        double anger_affects_response = 1.0;
        bool assist_friend_in_need = true;
        double attacker_switch_time = 15.0;
        bool auto_dock = false;
        bool capped_faction_rating = true;
        std::string choose_destination_script = "";
        double comm_initiate_time = 300.0;
        double comm_to_player_percent = 0.0;
        double comm_to_target_percent = 0.25;
        double contraband_initiate_time = 3000.0;
        int contraband_madness = 5;
        double contraband_to_player_percent = 0.0;
        double contraband_to_target_percent = 0.01;
        double contraband_update_time = 1.0;
        int debug_level = 0;
        double default_rank = 0.01;
        bool dock_on_load = true;
        bool dock_to_area = false;
        std::string docked_to_script = "";
        double ease_to_anger = -0.5;
        double ease_to_appease = 0.5;
        bool eject_attacks = false;
        double evasion_angle = 45.0;
        double faction_contraband_relation_adjust = -0.025;
        double fg_nav_select_time = 120.0;
        double force_jump_after_time = 120.0;
        double friend_factor = 0.1;
        bool hostile_lurk = true;
        double how_far_to_stop_navigating = 100.0;
        int hull_damage_anger = 10;
        double hull_percent_for_comm = 0.75;
        bool jump_cheat = true;
        bool jump_without_energy = false;
        double kill_factor = 0.2;
        double lurk_time = 600.0;
        double lowest_negative_comm_choice = -1e-05;
        double lowest_positive_comm_choice = 0.0;
        double max_faction_contraband_relation = -0.05;
        int max_player_attackers = 0;
        double min_relationship = -20.0;
        double min_time_to_auto = 25.0;
        double missile_gun_delay = 4.0;
        double mood_affects_response = 0.0;
        double mood_swing_level = 0.2;
        int num_contraband_scans_per_search = 10;
        int num_pirates_per_asteroid_field = 12;
        bool only_upgrade_speed_reference = false;
        double percentage_speed_change_to_stop_search = 1.0;
        double pirate_bonus_for_empty_hold = 0.75;
        double pirate_nav_select_time = 400.0;
        double random_response_range = 0.8;
        double random_spacing_factor = 4.0;
        double reaction_time = 0.2;
        bool resistance_to_side_movement = false;
        double resistance_to_side_force_percent = 1.0;
        double resistance_to_side_movement_percent = 0.01;
        double safety_spacing = 2500.0;
        int shield_damage_anger = 1;
        double slow_diplomacy_for_enemies = 0.25;
        std::string start_docked_to = "Atlantis";
        double static_relationship_affects_response = 1.0;
        bool switch_nonowned_units = true;
        double unknown_relation_enemy = -0.05;
        double unknown_relation_hit_cost = 0.01;
        bool warp_to_enemies = true;
        bool warp_to_no_enemies = true;

        struct {
            double aggressivity = 15.0;
            double in_weapon_range = 1.2;
            double missile_probability = 0.04;
            double reaction_time = 0.2;
            double turret_dot_cutoff = 0.4;
            double turret_missile_probability = 0.01;

            struct {
                int minagg = 10;
                int maxagg = 18;

            } maximum_firing_angle;

        } firing;

        struct {
            bool assign_point_def = true;
            double escort_distance = 10.0;
            double mass_inertial_priority_cutoff = 5000.0;
            double mass_inertial_priority_scale = 1e-07;
            int max_number_of_pollers_per_frame = 49;
            int min_number_of_pollers_per_frame = 5;
            int min_rechoose_interval = 128;
            double min_time_to_switch_targets = 3.0;
            double min_null_time_to_switch_targets = 5.0;
            double mountless_gun_range = 300000000.0;
            double obedience = 0.99;
            double search_extra_radius = 1000.0;
            int search_max_candidates = 64;
            int search_max_role_priority = 16;
            double threat_weight = 0.5;
            double time_to_recommand_wing = 100.0;
            double time_until_switch = 20.0;
            double turn_leader_distance = 5.0;

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
        bool sound = true;
        double sound_gain = 0.5;
        int thread_time = 1;
        double time_between_music = 180.0;
        std::string victory_playlist = "victory.m3u";
        double victory_relationship = 0.5;
        double volume = 100.0;
        double weapon_gain = 0.25;

        struct {
            bool clear_history_on_list_change = true;
            int history_depth = 5;

        } shuffle_songs_section;

        struct {
            std::string afterburner = "engine_5.wav";
            std::string armor = "armorhit2m.wav";
            std::string cloak = "cloak.wav";
            std::string explode = "explosion.wav";
            std::string hull = "armorhit.wav";
            std::string jump_arrive = "sfx43.wav";
            std::string jump_leave = "sfx43.wav";
            std::string locked = "locked.wav";
            std::string locking = "locking.wav";
            std::string locking_torp = "locking.wav";
            bool locking_torp_trumps_music = false;
            bool locking_trumps_music = false;
            std::string player_afterburner = "sfx10.wav";
            std::string player_armor_hit = "armorhit2.wav";
            std::string player_hull_hit = "armorhit.wav";
            std::string player_shield_hit = "shieldhit4short.wav";
            std::string player_tractor_cargo = "tractor_onboard.wav";
            std::string player_tractor_cargo_fromturret = "tractor_onboard.wav";
            std::string shield = "shieldhit3short.wav";

        } unit_audio;

    } audio;

    struct {
        double junk_starship_mass = 50.0;
        double junk_starship_price = 100000.0;
        double junk_starship_volume = 1500.0;
        double max_price_quant_adj = 5.0;
        double min_cargo_price = 0.01;
        double min_price_quant_adj = 1.0;
        bool news_from_cargo_list = false;
        double price_quant_adj_power = 1.0;
        double price_recenter_factor = 0.75;
        double space_junk_price = 10.0;

    } cargo;

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
        double repair_price = 5000.0;
        double sellback_shipping_price = 6000.0;
        double ship_sellback_price = 0.5;
        double shipping_price_base = 0.0;
        double shipping_price_insys = 1000.0;
        double shipping_price_perjump = 25000.0;

    } economics;

    struct {

        struct {
            bool initial = false;
            bool enable = true;

        } inertial;

    } flight;

    struct {
        double ambient_light_factor = 0.0;
        double atmosphere_probability = 0.5;
        double compactness_scale = 27.0;
        std::string default_atmosphere_texture = "sol/earthcloudmaptrans.png";
        std::string default_ring_texture = "planets/ring.pngwrapx36wrapy2";
        double double_ring_probability = 0.025;
        double inner_ring_radius = 1.5;
        double jump_compactness_scale = 54.0;
        int mean_natural_phenomena = 1;
        int mean_star_bases = 2;
        double moon_relative_to_moon = 0.8;
        double moon_relative_to_planet = 0.4;
        double outer_ring_radius = 2.5;
        std::string police_faction = "homeland-security";
        std::string prison_system = "Sol/Nu_Pheonix";
        bool push_values_to_mean = false;
        double ring_probability = 0.1;
        double rocky_relative_to_primary = 0.009;
        double second_ring_difference = 0.4;
        double star_radius_scale = 50000.0;

    } galaxy;

    struct {
        int debug_level = 0;
        bool start_game = true;
        bool trace = false;

    } interpreter;

    struct {
        double deadband = 0.05;
        bool debug_digital_hatswitch = false;
        int double_mouse_position = 1280;
        double double_mouse_factor = 2.0;
        int ff_device = 0;
        bool force_feedback = false;
        bool force_use_of_joystick = false;
        std::string initial_mode = "normal";
        double joystick_exponent = 1.0;
        double mouse_blur = 0.025;
        std::string mouse_crosshair = "crosshairs.spr";
        double mouse_deadband = 0.025;
        double mouse_exponent = 3.0;
        double mouse_sensitivity = 50.0;
        int polling_rate = 0;
        bool reverse_mouse_spr = true;
        bool warp_mouse = false;
        int warp_mouse_zone = 100;

    } joystick;

    struct {
        bool enable_unicode = true;

    } keyboard;

    struct {
        int vsdebug = 0;
        bool verbose_debug = false;

    } logging;

    struct {
        std::string account_server_url = "http://vegastrike.sourceforge.net/cgi-bin/accountserver.py";
        bool chat_only_in_network = false;
        bool force_client_connect = false;
        std::string server_ip = "localhost";
        std::string server_port = "6777";
        bool use_account_server = true;

    } network;

    struct {
        bool ai_lock_cheat = true;
        bool ai_pilot_when_in_turret = false;
        bool allow_mission_abort = true;
        bool allow_special_and_normal_gun_combo = true;
        double asteroid_difficulty = 0.1;
        bool asteroid_weapon_collision = false;
        double auto_pilot_no_enemies_distance_multiplier = 4.0;
        double auto_pilot_planet_radius_percent = 0.2;
        double auto_pilot_spec_lining_up_angle = 3.0;
        double auto_pilot_termination_distance = 2500.0;
        double auto_pilot_termination_distance_no_enemies = 6000.0;
        std::string auto_landing_exclude_list = "";
        std::string auto_landing_exclude_warning_list = "";
        double auto_time_in_seconds = 10.0;
        bool auto_undock = true;
        double autogen_compactness = 1.0;
        double capship_size = 500.0;
        bool car_control = false;
        double cargo_live_time = 600.0;
        bool cargo_wingmen_only_with_dockport = false;
        bool change_docking_orientation = false;
        double close_enough_to_autotrack = 4.0;
        double collision_scale_factor = 1.0;
        double computer_warp_ramp_up_time = 10.0;
        double contraband_assist_range = 50000.0;
        double debris_mass = 1e-05;
        double debris_time = 500.0;
        double default_interdiction = 0.01;
        double default_shield_tightness = 0.0;
        double density_of_jump_point = 100000.0;
        double density_of_rock = 3.0;
        bool difficulty_based_enemy_damage = true;
        double difficulty_speed_exponent = 0.2;
        bool difficulty_based_shield_recharge = true;
        bool direct_interstellar_journey = true;
        bool display_in_meters = true;
        double distance_to_warp = 1000000000000.0;
        bool dock_with_clear_planets = true;
        bool dock_within_base_shield = false;
        double docking_time = 20.0;
        bool dockport_allows_upgrade_storage = false;
        bool does_missile_bounce = false;
        std::string drone = "beholder";
        int eject_cargo_on_blowup = 0;
        double eject_cargo_speed = 1.0;
        double eject_distance = 20.0;
        double eject_live_time = 0.0;
        bool engine_energy_takes_priority = true;
        double explosion_damage_center = 1.0;
        double explosion_damage_edge = 0.125;
        double extra_space_drag_for_cargo = 0.005;
        bool face_target_on_auto = false;
        bool fire_missing_autotrackers = true;
        double friendly_auto_radius = 0.0;
        double game_accel = 1.0;
        double game_speed = 1.0;
        bool game_speed_affects_autogen_systems = false;
        bool game_speed_lying = false;
        double gun_speed = 1.0;
        bool gun_speed_adjusted_game_speed = true;
        bool hardware_cursor = false;
        double hostile_auto_radius = 1000.0;
        double hud_repair_unit = 0.25;
        bool in_system_jump_or_timeless_auto_pilot = false;
        double inactive_system_time = 0.3;
        std::string indestructible_cargo_items = "";
        double indiscriminate_system_destruction = 0.25;
        double inelastic_scale = 0.8;
        bool jump_disables_shields = true;
        double jump_mesh_radius_scale = 0.5;
        double jump_radius_scale = 2.0;
        bool jump_weapon_collision = false;
        int max_collide_trees = 16384;
        double max_damage = 0.999;
        int max_ecm = 4;
        double max_force_multiplier = 5.0;
        double max_lost_target_live_time = 30.0;
        int max_missions = 4;
        double max_non_player_rotation_rate = 360.0;
        double max_player_rotation_rate = 24.0;
        double max_radar_cone_damage = 0.9;
        double max_radar_lock_cone_damage = 0.95;
        double max_radar_track_cone_damage = 0.98;
        bool max_shield_lowers_capacitance = false;
        double max_torque_multiplier = 0.67;
        double max_warp_effect_size = 3000.0;
        double min_asteroid_distance = -100.0;
        double min_damage = 0.001;
        double min_shield_speeding_discharge = 0.1;
        double min_warp_effect_size = 100.0;
        double minimum_mass = 1e-06;
        double minimum_time = 0.1;
        double mount_repair_unit = 0.25;
        bool must_lock_to_autotrack = true;
        bool must_repair_to_sell = true;
        double near_autotrack_cone = 0.9;
        double nebula_shield_recharge = 0.5;
        double neutral_auto_radius = 0.0;
        bool no_damage_to_docked_ships = true;
        bool no_spec_jump = true;
        bool no_unit_collisions = false;
        int num_running_systems = 4;
        int num_times_to_simulate_new_star_system = 20;
        bool only_show_best_downgrade = true;
        double percent_missile_match_target_velocity = 1.0;
        bool persistent_on_load = true;
        double planet_dock_min_port_size = 300.0;
        double planet_dock_port_size = 1.2;
        bool planets_always_neutral = true;
        bool planets_can_have_subunits = false;
        double player_godliness = 0.0;
        std::string player_tractorability_mask = "p";
        double refire_difficulty_scaling = 3.0;
        double repair_droid_check_time = 5.0;
        double repair_droid_time = 120.0;
        double respawn_unit_size = 400.0;
        double runtime_compactness = 1.0;
        bool separate_system_flakiness_component = false;
        double shield_energy_capacitance = 0.2;
        double shield_maintenance_charge = 0.25;
        bool shields_in_spec = false;
        bool simulate_while_docked = false;
        double speeding_discharge = 0.25;
        double star_system_scale = 1.0;
        bool starships_as_cargo = true;
        bool steady_itts = false;
        bool system_damage_on_armor = false;
        double target_distance_to_warp_bonus = 1.33;
        double thruster_hit_chance = 0.25;
        double un_docking_time = 10.0;
        double unit_space_drag = 0.0;
        bool use_cargo_mass = true;
        bool use_hidden_cargo_space = true;
        bool use_max_shield_energy_usage = false;
        double velocity_max = 10000.0;
        double warp_behind_angle = 150.0;
        double warp_cruise_mult = 15000.0;
        double warp_curve_degree = 1.3;
        bool warp_is_interstellar = false;
        double warp_ramp_down_time = 0.5;
        double warp_region0 = 1.0;
        double warp_region1 = 5000000.0;
        double weapon_damage_efficiency = 1.0;
        double year_scale = 16.0;

        struct {
            double auto_eject_percent = 0.5;
            double eject_cargo_percent = 1.0;
            int max_dumped_cargo = 15;
            double hull_damage_to_eject = 100.0;
            bool player_auto_eject = true;

        } ejection;

        struct {
            int asteroid_high = 2;
            int asteroid_low = 32;
            int asteroid_parent = 1;
            int dockable = 1;
            bool force_top_priority = false;
            int high = 2;
            int inert = 64;
            int low = 32;
            int medium = 8;
            int medium_high = 4;
            std::string min_asteroid_distance = "none";
            int missile = 1;
            int no_enemies = 64;
            int not_visible_combat_high = 10;
            int not_visible_combat_low = 40;
            int not_visible_combat_medium = 20;
            int player = 1;
            double player_threat_distance_factor = 2.0;
            double threat_distance_factor = 1.0;
            int system_installation = 3;

            struct {
                double max_distance_factor = 4.0;
                double min_distance_factor = 0.25;
                double target_fps = 30.0;

            } dynamic_throttle;

        } priorities;

        struct {
            int max_relative_speed = 150;
            double percent_to_tractor = 0.1;
            bool scoop = false;
            double scoop_alpha_multiplier = 2.5;
            double scoop_angle = 0.5;
            double scoop_fov = 0.5;
            int scoop_long_slices = 10;
            int scoop_rad_slices = 10;

            struct {
                double distance = 0.0;
                double distance_own_rsize = 1.5;
                double distance_tgt_rsize = 1.1;
                bool force_push = true;
                bool force_in = true;

            } cargo;

            struct {
                double distance = 0.0;
                double distance_own_rsize = 1.5;
                double distance_tgt_rsize = 1.1;
                bool force_push = true;
                bool force_in = true;

            } disabled;

            struct {
                double distance = 0.0;
                double distance_own_rsize = 2.2;
                double distance_tgt_rsize = 2.2;
                bool force_push = true;
                bool force_in = true;

            } faction;

            struct {
                double distance = 0.0;
                double distance_own_rsize = 1.5;
                double distance_tgt_rsize = 1.1;
                bool force_push = true;
                bool force_in = true;

            } upgrade;

            struct {
                double distance = 0.0;
                double distance_own_rsize = 1.1;
                double distance_tgt_rsize = 1.1;
                bool force_push = false;
                bool force_in = false;

            } others;

        } tractor;

    } physics;

    struct {
        std::string callsign = "";
        std::string password = "";

    } player;

    struct {
        bool auto_hide = true;

    } splash;

    struct {
        double mass = 100.0;
        double radius = 10000.0;
        double xscale = 1.0;
        double yscale = 1.0;
        double zscale = 1.0;

    } terrain;

    struct {
        double default_aggressivity = 2.01;

    } unit;

    struct {
        double bleed_factor = 2.0;
        double computer_warp_ramp_up_time = 10.0;
        double in_system_jump_cost = 0.1;
        double max_effective_velocity = 29202000000.0;
        double player_warp_energy_multiplier = 0.12;
        bool use_warp_energy_for_cloak = true;
        double warp_energy_multiplier = 0.12;
        double warp_memory_effect = 0.9;
        double warp_multiplier_max = 300000000.0;
        double warp_multiplier_min = 1.5;
        double warp_ramp_down_time = 0.5;
        double warp_ramp_up_time = 5.0;
        double warp_stretch_cutoff = 500000.0;
        double warp_stretch_decel_cutoff = 500000.0;

        struct {
            double reference_kps = 0.0;
            double smoothing = 0.4;

            struct {
                bool asymptotic = true;
                double exp = 1.5;
                double high_ref = 200000.0;
                double low_ref = 10000.0;
                double speed = 10.0;

                struct {
                    double back = 0.0;
                    double front = 0.0;
                    double perpendicular = 0.0;

                } offset;

                struct {
                    double back = 0.0;
                    double front = 0.0;
                    double perpendicular = 0.0;

                } multiplier;

            } shake;

            struct {
                bool asymptotic = true;
                double exp = 0.5;
                double high_ref = 100000.0;
                double low_ref = 1.0;

                struct {
                    double back = 0.0;
                    double front = 0.0;
                    double perpendicular = 0.0;

                } offset;

                struct {
                    double back = 1.5;
                    double front = 0.85;
                    double perpendicular = 1.25;

                } multiplier;

            } stable;

        } fov_link;

    } warp;

    struct {
        bool can_fire_in_cloak = false;
        bool can_fire_in_spec = false;

    } weapons;

    };

    extern std::shared_ptr<Config> config;
}

#endif //VEGA_STRIKE_ENGINE_CONFIG_CONFIGURATION_H
