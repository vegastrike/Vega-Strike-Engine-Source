/*
 * configuration.cpp
 *
 * Copyright (C) 2021-2025 Daniel Horn, Roy Falk, ministerofinformation,
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


#include <iostream>
#include <exception>
#include <boost/json.hpp>
#include <boost/json/object.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "configuration.h"

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <math.h>
#include <vs_logging.h>
#include <boost/format/format_fwd.hpp>
#include <boost/program_options/errors.hpp>

vega_config::Config::Config(const std::string& json_text) {
    load_config(json_text);
}

vega_config::Config::Config(const boost::filesystem::path& config_file_path)
{
    load_config(config_file_path);
}

void vega_config::Config::load_config(const boost::filesystem::path& config_file_path) {
    try {
        std::ifstream config_file(config_file_path.string());
        std::string str;
        std::string file_contents;
        while (std::getline(config_file, str)) {
            file_contents += str;
            file_contents.push_back('\n');
        }
        load_config(file_contents);
    } catch (const std::exception& e) {
        VS_LOG(error, (boost::format("Error in %1%: '%2%'") % __FUNCTION__ % e.what()));
    }
}

void vega_config::Config::load_config(const std::string& json_text) {
    try {
        if (json_text.empty()) {
            return;
        }
        boost::json::value json_value = boost::json::parse(json_text);
        if (json_value.is_null()) {
            return;
        }
        const boost::json::object & root_object = json_value.get_object();



        const boost::json::value * general_value_ptr = root_object.if_contains("general");
        if (general_value_ptr != nullptr) {
            boost::json::object general_object = general_value_ptr->get_object();
            const boost::json::value * audio_atom_value_ptr = general_object.if_contains("audio_atom");
            if (audio_atom_value_ptr != nullptr) {
                general.audio_atom = boost::json::value_to<double>(*audio_atom_value_ptr);
            }

            const boost::json::value * command_interpreter_value_ptr = general_object.if_contains("command_interpreter");
            if (command_interpreter_value_ptr != nullptr) {
                general.command_interpreter = boost::json::value_to<bool>(*command_interpreter_value_ptr);
            }

            const boost::json::value * custom_python_value_ptr = general_object.if_contains("custom_python");
            if (custom_python_value_ptr != nullptr) {
                general.custom_python = boost::json::value_to<std::string>(*custom_python_value_ptr);
            }

            const boost::json::value * debug_config_value_ptr = general_object.if_contains("debug_config");
            if (debug_config_value_ptr != nullptr) {
                general.debug_config = boost::json::value_to<bool>(*debug_config_value_ptr);
            }

            const boost::json::value * debug_fs_value_ptr = general_object.if_contains("debug_fs");
            if (debug_fs_value_ptr != nullptr) {
                general.debug_fs = boost::json::value_to<int>(*debug_fs_value_ptr);
            }

            const boost::json::value * delete_old_systems_value_ptr = general_object.if_contains("delete_old_systems");
            if (delete_old_systems_value_ptr != nullptr) {
                general.delete_old_systems = boost::json::value_to<bool>(*delete_old_systems_value_ptr);
            }

            const boost::json::value * docking_fee_value_ptr = general_object.if_contains("docking_fee");
            if (docking_fee_value_ptr != nullptr) {
                general.docking_fee = boost::json::value_to<double>(*docking_fee_value_ptr);
            }

            const boost::json::value * empty_mission_value_ptr = general_object.if_contains("empty_mission");
            if (empty_mission_value_ptr != nullptr) {
                general.empty_mission = boost::json::value_to<std::string>(*empty_mission_value_ptr);
            }

            const boost::json::value * force_anonymous_mission_names_value_ptr = general_object.if_contains("force_anonymous_mission_names");
            if (force_anonymous_mission_names_value_ptr != nullptr) {
                general.force_anonymous_mission_names = boost::json::value_to<bool>(*force_anonymous_mission_names_value_ptr);
            }

            const boost::json::value * fuel_docking_fee_value_ptr = general_object.if_contains("fuel_docking_fee");
            if (fuel_docking_fee_value_ptr != nullptr) {
                general.fuel_docking_fee = boost::json::value_to<double>(*fuel_docking_fee_value_ptr);
            }

            const boost::json::value * garbage_collect_frequency_value_ptr = general_object.if_contains("garbage_collect_frequency");
            if (garbage_collect_frequency_value_ptr != nullptr) {
                general.garbage_collect_frequency = boost::json::value_to<int>(*garbage_collect_frequency_value_ptr);
            }

            const boost::json::value * jump_key_delay_value_ptr = general_object.if_contains("jump_key_delay");
            if (jump_key_delay_value_ptr != nullptr) {
                general.jump_key_delay = boost::json::value_to<double>(*jump_key_delay_value_ptr);
            }

            const boost::json::value * load_last_savegame_value_ptr = general_object.if_contains("load_last_savegame");
            if (load_last_savegame_value_ptr != nullptr) {
                general.load_last_savegame = boost::json::value_to<bool>(*load_last_savegame_value_ptr);
            }

            const boost::json::value * new_game_save_name_value_ptr = general_object.if_contains("new_game_save_name");
            if (new_game_save_name_value_ptr != nullptr) {
                general.new_game_save_name = boost::json::value_to<std::string>(*new_game_save_name_value_ptr);
            }

            const boost::json::value * num_old_systems_value_ptr = general_object.if_contains("num_old_systems");
            if (num_old_systems_value_ptr != nullptr) {
                general.num_old_systems = boost::json::value_to<int>(*num_old_systems_value_ptr);
            }

            const boost::json::value * percentage_speed_change_to_fault_search_value_ptr = general_object.if_contains("percentage_speed_change_to_fault_search");
            if (percentage_speed_change_to_fault_search_value_ptr != nullptr) {
                general.percentage_speed_change_to_fault_search = boost::json::value_to<double>(*percentage_speed_change_to_fault_search_value_ptr);
            }

            const boost::json::value * persistent_mission_across_ship_switch_value_ptr = general_object.if_contains("persistent_mission_across_ship_switch");
            if (persistent_mission_across_ship_switch_value_ptr != nullptr) {
                general.persistent_mission_across_ship_switch = boost::json::value_to<bool>(*persistent_mission_across_ship_switch_value_ptr);
            }

            const boost::json::value * pitch_value_ptr = general_object.if_contains("pitch");
            if (pitch_value_ptr != nullptr) {
                general.pitch = boost::json::value_to<double>(*pitch_value_ptr);
            }

            const boost::json::value * quick_savegame_summaries_value_ptr = general_object.if_contains("quick_savegame_summaries");
            if (quick_savegame_summaries_value_ptr != nullptr) {
                general.quick_savegame_summaries = boost::json::value_to<bool>(*quick_savegame_summaries_value_ptr);
            }

            const boost::json::value * quick_savegame_summaries_buffer_size_value_ptr = general_object.if_contains("quick_savegame_summaries_buffer_size");
            if (quick_savegame_summaries_buffer_size_value_ptr != nullptr) {
                general.quick_savegame_summaries_buffer_size = boost::json::value_to<int>(*quick_savegame_summaries_buffer_size_value_ptr);
            }

            const boost::json::value * remember_savegame_value_ptr = general_object.if_contains("remember_savegame");
            if (remember_savegame_value_ptr != nullptr) {
                general.remember_savegame = boost::json::value_to<bool>(*remember_savegame_value_ptr);
            }

            const boost::json::value * remove_downgrades_less_than_percent_value_ptr = general_object.if_contains("remove_downgrades_less_than_percent");
            if (remove_downgrades_less_than_percent_value_ptr != nullptr) {
                general.remove_downgrades_less_than_percent = boost::json::value_to<double>(*remove_downgrades_less_than_percent_value_ptr);
            }

            const boost::json::value * remove_impossible_downgrades_value_ptr = general_object.if_contains("remove_impossible_downgrades");
            if (remove_impossible_downgrades_value_ptr != nullptr) {
                general.remove_impossible_downgrades = boost::json::value_to<bool>(*remove_impossible_downgrades_value_ptr);
            }

            const boost::json::value * roll_value_ptr = general_object.if_contains("roll");
            if (roll_value_ptr != nullptr) {
                general.roll = boost::json::value_to<double>(*roll_value_ptr);
            }

            const boost::json::value * screen_value_ptr = general_object.if_contains("screen");
            if (screen_value_ptr != nullptr) {
                general.screen = boost::json::value_to<int>(*screen_value_ptr);
            }

            const boost::json::value * simulation_atom_value_ptr = general_object.if_contains("simulation_atom");
            if (simulation_atom_value_ptr != nullptr) {
                general.simulation_atom = boost::json::value_to<double>(*simulation_atom_value_ptr);
            }

            const boost::json::value * times_to_show_help_screen_value_ptr = general_object.if_contains("times_to_show_help_screen");
            if (times_to_show_help_screen_value_ptr != nullptr) {
                general.times_to_show_help_screen = boost::json::value_to<int>(*times_to_show_help_screen_value_ptr);
            }

            const boost::json::value * trade_interface_tracks_prices_value_ptr = general_object.if_contains("trade_interface_tracks_prices");
            if (trade_interface_tracks_prices_value_ptr != nullptr) {
                general.trade_interface_tracks_prices = boost::json::value_to<bool>(*trade_interface_tracks_prices_value_ptr);
            }

            const boost::json::value * trade_interface_tracks_prices_top_rank_value_ptr = general_object.if_contains("trade_interface_tracks_prices_top_rank");
            if (trade_interface_tracks_prices_top_rank_value_ptr != nullptr) {
                general.trade_interface_tracks_prices_top_rank = boost::json::value_to<int>(*trade_interface_tracks_prices_top_rank_value_ptr);
            }

            const boost::json::value * verbose_output_value_ptr = general_object.if_contains("verbose_output");
            if (verbose_output_value_ptr != nullptr) {
                general.verbose_output = boost::json::value_to<int>(*verbose_output_value_ptr);
            }

            const boost::json::value * wheel_increment_lines_value_ptr = general_object.if_contains("wheel_increment_lines");
            if (wheel_increment_lines_value_ptr != nullptr) {
                general.wheel_increment_lines = boost::json::value_to<int>(*wheel_increment_lines_value_ptr);
            }

            const boost::json::value * while_loading_star_system_value_ptr = general_object.if_contains("while_loading_star_system");
            if (while_loading_star_system_value_ptr != nullptr) {
                general.while_loading_star_system = boost::json::value_to<bool>(*while_loading_star_system_value_ptr);
            }

            const boost::json::value * write_savegame_on_exit_value_ptr = general_object.if_contains("write_savegame_on_exit");
            if (write_savegame_on_exit_value_ptr != nullptr) {
                general.write_savegame_on_exit = boost::json::value_to<bool>(*write_savegame_on_exit_value_ptr);
            }

            const boost::json::value * yaw_value_ptr = general_object.if_contains("yaw");
            if (yaw_value_ptr != nullptr) {
                general.yaw = boost::json::value_to<double>(*yaw_value_ptr);
            }

        }


        const boost::json::value * graphics_value_ptr = root_object.if_contains("graphics");
        if (graphics_value_ptr != nullptr) {
            boost::json::object graphics_object = graphics_value_ptr->get_object();
            const boost::json::value * afterburner_color_red_value_ptr = graphics_object.if_contains("afterburner_color_red");
            if (afterburner_color_red_value_ptr != nullptr) {
                graphics.afterburner_color_red = boost::json::value_to<double>(*afterburner_color_red_value_ptr);
            }

            const boost::json::value * afterburner_color_green_value_ptr = graphics_object.if_contains("afterburner_color_green");
            if (afterburner_color_green_value_ptr != nullptr) {
                graphics.afterburner_color_green = boost::json::value_to<double>(*afterburner_color_green_value_ptr);
            }

            const boost::json::value * afterburner_color_blue_value_ptr = graphics_object.if_contains("afterburner_color_blue");
            if (afterburner_color_blue_value_ptr != nullptr) {
                graphics.afterburner_color_blue = boost::json::value_to<double>(*afterburner_color_blue_value_ptr);
            }

            const boost::json::value * always_make_smooth_cam_value_ptr = graphics_object.if_contains("always_make_smooth_cam");
            if (always_make_smooth_cam_value_ptr != nullptr) {
                graphics.always_make_smooth_cam = boost::json::value_to<bool>(*always_make_smooth_cam_value_ptr);
            }

            const boost::json::value * anim_far_percent_value_ptr = graphics_object.if_contains("anim_far_percent");
            if (anim_far_percent_value_ptr != nullptr) {
                graphics.anim_far_percent = boost::json::value_to<double>(*anim_far_percent_value_ptr);
            }

            const boost::json::value * armor_flash_animation_value_ptr = graphics_object.if_contains("armor_flash_animation");
            if (armor_flash_animation_value_ptr != nullptr) {
                graphics.armor_flash_animation = boost::json::value_to<std::string>(*armor_flash_animation_value_ptr);
            }

            const boost::json::value * aspect_value_ptr = graphics_object.if_contains("aspect");
            if (aspect_value_ptr != nullptr) {
                graphics.aspect = boost::json::value_to<double>(*aspect_value_ptr);
            }

            const boost::json::value * atmosphere_emissive_value_ptr = graphics_object.if_contains("atmosphere_emissive");
            if (atmosphere_emissive_value_ptr != nullptr) {
                graphics.atmosphere_emissive = boost::json::value_to<double>(*atmosphere_emissive_value_ptr);
            }

            const boost::json::value * atmosphere_diffuse_value_ptr = graphics_object.if_contains("atmosphere_diffuse");
            if (atmosphere_diffuse_value_ptr != nullptr) {
                graphics.atmosphere_diffuse = boost::json::value_to<double>(*atmosphere_diffuse_value_ptr);
            }

            const boost::json::value * auto_message_nav_switch_time_lim_value_ptr = graphics_object.if_contains("auto_message_nav_switch_time_lim");
            if (auto_message_nav_switch_time_lim_value_ptr != nullptr) {
                graphics.auto_message_nav_switch_time_lim = boost::json::value_to<double>(*auto_message_nav_switch_time_lim_value_ptr);
            }

            const boost::json::value * auto_message_time_lim_value_ptr = graphics_object.if_contains("auto_message_time_lim");
            if (auto_message_time_lim_value_ptr != nullptr) {
                graphics.auto_message_time_lim = boost::json::value_to<double>(*auto_message_time_lim_value_ptr);
            }

            const boost::json::value * automatic_landing_zone_warning_value_ptr = graphics_object.if_contains("automatic_landing_zone_warning");
            if (automatic_landing_zone_warning_value_ptr != nullptr) {
                graphics.automatic_landing_zone_warning = boost::json::value_to<std::string>(*automatic_landing_zone_warning_value_ptr);
            }

            const boost::json::value * automatic_landing_zone_warning1_value_ptr = graphics_object.if_contains("automatic_landing_zone_warning1");
            if (automatic_landing_zone_warning1_value_ptr != nullptr) {
                graphics.automatic_landing_zone_warning1 = boost::json::value_to<std::string>(*automatic_landing_zone_warning1_value_ptr);
            }

            const boost::json::value * automatic_landing_zone_warning2_value_ptr = graphics_object.if_contains("automatic_landing_zone_warning2");
            if (automatic_landing_zone_warning2_value_ptr != nullptr) {
                graphics.automatic_landing_zone_warning2 = boost::json::value_to<std::string>(*automatic_landing_zone_warning2_value_ptr);
            }

            const boost::json::value * automatic_landing_zone_warning_text_value_ptr = graphics_object.if_contains("automatic_landing_zone_warning_text");
            if (automatic_landing_zone_warning_text_value_ptr != nullptr) {
                graphics.automatic_landing_zone_warning_text = boost::json::value_to<std::string>(*automatic_landing_zone_warning_text_value_ptr);
            }

            const boost::json::value * autopilot_front_bias_value_ptr = graphics_object.if_contains("autopilot_front_bias");
            if (autopilot_front_bias_value_ptr != nullptr) {
                graphics.autopilot_front_bias = boost::json::value_to<double>(*autopilot_front_bias_value_ptr);
            }

            const boost::json::value * autopilot_rotation_speed_value_ptr = graphics_object.if_contains("autopilot_rotation_speed");
            if (autopilot_rotation_speed_value_ptr != nullptr) {
                graphics.autopilot_rotation_speed = boost::json::value_to<double>(*autopilot_rotation_speed_value_ptr);
            }

            const boost::json::value * autopilot_side_bias_value_ptr = graphics_object.if_contains("autopilot_side_bias");
            if (autopilot_side_bias_value_ptr != nullptr) {
                graphics.autopilot_side_bias = boost::json::value_to<double>(*autopilot_side_bias_value_ptr);
            }

            const boost::json::value * background_value_ptr = graphics_object.if_contains("background");
            if (background_value_ptr != nullptr) {
                graphics.background = boost::json::value_to<bool>(*background_value_ptr);
            }

            const boost::json::value * background_edge_fixup_value_ptr = graphics_object.if_contains("background_edge_fixup");
            if (background_edge_fixup_value_ptr != nullptr) {
                graphics.background_edge_fixup = boost::json::value_to<double>(*background_edge_fixup_value_ptr);
            }

            const boost::json::value * background_star_streak_velocity_scale_value_ptr = graphics_object.if_contains("background_star_streak_velocity_scale");
            if (background_star_streak_velocity_scale_value_ptr != nullptr) {
                graphics.background_star_streak_velocity_scale = boost::json::value_to<double>(*background_star_streak_velocity_scale_value_ptr);
            }

            const boost::json::value * base_faction_color_darkness_value_ptr = graphics_object.if_contains("base_faction_color_darkness");
            if (base_faction_color_darkness_value_ptr != nullptr) {
                graphics.base_faction_color_darkness = boost::json::value_to<double>(*base_faction_color_darkness_value_ptr);
            }

            const boost::json::value * beam_fadeout_length_value_ptr = graphics_object.if_contains("beam_fadeout_length");
            if (beam_fadeout_length_value_ptr != nullptr) {
                graphics.beam_fadeout_length = boost::json::value_to<double>(*beam_fadeout_length_value_ptr);
            }

            const boost::json::value * beam_fadeout_hit_length_value_ptr = graphics_object.if_contains("beam_fadeout_hit_length");
            if (beam_fadeout_hit_length_value_ptr != nullptr) {
                graphics.beam_fadeout_hit_length = boost::json::value_to<double>(*beam_fadeout_hit_length_value_ptr);
            }

            const boost::json::value * base_saturation_value_ptr = graphics_object.if_contains("base_saturation");
            if (base_saturation_value_ptr != nullptr) {
                graphics.base_saturation = boost::json::value_to<double>(*base_saturation_value_ptr);
            }

            const boost::json::value * bitmap_alphamap_value_ptr = graphics_object.if_contains("bitmap_alphamap");
            if (bitmap_alphamap_value_ptr != nullptr) {
                graphics.bitmap_alphamap = boost::json::value_to<bool>(*bitmap_alphamap_value_ptr);
            }

            const boost::json::value * blend_cockpit_value_ptr = graphics_object.if_contains("blend_cockpit");
            if (blend_cockpit_value_ptr != nullptr) {
                graphics.blend_cockpit = boost::json::value_to<bool>(*blend_cockpit_value_ptr);
            }

            const boost::json::value * blend_guns_value_ptr = graphics_object.if_contains("blend_guns");
            if (blend_guns_value_ptr != nullptr) {
                graphics.blend_guns = boost::json::value_to<bool>(*blend_guns_value_ptr);
            }

            const boost::json::value * blend_panels_value_ptr = graphics_object.if_contains("blend_panels");
            if (blend_panels_value_ptr != nullptr) {
                graphics.blend_panels = boost::json::value_to<bool>(*blend_panels_value_ptr);
            }

            const boost::json::value * bolt_offset_value_ptr = graphics_object.if_contains("bolt_offset");
            if (bolt_offset_value_ptr != nullptr) {
                graphics.bolt_offset = boost::json::value_to<double>(*bolt_offset_value_ptr);
            }

            const boost::json::value * bolt_pixel_size_value_ptr = graphics_object.if_contains("bolt_pixel_size");
            if (bolt_pixel_size_value_ptr != nullptr) {
                graphics.bolt_pixel_size = boost::json::value_to<double>(*bolt_pixel_size_value_ptr);
            }

            const boost::json::value * camera_pan_speed_value_ptr = graphics_object.if_contains("camera_pan_speed");
            if (camera_pan_speed_value_ptr != nullptr) {
                graphics.camera_pan_speed = boost::json::value_to<double>(*camera_pan_speed_value_ptr);
            }

            const boost::json::value * can_target_asteroid_value_ptr = graphics_object.if_contains("can_target_asteroid");
            if (can_target_asteroid_value_ptr != nullptr) {
                graphics.can_target_asteroid = boost::json::value_to<bool>(*can_target_asteroid_value_ptr);
            }

            const boost::json::value * can_target_cargo_value_ptr = graphics_object.if_contains("can_target_cargo");
            if (can_target_cargo_value_ptr != nullptr) {
                graphics.can_target_cargo = boost::json::value_to<bool>(*can_target_cargo_value_ptr);
            }

            const boost::json::value * can_target_sun_value_ptr = graphics_object.if_contains("can_target_sun");
            if (can_target_sun_value_ptr != nullptr) {
                graphics.can_target_sun = boost::json::value_to<bool>(*can_target_sun_value_ptr);
            }

            const boost::json::value * cargo_rotates_at_same_speed_value_ptr = graphics_object.if_contains("cargo_rotates_at_same_speed");
            if (cargo_rotates_at_same_speed_value_ptr != nullptr) {
                graphics.cargo_rotates_at_same_speed = boost::json::value_to<bool>(*cargo_rotates_at_same_speed_value_ptr);
            }

            const boost::json::value * cargo_rotation_speed_value_ptr = graphics_object.if_contains("cargo_rotation_speed");
            if (cargo_rotation_speed_value_ptr != nullptr) {
                graphics.cargo_rotation_speed = boost::json::value_to<double>(*cargo_rotation_speed_value_ptr);
            }

            const boost::json::value * chat_text_value_ptr = graphics_object.if_contains("chat_text");
            if (chat_text_value_ptr != nullptr) {
                graphics.chat_text = boost::json::value_to<bool>(*chat_text_value_ptr);
            }

            const boost::json::value * circle_accuracy_value_ptr = graphics_object.if_contains("circle_accuracy");
            if (circle_accuracy_value_ptr != nullptr) {
                graphics.circle_accuracy = boost::json::value_to<double>(*circle_accuracy_value_ptr);
            }

            const boost::json::value * city_light_filter_value_ptr = graphics_object.if_contains("city_light_filter");
            if (city_light_filter_value_ptr != nullptr) {
                graphics.city_light_filter = boost::json::value_to<int>(*city_light_filter_value_ptr);
            }

            const boost::json::value * city_light_strength_value_ptr = graphics_object.if_contains("city_light_strength");
            if (city_light_strength_value_ptr != nullptr) {
                graphics.city_light_strength = boost::json::value_to<double>(*city_light_strength_value_ptr);
            }

            const boost::json::value * clear_on_startup_value_ptr = graphics_object.if_contains("clear_on_startup");
            if (clear_on_startup_value_ptr != nullptr) {
                graphics.clear_on_startup = boost::json::value_to<bool>(*clear_on_startup_value_ptr);
            }

            const boost::json::value * cockpit_value_ptr = graphics_object.if_contains("cockpit");
            if (cockpit_value_ptr != nullptr) {
                graphics.cockpit = boost::json::value_to<bool>(*cockpit_value_ptr);
            }

            const boost::json::value * cockpit_drift_amount_value_ptr = graphics_object.if_contains("cockpit_drift_amount");
            if (cockpit_drift_amount_value_ptr != nullptr) {
                graphics.cockpit_drift_amount = boost::json::value_to<double>(*cockpit_drift_amount_value_ptr);
            }

            const boost::json::value * cockpit_drift_limit_value_ptr = graphics_object.if_contains("cockpit_drift_limit");
            if (cockpit_drift_limit_value_ptr != nullptr) {
                graphics.cockpit_drift_limit = boost::json::value_to<double>(*cockpit_drift_limit_value_ptr);
            }

            const boost::json::value * cockpit_drift_ref_accel_value_ptr = graphics_object.if_contains("cockpit_drift_ref_accel");
            if (cockpit_drift_ref_accel_value_ptr != nullptr) {
                graphics.cockpit_drift_ref_accel = boost::json::value_to<double>(*cockpit_drift_ref_accel_value_ptr);
            }

            const boost::json::value * cockpit_nav_zfloor_value_ptr = graphics_object.if_contains("cockpit_nav_zfloor");
            if (cockpit_nav_zfloor_value_ptr != nullptr) {
                graphics.cockpit_nav_zfloor = boost::json::value_to<double>(*cockpit_nav_zfloor_value_ptr);
            }

            const boost::json::value * cockpit_nav_zrange_value_ptr = graphics_object.if_contains("cockpit_nav_zrange");
            if (cockpit_nav_zrange_value_ptr != nullptr) {
                graphics.cockpit_nav_zrange = boost::json::value_to<double>(*cockpit_nav_zrange_value_ptr);
            }

            const boost::json::value * cockpit_shake_value_ptr = graphics_object.if_contains("cockpit_shake");
            if (cockpit_shake_value_ptr != nullptr) {
                graphics.cockpit_shake = boost::json::value_to<double>(*cockpit_shake_value_ptr);
            }

            const boost::json::value * cockpit_shake_max_value_ptr = graphics_object.if_contains("cockpit_shake_max");
            if (cockpit_shake_max_value_ptr != nullptr) {
                graphics.cockpit_shake_max = boost::json::value_to<double>(*cockpit_shake_max_value_ptr);
            }

            const boost::json::value * cockpit_smooth_texture_value_ptr = graphics_object.if_contains("cockpit_smooth_texture");
            if (cockpit_smooth_texture_value_ptr != nullptr) {
                graphics.cockpit_smooth_texture = boost::json::value_to<bool>(*cockpit_smooth_texture_value_ptr);
            }

            const boost::json::value * cockpit_z_partitions_value_ptr = graphics_object.if_contains("cockpit_z_partitions");
            if (cockpit_z_partitions_value_ptr != nullptr) {
                graphics.cockpit_z_partitions = boost::json::value_to<int>(*cockpit_z_partitions_value_ptr);
            }

            const boost::json::value * color_depth_value_ptr = graphics_object.if_contains("color_depth");
            if (color_depth_value_ptr != nullptr) {
                graphics.color_depth = boost::json::value_to<int>(*color_depth_value_ptr);
            }

            const boost::json::value * comm_static_value_ptr = graphics_object.if_contains("comm_static");
            if (comm_static_value_ptr != nullptr) {
                graphics.comm_static = boost::json::value_to<std::string>(*comm_static_value_ptr);
            }

            const boost::json::value * crosshair_smooth_texture_value_ptr = graphics_object.if_contains("crosshair_smooth_texture");
            if (crosshair_smooth_texture_value_ptr != nullptr) {
                graphics.crosshair_smooth_texture = boost::json::value_to<bool>(*crosshair_smooth_texture_value_ptr);
            }

            const boost::json::value * damage_flash_alpha_value_ptr = graphics_object.if_contains("damage_flash_alpha");
            if (damage_flash_alpha_value_ptr != nullptr) {
                graphics.damage_flash_alpha = boost::json::value_to<bool>(*damage_flash_alpha_value_ptr);
            }

            const boost::json::value * damage_flash_length_value_ptr = graphics_object.if_contains("damage_flash_length");
            if (damage_flash_length_value_ptr != nullptr) {
                graphics.damage_flash_length = boost::json::value_to<double>(*damage_flash_length_value_ptr);
            }

            const boost::json::value * day_city_light_strength_value_ptr = graphics_object.if_contains("day_city_light_strength");
            if (day_city_light_strength_value_ptr != nullptr) {
                graphics.day_city_light_strength = boost::json::value_to<double>(*day_city_light_strength_value_ptr);
            }

            const boost::json::value * death_menu_script_value_ptr = graphics_object.if_contains("death_menu_script");
            if (death_menu_script_value_ptr != nullptr) {
                graphics.death_menu_script = boost::json::value_to<std::string>(*death_menu_script_value_ptr);
            }

            const boost::json::value * death_scene_time_value_ptr = graphics_object.if_contains("death_scene_time");
            if (death_scene_time_value_ptr != nullptr) {
                graphics.death_scene_time = boost::json::value_to<double>(*death_scene_time_value_ptr);
            }

            const boost::json::value * default_boot_message_value_ptr = graphics_object.if_contains("default_boot_message");
            if (default_boot_message_value_ptr != nullptr) {
                graphics.default_boot_message = boost::json::value_to<std::string>(*default_boot_message_value_ptr);
            }

            const boost::json::value * default_engine_activation_value_ptr = graphics_object.if_contains("default_engine_activation");
            if (default_engine_activation_value_ptr != nullptr) {
                graphics.default_engine_activation = boost::json::value_to<double>(*default_engine_activation_value_ptr);
            }

            const boost::json::value * default_full_technique_value_ptr = graphics_object.if_contains("default_full_technique");
            if (default_full_technique_value_ptr != nullptr) {
                graphics.default_full_technique = boost::json::value_to<std::string>(*default_full_technique_value_ptr);
            }

            const boost::json::value * default_simple_technique_value_ptr = graphics_object.if_contains("default_simple_technique");
            if (default_simple_technique_value_ptr != nullptr) {
                graphics.default_simple_technique = boost::json::value_to<std::string>(*default_simple_technique_value_ptr);
            }

            const boost::json::value * detail_texture_scale_value_ptr = graphics_object.if_contains("detail_texture_scale");
            if (detail_texture_scale_value_ptr != nullptr) {
                graphics.detail_texture_scale = boost::json::value_to<double>(*detail_texture_scale_value_ptr);
            }

            const boost::json::value * detail_texture_trilinear_value_ptr = graphics_object.if_contains("detail_texture_trilinear");
            if (detail_texture_trilinear_value_ptr != nullptr) {
                graphics.detail_texture_trilinear = boost::json::value_to<bool>(*detail_texture_trilinear_value_ptr);
            }

            const boost::json::value * disabled_cockpit_allowed_value_ptr = graphics_object.if_contains("disabled_cockpit_allowed");
            if (disabled_cockpit_allowed_value_ptr != nullptr) {
                graphics.disabled_cockpit_allowed = boost::json::value_to<bool>(*disabled_cockpit_allowed_value_ptr);
            }

            const boost::json::value * displaylists_value_ptr = graphics_object.if_contains("displaylists");
            if (displaylists_value_ptr != nullptr) {
                graphics.displaylists = boost::json::value_to<bool>(*displaylists_value_ptr);
            }

            const boost::json::value * draw_cockpit_from_chase_cam_value_ptr = graphics_object.if_contains("draw_cockpit_from_chase_cam");
            if (draw_cockpit_from_chase_cam_value_ptr != nullptr) {
                graphics.draw_cockpit_from_chase_cam = boost::json::value_to<bool>(*draw_cockpit_from_chase_cam_value_ptr);
            }

            const boost::json::value * draw_cockpit_from_padlock_cam_value_ptr = graphics_object.if_contains("draw_cockpit_from_padlock_cam");
            if (draw_cockpit_from_padlock_cam_value_ptr != nullptr) {
                graphics.draw_cockpit_from_padlock_cam = boost::json::value_to<bool>(*draw_cockpit_from_padlock_cam_value_ptr);
            }

            const boost::json::value * draw_cockpit_from_panning_cam_value_ptr = graphics_object.if_contains("draw_cockpit_from_panning_cam");
            if (draw_cockpit_from_panning_cam_value_ptr != nullptr) {
                graphics.draw_cockpit_from_panning_cam = boost::json::value_to<bool>(*draw_cockpit_from_panning_cam_value_ptr);
            }

            const boost::json::value * draw_cockpit_from_target_cam_value_ptr = graphics_object.if_contains("draw_cockpit_from_target_cam");
            if (draw_cockpit_from_target_cam_value_ptr != nullptr) {
                graphics.draw_cockpit_from_target_cam = boost::json::value_to<bool>(*draw_cockpit_from_target_cam_value_ptr);
            }

            const boost::json::value * draw_docking_boxes_value_ptr = graphics_object.if_contains("draw_docking_boxes");
            if (draw_docking_boxes_value_ptr != nullptr) {
                graphics.draw_docking_boxes = boost::json::value_to<bool>(*draw_docking_boxes_value_ptr);
            }

            const boost::json::value * draw_heading_marker_value_ptr = graphics_object.if_contains("draw_heading_marker");
            if (draw_heading_marker_value_ptr != nullptr) {
                graphics.draw_heading_marker = boost::json::value_to<bool>(*draw_heading_marker_value_ptr);
            }

            const boost::json::value * draw_jump_target_nav_symbol_value_ptr = graphics_object.if_contains("draw_jump_target_nav_symbol");
            if (draw_jump_target_nav_symbol_value_ptr != nullptr) {
                graphics.draw_jump_target_nav_symbol = boost::json::value_to<bool>(*draw_jump_target_nav_symbol_value_ptr);
            }

            const boost::json::value * draw_nav_button_labels_value_ptr = graphics_object.if_contains("draw_nav_button_labels");
            if (draw_nav_button_labels_value_ptr != nullptr) {
                graphics.draw_nav_button_labels = boost::json::value_to<bool>(*draw_nav_button_labels_value_ptr);
            }

            const boost::json::value * draw_near_stars_in_front_of_planets_value_ptr = graphics_object.if_contains("draw_near_stars_in_front_of_planets");
            if (draw_near_stars_in_front_of_planets_value_ptr != nullptr) {
                graphics.draw_near_stars_in_front_of_planets = boost::json::value_to<bool>(*draw_near_stars_in_front_of_planets_value_ptr);
            }

            const boost::json::value * draw_rendered_crosshairs_value_ptr = graphics_object.if_contains("draw_rendered_crosshairs");
            if (draw_rendered_crosshairs_value_ptr != nullptr) {
                graphics.draw_rendered_crosshairs = boost::json::value_to<bool>(*draw_rendered_crosshairs_value_ptr);
            }

            const boost::json::value * draw_star_body_value_ptr = graphics_object.if_contains("draw_star_body");
            if (draw_star_body_value_ptr != nullptr) {
                graphics.draw_star_body = boost::json::value_to<bool>(*draw_star_body_value_ptr);
            }

            const boost::json::value * draw_star_glow_value_ptr = graphics_object.if_contains("draw_star_glow");
            if (draw_star_glow_value_ptr != nullptr) {
                graphics.draw_star_glow = boost::json::value_to<bool>(*draw_star_glow_value_ptr);
            }

            const boost::json::value * draw_star_glow_halo_value_ptr = graphics_object.if_contains("draw_star_glow_halo");
            if (draw_star_glow_halo_value_ptr != nullptr) {
                graphics.draw_star_glow_halo = boost::json::value_to<bool>(*draw_star_glow_halo_value_ptr);
            }

            const boost::json::value * draw_target_nav_symbol_value_ptr = graphics_object.if_contains("draw_target_nav_symbol");
            if (draw_target_nav_symbol_value_ptr != nullptr) {
                graphics.draw_target_nav_symbol = boost::json::value_to<bool>(*draw_target_nav_symbol_value_ptr);
            }

            const boost::json::value * draw_vdus_from_chase_cam_value_ptr = graphics_object.if_contains("draw_vdus_from_chase_cam");
            if (draw_vdus_from_chase_cam_value_ptr != nullptr) {
                graphics.draw_vdus_from_chase_cam = boost::json::value_to<bool>(*draw_vdus_from_chase_cam_value_ptr);
            }

            const boost::json::value * draw_vdus_from_padlock_cam_value_ptr = graphics_object.if_contains("draw_vdus_from_padlock_cam");
            if (draw_vdus_from_padlock_cam_value_ptr != nullptr) {
                graphics.draw_vdus_from_padlock_cam = boost::json::value_to<bool>(*draw_vdus_from_padlock_cam_value_ptr);
            }

            const boost::json::value * draw_vdus_from_panning_cam_value_ptr = graphics_object.if_contains("draw_vdus_from_panning_cam");
            if (draw_vdus_from_panning_cam_value_ptr != nullptr) {
                graphics.draw_vdus_from_panning_cam = boost::json::value_to<bool>(*draw_vdus_from_panning_cam_value_ptr);
            }

            const boost::json::value * draw_vdus_from_target_cam_value_ptr = graphics_object.if_contains("draw_vdus_from_target_cam");
            if (draw_vdus_from_target_cam_value_ptr != nullptr) {
                graphics.draw_vdus_from_target_cam = boost::json::value_to<bool>(*draw_vdus_from_target_cam_value_ptr);
            }

            const boost::json::value * draw_weapons_value_ptr = graphics_object.if_contains("draw_weapons");
            if (draw_weapons_value_ptr != nullptr) {
                graphics.draw_weapons = boost::json::value_to<bool>(*draw_weapons_value_ptr);
            }

            const boost::json::value * eject_rotation_speed_value_ptr = graphics_object.if_contains("eject_rotation_speed");
            if (eject_rotation_speed_value_ptr != nullptr) {
                graphics.eject_rotation_speed = boost::json::value_to<double>(*eject_rotation_speed_value_ptr);
            }

            const boost::json::value * engine_color_red_value_ptr = graphics_object.if_contains("engine_color_red");
            if (engine_color_red_value_ptr != nullptr) {
                graphics.engine_color_red = boost::json::value_to<double>(*engine_color_red_value_ptr);
            }

            const boost::json::value * engine_color_green_value_ptr = graphics_object.if_contains("engine_color_green");
            if (engine_color_green_value_ptr != nullptr) {
                graphics.engine_color_green = boost::json::value_to<double>(*engine_color_green_value_ptr);
            }

            const boost::json::value * engine_color_blue_value_ptr = graphics_object.if_contains("engine_color_blue");
            if (engine_color_blue_value_ptr != nullptr) {
                graphics.engine_color_blue = boost::json::value_to<double>(*engine_color_blue_value_ptr);
            }

            const boost::json::value * engine_length_scale_value_ptr = graphics_object.if_contains("engine_length_scale");
            if (engine_length_scale_value_ptr != nullptr) {
                graphics.engine_length_scale = boost::json::value_to<double>(*engine_length_scale_value_ptr);
            }

            const boost::json::value * engine_radii_scale_value_ptr = graphics_object.if_contains("engine_radii_scale");
            if (engine_radii_scale_value_ptr != nullptr) {
                graphics.engine_radii_scale = boost::json::value_to<double>(*engine_radii_scale_value_ptr);
            }

            const boost::json::value * explore_for_map_value_ptr = graphics_object.if_contains("explore_for_map");
            if (explore_for_map_value_ptr != nullptr) {
                graphics.explore_for_map = boost::json::value_to<bool>(*explore_for_map_value_ptr);
            }

            const boost::json::value * explosion_animation_value_ptr = graphics_object.if_contains("explosion_animation");
            if (explosion_animation_value_ptr != nullptr) {
                graphics.explosion_animation = boost::json::value_to<std::string>(*explosion_animation_value_ptr);
            }

            const boost::json::value * explosion_face_player_value_ptr = graphics_object.if_contains("explosion_face_player");
            if (explosion_face_player_value_ptr != nullptr) {
                graphics.explosion_face_player = boost::json::value_to<bool>(*explosion_face_player_value_ptr);
            }

            const boost::json::value * explosion_force_value_ptr = graphics_object.if_contains("explosion_force");
            if (explosion_force_value_ptr != nullptr) {
                graphics.explosion_force = boost::json::value_to<double>(*explosion_force_value_ptr);
            }

            const boost::json::value * explosion_size_value_ptr = graphics_object.if_contains("explosion_size");
            if (explosion_size_value_ptr != nullptr) {
                graphics.explosion_size = boost::json::value_to<double>(*explosion_size_value_ptr);
            }

            const boost::json::value * explosion_torque_value_ptr = graphics_object.if_contains("explosion_torque");
            if (explosion_torque_value_ptr != nullptr) {
                graphics.explosion_torque = boost::json::value_to<double>(*explosion_torque_value_ptr);
            }

            const boost::json::value * ext_clamp_to_border_value_ptr = graphics_object.if_contains("ext_clamp_to_border");
            if (ext_clamp_to_border_value_ptr != nullptr) {
                graphics.ext_clamp_to_border = boost::json::value_to<bool>(*ext_clamp_to_border_value_ptr);
            }

            const boost::json::value * ext_clamp_to_edge_value_ptr = graphics_object.if_contains("ext_clamp_to_edge");
            if (ext_clamp_to_edge_value_ptr != nullptr) {
                graphics.ext_clamp_to_edge = boost::json::value_to<bool>(*ext_clamp_to_edge_value_ptr);
            }

            const boost::json::value * faction_dependent_textures_value_ptr = graphics_object.if_contains("faction_dependent_textures");
            if (faction_dependent_textures_value_ptr != nullptr) {
                graphics.faction_dependent_textures = boost::json::value_to<bool>(*faction_dependent_textures_value_ptr);
            }

            const boost::json::value * far_stars_sprite_size_value_ptr = graphics_object.if_contains("far_stars_sprite_size");
            if (far_stars_sprite_size_value_ptr != nullptr) {
                graphics.far_stars_sprite_size = boost::json::value_to<double>(*far_stars_sprite_size_value_ptr);
            }

            const boost::json::value * far_stars_sprite_texture_value_ptr = graphics_object.if_contains("far_stars_sprite_texture");
            if (far_stars_sprite_texture_value_ptr != nullptr) {
                graphics.far_stars_sprite_texture = boost::json::value_to<std::string>(*far_stars_sprite_texture_value_ptr);
            }

            const boost::json::value * flash_behind_hud_value_ptr = graphics_object.if_contains("flash_behind_hud");
            if (flash_behind_hud_value_ptr != nullptr) {
                graphics.flash_behind_hud = boost::json::value_to<bool>(*flash_behind_hud_value_ptr);
            }

            const boost::json::value * fog_detail_value_ptr = graphics_object.if_contains("fog_detail");
            if (fog_detail_value_ptr != nullptr) {
                graphics.fog_detail = boost::json::value_to<int>(*fog_detail_value_ptr);
            }

            const boost::json::value * font_value_ptr = graphics_object.if_contains("font");
            if (font_value_ptr != nullptr) {
                graphics.font = boost::json::value_to<std::string>(*font_value_ptr);
            }

            const boost::json::value * font_antialias_value_ptr = graphics_object.if_contains("font_antialias");
            if (font_antialias_value_ptr != nullptr) {
                graphics.font_antialias = boost::json::value_to<bool>(*font_antialias_value_ptr);
            }

            const boost::json::value * font_point_value_ptr = graphics_object.if_contains("font_point");
            if (font_point_value_ptr != nullptr) {
                graphics.font_point = boost::json::value_to<double>(*font_point_value_ptr);
            }

            const boost::json::value * font_width_hack_value_ptr = graphics_object.if_contains("font_width_hack");
            if (font_width_hack_value_ptr != nullptr) {
                graphics.font_width_hack = boost::json::value_to<double>(*font_width_hack_value_ptr);
            }

            const boost::json::value * force_glowmap_restore_write_to_depthmap_value_ptr = graphics_object.if_contains("force_glowmap_restore_write_to_depthmap");
            if (force_glowmap_restore_write_to_depthmap_value_ptr != nullptr) {
                graphics.force_glowmap_restore_write_to_depthmap = boost::json::value_to<bool>(*force_glowmap_restore_write_to_depthmap_value_ptr);
            }

            const boost::json::value * force_lighting_value_ptr = graphics_object.if_contains("force_lighting");
            if (force_lighting_value_ptr != nullptr) {
                graphics.force_lighting = boost::json::value_to<bool>(*force_lighting_value_ptr);
            }

            const boost::json::value * force_one_one_shield_blend_value_ptr = graphics_object.if_contains("force_one_one_shield_blend");
            if (force_one_one_shield_blend_value_ptr != nullptr) {
                graphics.force_one_one_shield_blend = boost::json::value_to<bool>(*force_one_one_shield_blend_value_ptr);
            }

            const boost::json::value * fov_value_ptr = graphics_object.if_contains("fov");
            if (fov_value_ptr != nullptr) {
                graphics.fov = boost::json::value_to<double>(*fov_value_ptr);
            }

            const boost::json::value * framerate_changes_shader_value_ptr = graphics_object.if_contains("framerate_changes_shader");
            if (framerate_changes_shader_value_ptr != nullptr) {
                graphics.framerate_changes_shader = boost::json::value_to<bool>(*framerate_changes_shader_value_ptr);
            }

            const boost::json::value * full_screen_value_ptr = graphics_object.if_contains("full_screen");
            if (full_screen_value_ptr != nullptr) {
                graphics.full_screen = boost::json::value_to<bool>(*full_screen_value_ptr);
            }

            const boost::json::value * gauge_static_value_ptr = graphics_object.if_contains("gauge_static");
            if (gauge_static_value_ptr != nullptr) {
                graphics.gauge_static = boost::json::value_to<std::string>(*gauge_static_value_ptr);
            }

            const boost::json::value * generic_cargo_rotation_speed_value_ptr = graphics_object.if_contains("generic_cargo_rotation_speed");
            if (generic_cargo_rotation_speed_value_ptr != nullptr) {
                graphics.generic_cargo_rotation_speed = boost::json::value_to<double>(*generic_cargo_rotation_speed_value_ptr);
            }

            const boost::json::value * gl_accelerated_visual_value_ptr = graphics_object.if_contains("gl_accelerated_visual");
            if (gl_accelerated_visual_value_ptr != nullptr) {
                graphics.gl_accelerated_visual = boost::json::value_to<bool>(*gl_accelerated_visual_value_ptr);
            }

            const boost::json::value * glow_ambient_star_light_value_ptr = graphics_object.if_contains("glow_ambient_star_light");
            if (glow_ambient_star_light_value_ptr != nullptr) {
                graphics.glow_ambient_star_light = boost::json::value_to<bool>(*glow_ambient_star_light_value_ptr);
            }

            const boost::json::value * glow_diffuse_star_light_value_ptr = graphics_object.if_contains("glow_diffuse_star_light");
            if (glow_diffuse_star_light_value_ptr != nullptr) {
                graphics.glow_diffuse_star_light = boost::json::value_to<bool>(*glow_diffuse_star_light_value_ptr);
            }

            const boost::json::value * glut_stencil_value_ptr = graphics_object.if_contains("glut_stencil");
            if (glut_stencil_value_ptr != nullptr) {
                graphics.glut_stencil = boost::json::value_to<bool>(*glut_stencil_value_ptr);
            }

            const boost::json::value * gui_font_scale_value_ptr = graphics_object.if_contains("gui_font_scale");
            if (gui_font_scale_value_ptr != nullptr) {
                graphics.gui_font_scale = boost::json::value_to<double>(*gui_font_scale_value_ptr);
            }

            const boost::json::value * halo_far_draw_value_ptr = graphics_object.if_contains("halo_far_draw");
            if (halo_far_draw_value_ptr != nullptr) {
                graphics.halo_far_draw = boost::json::value_to<bool>(*halo_far_draw_value_ptr);
            }

            const boost::json::value * halo_offset_value_ptr = graphics_object.if_contains("halo_offset");
            if (halo_offset_value_ptr != nullptr) {
                graphics.halo_offset = boost::json::value_to<double>(*halo_offset_value_ptr);
            }

            const boost::json::value * halo_sparkle_rate_value_ptr = graphics_object.if_contains("halo_sparkle_rate");
            if (halo_sparkle_rate_value_ptr != nullptr) {
                graphics.halo_sparkle_rate = boost::json::value_to<double>(*halo_sparkle_rate_value_ptr);
            }

            const boost::json::value * halo_sparkle_scale_value_ptr = graphics_object.if_contains("halo_sparkle_scale");
            if (halo_sparkle_scale_value_ptr != nullptr) {
                graphics.halo_sparkle_scale = boost::json::value_to<double>(*halo_sparkle_scale_value_ptr);
            }

            const boost::json::value * halo_sparkle_speed_value_ptr = graphics_object.if_contains("halo_sparkle_speed");
            if (halo_sparkle_speed_value_ptr != nullptr) {
                graphics.halo_sparkle_speed = boost::json::value_to<double>(*halo_sparkle_speed_value_ptr);
            }

            const boost::json::value * halos_by_velocity_value_ptr = graphics_object.if_contains("halos_by_velocity");
            if (halos_by_velocity_value_ptr != nullptr) {
                graphics.halos_by_velocity = boost::json::value_to<bool>(*halos_by_velocity_value_ptr);
            }

            const boost::json::value * head_lag_value_ptr = graphics_object.if_contains("head_lag");
            if (head_lag_value_ptr != nullptr) {
                graphics.head_lag = boost::json::value_to<int>(*head_lag_value_ptr);
            }

            const boost::json::value * high_quality_font_value_ptr = graphics_object.if_contains("high_quality_font");
            if (high_quality_font_value_ptr != nullptr) {
                graphics.high_quality_font = boost::json::value_to<bool>(*high_quality_font_value_ptr);
            }

            const boost::json::value * high_quality_font_computer_value_ptr = graphics_object.if_contains("high_quality_font_computer");
            if (high_quality_font_computer_value_ptr != nullptr) {
                graphics.high_quality_font_computer = boost::json::value_to<bool>(*high_quality_font_computer_value_ptr);
            }

            const boost::json::value * high_quality_sprites_value_ptr = graphics_object.if_contains("high_quality_sprites");
            if (high_quality_sprites_value_ptr != nullptr) {
                graphics.high_quality_sprites = boost::json::value_to<bool>(*high_quality_sprites_value_ptr);
            }

            const boost::json::value * hull_flash_animation_value_ptr = graphics_object.if_contains("hull_flash_animation");
            if (hull_flash_animation_value_ptr != nullptr) {
                graphics.hull_flash_animation = boost::json::value_to<std::string>(*hull_flash_animation_value_ptr);
            }

            const boost::json::value * in_system_jump_ani_second_ahead_value_ptr = graphics_object.if_contains("in_system_jump_ani_second_ahead");
            if (in_system_jump_ani_second_ahead_value_ptr != nullptr) {
                graphics.in_system_jump_ani_second_ahead = boost::json::value_to<double>(*in_system_jump_ani_second_ahead_value_ptr);
            }

            const boost::json::value * in_system_jump_ani_second_ahead_end_value_ptr = graphics_object.if_contains("in_system_jump_ani_second_ahead_end");
            if (in_system_jump_ani_second_ahead_end_value_ptr != nullptr) {
                graphics.in_system_jump_ani_second_ahead_end = boost::json::value_to<double>(*in_system_jump_ani_second_ahead_end_value_ptr);
            }

            const boost::json::value * in_system_jump_animation_value_ptr = graphics_object.if_contains("in_system_jump_animation");
            if (in_system_jump_animation_value_ptr != nullptr) {
                graphics.in_system_jump_animation = boost::json::value_to<std::string>(*in_system_jump_animation_value_ptr);
            }

            const boost::json::value * in_system_jump_animation_growth_value_ptr = graphics_object.if_contains("in_system_jump_animation_growth");
            if (in_system_jump_animation_growth_value_ptr != nullptr) {
                graphics.in_system_jump_animation_growth = boost::json::value_to<double>(*in_system_jump_animation_growth_value_ptr);
            }

            const boost::json::value * in_system_jump_animation_size_value_ptr = graphics_object.if_contains("in_system_jump_animation_size");
            if (in_system_jump_animation_size_value_ptr != nullptr) {
                graphics.in_system_jump_animation_size = boost::json::value_to<double>(*in_system_jump_animation_size_value_ptr);
            }

            const boost::json::value * initial_boot_message_value_ptr = graphics_object.if_contains("initial_boot_message");
            if (initial_boot_message_value_ptr != nullptr) {
                graphics.initial_boot_message = boost::json::value_to<std::string>(*initial_boot_message_value_ptr);
            }

            const boost::json::value * initial_zoom_factor_value_ptr = graphics_object.if_contains("initial_zoom_factor");
            if (initial_zoom_factor_value_ptr != nullptr) {
                graphics.initial_zoom_factor = boost::json::value_to<double>(*initial_zoom_factor_value_ptr);
            }

            const boost::json::value * jump_animation_shrink_value_ptr = graphics_object.if_contains("jump_animation_shrink");
            if (jump_animation_shrink_value_ptr != nullptr) {
                graphics.jump_animation_shrink = boost::json::value_to<double>(*jump_animation_shrink_value_ptr);
            }

            const boost::json::value * jump_gate_value_ptr = graphics_object.if_contains("jump_gate");
            if (jump_gate_value_ptr != nullptr) {
                graphics.jump_gate = boost::json::value_to<std::string>(*jump_gate_value_ptr);
            }

            const boost::json::value * jump_gate_size_value_ptr = graphics_object.if_contains("jump_gate_size");
            if (jump_gate_size_value_ptr != nullptr) {
                graphics.jump_gate_size = boost::json::value_to<double>(*jump_gate_size_value_ptr);
            }

            const boost::json::value * last_message_time_value_ptr = graphics_object.if_contains("last_message_time");
            if (last_message_time_value_ptr != nullptr) {
                graphics.last_message_time = boost::json::value_to<double>(*last_message_time_value_ptr);
            }

            const boost::json::value * light_cutoff_value_ptr = graphics_object.if_contains("light_cutoff");
            if (light_cutoff_value_ptr != nullptr) {
                graphics.light_cutoff = boost::json::value_to<double>(*light_cutoff_value_ptr);
            }

            const boost::json::value * light_optimal_intensity_value_ptr = graphics_object.if_contains("light_optimal_intensity");
            if (light_optimal_intensity_value_ptr != nullptr) {
                graphics.light_optimal_intensity = boost::json::value_to<double>(*light_optimal_intensity_value_ptr);
            }

            const boost::json::value * light_saturation_value_ptr = graphics_object.if_contains("light_saturation");
            if (light_saturation_value_ptr != nullptr) {
                graphics.light_saturation = boost::json::value_to<double>(*light_saturation_value_ptr);
            }

            const boost::json::value * lock_significant_target_box_value_ptr = graphics_object.if_contains("lock_significant_target_box");
            if (lock_significant_target_box_value_ptr != nullptr) {
                graphics.lock_significant_target_box = boost::json::value_to<bool>(*lock_significant_target_box_value_ptr);
            }

            const boost::json::value * lock_vertex_arrays_value_ptr = graphics_object.if_contains("lock_vertex_arrays");
            if (lock_vertex_arrays_value_ptr != nullptr) {
                graphics.lock_vertex_arrays = boost::json::value_to<bool>(*lock_vertex_arrays_value_ptr);
            }

            const boost::json::value * locklight_time_value_ptr = graphics_object.if_contains("locklight_time");
            if (locklight_time_value_ptr != nullptr) {
                graphics.locklight_time = boost::json::value_to<double>(*locklight_time_value_ptr);
            }

            const boost::json::value * logo_offset_value_ptr = graphics_object.if_contains("logo_offset");
            if (logo_offset_value_ptr != nullptr) {
                graphics.logo_offset = boost::json::value_to<double>(*logo_offset_value_ptr);
            }

            const boost::json::value * logo_offset_scale_value_ptr = graphics_object.if_contains("logo_offset_scale");
            if (logo_offset_scale_value_ptr != nullptr) {
                graphics.logo_offset_scale = boost::json::value_to<double>(*logo_offset_scale_value_ptr);
            }

            const boost::json::value * mac_shader_name_value_ptr = graphics_object.if_contains("mac_shader_name");
            if (mac_shader_name_value_ptr != nullptr) {
                graphics.mac_shader_name = boost::json::value_to<std::string>(*mac_shader_name_value_ptr);
            }

            const boost::json::value * main_menu_value_ptr = graphics_object.if_contains("main_menu");
            if (main_menu_value_ptr != nullptr) {
                graphics.main_menu = boost::json::value_to<bool>(*main_menu_value_ptr);
            }

            const boost::json::value * max_cubemap_size_value_ptr = graphics_object.if_contains("max_cubemap_size");
            if (max_cubemap_size_value_ptr != nullptr) {
                graphics.max_cubemap_size = boost::json::value_to<int>(*max_cubemap_size_value_ptr);
            }

            const boost::json::value * max_map_nodes_value_ptr = graphics_object.if_contains("max_map_nodes");
            if (max_map_nodes_value_ptr != nullptr) {
                graphics.max_map_nodes = boost::json::value_to<int>(*max_map_nodes_value_ptr);
            }

            const boost::json::value * max_movie_dimension_value_ptr = graphics_object.if_contains("max_movie_dimension");
            if (max_movie_dimension_value_ptr != nullptr) {
                graphics.max_movie_dimension = boost::json::value_to<int>(*max_movie_dimension_value_ptr);
            }

            const boost::json::value * max_texture_dimension_value_ptr = graphics_object.if_contains("max_texture_dimension");
            if (max_texture_dimension_value_ptr != nullptr) {
                graphics.max_texture_dimension = boost::json::value_to<int>(*max_texture_dimension_value_ptr);
            }

            const boost::json::value * mesh_animation_max_frames_skipped_value_ptr = graphics_object.if_contains("mesh_animation_max_frames_skipped");
            if (mesh_animation_max_frames_skipped_value_ptr != nullptr) {
                graphics.mesh_animation_max_frames_skipped = boost::json::value_to<double>(*mesh_animation_max_frames_skipped_value_ptr);
            }

            const boost::json::value * mesh_far_percent_value_ptr = graphics_object.if_contains("mesh_far_percent");
            if (mesh_far_percent_value_ptr != nullptr) {
                graphics.mesh_far_percent = boost::json::value_to<double>(*mesh_far_percent_value_ptr);
            }

            const boost::json::value * mipmap_detail_value_ptr = graphics_object.if_contains("mipmap_detail");
            if (mipmap_detail_value_ptr != nullptr) {
                graphics.mipmap_detail = boost::json::value_to<int>(*mipmap_detail_value_ptr);
            }

            const boost::json::value * missile_explosion_radius_mult_value_ptr = graphics_object.if_contains("missile_explosion_radius_mult");
            if (missile_explosion_radius_mult_value_ptr != nullptr) {
                graphics.missile_explosion_radius_mult = boost::json::value_to<double>(*missile_explosion_radius_mult_value_ptr);
            }

            const boost::json::value * missile_sparkle_value_ptr = graphics_object.if_contains("missile_sparkle");
            if (missile_sparkle_value_ptr != nullptr) {
                graphics.missile_sparkle = boost::json::value_to<bool>(*missile_sparkle_value_ptr);
            }

            const boost::json::value * model_detail_value_ptr = graphics_object.if_contains("model_detail");
            if (model_detail_value_ptr != nullptr) {
                graphics.model_detail = boost::json::value_to<double>(*model_detail_value_ptr);
            }

            const boost::json::value * multi_texture_glow_maps_value_ptr = graphics_object.if_contains("multi_texture_glow_maps");
            if (multi_texture_glow_maps_value_ptr != nullptr) {
                graphics.multi_texture_glow_maps = boost::json::value_to<bool>(*multi_texture_glow_maps_value_ptr);
            }

            const boost::json::value * near_stars_alpha_value_ptr = graphics_object.if_contains("near_stars_alpha");
            if (near_stars_alpha_value_ptr != nullptr) {
                graphics.near_stars_alpha = boost::json::value_to<bool>(*near_stars_alpha_value_ptr);
            }

            const boost::json::value * near_stars_alpha_blend_value_ptr = graphics_object.if_contains("near_stars_alpha_blend");
            if (near_stars_alpha_blend_value_ptr != nullptr) {
                graphics.near_stars_alpha_blend = boost::json::value_to<bool>(*near_stars_alpha_blend_value_ptr);
            }

            const boost::json::value * near_stars_sprite_size_value_ptr = graphics_object.if_contains("near_stars_sprite_size");
            if (near_stars_sprite_size_value_ptr != nullptr) {
                graphics.near_stars_sprite_size = boost::json::value_to<double>(*near_stars_sprite_size_value_ptr);
            }

            const boost::json::value * near_stars_sprite_texture_value_ptr = graphics_object.if_contains("near_stars_sprite_texture");
            if (near_stars_sprite_texture_value_ptr != nullptr) {
                graphics.near_stars_sprite_texture = boost::json::value_to<std::string>(*near_stars_sprite_texture_value_ptr);
            }

            const boost::json::value * num_far_stars_value_ptr = graphics_object.if_contains("num_far_stars");
            if (num_far_stars_value_ptr != nullptr) {
                graphics.num_far_stars = boost::json::value_to<int>(*num_far_stars_value_ptr);
            }

            const boost::json::value * num_lights_value_ptr = graphics_object.if_contains("num_lights");
            if (num_lights_value_ptr != nullptr) {
                graphics.num_lights = boost::json::value_to<int>(*num_lights_value_ptr);
            }

            const boost::json::value * num_messages_value_ptr = graphics_object.if_contains("num_messages");
            if (num_messages_value_ptr != nullptr) {
                graphics.num_messages = boost::json::value_to<int>(*num_messages_value_ptr);
            }

            const boost::json::value * num_near_stars_value_ptr = graphics_object.if_contains("num_near_stars");
            if (num_near_stars_value_ptr != nullptr) {
                graphics.num_near_stars = boost::json::value_to<int>(*num_near_stars_value_ptr);
            }

            const boost::json::value * num_times_to_draw_shine_value_ptr = graphics_object.if_contains("num_times_to_draw_shine");
            if (num_times_to_draw_shine_value_ptr != nullptr) {
                graphics.num_times_to_draw_shine = boost::json::value_to<int>(*num_times_to_draw_shine_value_ptr);
            }

            const boost::json::value * offset_sprites_by_pos_value_ptr = graphics_object.if_contains("offset_sprites_by_pos");
            if (offset_sprites_by_pos_value_ptr != nullptr) {
                graphics.offset_sprites_by_pos = boost::json::value_to<bool>(*offset_sprites_by_pos_value_ptr);
            }

            const boost::json::value * only_scanner_modes_static_value_ptr = graphics_object.if_contains("only_scanner_modes_static");
            if (only_scanner_modes_static_value_ptr != nullptr) {
                graphics.only_scanner_modes_static = boost::json::value_to<bool>(*only_scanner_modes_static_value_ptr);
            }

            const boost::json::value * only_stretch_in_warp_value_ptr = graphics_object.if_contains("only_stretch_in_warp");
            if (only_stretch_in_warp_value_ptr != nullptr) {
                graphics.only_stretch_in_warp = boost::json::value_to<bool>(*only_stretch_in_warp_value_ptr);
            }

            const boost::json::value * open_picker_categories_value_ptr = graphics_object.if_contains("open_picker_categories");
            if (open_picker_categories_value_ptr != nullptr) {
                graphics.open_picker_categories = boost::json::value_to<bool>(*open_picker_categories_value_ptr);
            }

            const boost::json::value * optimize_vertex_arrays_value_ptr = graphics_object.if_contains("optimize_vertex_arrays");
            if (optimize_vertex_arrays_value_ptr != nullptr) {
                graphics.optimize_vertex_arrays = boost::json::value_to<bool>(*optimize_vertex_arrays_value_ptr);
            }

            const boost::json::value * optimize_vertex_condition_value_ptr = graphics_object.if_contains("optimize_vertex_condition");
            if (optimize_vertex_condition_value_ptr != nullptr) {
                graphics.optimize_vertex_condition = boost::json::value_to<double>(*optimize_vertex_condition_value_ptr);
            }

            const boost::json::value * pan_on_auto_value_ptr = graphics_object.if_contains("pan_on_auto");
            if (pan_on_auto_value_ptr != nullptr) {
                graphics.pan_on_auto = boost::json::value_to<bool>(*pan_on_auto_value_ptr);
            }

            const boost::json::value * panel_smooth_texture_value_ptr = graphics_object.if_contains("panel_smooth_texture");
            if (panel_smooth_texture_value_ptr != nullptr) {
                graphics.panel_smooth_texture = boost::json::value_to<bool>(*panel_smooth_texture_value_ptr);
            }

            const boost::json::value * percent_afterburner_color_change_value_ptr = graphics_object.if_contains("percent_afterburner_color_change");
            if (percent_afterburner_color_change_value_ptr != nullptr) {
                graphics.percent_afterburner_color_change = boost::json::value_to<double>(*percent_afterburner_color_change_value_ptr);
            }

            const boost::json::value * percent_halo_fade_in_value_ptr = graphics_object.if_contains("percent_halo_fade_in");
            if (percent_halo_fade_in_value_ptr != nullptr) {
                graphics.percent_halo_fade_in = boost::json::value_to<double>(*percent_halo_fade_in_value_ptr);
            }

            const boost::json::value * percent_shockwave_value_ptr = graphics_object.if_contains("percent_shockwave");
            if (percent_shockwave_value_ptr != nullptr) {
                graphics.percent_shockwave = boost::json::value_to<double>(*percent_shockwave_value_ptr);
            }

            const boost::json::value * per_pixel_lighting_value_ptr = graphics_object.if_contains("per_pixel_lighting");
            if (per_pixel_lighting_value_ptr != nullptr) {
                graphics.per_pixel_lighting = boost::json::value_to<bool>(*per_pixel_lighting_value_ptr);
            }

            const boost::json::value * planet_detail_level_value_ptr = graphics_object.if_contains("planet_detail_level");
            if (planet_detail_level_value_ptr != nullptr) {
                graphics.planet_detail_level = boost::json::value_to<int>(*planet_detail_level_value_ptr);
            }

            const boost::json::value * pot_video_textures_value_ptr = graphics_object.if_contains("pot_video_textures");
            if (pot_video_textures_value_ptr != nullptr) {
                graphics.pot_video_textures = boost::json::value_to<bool>(*pot_video_textures_value_ptr);
            }

            const boost::json::value * precull_dist_value_ptr = graphics_object.if_contains("precull_dist");
            if (precull_dist_value_ptr != nullptr) {
                graphics.precull_dist = boost::json::value_to<double>(*precull_dist_value_ptr);
            }

            const boost::json::value * print_ecm_status_value_ptr = graphics_object.if_contains("print_ecm_status");
            if (print_ecm_status_value_ptr != nullptr) {
                graphics.print_ecm_status = boost::json::value_to<bool>(*print_ecm_status_value_ptr);
            }

            const boost::json::value * rect_textures_value_ptr = graphics_object.if_contains("rect_textures");
            if (rect_textures_value_ptr != nullptr) {
                graphics.rect_textures = boost::json::value_to<bool>(*rect_textures_value_ptr);
            }

            const boost::json::value * reduced_vdus_width_value_ptr = graphics_object.if_contains("reduced_vdus_width");
            if (reduced_vdus_width_value_ptr != nullptr) {
                graphics.reduced_vdus_width = boost::json::value_to<double>(*reduced_vdus_width_value_ptr);
            }

            const boost::json::value * reduced_vdus_height_value_ptr = graphics_object.if_contains("reduced_vdus_height");
            if (reduced_vdus_height_value_ptr != nullptr) {
                graphics.reduced_vdus_height = boost::json::value_to<double>(*reduced_vdus_height_value_ptr);
            }

            const boost::json::value * reflection_value_ptr = graphics_object.if_contains("reflection");
            if (reflection_value_ptr != nullptr) {
                graphics.reflection = boost::json::value_to<bool>(*reflection_value_ptr);
            }

            const boost::json::value * reflectivity_value_ptr = graphics_object.if_contains("reflectivity");
            if (reflectivity_value_ptr != nullptr) {
                graphics.reflectivity = boost::json::value_to<double>(*reflectivity_value_ptr);
            }

            const boost::json::value * resolution_x_value_ptr = graphics_object.if_contains("resolution_x");
            if (resolution_x_value_ptr != nullptr) {
                graphics.resolution_x = boost::json::value_to<int>(*resolution_x_value_ptr);
            }

            const boost::json::value * resolution_y_value_ptr = graphics_object.if_contains("resolution_y");
            if (resolution_y_value_ptr != nullptr) {
                graphics.resolution_y = boost::json::value_to<int>(*resolution_y_value_ptr);
            }

            const boost::json::value * rgb_pixel_format_value_ptr = graphics_object.if_contains("rgb_pixel_format");
            if (rgb_pixel_format_value_ptr != nullptr) {
                graphics.rgb_pixel_format = boost::json::value_to<std::string>(*rgb_pixel_format_value_ptr);
            }

            const boost::json::value * screen_value_ptr = graphics_object.if_contains("screen");
            if (screen_value_ptr != nullptr) {
                graphics.screen = boost::json::value_to<int>(*screen_value_ptr);
            }

            const boost::json::value * s3tc_value_ptr = graphics_object.if_contains("s3tc");
            if (s3tc_value_ptr != nullptr) {
                graphics.s3tc = boost::json::value_to<bool>(*s3tc_value_ptr);
            }

            const boost::json::value * sector_map_ortho_view_value_ptr = graphics_object.if_contains("sector_map_ortho_view");
            if (sector_map_ortho_view_value_ptr != nullptr) {
                graphics.sector_map_ortho_view = boost::json::value_to<bool>(*sector_map_ortho_view_value_ptr);
            }

            const boost::json::value * separate_specular_color_value_ptr = graphics_object.if_contains("separate_specular_color");
            if (separate_specular_color_value_ptr != nullptr) {
                graphics.separate_specular_color = boost::json::value_to<bool>(*separate_specular_color_value_ptr);
            }

            const boost::json::value * set_raster_text_color_value_ptr = graphics_object.if_contains("set_raster_text_color");
            if (set_raster_text_color_value_ptr != nullptr) {
                graphics.set_raster_text_color = boost::json::value_to<bool>(*set_raster_text_color_value_ptr);
            }

            const boost::json::value * shader_name_value_ptr = graphics_object.if_contains("shader_name");
            if (shader_name_value_ptr != nullptr) {
                graphics.shader_name = boost::json::value_to<std::string>(*shader_name_value_ptr);
            }

            const boost::json::value * shake_limit_value_ptr = graphics_object.if_contains("shake_limit");
            if (shake_limit_value_ptr != nullptr) {
                graphics.shake_limit = boost::json::value_to<double>(*shake_limit_value_ptr);
            }

            const boost::json::value * shake_magnitude_value_ptr = graphics_object.if_contains("shake_magnitude");
            if (shake_magnitude_value_ptr != nullptr) {
                graphics.shake_magnitude = boost::json::value_to<double>(*shake_magnitude_value_ptr);
            }

            const boost::json::value * shake_reduction_value_ptr = graphics_object.if_contains("shake_reduction");
            if (shake_reduction_value_ptr != nullptr) {
                graphics.shake_reduction = boost::json::value_to<double>(*shake_reduction_value_ptr);
            }

            const boost::json::value * shake_speed_value_ptr = graphics_object.if_contains("shake_speed");
            if (shake_speed_value_ptr != nullptr) {
                graphics.shake_speed = boost::json::value_to<double>(*shake_speed_value_ptr);
            }

            const boost::json::value * shared_vertex_arrays_value_ptr = graphics_object.if_contains("shared_vertex_arrays");
            if (shared_vertex_arrays_value_ptr != nullptr) {
                graphics.shared_vertex_arrays = boost::json::value_to<bool>(*shared_vertex_arrays_value_ptr);
            }

            const boost::json::value * shield_detail_level_value_ptr = graphics_object.if_contains("shield_detail_level");
            if (shield_detail_level_value_ptr != nullptr) {
                graphics.shield_detail_level = boost::json::value_to<int>(*shield_detail_level_value_ptr);
            }

            const boost::json::value * shield_flash_animation_value_ptr = graphics_object.if_contains("shield_flash_animation");
            if (shield_flash_animation_value_ptr != nullptr) {
                graphics.shield_flash_animation = boost::json::value_to<std::string>(*shield_flash_animation_value_ptr);
            }

            const boost::json::value * shield_technique_value_ptr = graphics_object.if_contains("shield_technique");
            if (shield_technique_value_ptr != nullptr) {
                graphics.shield_technique = boost::json::value_to<std::string>(*shield_technique_value_ptr);
            }

            const boost::json::value * shield_texture_value_ptr = graphics_object.if_contains("shield_texture");
            if (shield_texture_value_ptr != nullptr) {
                graphics.shield_texture = boost::json::value_to<std::string>(*shield_texture_value_ptr);
            }

            const boost::json::value * shockwave_animation_value_ptr = graphics_object.if_contains("shockwave_animation");
            if (shockwave_animation_value_ptr != nullptr) {
                graphics.shockwave_animation = boost::json::value_to<std::string>(*shockwave_animation_value_ptr);
            }

            const boost::json::value * shockwave_growth_value_ptr = graphics_object.if_contains("shockwave_growth");
            if (shockwave_growth_value_ptr != nullptr) {
                graphics.shockwave_growth = boost::json::value_to<double>(*shockwave_growth_value_ptr);
            }

            const boost::json::value * shove_camera_down_value_ptr = graphics_object.if_contains("shove_camera_down");
            if (shove_camera_down_value_ptr != nullptr) {
                graphics.shove_camera_down = boost::json::value_to<double>(*shove_camera_down_value_ptr);
            }

            const boost::json::value * show_respawn_text_value_ptr = graphics_object.if_contains("show_respawn_text");
            if (show_respawn_text_value_ptr != nullptr) {
                graphics.show_respawn_text = boost::json::value_to<bool>(*show_respawn_text_value_ptr);
            }

            const boost::json::value * show_stardate_value_ptr = graphics_object.if_contains("show_stardate");
            if (show_stardate_value_ptr != nullptr) {
                graphics.show_stardate = boost::json::value_to<bool>(*show_stardate_value_ptr);
            }

            const boost::json::value * smooth_lines_value_ptr = graphics_object.if_contains("smooth_lines");
            if (smooth_lines_value_ptr != nullptr) {
                graphics.smooth_lines = boost::json::value_to<bool>(*smooth_lines_value_ptr);
            }

            const boost::json::value * smooth_points_value_ptr = graphics_object.if_contains("smooth_points");
            if (smooth_points_value_ptr != nullptr) {
                graphics.smooth_points = boost::json::value_to<bool>(*smooth_points_value_ptr);
            }

            const boost::json::value * smooth_shade_value_ptr = graphics_object.if_contains("smooth_shade");
            if (smooth_shade_value_ptr != nullptr) {
                graphics.smooth_shade = boost::json::value_to<bool>(*smooth_shade_value_ptr);
            }

            const boost::json::value * sparkle_absolute_speed_value_ptr = graphics_object.if_contains("sparkle_absolute_speed");
            if (sparkle_absolute_speed_value_ptr != nullptr) {
                graphics.sparkle_absolute_speed = boost::json::value_to<double>(*sparkle_absolute_speed_value_ptr);
            }

            const boost::json::value * sparkle_engine_size_relative_to_ship_value_ptr = graphics_object.if_contains("sparkle_engine_size_relative_to_ship");
            if (sparkle_engine_size_relative_to_ship_value_ptr != nullptr) {
                graphics.sparkle_engine_size_relative_to_ship = boost::json::value_to<double>(*sparkle_engine_size_relative_to_ship_value_ptr);
            }

            const boost::json::value * sparkle_rate_value_ptr = graphics_object.if_contains("sparkle_rate");
            if (sparkle_rate_value_ptr != nullptr) {
                graphics.sparkle_rate = boost::json::value_to<double>(*sparkle_rate_value_ptr);
            }

            const boost::json::value * specmap_with_reflection_value_ptr = graphics_object.if_contains("specmap_with_reflection");
            if (specmap_with_reflection_value_ptr != nullptr) {
                graphics.specmap_with_reflection = boost::json::value_to<bool>(*specmap_with_reflection_value_ptr);
            }

            const boost::json::value * splash_audio_value_ptr = graphics_object.if_contains("splash_audio");
            if (splash_audio_value_ptr != nullptr) {
                graphics.splash_audio = boost::json::value_to<std::string>(*splash_audio_value_ptr);
            }

            const boost::json::value * splash_screen_value_ptr = graphics_object.if_contains("splash_screen");
            if (splash_screen_value_ptr != nullptr) {
                graphics.splash_screen = boost::json::value_to<std::string>(*splash_screen_value_ptr);
            }

            const boost::json::value * split_dead_subunits_value_ptr = graphics_object.if_contains("split_dead_subunits");
            if (split_dead_subunits_value_ptr != nullptr) {
                graphics.split_dead_subunits = boost::json::value_to<bool>(*split_dead_subunits_value_ptr);
            }

            const boost::json::value * star_allowable_sectors_value_ptr = graphics_object.if_contains("star_allowable_sectors");
            if (star_allowable_sectors_value_ptr != nullptr) {
                graphics.star_allowable_sectors = boost::json::value_to<std::string>(*star_allowable_sectors_value_ptr);
            }

            const boost::json::value * star_blend_value_ptr = graphics_object.if_contains("star_blend");
            if (star_blend_value_ptr != nullptr) {
                graphics.star_blend = boost::json::value_to<bool>(*star_blend_value_ptr);
            }

            const boost::json::value * star_color_average_value_ptr = graphics_object.if_contains("star_color_average");
            if (star_color_average_value_ptr != nullptr) {
                graphics.star_color_average = boost::json::value_to<double>(*star_color_average_value_ptr);
            }

            const boost::json::value * star_color_cutoff_value_ptr = graphics_object.if_contains("star_color_cutoff");
            if (star_color_cutoff_value_ptr != nullptr) {
                graphics.star_color_cutoff = boost::json::value_to<double>(*star_color_cutoff_value_ptr);
            }

            const boost::json::value * star_color_increment_value_ptr = graphics_object.if_contains("star_color_increment");
            if (star_color_increment_value_ptr != nullptr) {
                graphics.star_color_increment = boost::json::value_to<double>(*star_color_increment_value_ptr);
            }

            const boost::json::value * star_color_power_value_ptr = graphics_object.if_contains("star_color_power");
            if (star_color_power_value_ptr != nullptr) {
                graphics.star_color_power = boost::json::value_to<double>(*star_color_power_value_ptr);
            }

            const boost::json::value * star_body_radius_value_ptr = graphics_object.if_contains("star_body_radius");
            if (star_body_radius_value_ptr != nullptr) {
                graphics.star_body_radius = boost::json::value_to<double>(*star_body_radius_value_ptr);
            }

            const boost::json::value * star_glow_radius_value_ptr = graphics_object.if_contains("star_glow_radius");
            if (star_glow_radius_value_ptr != nullptr) {
                graphics.star_glow_radius = boost::json::value_to<double>(*star_glow_radius_value_ptr);
            }

            const boost::json::value * star_lumin_scale_value_ptr = graphics_object.if_contains("star_lumin_scale");
            if (star_lumin_scale_value_ptr != nullptr) {
                graphics.star_lumin_scale = boost::json::value_to<double>(*star_lumin_scale_value_ptr);
            }

            const boost::json::value * star_minimum_color_value_value_ptr = graphics_object.if_contains("star_minimum_color_value");
            if (star_minimum_color_value_value_ptr != nullptr) {
                graphics.star_minimum_color_value = boost::json::value_to<double>(*star_minimum_color_value_value_ptr);
            }

            const boost::json::value * star_overlap_value_ptr = graphics_object.if_contains("star_overlap");
            if (star_overlap_value_ptr != nullptr) {
                graphics.star_overlap = boost::json::value_to<double>(*star_overlap_value_ptr);
            }

            const boost::json::value * star_shine_value_ptr = graphics_object.if_contains("star_shine");
            if (star_shine_value_ptr != nullptr) {
                graphics.star_shine = boost::json::value_to<std::string>(*star_shine_value_ptr);
            }

            const boost::json::value * star_spread_attenuation_value_ptr = graphics_object.if_contains("star_spread_attenuation");
            if (star_spread_attenuation_value_ptr != nullptr) {
                graphics.star_spread_attenuation = boost::json::value_to<double>(*star_spread_attenuation_value_ptr);
            }

            const boost::json::value * star_spreading_value_ptr = graphics_object.if_contains("star_spreading");
            if (star_spreading_value_ptr != nullptr) {
                graphics.star_spreading = boost::json::value_to<double>(*star_spreading_value_ptr);
            }

            const boost::json::value * star_streaks_value_ptr = graphics_object.if_contains("star_streaks");
            if (star_streaks_value_ptr != nullptr) {
                graphics.star_streaks = boost::json::value_to<bool>(*star_streaks_value_ptr);
            }

            const boost::json::value * stars_alpha_test_cutoff_value_ptr = graphics_object.if_contains("stars_alpha_test_cutoff");
            if (stars_alpha_test_cutoff_value_ptr != nullptr) {
                graphics.stars_alpha_test_cutoff = boost::json::value_to<double>(*stars_alpha_test_cutoff_value_ptr);
            }

            const boost::json::value * stars_dont_move_value_ptr = graphics_object.if_contains("stars_dont_move");
            if (stars_dont_move_value_ptr != nullptr) {
                graphics.stars_dont_move = boost::json::value_to<bool>(*stars_dont_move_value_ptr);
            }

            const boost::json::value * startup_cockpit_view_value_ptr = graphics_object.if_contains("startup_cockpit_view");
            if (startup_cockpit_view_value_ptr != nullptr) {
                graphics.startup_cockpit_view = boost::json::value_to<std::string>(*startup_cockpit_view_value_ptr);
            }

            const boost::json::value * stretch_bolts_value_ptr = graphics_object.if_contains("stretch_bolts");
            if (stretch_bolts_value_ptr != nullptr) {
                graphics.stretch_bolts = boost::json::value_to<double>(*stretch_bolts_value_ptr);
            }

            const boost::json::value * switch_cockpit_to_default_on_unit_switch_value_ptr = graphics_object.if_contains("switch_cockpit_to_default_on_unit_switch");
            if (switch_cockpit_to_default_on_unit_switch_value_ptr != nullptr) {
                graphics.switch_cockpit_to_default_on_unit_switch = boost::json::value_to<bool>(*switch_cockpit_to_default_on_unit_switch_value_ptr);
            }

            const boost::json::value * system_map_ortho_view_value_ptr = graphics_object.if_contains("system_map_ortho_view");
            if (system_map_ortho_view_value_ptr != nullptr) {
                graphics.system_map_ortho_view = boost::json::value_to<bool>(*system_map_ortho_view_value_ptr);
            }

            const boost::json::value * technique_set_value_ptr = graphics_object.if_contains("technique_set");
            if (technique_set_value_ptr != nullptr) {
                graphics.technique_set = boost::json::value_to<std::string>(*technique_set_value_ptr);
            }

            const boost::json::value * text_delay_value_ptr = graphics_object.if_contains("text_delay");
            if (text_delay_value_ptr != nullptr) {
                graphics.text_delay = boost::json::value_to<double>(*text_delay_value_ptr);
            }

            const boost::json::value * text_display_lists_value_ptr = graphics_object.if_contains("text_display_lists");
            if (text_display_lists_value_ptr != nullptr) {
                graphics.text_display_lists = boost::json::value_to<bool>(*text_display_lists_value_ptr);
            }

            const boost::json::value * text_speed_value_ptr = graphics_object.if_contains("text_speed");
            if (text_speed_value_ptr != nullptr) {
                graphics.text_speed = boost::json::value_to<double>(*text_speed_value_ptr);
            }

            const boost::json::value * texture_value_ptr = graphics_object.if_contains("texture");
            if (texture_value_ptr != nullptr) {
                graphics.texture = boost::json::value_to<std::string>(*texture_value_ptr);
            }

            const boost::json::value * texture_compression_value_ptr = graphics_object.if_contains("texture_compression");
            if (texture_compression_value_ptr != nullptr) {
                graphics.texture_compression = boost::json::value_to<int>(*texture_compression_value_ptr);
            }

            const boost::json::value * torque_star_streak_scale_value_ptr = graphics_object.if_contains("torque_star_streak_scale");
            if (torque_star_streak_scale_value_ptr != nullptr) {
                graphics.torque_star_streak_scale = boost::json::value_to<double>(*torque_star_streak_scale_value_ptr);
            }

            const boost::json::value * unit_switch_cockpit_change_value_ptr = graphics_object.if_contains("unit_switch_cockpit_change");
            if (unit_switch_cockpit_change_value_ptr != nullptr) {
                graphics.unit_switch_cockpit_change = boost::json::value_to<bool>(*unit_switch_cockpit_change_value_ptr);
            }

            const boost::json::value * unprintable_faction_extension_value_ptr = graphics_object.if_contains("unprintable_faction_extension");
            if (unprintable_faction_extension_value_ptr != nullptr) {
                graphics.unprintable_faction_extension = boost::json::value_to<std::string>(*unprintable_faction_extension_value_ptr);
            }

            const boost::json::value * unprintable_factions_value_ptr = graphics_object.if_contains("unprintable_factions");
            if (unprintable_factions_value_ptr != nullptr) {
                graphics.unprintable_factions = boost::json::value_to<std::string>(*unprintable_factions_value_ptr);
            }

            const boost::json::value * update_nav_after_jump_value_ptr = graphics_object.if_contains("update_nav_after_jump");
            if (update_nav_after_jump_value_ptr != nullptr) {
                graphics.update_nav_after_jump = boost::json::value_to<bool>(*update_nav_after_jump_value_ptr);
            }

            const boost::json::value * use_animations_value_ptr = graphics_object.if_contains("use_animations");
            if (use_animations_value_ptr != nullptr) {
                graphics.use_animations = boost::json::value_to<bool>(*use_animations_value_ptr);
            }

            const boost::json::value * use_detail_texture_value_ptr = graphics_object.if_contains("use_detail_texture");
            if (use_detail_texture_value_ptr != nullptr) {
                graphics.use_detail_texture = boost::json::value_to<bool>(*use_detail_texture_value_ptr);
            }

            const boost::json::value * use_faction_gui_background_color_value_ptr = graphics_object.if_contains("use_faction_gui_background_color");
            if (use_faction_gui_background_color_value_ptr != nullptr) {
                graphics.use_faction_gui_background_color = boost::json::value_to<bool>(*use_faction_gui_background_color_value_ptr);
            }

            const boost::json::value * use_logos_value_ptr = graphics_object.if_contains("use_logos");
            if (use_logos_value_ptr != nullptr) {
                graphics.use_logos = boost::json::value_to<bool>(*use_logos_value_ptr);
            }

            const boost::json::value * use_planet_atmosphere_value_ptr = graphics_object.if_contains("use_planet_atmosphere");
            if (use_planet_atmosphere_value_ptr != nullptr) {
                graphics.use_planet_atmosphere = boost::json::value_to<bool>(*use_planet_atmosphere_value_ptr);
            }

            const boost::json::value * use_planet_fog_value_ptr = graphics_object.if_contains("use_planet_fog");
            if (use_planet_fog_value_ptr != nullptr) {
                graphics.use_planet_fog = boost::json::value_to<bool>(*use_planet_fog_value_ptr);
            }

            const boost::json::value * use_star_coords_value_ptr = graphics_object.if_contains("use_star_coords");
            if (use_star_coords_value_ptr != nullptr) {
                graphics.use_star_coords = boost::json::value_to<bool>(*use_star_coords_value_ptr);
            }

            const boost::json::value * use_textures_value_ptr = graphics_object.if_contains("use_textures");
            if (use_textures_value_ptr != nullptr) {
                graphics.use_textures = boost::json::value_to<bool>(*use_textures_value_ptr);
            }

            const boost::json::value * use_planet_textures_value_ptr = graphics_object.if_contains("use_planet_textures");
            if (use_planet_textures_value_ptr != nullptr) {
                graphics.use_planet_textures = boost::json::value_to<bool>(*use_planet_textures_value_ptr);
            }

            const boost::json::value * use_ship_textures_value_ptr = graphics_object.if_contains("use_ship_textures");
            if (use_ship_textures_value_ptr != nullptr) {
                graphics.use_ship_textures = boost::json::value_to<bool>(*use_ship_textures_value_ptr);
            }

            const boost::json::value * use_videos_value_ptr = graphics_object.if_contains("use_videos");
            if (use_videos_value_ptr != nullptr) {
                graphics.use_videos = boost::json::value_to<bool>(*use_videos_value_ptr);
            }

            const boost::json::value * use_vs_sprites_value_ptr = graphics_object.if_contains("use_vs_sprites");
            if (use_vs_sprites_value_ptr != nullptr) {
                graphics.use_vs_sprites = boost::json::value_to<bool>(*use_vs_sprites_value_ptr);
            }

            const boost::json::value * use_wireframe_value_ptr = graphics_object.if_contains("use_wireframe");
            if (use_wireframe_value_ptr != nullptr) {
                graphics.use_wireframe = boost::json::value_to<bool>(*use_wireframe_value_ptr);
            }

            const boost::json::value * vbo_value_ptr = graphics_object.if_contains("vbo");
            if (vbo_value_ptr != nullptr) {
                graphics.vbo = boost::json::value_to<bool>(*vbo_value_ptr);
            }

            const boost::json::value * vdu_builtin_shields_value_ptr = graphics_object.if_contains("vdu_builtin_shields");
            if (vdu_builtin_shields_value_ptr != nullptr) {
                graphics.vdu_builtin_shields = boost::json::value_to<bool>(*vdu_builtin_shields_value_ptr);
            }

            const boost::json::value * velocity_star_streak_max_value_ptr = graphics_object.if_contains("velocity_star_streak_max");
            if (velocity_star_streak_max_value_ptr != nullptr) {
                graphics.velocity_star_streak_max = boost::json::value_to<double>(*velocity_star_streak_max_value_ptr);
            }

            const boost::json::value * velocity_star_streak_min_value_ptr = graphics_object.if_contains("velocity_star_streak_min");
            if (velocity_star_streak_min_value_ptr != nullptr) {
                graphics.velocity_star_streak_min = boost::json::value_to<double>(*velocity_star_streak_min_value_ptr);
            }

            const boost::json::value * velocity_star_streak_scale_value_ptr = graphics_object.if_contains("velocity_star_streak_scale");
            if (velocity_star_streak_scale_value_ptr != nullptr) {
                graphics.velocity_star_streak_scale = boost::json::value_to<double>(*velocity_star_streak_scale_value_ptr);
            }

            const boost::json::value * vista_draw_stars_value_ptr = graphics_object.if_contains("vista_draw_stars");
            if (vista_draw_stars_value_ptr != nullptr) {
                graphics.vista_draw_stars = boost::json::value_to<bool>(*vista_draw_stars_value_ptr);
            }

            const boost::json::value * warp_shake_magnitude_value_ptr = graphics_object.if_contains("warp_shake_magnitude");
            if (warp_shake_magnitude_value_ptr != nullptr) {
                graphics.warp_shake_magnitude = boost::json::value_to<double>(*warp_shake_magnitude_value_ptr);
            }

            const boost::json::value * warp_shake_ref_value_ptr = graphics_object.if_contains("warp_shake_ref");
            if (warp_shake_ref_value_ptr != nullptr) {
                graphics.warp_shake_ref = boost::json::value_to<double>(*warp_shake_ref_value_ptr);
            }

            const boost::json::value * warp_shake_speed_value_ptr = graphics_object.if_contains("warp_shake_speed");
            if (warp_shake_speed_value_ptr != nullptr) {
                graphics.warp_shake_speed = boost::json::value_to<double>(*warp_shake_speed_value_ptr);
            }

            const boost::json::value * warp_stretch_cutoff_value_ptr = graphics_object.if_contains("warp_stretch_cutoff");
            if (warp_stretch_cutoff_value_ptr != nullptr) {
                graphics.warp_stretch_cutoff = boost::json::value_to<double>(*warp_stretch_cutoff_value_ptr);
            }

            const boost::json::value * warp_stretch_max_value_ptr = graphics_object.if_contains("warp_stretch_max");
            if (warp_stretch_max_value_ptr != nullptr) {
                graphics.warp_stretch_max = boost::json::value_to<double>(*warp_stretch_max_value_ptr);
            }

            const boost::json::value * warp_stretch_max_region0_speed_value_ptr = graphics_object.if_contains("warp_stretch_max_region0_speed");
            if (warp_stretch_max_region0_speed_value_ptr != nullptr) {
                graphics.warp_stretch_max_region0_speed = boost::json::value_to<double>(*warp_stretch_max_region0_speed_value_ptr);
            }

            const boost::json::value * warp_stretch_max_speed_value_ptr = graphics_object.if_contains("warp_stretch_max_speed");
            if (warp_stretch_max_speed_value_ptr != nullptr) {
                graphics.warp_stretch_max_speed = boost::json::value_to<double>(*warp_stretch_max_speed_value_ptr);
            }

            const boost::json::value * warp_stretch_region0_max_value_ptr = graphics_object.if_contains("warp_stretch_region0_max");
            if (warp_stretch_region0_max_value_ptr != nullptr) {
                graphics.warp_stretch_region0_max = boost::json::value_to<double>(*warp_stretch_region0_max_value_ptr);
            }

            const boost::json::value * warp_trail_value_ptr = graphics_object.if_contains("warp_trail");
            if (warp_trail_value_ptr != nullptr) {
                graphics.warp_trail = boost::json::value_to<bool>(*warp_trail_value_ptr);
            }

            const boost::json::value * warp_trail_time_value_ptr = graphics_object.if_contains("warp_trail_time");
            if (warp_trail_time_value_ptr != nullptr) {
                graphics.warp_trail_time = boost::json::value_to<double>(*warp_trail_time_value_ptr);
            }

            const boost::json::value * weapon_gamma_value_ptr = graphics_object.if_contains("weapon_gamma");
            if (weapon_gamma_value_ptr != nullptr) {
                graphics.weapon_gamma = boost::json::value_to<double>(*weapon_gamma_value_ptr);
            }

            const boost::json::value * weapon_xyscale_value_ptr = graphics_object.if_contains("weapon_xyscale");
            if (weapon_xyscale_value_ptr != nullptr) {
                graphics.weapon_xyscale = boost::json::value_to<double>(*weapon_xyscale_value_ptr);
            }

            const boost::json::value * weapon_zscale_value_ptr = graphics_object.if_contains("weapon_zscale");
            if (weapon_zscale_value_ptr != nullptr) {
                graphics.weapon_zscale = boost::json::value_to<double>(*weapon_zscale_value_ptr);
            }

            const boost::json::value * wheel_zoom_amount_value_ptr = graphics_object.if_contains("wheel_zoom_amount");
            if (wheel_zoom_amount_value_ptr != nullptr) {
                graphics.wheel_zoom_amount = boost::json::value_to<double>(*wheel_zoom_amount_value_ptr);
            }

            const boost::json::value * wormhole_unit_value_ptr = graphics_object.if_contains("wormhole_unit");
            if (wormhole_unit_value_ptr != nullptr) {
                graphics.wormhole_unit = boost::json::value_to<std::string>(*wormhole_unit_value_ptr);
            }

            const boost::json::value * z_pixel_format_value_ptr = graphics_object.if_contains("z_pixel_format");
            if (z_pixel_format_value_ptr != nullptr) {
                graphics.z_pixel_format = boost::json::value_to<int>(*z_pixel_format_value_ptr);
            }

            const boost::json::value * zfar_value_ptr = graphics_object.if_contains("zfar");
            if (zfar_value_ptr != nullptr) {
                graphics.zfar = boost::json::value_to<double>(*zfar_value_ptr);
            }

            const boost::json::value * znear_value_ptr = graphics_object.if_contains("znear");
            if (znear_value_ptr != nullptr) {
                graphics.znear = boost::json::value_to<double>(*znear_value_ptr);
            }

        const boost::json::value * bases_value_ptr = graphics_object.if_contains("bases");
        if (bases_value_ptr != nullptr) {
            boost::json::object bases_object = bases_value_ptr->get_object();
            const boost::json::value * alpha_test_cutoff_value_ptr = bases_object.if_contains("alpha_test_cutoff");
            if (alpha_test_cutoff_value_ptr != nullptr) {
                graphics.bases.alpha_test_cutoff = boost::json::value_to<double>(*alpha_test_cutoff_value_ptr);
            }

            const boost::json::value * draw_location_borders_value_ptr = bases_object.if_contains("draw_location_borders");
            if (draw_location_borders_value_ptr != nullptr) {
                graphics.bases.draw_location_borders = boost::json::value_to<bool>(*draw_location_borders_value_ptr);
            }

            const boost::json::value * draw_location_text_value_ptr = bases_object.if_contains("draw_location_text");
            if (draw_location_text_value_ptr != nullptr) {
                graphics.bases.draw_location_text = boost::json::value_to<bool>(*draw_location_text_value_ptr);
            }

            const boost::json::value * enable_debug_markers_value_ptr = bases_object.if_contains("enable_debug_markers");
            if (enable_debug_markers_value_ptr != nullptr) {
                graphics.bases.enable_debug_markers = boost::json::value_to<bool>(*enable_debug_markers_value_ptr);
            }

            const boost::json::value * enable_location_markers_value_ptr = bases_object.if_contains("enable_location_markers");
            if (enable_location_markers_value_ptr != nullptr) {
                graphics.bases.enable_location_markers = boost::json::value_to<bool>(*enable_location_markers_value_ptr);
            }

            const boost::json::value * include_base_name_on_dock_value_ptr = bases_object.if_contains("include_base_name_on_dock");
            if (include_base_name_on_dock_value_ptr != nullptr) {
                graphics.bases.include_base_name_on_dock = boost::json::value_to<bool>(*include_base_name_on_dock_value_ptr);
            }

            const boost::json::value * location_marker_distance_value_ptr = bases_object.if_contains("location_marker_distance");
            if (location_marker_distance_value_ptr != nullptr) {
                graphics.bases.location_marker_distance = boost::json::value_to<double>(*location_marker_distance_value_ptr);
            }

            const boost::json::value * location_marker_draw_always_value_ptr = bases_object.if_contains("location_marker_draw_always");
            if (location_marker_draw_always_value_ptr != nullptr) {
                graphics.bases.location_marker_draw_always = boost::json::value_to<bool>(*location_marker_draw_always_value_ptr);
            }

            const boost::json::value * location_marker_sprite_value_ptr = bases_object.if_contains("location_marker_sprite");
            if (location_marker_sprite_value_ptr != nullptr) {
                graphics.bases.location_marker_sprite = boost::json::value_to<std::string>(*location_marker_sprite_value_ptr);
            }

            const boost::json::value * location_marker_text_color_r_value_ptr = bases_object.if_contains("location_marker_text_color_r");
            if (location_marker_text_color_r_value_ptr != nullptr) {
                graphics.bases.location_marker_text_color_r = boost::json::value_to<double>(*location_marker_text_color_r_value_ptr);
            }

            const boost::json::value * location_marker_text_color_g_value_ptr = bases_object.if_contains("location_marker_text_color_g");
            if (location_marker_text_color_g_value_ptr != nullptr) {
                graphics.bases.location_marker_text_color_g = boost::json::value_to<double>(*location_marker_text_color_g_value_ptr);
            }

            const boost::json::value * location_marker_text_color_b_value_ptr = bases_object.if_contains("location_marker_text_color_b");
            if (location_marker_text_color_b_value_ptr != nullptr) {
                graphics.bases.location_marker_text_color_b = boost::json::value_to<double>(*location_marker_text_color_b_value_ptr);
            }

            const boost::json::value * location_marker_text_offset_x_value_ptr = bases_object.if_contains("location_marker_text_offset_x");
            if (location_marker_text_offset_x_value_ptr != nullptr) {
                graphics.bases.location_marker_text_offset_x = boost::json::value_to<double>(*location_marker_text_offset_x_value_ptr);
            }

            const boost::json::value * location_marker_text_offset_y_value_ptr = bases_object.if_contains("location_marker_text_offset_y");
            if (location_marker_text_offset_y_value_ptr != nullptr) {
                graphics.bases.location_marker_text_offset_y = boost::json::value_to<double>(*location_marker_text_offset_y_value_ptr);
            }

            const boost::json::value * font_value_ptr = bases_object.if_contains("font");
            if (font_value_ptr != nullptr) {
                graphics.bases.font = boost::json::value_to<std::string>(*font_value_ptr);
            }

            const boost::json::value * fov_value_ptr = bases_object.if_contains("fov");
            if (fov_value_ptr != nullptr) {
                graphics.bases.fov = boost::json::value_to<double>(*fov_value_ptr);
            }

            const boost::json::value * max_width_value_ptr = bases_object.if_contains("max_width");
            if (max_width_value_ptr != nullptr) {
                graphics.bases.max_width = boost::json::value_to<int>(*max_width_value_ptr);
            }

            const boost::json::value * max_height_value_ptr = bases_object.if_contains("max_height");
            if (max_height_value_ptr != nullptr) {
                graphics.bases.max_height = boost::json::value_to<int>(*max_height_value_ptr);
            }

            const boost::json::value * print_cargo_volume_value_ptr = bases_object.if_contains("print_cargo_volume");
            if (print_cargo_volume_value_ptr != nullptr) {
                graphics.bases.print_cargo_volume = boost::json::value_to<bool>(*print_cargo_volume_value_ptr);
            }

            const boost::json::value * text_background_alpha_value_ptr = bases_object.if_contains("text_background_alpha");
            if (text_background_alpha_value_ptr != nullptr) {
                graphics.bases.text_background_alpha = boost::json::value_to<double>(*text_background_alpha_value_ptr);
            }

            const boost::json::value * blur_bases_value_ptr = bases_object.if_contains("blur_bases");
            if (blur_bases_value_ptr != nullptr) {
                graphics.bases.blur_bases = boost::json::value_to<bool>(*blur_bases_value_ptr);
            }

        }


        const boost::json::value * glow_flicker_value_ptr = graphics_object.if_contains("glow_flicker");
        if (glow_flicker_value_ptr != nullptr) {
            boost::json::object glow_flicker_object = glow_flicker_value_ptr->get_object();
            const boost::json::value * flicker_time_value_ptr = glow_flicker_object.if_contains("flicker_time");
            if (flicker_time_value_ptr != nullptr) {
                graphics.glow_flicker.flicker_time = boost::json::value_to<double>(*flicker_time_value_ptr);
            }

            const boost::json::value * flicker_off_time_value_ptr = glow_flicker_object.if_contains("flicker_off_time");
            if (flicker_off_time_value_ptr != nullptr) {
                graphics.glow_flicker.flicker_off_time = boost::json::value_to<double>(*flicker_off_time_value_ptr);
            }

            const boost::json::value * min_flicker_cycle_value_ptr = glow_flicker_object.if_contains("min_flicker_cycle");
            if (min_flicker_cycle_value_ptr != nullptr) {
                graphics.glow_flicker.min_flicker_cycle = boost::json::value_to<double>(*min_flicker_cycle_value_ptr);
            }

            const boost::json::value * num_times_per_second_on_value_ptr = glow_flicker_object.if_contains("num_times_per_second_on");
            if (num_times_per_second_on_value_ptr != nullptr) {
                graphics.glow_flicker.num_times_per_second_on = boost::json::value_to<double>(*num_times_per_second_on_value_ptr);
            }

            const boost::json::value * hull_for_total_dark_value_ptr = glow_flicker_object.if_contains("hull_for_total_dark");
            if (hull_for_total_dark_value_ptr != nullptr) {
                graphics.glow_flicker.hull_for_total_dark = boost::json::value_to<double>(*hull_for_total_dark_value_ptr);
            }

        }


        const boost::json::value * hud_value_ptr = graphics_object.if_contains("hud");
        if (hud_value_ptr != nullptr) {
            boost::json::object hud_object = hud_value_ptr->get_object();
            const boost::json::value * already_near_message_value_ptr = hud_object.if_contains("already_near_message");
            if (already_near_message_value_ptr != nullptr) {
                graphics.hud.already_near_message = boost::json::value_to<std::string>(*already_near_message_value_ptr);
            }

            const boost::json::value * armor_hull_size_value_ptr = hud_object.if_contains("armor_hull_size");
            if (armor_hull_size_value_ptr != nullptr) {
                graphics.hud.armor_hull_size = boost::json::value_to<double>(*armor_hull_size_value_ptr);
            }

            const boost::json::value * asteroids_near_message_value_ptr = hud_object.if_contains("asteroids_near_message");
            if (asteroids_near_message_value_ptr != nullptr) {
                graphics.hud.asteroids_near_message = boost::json::value_to<std::string>(*asteroids_near_message_value_ptr);
            }

            const boost::json::value * basename_colon_basename_value_ptr = hud_object.if_contains("basename_colon_basename");
            if (basename_colon_basename_value_ptr != nullptr) {
                graphics.hud.basename_colon_basename = boost::json::value_to<bool>(*basename_colon_basename_value_ptr);
            }

            const boost::json::value * box_line_thickness_value_ptr = hud_object.if_contains("box_line_thickness");
            if (box_line_thickness_value_ptr != nullptr) {
                graphics.hud.box_line_thickness = boost::json::value_to<double>(*box_line_thickness_value_ptr);
            }

            const boost::json::value * completed_objectives_last_value_ptr = hud_object.if_contains("completed_objectives_last");
            if (completed_objectives_last_value_ptr != nullptr) {
                graphics.hud.completed_objectives_last = boost::json::value_to<bool>(*completed_objectives_last_value_ptr);
            }

            const boost::json::value * crosshairs_on_chase_cam_value_ptr = hud_object.if_contains("crosshairs_on_chase_cam");
            if (crosshairs_on_chase_cam_value_ptr != nullptr) {
                graphics.hud.crosshairs_on_chase_cam = boost::json::value_to<bool>(*crosshairs_on_chase_cam_value_ptr);
            }

            const boost::json::value * crosshairs_on_padlock_value_ptr = hud_object.if_contains("crosshairs_on_padlock");
            if (crosshairs_on_padlock_value_ptr != nullptr) {
                graphics.hud.crosshairs_on_padlock = boost::json::value_to<bool>(*crosshairs_on_padlock_value_ptr);
            }

            const boost::json::value * damage_report_heading_value_ptr = hud_object.if_contains("damage_report_heading");
            if (damage_report_heading_value_ptr != nullptr) {
                graphics.hud.damage_report_heading = boost::json::value_to<std::string>(*damage_report_heading_value_ptr);
            }

            const boost::json::value * debug_position_value_ptr = hud_object.if_contains("debug_position");
            if (debug_position_value_ptr != nullptr) {
                graphics.hud.debug_position = boost::json::value_to<bool>(*debug_position_value_ptr);
            }

            const boost::json::value * diamond_line_thickness_value_ptr = hud_object.if_contains("diamond_line_thickness");
            if (diamond_line_thickness_value_ptr != nullptr) {
                graphics.hud.diamond_line_thickness = boost::json::value_to<double>(*diamond_line_thickness_value_ptr);
            }

            const boost::json::value * diamond_rotation_speed_value_ptr = hud_object.if_contains("diamond_rotation_speed");
            if (diamond_rotation_speed_value_ptr != nullptr) {
                graphics.hud.diamond_rotation_speed = boost::json::value_to<double>(*diamond_rotation_speed_value_ptr);
            }

            const boost::json::value * diamond_size_value_ptr = hud_object.if_contains("diamond_size");
            if (diamond_size_value_ptr != nullptr) {
                graphics.hud.diamond_size = boost::json::value_to<double>(*diamond_size_value_ptr);
            }

            const boost::json::value * display_relative_velocity_value_ptr = hud_object.if_contains("display_relative_velocity");
            if (display_relative_velocity_value_ptr != nullptr) {
                graphics.hud.display_relative_velocity = boost::json::value_to<bool>(*display_relative_velocity_value_ptr);
            }

            const boost::json::value * display_warp_energy_if_no_jump_drive_value_ptr = hud_object.if_contains("display_warp_energy_if_no_jump_drive");
            if (display_warp_energy_if_no_jump_drive_value_ptr != nullptr) {
                graphics.hud.display_warp_energy_if_no_jump_drive = boost::json::value_to<bool>(*display_warp_energy_if_no_jump_drive_value_ptr);
            }

            const boost::json::value * draw_all_target_boxes_value_ptr = hud_object.if_contains("draw_all_target_boxes");
            if (draw_all_target_boxes_value_ptr != nullptr) {
                graphics.hud.draw_all_target_boxes = boost::json::value_to<bool>(*draw_all_target_boxes_value_ptr);
            }

            const boost::json::value * draw_always_itts_value_ptr = hud_object.if_contains("draw_always_itts");
            if (draw_always_itts_value_ptr != nullptr) {
                graphics.hud.draw_always_itts = boost::json::value_to<bool>(*draw_always_itts_value_ptr);
            }

            const boost::json::value * draw_arrow_on_chase_cam_value_ptr = hud_object.if_contains("draw_arrow_on_chase_cam");
            if (draw_arrow_on_chase_cam_value_ptr != nullptr) {
                graphics.hud.draw_arrow_on_chase_cam = boost::json::value_to<bool>(*draw_arrow_on_chase_cam_value_ptr);
            }

            const boost::json::value * draw_arrow_on_pan_cam_value_ptr = hud_object.if_contains("draw_arrow_on_pan_cam");
            if (draw_arrow_on_pan_cam_value_ptr != nullptr) {
                graphics.hud.draw_arrow_on_pan_cam = boost::json::value_to<bool>(*draw_arrow_on_pan_cam_value_ptr);
            }

            const boost::json::value * draw_arrow_on_pan_target_value_ptr = hud_object.if_contains("draw_arrow_on_pan_target");
            if (draw_arrow_on_pan_target_value_ptr != nullptr) {
                graphics.hud.draw_arrow_on_pan_target = boost::json::value_to<bool>(*draw_arrow_on_pan_target_value_ptr);
            }

            const boost::json::value * draw_arrow_to_target_value_ptr = hud_object.if_contains("draw_arrow_to_target");
            if (draw_arrow_to_target_value_ptr != nullptr) {
                graphics.hud.draw_arrow_to_target = boost::json::value_to<bool>(*draw_arrow_to_target_value_ptr);
            }

            const boost::json::value * draw_blips_on_both_radar_value_ptr = hud_object.if_contains("draw_blips_on_both_radar");
            if (draw_blips_on_both_radar_value_ptr != nullptr) {
                graphics.hud.draw_blips_on_both_radar = boost::json::value_to<bool>(*draw_blips_on_both_radar_value_ptr);
            }

            const boost::json::value * draw_rendered_crosshairs_value_ptr = hud_object.if_contains("draw_rendered_crosshairs");
            if (draw_rendered_crosshairs_value_ptr != nullptr) {
                graphics.hud.draw_rendered_crosshairs = boost::json::value_to<bool>(*draw_rendered_crosshairs_value_ptr);
            }

            const boost::json::value * draw_damage_sprite_value_ptr = hud_object.if_contains("draw_damage_sprite");
            if (draw_damage_sprite_value_ptr != nullptr) {
                graphics.hud.draw_damage_sprite = boost::json::value_to<bool>(*draw_damage_sprite_value_ptr);
            }

            const boost::json::value * draw_gravitational_objects_value_ptr = hud_object.if_contains("draw_gravitational_objects");
            if (draw_gravitational_objects_value_ptr != nullptr) {
                graphics.hud.draw_gravitational_objects = boost::json::value_to<bool>(*draw_gravitational_objects_value_ptr);
            }

            const boost::json::value * draw_line_to_itts_value_ptr = hud_object.if_contains("draw_line_to_itts");
            if (draw_line_to_itts_value_ptr != nullptr) {
                graphics.hud.draw_line_to_itts = boost::json::value_to<bool>(*draw_line_to_itts_value_ptr);
            }

            const boost::json::value * draw_line_to_target_value_ptr = hud_object.if_contains("draw_line_to_target");
            if (draw_line_to_target_value_ptr != nullptr) {
                graphics.hud.draw_line_to_target = boost::json::value_to<bool>(*draw_line_to_target_value_ptr);
            }

            const boost::json::value * draw_line_to_targets_target_value_ptr = hud_object.if_contains("draw_line_to_targets_target");
            if (draw_line_to_targets_target_value_ptr != nullptr) {
                graphics.hud.draw_line_to_targets_target = boost::json::value_to<bool>(*draw_line_to_targets_target_value_ptr);
            }

            const boost::json::value * draw_nav_grid_value_ptr = hud_object.if_contains("draw_nav_grid");
            if (draw_nav_grid_value_ptr != nullptr) {
                graphics.hud.draw_nav_grid = boost::json::value_to<bool>(*draw_nav_grid_value_ptr);
            }

            const boost::json::value * draw_nav_symbol_value_ptr = hud_object.if_contains("draw_nav_symbol");
            if (draw_nav_symbol_value_ptr != nullptr) {
                graphics.hud.draw_nav_symbol = boost::json::value_to<bool>(*draw_nav_symbol_value_ptr);
            }

            const boost::json::value * draw_tactical_target_value_ptr = hud_object.if_contains("draw_tactical_target");
            if (draw_tactical_target_value_ptr != nullptr) {
                graphics.hud.draw_tactical_target = boost::json::value_to<bool>(*draw_tactical_target_value_ptr);
            }

            const boost::json::value * draw_targetting_boxes_value_ptr = hud_object.if_contains("draw_targetting_boxes");
            if (draw_targetting_boxes_value_ptr != nullptr) {
                graphics.hud.draw_targetting_boxes = boost::json::value_to<bool>(*draw_targetting_boxes_value_ptr);
            }

            const boost::json::value * draw_targetting_boxes_inside_value_ptr = hud_object.if_contains("draw_targetting_boxes_inside");
            if (draw_targetting_boxes_inside_value_ptr != nullptr) {
                graphics.hud.draw_targetting_boxes_inside = boost::json::value_to<bool>(*draw_targetting_boxes_inside_value_ptr);
            }

            const boost::json::value * draw_significant_blips_value_ptr = hud_object.if_contains("draw_significant_blips");
            if (draw_significant_blips_value_ptr != nullptr) {
                graphics.hud.draw_significant_blips = boost::json::value_to<bool>(*draw_significant_blips_value_ptr);
            }

            const boost::json::value * draw_star_direction_value_ptr = hud_object.if_contains("draw_star_direction");
            if (draw_star_direction_value_ptr != nullptr) {
                graphics.hud.draw_star_direction = boost::json::value_to<bool>(*draw_star_direction_value_ptr);
            }

            const boost::json::value * draw_unit_on_chase_cam_value_ptr = hud_object.if_contains("draw_unit_on_chase_cam");
            if (draw_unit_on_chase_cam_value_ptr != nullptr) {
                graphics.hud.draw_unit_on_chase_cam = boost::json::value_to<bool>(*draw_unit_on_chase_cam_value_ptr);
            }

            const boost::json::value * draw_vdu_view_shields_value_ptr = hud_object.if_contains("draw_vdu_view_shields");
            if (draw_vdu_view_shields_value_ptr != nullptr) {
                graphics.hud.draw_vdu_view_shields = boost::json::value_to<bool>(*draw_vdu_view_shields_value_ptr);
            }

            const boost::json::value * draw_weapon_sprite_value_ptr = hud_object.if_contains("draw_weapon_sprite");
            if (draw_weapon_sprite_value_ptr != nullptr) {
                graphics.hud.draw_weapon_sprite = boost::json::value_to<bool>(*draw_weapon_sprite_value_ptr);
            }

            const boost::json::value * enemy_near_message_value_ptr = hud_object.if_contains("enemy_near_message");
            if (enemy_near_message_value_ptr != nullptr) {
                graphics.hud.enemy_near_message = boost::json::value_to<std::string>(*enemy_near_message_value_ptr);
            }

            const boost::json::value * gun_list_columns_value_ptr = hud_object.if_contains("gun_list_columns");
            if (gun_list_columns_value_ptr != nullptr) {
                graphics.hud.gun_list_columns = boost::json::value_to<int>(*gun_list_columns_value_ptr);
            }

            const boost::json::value * invert_friendly_shields_value_ptr = hud_object.if_contains("invert_friendly_shields");
            if (invert_friendly_shields_value_ptr != nullptr) {
                graphics.hud.invert_friendly_shields = boost::json::value_to<bool>(*invert_friendly_shields_value_ptr);
            }

            const boost::json::value * invert_friendly_sprite_value_ptr = hud_object.if_contains("invert_friendly_sprite");
            if (invert_friendly_sprite_value_ptr != nullptr) {
                graphics.hud.invert_friendly_sprite = boost::json::value_to<bool>(*invert_friendly_sprite_value_ptr);
            }

            const boost::json::value * invert_target_shields_value_ptr = hud_object.if_contains("invert_target_shields");
            if (invert_target_shields_value_ptr != nullptr) {
                graphics.hud.invert_target_shields = boost::json::value_to<bool>(*invert_target_shields_value_ptr);
            }

            const boost::json::value * invert_target_sprite_value_ptr = hud_object.if_contains("invert_target_sprite");
            if (invert_target_sprite_value_ptr != nullptr) {
                graphics.hud.invert_target_sprite = boost::json::value_to<bool>(*invert_target_sprite_value_ptr);
            }

            const boost::json::value * invert_view_shields_value_ptr = hud_object.if_contains("invert_view_shields");
            if (invert_view_shields_value_ptr != nullptr) {
                graphics.hud.invert_view_shields = boost::json::value_to<bool>(*invert_view_shields_value_ptr);
            }

            const boost::json::value * itts_for_beams_value_ptr = hud_object.if_contains("itts_for_beams");
            if (itts_for_beams_value_ptr != nullptr) {
                graphics.hud.itts_for_beams = boost::json::value_to<bool>(*itts_for_beams_value_ptr);
            }

            const boost::json::value * itts_for_lockable_value_ptr = hud_object.if_contains("itts_for_lockable");
            if (itts_for_lockable_value_ptr != nullptr) {
                graphics.hud.itts_for_lockable = boost::json::value_to<bool>(*itts_for_lockable_value_ptr);
            }

            const boost::json::value * itts_line_to_mark_alpha_value_ptr = hud_object.if_contains("itts_line_to_mark_alpha");
            if (itts_line_to_mark_alpha_value_ptr != nullptr) {
                graphics.hud.itts_line_to_mark_alpha = boost::json::value_to<double>(*itts_line_to_mark_alpha_value_ptr);
            }

            const boost::json::value * itts_use_average_gun_speed_value_ptr = hud_object.if_contains("itts_use_average_gun_speed");
            if (itts_use_average_gun_speed_value_ptr != nullptr) {
                graphics.hud.itts_use_average_gun_speed = boost::json::value_to<bool>(*itts_use_average_gun_speed_value_ptr);
            }

            const boost::json::value * lock_center_crosshair_value_ptr = hud_object.if_contains("lock_center_crosshair");
            if (lock_center_crosshair_value_ptr != nullptr) {
                graphics.hud.lock_center_crosshair = boost::json::value_to<bool>(*lock_center_crosshair_value_ptr);
            }

            const boost::json::value * lock_confirm_line_length_value_ptr = hud_object.if_contains("lock_confirm_line_length");
            if (lock_confirm_line_length_value_ptr != nullptr) {
                graphics.hud.lock_confirm_line_length = boost::json::value_to<double>(*lock_confirm_line_length_value_ptr);
            }

            const boost::json::value * manifest_heading_value_ptr = hud_object.if_contains("manifest_heading");
            if (manifest_heading_value_ptr != nullptr) {
                graphics.hud.manifest_heading = boost::json::value_to<std::string>(*manifest_heading_value_ptr);
            }

            const boost::json::value * max_missile_bracket_size_value_ptr = hud_object.if_contains("max_missile_bracket_size");
            if (max_missile_bracket_size_value_ptr != nullptr) {
                graphics.hud.max_missile_bracket_size = boost::json::value_to<double>(*max_missile_bracket_size_value_ptr);
            }

            const boost::json::value * message_prefix_value_ptr = hud_object.if_contains("message_prefix");
            if (message_prefix_value_ptr != nullptr) {
                graphics.hud.message_prefix = boost::json::value_to<std::string>(*message_prefix_value_ptr);
            }

            const boost::json::value * min_missile_bracket_size_value_ptr = hud_object.if_contains("min_missile_bracket_size");
            if (min_missile_bracket_size_value_ptr != nullptr) {
                graphics.hud.min_missile_bracket_size = boost::json::value_to<double>(*min_missile_bracket_size_value_ptr);
            }

            const boost::json::value * min_lock_box_size_value_ptr = hud_object.if_contains("min_lock_box_size");
            if (min_lock_box_size_value_ptr != nullptr) {
                graphics.hud.min_lock_box_size = boost::json::value_to<double>(*min_lock_box_size_value_ptr);
            }

            const boost::json::value * min_radar_blip_size_value_ptr = hud_object.if_contains("min_radar_blip_size");
            if (min_radar_blip_size_value_ptr != nullptr) {
                graphics.hud.min_radar_blip_size = boost::json::value_to<double>(*min_radar_blip_size_value_ptr);
            }

            const boost::json::value * min_target_box_size_value_ptr = hud_object.if_contains("min_target_box_size");
            if (min_target_box_size_value_ptr != nullptr) {
                graphics.hud.min_target_box_size = boost::json::value_to<double>(*min_target_box_size_value_ptr);
            }

            const boost::json::value * mounts_list_empty_value_ptr = hud_object.if_contains("mounts_list_empty");
            if (mounts_list_empty_value_ptr != nullptr) {
                graphics.hud.mounts_list_empty = boost::json::value_to<std::string>(*mounts_list_empty_value_ptr);
            }

            const boost::json::value * nav_cross_line_thickness_value_ptr = hud_object.if_contains("nav_cross_line_thickness");
            if (nav_cross_line_thickness_value_ptr != nullptr) {
                graphics.hud.nav_cross_line_thickness = boost::json::value_to<double>(*nav_cross_line_thickness_value_ptr);
            }

            const boost::json::value * not_included_in_damage_report_value_ptr = hud_object.if_contains("not_included_in_damage_report");
            if (not_included_in_damage_report_value_ptr != nullptr) {
                graphics.hud.not_included_in_damage_report = boost::json::value_to<std::string>(*not_included_in_damage_report_value_ptr);
            }

            const boost::json::value * out_of_cone_distance_value_ptr = hud_object.if_contains("out_of_cone_distance");
            if (out_of_cone_distance_value_ptr != nullptr) {
                graphics.hud.out_of_cone_distance = boost::json::value_to<bool>(*out_of_cone_distance_value_ptr);
            }

            const boost::json::value * padlock_view_lag_value_ptr = hud_object.if_contains("padlock_view_lag");
            if (padlock_view_lag_value_ptr != nullptr) {
                graphics.hud.padlock_view_lag = boost::json::value_to<double>(*padlock_view_lag_value_ptr);
            }

            const boost::json::value * padlock_view_lag_fix_zone_value_ptr = hud_object.if_contains("padlock_view_lag_fix_zone");
            if (padlock_view_lag_fix_zone_value_ptr != nullptr) {
                graphics.hud.padlock_view_lag_fix_zone = boost::json::value_to<double>(*padlock_view_lag_fix_zone_value_ptr);
            }

            const boost::json::value * planet_near_message_value_ptr = hud_object.if_contains("planet_near_message");
            if (planet_near_message_value_ptr != nullptr) {
                graphics.hud.planet_near_message = boost::json::value_to<std::string>(*planet_near_message_value_ptr);
            }

            const boost::json::value * print_damage_percent_value_ptr = hud_object.if_contains("print_damage_percent");
            if (print_damage_percent_value_ptr != nullptr) {
                graphics.hud.print_damage_percent = boost::json::value_to<bool>(*print_damage_percent_value_ptr);
            }

            const boost::json::value * print_faction_value_ptr = hud_object.if_contains("print_faction");
            if (print_faction_value_ptr != nullptr) {
                graphics.hud.print_faction = boost::json::value_to<bool>(*print_faction_value_ptr);
            }

            const boost::json::value * print_fg_name_value_ptr = hud_object.if_contains("print_fg_name");
            if (print_fg_name_value_ptr != nullptr) {
                graphics.hud.print_fg_name = boost::json::value_to<bool>(*print_fg_name_value_ptr);
            }

            const boost::json::value * print_fg_sub_id_value_ptr = hud_object.if_contains("print_fg_sub_id");
            if (print_fg_sub_id_value_ptr != nullptr) {
                graphics.hud.print_fg_sub_id = boost::json::value_to<bool>(*print_fg_sub_id_value_ptr);
            }

            const boost::json::value * print_request_docking_value_ptr = hud_object.if_contains("print_request_docking");
            if (print_request_docking_value_ptr != nullptr) {
                graphics.hud.print_request_docking = boost::json::value_to<bool>(*print_request_docking_value_ptr);
            }

            const boost::json::value * print_ship_type_value_ptr = hud_object.if_contains("print_ship_type");
            if (print_ship_type_value_ptr != nullptr) {
                graphics.hud.print_ship_type = boost::json::value_to<bool>(*print_ship_type_value_ptr);
            }

            const boost::json::value * projectile_means_missile_value_ptr = hud_object.if_contains("projectile_means_missile");
            if (projectile_means_missile_value_ptr != nullptr) {
                graphics.hud.projectile_means_missile = boost::json::value_to<bool>(*projectile_means_missile_value_ptr);
            }

            const boost::json::value * radar_type_value_ptr = hud_object.if_contains("radar_type");
            if (radar_type_value_ptr != nullptr) {
                graphics.hud.radar_type = boost::json::value_to<std::string>(*radar_type_value_ptr);
            }

            const boost::json::value * radar_search_extra_radius_value_ptr = hud_object.if_contains("radar_search_extra_radius");
            if (radar_search_extra_radius_value_ptr != nullptr) {
                graphics.hud.radar_search_extra_radius = boost::json::value_to<double>(*radar_search_extra_radius_value_ptr);
            }

            const boost::json::value * rotating_bracket_inner_value_ptr = hud_object.if_contains("rotating_bracket_inner");
            if (rotating_bracket_inner_value_ptr != nullptr) {
                graphics.hud.rotating_bracket_inner = boost::json::value_to<bool>(*rotating_bracket_inner_value_ptr);
            }

            const boost::json::value * rotating_bracket_size_value_ptr = hud_object.if_contains("rotating_bracket_size");
            if (rotating_bracket_size_value_ptr != nullptr) {
                graphics.hud.rotating_bracket_size = boost::json::value_to<double>(*rotating_bracket_size_value_ptr);
            }

            const boost::json::value * rotating_bracket_width_value_ptr = hud_object.if_contains("rotating_bracket_width");
            if (rotating_bracket_width_value_ptr != nullptr) {
                graphics.hud.rotating_bracket_width = boost::json::value_to<double>(*rotating_bracket_width_value_ptr);
            }

            const boost::json::value * scale_relationship_color_value_ptr = hud_object.if_contains("scale_relationship_color");
            if (scale_relationship_color_value_ptr != nullptr) {
                graphics.hud.scale_relationship_color = boost::json::value_to<double>(*scale_relationship_color_value_ptr);
            }

            const boost::json::value * shield_vdu_fade_value_ptr = hud_object.if_contains("shield_vdu_fade");
            if (shield_vdu_fade_value_ptr != nullptr) {
                graphics.hud.shield_vdu_fade = boost::json::value_to<bool>(*shield_vdu_fade_value_ptr);
            }

            const boost::json::value * shield_vdu_thresh0_value_ptr = hud_object.if_contains("shield_vdu_thresh0");
            if (shield_vdu_thresh0_value_ptr != nullptr) {
                graphics.hud.shield_vdu_thresh0 = boost::json::value_to<double>(*shield_vdu_thresh0_value_ptr);
            }

            const boost::json::value * shield_vdu_thresh1_value_ptr = hud_object.if_contains("shield_vdu_thresh1");
            if (shield_vdu_thresh1_value_ptr != nullptr) {
                graphics.hud.shield_vdu_thresh1 = boost::json::value_to<double>(*shield_vdu_thresh1_value_ptr);
            }

            const boost::json::value * shield_vdu_thresh2_value_ptr = hud_object.if_contains("shield_vdu_thresh2");
            if (shield_vdu_thresh2_value_ptr != nullptr) {
                graphics.hud.shield_vdu_thresh2 = boost::json::value_to<double>(*shield_vdu_thresh2_value_ptr);
            }

            const boost::json::value * show_negative_blips_as_positive_value_ptr = hud_object.if_contains("show_negative_blips_as_positive");
            if (show_negative_blips_as_positive_value_ptr != nullptr) {
                graphics.hud.show_negative_blips_as_positive = boost::json::value_to<bool>(*show_negative_blips_as_positive_value_ptr);
            }

            const boost::json::value * simple_manifest_value_ptr = hud_object.if_contains("simple_manifest");
            if (simple_manifest_value_ptr != nullptr) {
                graphics.hud.simple_manifest = boost::json::value_to<bool>(*simple_manifest_value_ptr);
            }

            const boost::json::value * starship_near_message_value_ptr = hud_object.if_contains("starship_near_message");
            if (starship_near_message_value_ptr != nullptr) {
                graphics.hud.starship_near_message = boost::json::value_to<std::string>(*starship_near_message_value_ptr);
            }

            const boost::json::value * switch_to_target_mode_on_key_value_ptr = hud_object.if_contains("switch_to_target_mode_on_key");
            if (switch_to_target_mode_on_key_value_ptr != nullptr) {
                graphics.hud.switch_to_target_mode_on_key = boost::json::value_to<bool>(*switch_to_target_mode_on_key_value_ptr);
            }

            const boost::json::value * switch_back_from_comms_value_ptr = hud_object.if_contains("switch_back_from_comms");
            if (switch_back_from_comms_value_ptr != nullptr) {
                graphics.hud.switch_back_from_comms = boost::json::value_to<bool>(*switch_back_from_comms_value_ptr);
            }

            const boost::json::value * tac_target_foci_value_ptr = hud_object.if_contains("tac_target_foci");
            if (tac_target_foci_value_ptr != nullptr) {
                graphics.hud.tac_target_foci = boost::json::value_to<double>(*tac_target_foci_value_ptr);
            }

            const boost::json::value * tac_target_length_value_ptr = hud_object.if_contains("tac_target_length");
            if (tac_target_length_value_ptr != nullptr) {
                graphics.hud.tac_target_length = boost::json::value_to<double>(*tac_target_length_value_ptr);
            }

            const boost::json::value * tac_target_thickness_value_ptr = hud_object.if_contains("tac_target_thickness");
            if (tac_target_thickness_value_ptr != nullptr) {
                graphics.hud.tac_target_thickness = boost::json::value_to<double>(*tac_target_thickness_value_ptr);
            }

            const boost::json::value * text_background_alpha_value_ptr = hud_object.if_contains("text_background_alpha");
            if (text_background_alpha_value_ptr != nullptr) {
                graphics.hud.text_background_alpha = boost::json::value_to<double>(*text_background_alpha_value_ptr);
            }

            const boost::json::value * top_view_value_ptr = hud_object.if_contains("top_view");
            if (top_view_value_ptr != nullptr) {
                graphics.hud.top_view = boost::json::value_to<bool>(*top_view_value_ptr);
            }

            const boost::json::value * untarget_beyond_cone_value_ptr = hud_object.if_contains("untarget_beyond_cone");
            if (untarget_beyond_cone_value_ptr != nullptr) {
                graphics.hud.untarget_beyond_cone = boost::json::value_to<bool>(*untarget_beyond_cone_value_ptr);
            }

        }


        const boost::json::value * nav_value_ptr = graphics_object.if_contains("nav");
        if (nav_value_ptr != nullptr) {
            boost::json::object nav_object = nav_value_ptr->get_object();
            const boost::json::value * max_search_size_value_ptr = nav_object.if_contains("max_search_size");
            if (max_search_size_value_ptr != nullptr) {
                graphics.nav.max_search_size = boost::json::value_to<int>(*max_search_size_value_ptr);
            }

            const boost::json::value * modern_mouse_cursor_value_ptr = nav_object.if_contains("modern_mouse_cursor");
            if (modern_mouse_cursor_value_ptr != nullptr) {
                graphics.nav.modern_mouse_cursor = boost::json::value_to<bool>(*modern_mouse_cursor_value_ptr);
            }

            const boost::json::value * mouse_cursor_sprite_value_ptr = nav_object.if_contains("mouse_cursor_sprite");
            if (mouse_cursor_sprite_value_ptr != nullptr) {
                graphics.nav.mouse_cursor_sprite = boost::json::value_to<std::string>(*mouse_cursor_sprite_value_ptr);
            }

            const boost::json::value * symbol_size_value_ptr = nav_object.if_contains("symbol_size");
            if (symbol_size_value_ptr != nullptr) {
                graphics.nav.symbol_size = boost::json::value_to<double>(*symbol_size_value_ptr);
            }

        }


        }


        const boost::json::value * components_value_ptr = root_object.if_contains("components");
        if (components_value_ptr != nullptr) {
            boost::json::object components_object = components_value_ptr->get_object();
        const boost::json::value * afterburner_value_ptr = components_object.if_contains("afterburner");
        if (afterburner_value_ptr != nullptr) {
            boost::json::object afterburner_object = afterburner_value_ptr->get_object();
            const boost::json::value * energy_source_value_ptr = afterburner_object.if_contains("energy_source");
            if (energy_source_value_ptr != nullptr) {
                components.afterburner.energy_source = boost::json::value_to<std::string>(*energy_source_value_ptr);
            }

        }


        const boost::json::value * cloak_value_ptr = components_object.if_contains("cloak");
        if (cloak_value_ptr != nullptr) {
            boost::json::object cloak_object = cloak_value_ptr->get_object();
            const boost::json::value * energy_source_value_ptr = cloak_object.if_contains("energy_source");
            if (energy_source_value_ptr != nullptr) {
                components.cloak.energy_source = boost::json::value_to<std::string>(*energy_source_value_ptr);
            }

        }


        const boost::json::value * computer_value_ptr = components_object.if_contains("computer");
        if (computer_value_ptr != nullptr) {
            boost::json::object computer_object = computer_value_ptr->get_object();
            const boost::json::value * default_lock_cone_value_ptr = computer_object.if_contains("default_lock_cone");
            if (default_lock_cone_value_ptr != nullptr) {
                components.computer.default_lock_cone = boost::json::value_to<double>(*default_lock_cone_value_ptr);
            }

            const boost::json::value * default_max_range_value_ptr = computer_object.if_contains("default_max_range");
            if (default_max_range_value_ptr != nullptr) {
                components.computer.default_max_range = boost::json::value_to<double>(*default_max_range_value_ptr);
            }

            const boost::json::value * default_tracking_cone_value_ptr = computer_object.if_contains("default_tracking_cone");
            if (default_tracking_cone_value_ptr != nullptr) {
                components.computer.default_tracking_cone = boost::json::value_to<double>(*default_tracking_cone_value_ptr);
            }

        }


        const boost::json::value * drive_value_ptr = components_object.if_contains("drive");
        if (drive_value_ptr != nullptr) {
            boost::json::object drive_object = drive_value_ptr->get_object();
            const boost::json::value * energy_source_value_ptr = drive_object.if_contains("energy_source");
            if (energy_source_value_ptr != nullptr) {
                components.drive.energy_source = boost::json::value_to<std::string>(*energy_source_value_ptr);
            }

            const boost::json::value * non_combat_mode_multiplier_value_ptr = drive_object.if_contains("non_combat_mode_multiplier");
            if (non_combat_mode_multiplier_value_ptr != nullptr) {
                components.drive.non_combat_mode_multiplier = boost::json::value_to<int>(*non_combat_mode_multiplier_value_ptr);
            }

            const boost::json::value * minimum_drive_value_ptr = drive_object.if_contains("minimum_drive");
            if (minimum_drive_value_ptr != nullptr) {
                components.drive.minimum_drive = boost::json::value_to<double>(*minimum_drive_value_ptr);
            }

        }


        const boost::json::value * energy_value_ptr = components_object.if_contains("energy");
        if (energy_value_ptr != nullptr) {
            boost::json::object energy_object = energy_value_ptr->get_object();
            const boost::json::value * factor_value_ptr = energy_object.if_contains("factor");
            if (factor_value_ptr != nullptr) {
                components.energy.factor = boost::json::value_to<int>(*factor_value_ptr);
            }

        }


        const boost::json::value * fuel_value_ptr = components_object.if_contains("fuel");
        if (fuel_value_ptr != nullptr) {
            boost::json::object fuel_object = fuel_value_ptr->get_object();
            const boost::json::value * afterburner_fuel_usage_value_ptr = fuel_object.if_contains("afterburner_fuel_usage");
            if (afterburner_fuel_usage_value_ptr != nullptr) {
                components.fuel.afterburner_fuel_usage = boost::json::value_to<double>(*afterburner_fuel_usage_value_ptr);
            }

            const boost::json::value * deuterium_relative_efficiency_lithium_value_ptr = fuel_object.if_contains("deuterium_relative_efficiency_lithium");
            if (deuterium_relative_efficiency_lithium_value_ptr != nullptr) {
                components.fuel.deuterium_relative_efficiency_lithium = boost::json::value_to<double>(*deuterium_relative_efficiency_lithium_value_ptr);
            }

            const boost::json::value * ecm_energy_cost_value_ptr = fuel_object.if_contains("ecm_energy_cost");
            if (ecm_energy_cost_value_ptr != nullptr) {
                components.fuel.ecm_energy_cost = boost::json::value_to<double>(*ecm_energy_cost_value_ptr);
            }

            const boost::json::value * fmec_factor_value_ptr = fuel_object.if_contains("fmec_factor");
            if (fmec_factor_value_ptr != nullptr) {
                components.fuel.fmec_factor = boost::json::value_to<double>(*fmec_factor_value_ptr);
            }

            const boost::json::value * fuel_efficiency_value_ptr = fuel_object.if_contains("fuel_efficiency");
            if (fuel_efficiency_value_ptr != nullptr) {
                components.fuel.fuel_efficiency = boost::json::value_to<double>(*fuel_efficiency_value_ptr);
            }

            const boost::json::value * fuel_equals_warp_value_ptr = fuel_object.if_contains("fuel_equals_warp");
            if (fuel_equals_warp_value_ptr != nullptr) {
                components.fuel.fuel_equals_warp = boost::json::value_to<bool>(*fuel_equals_warp_value_ptr);
            }

            const boost::json::value * factor_value_ptr = fuel_object.if_contains("factor");
            if (factor_value_ptr != nullptr) {
                components.fuel.factor = boost::json::value_to<int>(*factor_value_ptr);
            }

            const boost::json::value * megajoules_factor_value_ptr = fuel_object.if_contains("megajoules_factor");
            if (megajoules_factor_value_ptr != nullptr) {
                components.fuel.megajoules_factor = boost::json::value_to<double>(*megajoules_factor_value_ptr);
            }

            const boost::json::value * min_reactor_efficiency_value_ptr = fuel_object.if_contains("min_reactor_efficiency");
            if (min_reactor_efficiency_value_ptr != nullptr) {
                components.fuel.min_reactor_efficiency = boost::json::value_to<double>(*min_reactor_efficiency_value_ptr);
            }

            const boost::json::value * no_fuel_afterburn_value_ptr = fuel_object.if_contains("no_fuel_afterburn");
            if (no_fuel_afterburn_value_ptr != nullptr) {
                components.fuel.no_fuel_afterburn = boost::json::value_to<double>(*no_fuel_afterburn_value_ptr);
            }

            const boost::json::value * no_fuel_thrust_value_ptr = fuel_object.if_contains("no_fuel_thrust");
            if (no_fuel_thrust_value_ptr != nullptr) {
                components.fuel.no_fuel_thrust = boost::json::value_to<double>(*no_fuel_thrust_value_ptr);
            }

            const boost::json::value * normal_fuel_usage_value_ptr = fuel_object.if_contains("normal_fuel_usage");
            if (normal_fuel_usage_value_ptr != nullptr) {
                components.fuel.normal_fuel_usage = boost::json::value_to<double>(*normal_fuel_usage_value_ptr);
            }

            const boost::json::value * reactor_idle_efficiency_value_ptr = fuel_object.if_contains("reactor_idle_efficiency");
            if (reactor_idle_efficiency_value_ptr != nullptr) {
                components.fuel.reactor_idle_efficiency = boost::json::value_to<double>(*reactor_idle_efficiency_value_ptr);
            }

            const boost::json::value * reactor_uses_fuel_value_ptr = fuel_object.if_contains("reactor_uses_fuel");
            if (reactor_uses_fuel_value_ptr != nullptr) {
                components.fuel.reactor_uses_fuel = boost::json::value_to<bool>(*reactor_uses_fuel_value_ptr);
            }

            const boost::json::value * variable_fuel_consumption_value_ptr = fuel_object.if_contains("variable_fuel_consumption");
            if (variable_fuel_consumption_value_ptr != nullptr) {
                components.fuel.variable_fuel_consumption = boost::json::value_to<bool>(*variable_fuel_consumption_value_ptr);
            }

            const boost::json::value * vsd_mj_yield_value_ptr = fuel_object.if_contains("vsd_mj_yield");
            if (vsd_mj_yield_value_ptr != nullptr) {
                components.fuel.vsd_mj_yield = boost::json::value_to<double>(*vsd_mj_yield_value_ptr);
            }

        }


        const boost::json::value * ftl_drive_value_ptr = components_object.if_contains("ftl_drive");
        if (ftl_drive_value_ptr != nullptr) {
            boost::json::object ftl_drive_object = ftl_drive_value_ptr->get_object();
            const boost::json::value * energy_source_value_ptr = ftl_drive_object.if_contains("energy_source");
            if (energy_source_value_ptr != nullptr) {
                components.ftl_drive.energy_source = boost::json::value_to<std::string>(*energy_source_value_ptr);
            }

            const boost::json::value * factor_value_ptr = ftl_drive_object.if_contains("factor");
            if (factor_value_ptr != nullptr) {
                components.ftl_drive.factor = boost::json::value_to<double>(*factor_value_ptr);
            }

        }


        const boost::json::value * ftl_energy_value_ptr = components_object.if_contains("ftl_energy");
        if (ftl_energy_value_ptr != nullptr) {
            boost::json::object ftl_energy_object = ftl_energy_value_ptr->get_object();
            const boost::json::value * factor_value_ptr = ftl_energy_object.if_contains("factor");
            if (factor_value_ptr != nullptr) {
                components.ftl_energy.factor = boost::json::value_to<double>(*factor_value_ptr);
            }

        }


        const boost::json::value * jump_drive_value_ptr = components_object.if_contains("jump_drive");
        if (jump_drive_value_ptr != nullptr) {
            boost::json::object jump_drive_object = jump_drive_value_ptr->get_object();
            const boost::json::value * energy_source_value_ptr = jump_drive_object.if_contains("energy_source");
            if (energy_source_value_ptr != nullptr) {
                components.jump_drive.energy_source = boost::json::value_to<std::string>(*energy_source_value_ptr);
            }

            const boost::json::value * factor_value_ptr = jump_drive_object.if_contains("factor");
            if (factor_value_ptr != nullptr) {
                components.jump_drive.factor = boost::json::value_to<double>(*factor_value_ptr);
            }

        }


        const boost::json::value * reactor_value_ptr = components_object.if_contains("reactor");
        if (reactor_value_ptr != nullptr) {
            boost::json::object reactor_object = reactor_value_ptr->get_object();
            const boost::json::value * energy_source_value_ptr = reactor_object.if_contains("energy_source");
            if (energy_source_value_ptr != nullptr) {
                components.reactor.energy_source = boost::json::value_to<std::string>(*energy_source_value_ptr);
            }

            const boost::json::value * factor_value_ptr = reactor_object.if_contains("factor");
            if (factor_value_ptr != nullptr) {
                components.reactor.factor = boost::json::value_to<double>(*factor_value_ptr);
            }

        }


        const boost::json::value * shield_value_ptr = components_object.if_contains("shield");
        if (shield_value_ptr != nullptr) {
            boost::json::object shield_object = shield_value_ptr->get_object();
            const boost::json::value * energy_source_value_ptr = shield_object.if_contains("energy_source");
            if (energy_source_value_ptr != nullptr) {
                components.shield.energy_source = boost::json::value_to<std::string>(*energy_source_value_ptr);
            }

            const boost::json::value * maintenance_factor_value_ptr = shield_object.if_contains("maintenance_factor");
            if (maintenance_factor_value_ptr != nullptr) {
                components.shield.maintenance_factor = boost::json::value_to<double>(*maintenance_factor_value_ptr);
            }

            const boost::json::value * regeneration_factor_value_ptr = shield_object.if_contains("regeneration_factor");
            if (regeneration_factor_value_ptr != nullptr) {
                components.shield.regeneration_factor = boost::json::value_to<double>(*regeneration_factor_value_ptr);
            }

        }


        }


        const boost::json::value * constants_value_ptr = root_object.if_contains("constants");
        if (constants_value_ptr != nullptr) {
            boost::json::object constants_object = constants_value_ptr->get_object();
            const boost::json::value * megajoules_multiplier_value_ptr = constants_object.if_contains("megajoules_multiplier");
            if (megajoules_multiplier_value_ptr != nullptr) {
                constants.megajoules_multiplier = boost::json::value_to<int>(*megajoules_multiplier_value_ptr);
            }

            const boost::json::value * kilo_value_ptr = constants_object.if_contains("kilo");
            if (kilo_value_ptr != nullptr) {
                constants.kilo = boost::json::value_to<int>(*kilo_value_ptr);
            }

            const boost::json::value * kj_per_unit_damage_value_ptr = constants_object.if_contains("kj_per_unit_damage");
            if (kj_per_unit_damage_value_ptr != nullptr) {
                constants.kj_per_unit_damage = boost::json::value_to<int>(*kj_per_unit_damage_value_ptr);
            }

        }


        const boost::json::value * data_value_ptr = root_object.if_contains("data");
        if (data_value_ptr != nullptr) {
            boost::json::object data_object = data_value_ptr->get_object();
            const boost::json::value * ai_directory_value_ptr = data_object.if_contains("ai_directory");
            if (ai_directory_value_ptr != nullptr) {
                data.ai_directory = boost::json::value_to<std::string>(*ai_directory_value_ptr);
            }

            const boost::json::value * animations_value_ptr = data_object.if_contains("animations");
            if (animations_value_ptr != nullptr) {
                data.animations = boost::json::value_to<std::string>(*animations_value_ptr);
            }

            const boost::json::value * capship_roles_value_ptr = data_object.if_contains("capship_roles");
            if (capship_roles_value_ptr != nullptr) {
                data.capship_roles = boost::json::value_to<std::string>(*capship_roles_value_ptr);
            }

            const boost::json::value * cockpits_value_ptr = data_object.if_contains("cockpits");
            if (cockpits_value_ptr != nullptr) {
                data.cockpits = boost::json::value_to<std::string>(*cockpits_value_ptr);
            }

            const boost::json::value * data_dir_value_ptr = data_object.if_contains("data_dir");
            if (data_dir_value_ptr != nullptr) {
                data.data_dir = boost::json::value_to<std::string>(*data_dir_value_ptr);
            }

            const boost::json::value * empty_cell_check_value_ptr = data_object.if_contains("empty_cell_check");
            if (empty_cell_check_value_ptr != nullptr) {
                data.empty_cell_check = boost::json::value_to<bool>(*empty_cell_check_value_ptr);
            }

            const boost::json::value * hqtextures_value_ptr = data_object.if_contains("hqtextures");
            if (hqtextures_value_ptr != nullptr) {
                data.hqtextures = boost::json::value_to<std::string>(*hqtextures_value_ptr);
            }

            const boost::json::value * master_part_list_value_ptr = data_object.if_contains("master_part_list");
            if (master_part_list_value_ptr != nullptr) {
                data.master_part_list = boost::json::value_to<std::string>(*master_part_list_value_ptr);
            }

            const boost::json::value * mod_unit_csv_value_ptr = data_object.if_contains("mod_unit_csv");
            if (mod_unit_csv_value_ptr != nullptr) {
                data.mod_unit_csv = boost::json::value_to<std::string>(*mod_unit_csv_value_ptr);
            }

            const boost::json::value * movies_value_ptr = data_object.if_contains("movies");
            if (movies_value_ptr != nullptr) {
                data.movies = boost::json::value_to<std::string>(*movies_value_ptr);
            }

            const boost::json::value * python_bases_value_ptr = data_object.if_contains("python_bases");
            if (python_bases_value_ptr != nullptr) {
                data.python_bases = boost::json::value_to<std::string>(*python_bases_value_ptr);
            }

            const boost::json::value * python_modules_value_ptr = data_object.if_contains("python_modules");
            if (python_modules_value_ptr != nullptr) {
                data.python_modules = boost::json::value_to<std::string>(*python_modules_value_ptr);
            }

            const boost::json::value * sectors_value_ptr = data_object.if_contains("sectors");
            if (sectors_value_ptr != nullptr) {
                data.sectors = boost::json::value_to<std::string>(*sectors_value_ptr);
            }

            const boost::json::value * serialized_xml_value_ptr = data_object.if_contains("serialized_xml");
            if (serialized_xml_value_ptr != nullptr) {
                data.serialized_xml = boost::json::value_to<std::string>(*serialized_xml_value_ptr);
            }

            const boost::json::value * shared_meshes_value_ptr = data_object.if_contains("shared_meshes");
            if (shared_meshes_value_ptr != nullptr) {
                data.shared_meshes = boost::json::value_to<std::string>(*shared_meshes_value_ptr);
            }

            const boost::json::value * shared_sounds_value_ptr = data_object.if_contains("shared_sounds");
            if (shared_sounds_value_ptr != nullptr) {
                data.shared_sounds = boost::json::value_to<std::string>(*shared_sounds_value_ptr);
            }

            const boost::json::value * shared_textures_value_ptr = data_object.if_contains("shared_textures");
            if (shared_textures_value_ptr != nullptr) {
                data.shared_textures = boost::json::value_to<std::string>(*shared_textures_value_ptr);
            }

            const boost::json::value * shared_units_value_ptr = data_object.if_contains("shared_units");
            if (shared_units_value_ptr != nullptr) {
                data.shared_units = boost::json::value_to<std::string>(*shared_units_value_ptr);
            }

            const boost::json::value * sprites_value_ptr = data_object.if_contains("sprites");
            if (sprites_value_ptr != nullptr) {
                data.sprites = boost::json::value_to<std::string>(*sprites_value_ptr);
            }

            const boost::json::value * techniques_base_path_value_ptr = data_object.if_contains("techniques_base_path");
            if (techniques_base_path_value_ptr != nullptr) {
                data.techniques_base_path = boost::json::value_to<std::string>(*techniques_base_path_value_ptr);
            }

            const boost::json::value * universe_path_value_ptr = data_object.if_contains("universe_path");
            if (universe_path_value_ptr != nullptr) {
                data.universe_path = boost::json::value_to<std::string>(*universe_path_value_ptr);
            }

            const boost::json::value * using_templates_value_ptr = data_object.if_contains("using_templates");
            if (using_templates_value_ptr != nullptr) {
                data.using_templates = boost::json::value_to<bool>(*using_templates_value_ptr);
            }

            const boost::json::value * volume_format_value_ptr = data_object.if_contains("volume_format");
            if (volume_format_value_ptr != nullptr) {
                data.volume_format = boost::json::value_to<std::string>(*volume_format_value_ptr);
            }

        }


        const boost::json::value * game_start_value_ptr = root_object.if_contains("game_start");
        if (game_start_value_ptr != nullptr) {
            boost::json::object game_start_object = game_start_value_ptr->get_object();
            const boost::json::value * campaigns_value_ptr = game_start_object.if_contains("campaigns");
            if (campaigns_value_ptr != nullptr) {
                game_start.campaigns = boost::json::value_to<std::string>(*campaigns_value_ptr);
            }

            const boost::json::value * default_mission_value_ptr = game_start_object.if_contains("default_mission");
            if (default_mission_value_ptr != nullptr) {
                game_start.default_mission = boost::json::value_to<std::string>(*default_mission_value_ptr);
            }

            const boost::json::value * introduction_value_ptr = game_start_object.if_contains("introduction");
            if (introduction_value_ptr != nullptr) {
                game_start.introduction = boost::json::value_to<std::string>(*introduction_value_ptr);
            }

            const boost::json::value * galaxy_value_ptr = game_start_object.if_contains("galaxy");
            if (galaxy_value_ptr != nullptr) {
                game_start.galaxy = boost::json::value_to<std::string>(*galaxy_value_ptr);
            }

        }


        const boost::json::value * advanced_value_ptr = root_object.if_contains("advanced");
        if (advanced_value_ptr != nullptr) {
            boost::json::object advanced_object = advanced_value_ptr->get_object();
        }


        const boost::json::value * ai_value_ptr = root_object.if_contains("ai");
        if (ai_value_ptr != nullptr) {
            boost::json::object ai_object = ai_value_ptr->get_object();
            const boost::json::value * afterburn_to_no_enemies_value_ptr = ai_object.if_contains("afterburn_to_no_enemies");
            if (afterburn_to_no_enemies_value_ptr != nullptr) {
                ai.afterburn_to_no_enemies = boost::json::value_to<bool>(*afterburn_to_no_enemies_value_ptr);
            }

            const boost::json::value * ai_cheat_dot_value_ptr = ai_object.if_contains("ai_cheat_dot");
            if (ai_cheat_dot_value_ptr != nullptr) {
                ai.ai_cheat_dot = boost::json::value_to<double>(*ai_cheat_dot_value_ptr);
            }

            const boost::json::value * allow_any_speed_reference_value_ptr = ai_object.if_contains("allow_any_speed_reference");
            if (allow_any_speed_reference_value_ptr != nullptr) {
                ai.allow_any_speed_reference = boost::json::value_to<bool>(*allow_any_speed_reference_value_ptr);
            }

            const boost::json::value * allow_civil_war_value_ptr = ai_object.if_contains("allow_civil_war");
            if (allow_civil_war_value_ptr != nullptr) {
                ai.allow_civil_war = boost::json::value_to<bool>(*allow_civil_war_value_ptr);
            }

            const boost::json::value * allow_nonplayer_faction_change_value_ptr = ai_object.if_contains("allow_nonplayer_faction_change");
            if (allow_nonplayer_faction_change_value_ptr != nullptr) {
                ai.allow_nonplayer_faction_change = boost::json::value_to<bool>(*allow_nonplayer_faction_change_value_ptr);
            }

            const boost::json::value * always_fire_autotrackers_value_ptr = ai_object.if_contains("always_fire_autotrackers");
            if (always_fire_autotrackers_value_ptr != nullptr) {
                ai.always_fire_autotrackers = boost::json::value_to<bool>(*always_fire_autotrackers_value_ptr);
            }

            const boost::json::value * always_have_jumpdrive_cheat_value_ptr = ai_object.if_contains("always_have_jumpdrive_cheat");
            if (always_have_jumpdrive_cheat_value_ptr != nullptr) {
                ai.always_have_jumpdrive_cheat = boost::json::value_to<bool>(*always_have_jumpdrive_cheat_value_ptr);
            }

            const boost::json::value * always_obedient_value_ptr = ai_object.if_contains("always_obedient");
            if (always_obedient_value_ptr != nullptr) {
                ai.always_obedient = boost::json::value_to<bool>(*always_obedient_value_ptr);
            }

            const boost::json::value * always_use_itts_value_ptr = ai_object.if_contains("always_use_itts");
            if (always_use_itts_value_ptr != nullptr) {
                ai.always_use_itts = boost::json::value_to<bool>(*always_use_itts_value_ptr);
            }

            const boost::json::value * anger_affects_response_value_ptr = ai_object.if_contains("anger_affects_response");
            if (anger_affects_response_value_ptr != nullptr) {
                ai.anger_affects_response = boost::json::value_to<double>(*anger_affects_response_value_ptr);
            }

            const boost::json::value * assist_friend_in_need_value_ptr = ai_object.if_contains("assist_friend_in_need");
            if (assist_friend_in_need_value_ptr != nullptr) {
                ai.assist_friend_in_need = boost::json::value_to<bool>(*assist_friend_in_need_value_ptr);
            }

            const boost::json::value * attacker_switch_time_value_ptr = ai_object.if_contains("attacker_switch_time");
            if (attacker_switch_time_value_ptr != nullptr) {
                ai.attacker_switch_time = boost::json::value_to<double>(*attacker_switch_time_value_ptr);
            }

            const boost::json::value * auto_dock_value_ptr = ai_object.if_contains("auto_dock");
            if (auto_dock_value_ptr != nullptr) {
                ai.auto_dock = boost::json::value_to<bool>(*auto_dock_value_ptr);
            }

            const boost::json::value * capped_faction_rating_value_ptr = ai_object.if_contains("capped_faction_rating");
            if (capped_faction_rating_value_ptr != nullptr) {
                ai.capped_faction_rating = boost::json::value_to<bool>(*capped_faction_rating_value_ptr);
            }

            const boost::json::value * choose_destination_script_value_ptr = ai_object.if_contains("choose_destination_script");
            if (choose_destination_script_value_ptr != nullptr) {
                ai.choose_destination_script = boost::json::value_to<std::string>(*choose_destination_script_value_ptr);
            }

            const boost::json::value * comm_initiate_time_value_ptr = ai_object.if_contains("comm_initiate_time");
            if (comm_initiate_time_value_ptr != nullptr) {
                ai.comm_initiate_time = boost::json::value_to<double>(*comm_initiate_time_value_ptr);
            }

            const boost::json::value * comm_response_time_value_ptr = ai_object.if_contains("comm_response_time");
            if (comm_response_time_value_ptr != nullptr) {
                ai.comm_response_time = boost::json::value_to<double>(*comm_response_time_value_ptr);
            }

            const boost::json::value * comm_to_player_percent_value_ptr = ai_object.if_contains("comm_to_player_percent");
            if (comm_to_player_percent_value_ptr != nullptr) {
                ai.comm_to_player_percent = boost::json::value_to<double>(*comm_to_player_percent_value_ptr);
            }

            const boost::json::value * comm_to_target_percent_value_ptr = ai_object.if_contains("comm_to_target_percent");
            if (comm_to_target_percent_value_ptr != nullptr) {
                ai.comm_to_target_percent = boost::json::value_to<double>(*comm_to_target_percent_value_ptr);
            }

            const boost::json::value * contraband_initiate_time_value_ptr = ai_object.if_contains("contraband_initiate_time");
            if (contraband_initiate_time_value_ptr != nullptr) {
                ai.contraband_initiate_time = boost::json::value_to<double>(*contraband_initiate_time_value_ptr);
            }

            const boost::json::value * contraband_madness_value_ptr = ai_object.if_contains("contraband_madness");
            if (contraband_madness_value_ptr != nullptr) {
                ai.contraband_madness = boost::json::value_to<int>(*contraband_madness_value_ptr);
            }

            const boost::json::value * contraband_to_player_percent_value_ptr = ai_object.if_contains("contraband_to_player_percent");
            if (contraband_to_player_percent_value_ptr != nullptr) {
                ai.contraband_to_player_percent = boost::json::value_to<double>(*contraband_to_player_percent_value_ptr);
            }

            const boost::json::value * contraband_to_target_percent_value_ptr = ai_object.if_contains("contraband_to_target_percent");
            if (contraband_to_target_percent_value_ptr != nullptr) {
                ai.contraband_to_target_percent = boost::json::value_to<double>(*contraband_to_target_percent_value_ptr);
            }

            const boost::json::value * contraband_update_time_value_ptr = ai_object.if_contains("contraband_update_time");
            if (contraband_update_time_value_ptr != nullptr) {
                ai.contraband_update_time = boost::json::value_to<double>(*contraband_update_time_value_ptr);
            }

            const boost::json::value * debug_level_value_ptr = ai_object.if_contains("debug_level");
            if (debug_level_value_ptr != nullptr) {
                ai.debug_level = boost::json::value_to<int>(*debug_level_value_ptr);
            }

            const boost::json::value * default_rank_value_ptr = ai_object.if_contains("default_rank");
            if (default_rank_value_ptr != nullptr) {
                ai.default_rank = boost::json::value_to<double>(*default_rank_value_ptr);
            }

            const boost::json::value * dock_on_load_value_ptr = ai_object.if_contains("dock_on_load");
            if (dock_on_load_value_ptr != nullptr) {
                ai.dock_on_load = boost::json::value_to<bool>(*dock_on_load_value_ptr);
            }

            const boost::json::value * dock_to_area_value_ptr = ai_object.if_contains("dock_to_area");
            if (dock_to_area_value_ptr != nullptr) {
                ai.dock_to_area = boost::json::value_to<bool>(*dock_to_area_value_ptr);
            }

            const boost::json::value * docked_to_script_value_ptr = ai_object.if_contains("docked_to_script");
            if (docked_to_script_value_ptr != nullptr) {
                ai.docked_to_script = boost::json::value_to<std::string>(*docked_to_script_value_ptr);
            }

            const boost::json::value * ease_to_anger_value_ptr = ai_object.if_contains("ease_to_anger");
            if (ease_to_anger_value_ptr != nullptr) {
                ai.ease_to_anger = boost::json::value_to<double>(*ease_to_anger_value_ptr);
            }

            const boost::json::value * ease_to_appease_value_ptr = ai_object.if_contains("ease_to_appease");
            if (ease_to_appease_value_ptr != nullptr) {
                ai.ease_to_appease = boost::json::value_to<double>(*ease_to_appease_value_ptr);
            }

            const boost::json::value * eject_attacks_value_ptr = ai_object.if_contains("eject_attacks");
            if (eject_attacks_value_ptr != nullptr) {
                ai.eject_attacks = boost::json::value_to<bool>(*eject_attacks_value_ptr);
            }

            const boost::json::value * evasion_angle_value_ptr = ai_object.if_contains("evasion_angle");
            if (evasion_angle_value_ptr != nullptr) {
                ai.evasion_angle = boost::json::value_to<double>(*evasion_angle_value_ptr);
            }

            const boost::json::value * faction_contraband_relation_adjust_value_ptr = ai_object.if_contains("faction_contraband_relation_adjust");
            if (faction_contraband_relation_adjust_value_ptr != nullptr) {
                ai.faction_contraband_relation_adjust = boost::json::value_to<double>(*faction_contraband_relation_adjust_value_ptr);
            }

            const boost::json::value * fg_nav_select_time_value_ptr = ai_object.if_contains("fg_nav_select_time");
            if (fg_nav_select_time_value_ptr != nullptr) {
                ai.fg_nav_select_time = boost::json::value_to<double>(*fg_nav_select_time_value_ptr);
            }

            const boost::json::value * force_jump_after_time_value_ptr = ai_object.if_contains("force_jump_after_time");
            if (force_jump_after_time_value_ptr != nullptr) {
                ai.force_jump_after_time = boost::json::value_to<double>(*force_jump_after_time_value_ptr);
            }

            const boost::json::value * friend_factor_value_ptr = ai_object.if_contains("friend_factor");
            if (friend_factor_value_ptr != nullptr) {
                ai.friend_factor = boost::json::value_to<double>(*friend_factor_value_ptr);
            }

            const boost::json::value * gun_range_percent_ok_value_ptr = ai_object.if_contains("gun_range_percent_ok");
            if (gun_range_percent_ok_value_ptr != nullptr) {
                ai.gun_range_percent_ok = boost::json::value_to<double>(*gun_range_percent_ok_value_ptr);
            }

            const boost::json::value * hostile_lurk_value_ptr = ai_object.if_contains("hostile_lurk");
            if (hostile_lurk_value_ptr != nullptr) {
                ai.hostile_lurk = boost::json::value_to<bool>(*hostile_lurk_value_ptr);
            }

            const boost::json::value * how_far_to_stop_navigating_value_ptr = ai_object.if_contains("how_far_to_stop_navigating");
            if (how_far_to_stop_navigating_value_ptr != nullptr) {
                ai.how_far_to_stop_navigating = boost::json::value_to<double>(*how_far_to_stop_navigating_value_ptr);
            }

            const boost::json::value * hull_damage_anger_value_ptr = ai_object.if_contains("hull_damage_anger");
            if (hull_damage_anger_value_ptr != nullptr) {
                ai.hull_damage_anger = boost::json::value_to<int>(*hull_damage_anger_value_ptr);
            }

            const boost::json::value * hull_percent_for_comm_value_ptr = ai_object.if_contains("hull_percent_for_comm");
            if (hull_percent_for_comm_value_ptr != nullptr) {
                ai.hull_percent_for_comm = boost::json::value_to<double>(*hull_percent_for_comm_value_ptr);
            }

            const boost::json::value * independent_turrets_value_ptr = ai_object.if_contains("independent_turrets");
            if (independent_turrets_value_ptr != nullptr) {
                ai.independent_turrets = boost::json::value_to<bool>(*independent_turrets_value_ptr);
            }

            const boost::json::value * jump_cheat_value_ptr = ai_object.if_contains("jump_cheat");
            if (jump_cheat_value_ptr != nullptr) {
                ai.jump_cheat = boost::json::value_to<bool>(*jump_cheat_value_ptr);
            }

            const boost::json::value * jump_without_energy_value_ptr = ai_object.if_contains("jump_without_energy");
            if (jump_without_energy_value_ptr != nullptr) {
                ai.jump_without_energy = boost::json::value_to<bool>(*jump_without_energy_value_ptr);
            }

            const boost::json::value * kill_factor_value_ptr = ai_object.if_contains("kill_factor");
            if (kill_factor_value_ptr != nullptr) {
                ai.kill_factor = boost::json::value_to<double>(*kill_factor_value_ptr);
            }

            const boost::json::value * loop_around_destination_distance_value_ptr = ai_object.if_contains("loop_around_destination_distance");
            if (loop_around_destination_distance_value_ptr != nullptr) {
                ai.loop_around_destination_distance = boost::json::value_to<double>(*loop_around_destination_distance_value_ptr);
            }

            const boost::json::value * loop_around_destination_lateral_value_ptr = ai_object.if_contains("loop_around_destination_lateral");
            if (loop_around_destination_lateral_value_ptr != nullptr) {
                ai.loop_around_destination_lateral = boost::json::value_to<double>(*loop_around_destination_lateral_value_ptr);
            }

            const boost::json::value * loop_around_destination_vertical_value_ptr = ai_object.if_contains("loop_around_destination_vertical");
            if (loop_around_destination_vertical_value_ptr != nullptr) {
                ai.loop_around_destination_vertical = boost::json::value_to<double>(*loop_around_destination_vertical_value_ptr);
            }

            const boost::json::value * loop_around_distance_value_ptr = ai_object.if_contains("loop_around_distance");
            if (loop_around_distance_value_ptr != nullptr) {
                ai.loop_around_distance = boost::json::value_to<double>(*loop_around_distance_value_ptr);
            }

            const boost::json::value * loop_around_pursuit_velocity_percent_value_ptr = ai_object.if_contains("loop_around_pursuit_velocity_percent");
            if (loop_around_pursuit_velocity_percent_value_ptr != nullptr) {
                ai.loop_around_pursuit_velocity_percent = boost::json::value_to<double>(*loop_around_pursuit_velocity_percent_value_ptr);
            }

            const boost::json::value * lurk_time_value_ptr = ai_object.if_contains("lurk_time");
            if (lurk_time_value_ptr != nullptr) {
                ai.lurk_time = boost::json::value_to<double>(*lurk_time_value_ptr);
            }

            const boost::json::value * lowest_negative_comm_choice_value_ptr = ai_object.if_contains("lowest_negative_comm_choice");
            if (lowest_negative_comm_choice_value_ptr != nullptr) {
                ai.lowest_negative_comm_choice = boost::json::value_to<double>(*lowest_negative_comm_choice_value_ptr);
            }

            const boost::json::value * lowest_positive_comm_choice_value_ptr = ai_object.if_contains("lowest_positive_comm_choice");
            if (lowest_positive_comm_choice_value_ptr != nullptr) {
                ai.lowest_positive_comm_choice = boost::json::value_to<double>(*lowest_positive_comm_choice_value_ptr);
            }

            const boost::json::value * match_velocity_cone_value_ptr = ai_object.if_contains("match_velocity_cone");
            if (match_velocity_cone_value_ptr != nullptr) {
                ai.match_velocity_cone = boost::json::value_to<double>(*match_velocity_cone_value_ptr);
            }

            const boost::json::value * match_velocity_of_pursuant_value_ptr = ai_object.if_contains("match_velocity_of_pursuant");
            if (match_velocity_of_pursuant_value_ptr != nullptr) {
                ai.match_velocity_of_pursuant = boost::json::value_to<bool>(*match_velocity_of_pursuant_value_ptr);
            }

            const boost::json::value * max_allowable_travel_time_value_ptr = ai_object.if_contains("max_allowable_travel_time");
            if (max_allowable_travel_time_value_ptr != nullptr) {
                ai.max_allowable_travel_time = boost::json::value_to<double>(*max_allowable_travel_time_value_ptr);
            }

            const boost::json::value * max_faction_contraband_relation_value_ptr = ai_object.if_contains("max_faction_contraband_relation");
            if (max_faction_contraband_relation_value_ptr != nullptr) {
                ai.max_faction_contraband_relation = boost::json::value_to<double>(*max_faction_contraband_relation_value_ptr);
            }

            const boost::json::value * max_player_attackers_value_ptr = ai_object.if_contains("max_player_attackers");
            if (max_player_attackers_value_ptr != nullptr) {
                ai.max_player_attackers = boost::json::value_to<int>(*max_player_attackers_value_ptr);
            }

            const boost::json::value * min_angular_accel_cheat_value_ptr = ai_object.if_contains("min_angular_accel_cheat");
            if (min_angular_accel_cheat_value_ptr != nullptr) {
                ai.min_angular_accel_cheat = boost::json::value_to<double>(*min_angular_accel_cheat_value_ptr);
            }

            const boost::json::value * min_energy_to_enter_warp_value_ptr = ai_object.if_contains("min_energy_to_enter_warp");
            if (min_energy_to_enter_warp_value_ptr != nullptr) {
                ai.min_energy_to_enter_warp = boost::json::value_to<double>(*min_energy_to_enter_warp_value_ptr);
            }

            const boost::json::value * min_relationship_value_ptr = ai_object.if_contains("min_relationship");
            if (min_relationship_value_ptr != nullptr) {
                ai.min_relationship = boost::json::value_to<double>(*min_relationship_value_ptr);
            }

            const boost::json::value * min_time_to_auto_value_ptr = ai_object.if_contains("min_time_to_auto");
            if (min_time_to_auto_value_ptr != nullptr) {
                ai.min_time_to_auto = boost::json::value_to<double>(*min_time_to_auto_value_ptr);
            }

            const boost::json::value * min_warp_to_try_value_ptr = ai_object.if_contains("min_warp_to_try");
            if (min_warp_to_try_value_ptr != nullptr) {
                ai.min_warp_to_try = boost::json::value_to<double>(*min_warp_to_try_value_ptr);
            }

            const boost::json::value * missile_gun_delay_value_ptr = ai_object.if_contains("missile_gun_delay");
            if (missile_gun_delay_value_ptr != nullptr) {
                ai.missile_gun_delay = boost::json::value_to<double>(*missile_gun_delay_value_ptr);
            }

            const boost::json::value * mood_affects_response_value_ptr = ai_object.if_contains("mood_affects_response");
            if (mood_affects_response_value_ptr != nullptr) {
                ai.mood_affects_response = boost::json::value_to<double>(*mood_affects_response_value_ptr);
            }

            const boost::json::value * mood_swing_level_value_ptr = ai_object.if_contains("mood_swing_level");
            if (mood_swing_level_value_ptr != nullptr) {
                ai.mood_swing_level = boost::json::value_to<double>(*mood_swing_level_value_ptr);
            }

            const boost::json::value * num_contraband_scans_per_search_value_ptr = ai_object.if_contains("num_contraband_scans_per_search");
            if (num_contraband_scans_per_search_value_ptr != nullptr) {
                ai.num_contraband_scans_per_search = boost::json::value_to<int>(*num_contraband_scans_per_search_value_ptr);
            }

            const boost::json::value * num_pirates_per_asteroid_field_value_ptr = ai_object.if_contains("num_pirates_per_asteroid_field");
            if (num_pirates_per_asteroid_field_value_ptr != nullptr) {
                ai.num_pirates_per_asteroid_field = boost::json::value_to<int>(*num_pirates_per_asteroid_field_value_ptr);
            }

            const boost::json::value * only_upgrade_speed_reference_value_ptr = ai_object.if_contains("only_upgrade_speed_reference");
            if (only_upgrade_speed_reference_value_ptr != nullptr) {
                ai.only_upgrade_speed_reference = boost::json::value_to<bool>(*only_upgrade_speed_reference_value_ptr);
            }

            const boost::json::value * percentage_speed_change_to_stop_search_value_ptr = ai_object.if_contains("percentage_speed_change_to_stop_search");
            if (percentage_speed_change_to_stop_search_value_ptr != nullptr) {
                ai.percentage_speed_change_to_stop_search = boost::json::value_to<double>(*percentage_speed_change_to_stop_search_value_ptr);
            }

            const boost::json::value * pirate_bonus_for_empty_hold_value_ptr = ai_object.if_contains("pirate_bonus_for_empty_hold");
            if (pirate_bonus_for_empty_hold_value_ptr != nullptr) {
                ai.pirate_bonus_for_empty_hold = boost::json::value_to<double>(*pirate_bonus_for_empty_hold_value_ptr);
            }

            const boost::json::value * pirate_nav_select_time_value_ptr = ai_object.if_contains("pirate_nav_select_time");
            if (pirate_nav_select_time_value_ptr != nullptr) {
                ai.pirate_nav_select_time = boost::json::value_to<double>(*pirate_nav_select_time_value_ptr);
            }

            const boost::json::value * random_response_range_value_ptr = ai_object.if_contains("random_response_range");
            if (random_response_range_value_ptr != nullptr) {
                ai.random_response_range = boost::json::value_to<double>(*random_response_range_value_ptr);
            }

            const boost::json::value * random_spacing_factor_value_ptr = ai_object.if_contains("random_spacing_factor");
            if (random_spacing_factor_value_ptr != nullptr) {
                ai.random_spacing_factor = boost::json::value_to<double>(*random_spacing_factor_value_ptr);
            }

            const boost::json::value * reaction_time_value_ptr = ai_object.if_contains("reaction_time");
            if (reaction_time_value_ptr != nullptr) {
                ai.reaction_time = boost::json::value_to<double>(*reaction_time_value_ptr);
            }

            const boost::json::value * resistance_to_side_movement_value_ptr = ai_object.if_contains("resistance_to_side_movement");
            if (resistance_to_side_movement_value_ptr != nullptr) {
                ai.resistance_to_side_movement = boost::json::value_to<bool>(*resistance_to_side_movement_value_ptr);
            }

            const boost::json::value * resistance_to_side_force_percent_value_ptr = ai_object.if_contains("resistance_to_side_force_percent");
            if (resistance_to_side_force_percent_value_ptr != nullptr) {
                ai.resistance_to_side_force_percent = boost::json::value_to<double>(*resistance_to_side_force_percent_value_ptr);
            }

            const boost::json::value * resistance_to_side_movement_percent_value_ptr = ai_object.if_contains("resistance_to_side_movement_percent");
            if (resistance_to_side_movement_percent_value_ptr != nullptr) {
                ai.resistance_to_side_movement_percent = boost::json::value_to<double>(*resistance_to_side_movement_percent_value_ptr);
            }

            const boost::json::value * roll_order_duration_value_ptr = ai_object.if_contains("roll_order_duration");
            if (roll_order_duration_value_ptr != nullptr) {
                ai.roll_order_duration = boost::json::value_to<double>(*roll_order_duration_value_ptr);
            }

            const boost::json::value * safety_spacing_value_ptr = ai_object.if_contains("safety_spacing");
            if (safety_spacing_value_ptr != nullptr) {
                ai.safety_spacing = boost::json::value_to<double>(*safety_spacing_value_ptr);
            }

            const boost::json::value * shield_damage_anger_value_ptr = ai_object.if_contains("shield_damage_anger");
            if (shield_damage_anger_value_ptr != nullptr) {
                ai.shield_damage_anger = boost::json::value_to<int>(*shield_damage_anger_value_ptr);
            }

            const boost::json::value * slow_diplomacy_for_enemies_value_ptr = ai_object.if_contains("slow_diplomacy_for_enemies");
            if (slow_diplomacy_for_enemies_value_ptr != nullptr) {
                ai.slow_diplomacy_for_enemies = boost::json::value_to<double>(*slow_diplomacy_for_enemies_value_ptr);
            }

            const boost::json::value * start_docked_to_value_ptr = ai_object.if_contains("start_docked_to");
            if (start_docked_to_value_ptr != nullptr) {
                ai.start_docked_to = boost::json::value_to<std::string>(*start_docked_to_value_ptr);
            }

            const boost::json::value * static_relationship_affects_response_value_ptr = ai_object.if_contains("static_relationship_affects_response");
            if (static_relationship_affects_response_value_ptr != nullptr) {
                ai.static_relationship_affects_response = boost::json::value_to<double>(*static_relationship_affects_response_value_ptr);
            }

            const boost::json::value * switch_nonowned_units_value_ptr = ai_object.if_contains("switch_nonowned_units");
            if (switch_nonowned_units_value_ptr != nullptr) {
                ai.switch_nonowned_units = boost::json::value_to<bool>(*switch_nonowned_units_value_ptr);
            }

            const boost::json::value * talk_relation_factor_value_ptr = ai_object.if_contains("talk_relation_factor");
            if (talk_relation_factor_value_ptr != nullptr) {
                ai.talk_relation_factor = boost::json::value_to<double>(*talk_relation_factor_value_ptr);
            }

            const boost::json::value * talking_faster_helps_value_ptr = ai_object.if_contains("talking_faster_helps");
            if (talking_faster_helps_value_ptr != nullptr) {
                ai.talking_faster_helps = boost::json::value_to<bool>(*talking_faster_helps_value_ptr);
            }

            const boost::json::value * too_close_for_warp_in_formation_value_ptr = ai_object.if_contains("too_close_for_warp_in_formation");
            if (too_close_for_warp_in_formation_value_ptr != nullptr) {
                ai.too_close_for_warp_in_formation = boost::json::value_to<double>(*too_close_for_warp_in_formation_value_ptr);
            }

            const boost::json::value * too_close_for_warp_tactic_value_ptr = ai_object.if_contains("too_close_for_warp_tactic");
            if (too_close_for_warp_tactic_value_ptr != nullptr) {
                ai.too_close_for_warp_tactic = boost::json::value_to<double>(*too_close_for_warp_tactic_value_ptr);
            }

            const boost::json::value * turn_cheat_value_ptr = ai_object.if_contains("turn_cheat");
            if (turn_cheat_value_ptr != nullptr) {
                ai.turn_cheat = boost::json::value_to<bool>(*turn_cheat_value_ptr);
            }

            const boost::json::value * unknown_relation_enemy_value_ptr = ai_object.if_contains("unknown_relation_enemy");
            if (unknown_relation_enemy_value_ptr != nullptr) {
                ai.unknown_relation_enemy = boost::json::value_to<double>(*unknown_relation_enemy_value_ptr);
            }

            const boost::json::value * unknown_relation_hit_cost_value_ptr = ai_object.if_contains("unknown_relation_hit_cost");
            if (unknown_relation_hit_cost_value_ptr != nullptr) {
                ai.unknown_relation_hit_cost = boost::json::value_to<double>(*unknown_relation_hit_cost_value_ptr);
            }

            const boost::json::value * use_afterburner_value_ptr = ai_object.if_contains("use_afterburner");
            if (use_afterburner_value_ptr != nullptr) {
                ai.use_afterburner = boost::json::value_to<bool>(*use_afterburner_value_ptr);
            }

            const boost::json::value * use_afterburner_to_follow_value_ptr = ai_object.if_contains("use_afterburner_to_follow");
            if (use_afterburner_to_follow_value_ptr != nullptr) {
                ai.use_afterburner_to_follow = boost::json::value_to<bool>(*use_afterburner_to_follow_value_ptr);
            }

            const boost::json::value * use_afterburner_to_run_value_ptr = ai_object.if_contains("use_afterburner_to_run");
            if (use_afterburner_to_run_value_ptr != nullptr) {
                ai.use_afterburner_to_run = boost::json::value_to<bool>(*use_afterburner_to_run_value_ptr);
            }

            const boost::json::value * warp_cone_value_ptr = ai_object.if_contains("warp_cone");
            if (warp_cone_value_ptr != nullptr) {
                ai.warp_cone = boost::json::value_to<double>(*warp_cone_value_ptr);
            }

            const boost::json::value * warp_to_enemies_value_ptr = ai_object.if_contains("warp_to_enemies");
            if (warp_to_enemies_value_ptr != nullptr) {
                ai.warp_to_enemies = boost::json::value_to<bool>(*warp_to_enemies_value_ptr);
            }

            const boost::json::value * warp_to_no_enemies_value_ptr = ai_object.if_contains("warp_to_no_enemies");
            if (warp_to_no_enemies_value_ptr != nullptr) {
                ai.warp_to_no_enemies = boost::json::value_to<bool>(*warp_to_no_enemies_value_ptr);
            }

            const boost::json::value * warp_to_wingmen_value_ptr = ai_object.if_contains("warp_to_wingmen");
            if (warp_to_wingmen_value_ptr != nullptr) {
                ai.warp_to_wingmen = boost::json::value_to<bool>(*warp_to_wingmen_value_ptr);
            }

        const boost::json::value * firing_value_ptr = ai_object.if_contains("firing");
        if (firing_value_ptr != nullptr) {
            boost::json::object firing_object = firing_value_ptr->get_object();
            const boost::json::value * aggressivity_value_ptr = firing_object.if_contains("aggressivity");
            if (aggressivity_value_ptr != nullptr) {
                ai.firing.aggressivity = boost::json::value_to<double>(*aggressivity_value_ptr);
            }

            const boost::json::value * in_weapon_range_value_ptr = firing_object.if_contains("in_weapon_range");
            if (in_weapon_range_value_ptr != nullptr) {
                ai.firing.in_weapon_range = boost::json::value_to<double>(*in_weapon_range_value_ptr);
            }

            const boost::json::value * missile_probability_value_ptr = firing_object.if_contains("missile_probability");
            if (missile_probability_value_ptr != nullptr) {
                ai.firing.missile_probability = boost::json::value_to<double>(*missile_probability_value_ptr);
            }

            const boost::json::value * reaction_time_value_ptr = firing_object.if_contains("reaction_time");
            if (reaction_time_value_ptr != nullptr) {
                ai.firing.reaction_time = boost::json::value_to<double>(*reaction_time_value_ptr);
            }

            const boost::json::value * turret_dot_cutoff_value_ptr = firing_object.if_contains("turret_dot_cutoff");
            if (turret_dot_cutoff_value_ptr != nullptr) {
                ai.firing.turret_dot_cutoff = boost::json::value_to<double>(*turret_dot_cutoff_value_ptr);
            }

            const boost::json::value * turret_missile_probability_value_ptr = firing_object.if_contains("turret_missile_probability");
            if (turret_missile_probability_value_ptr != nullptr) {
                ai.firing.turret_missile_probability = boost::json::value_to<double>(*turret_missile_probability_value_ptr);
            }

        const boost::json::value * maximum_firing_angle_value_ptr = firing_object.if_contains("maximum_firing_angle");
        if (maximum_firing_angle_value_ptr != nullptr) {
            boost::json::object maximum_firing_angle_object = maximum_firing_angle_value_ptr->get_object();
            const boost::json::value * minagg_value_ptr = maximum_firing_angle_object.if_contains("minagg");
            if (minagg_value_ptr != nullptr) {
                ai.firing.maximum_firing_angle.minagg = boost::json::value_to<int>(*minagg_value_ptr);
            }

            const boost::json::value * maxagg_value_ptr = maximum_firing_angle_object.if_contains("maxagg");
            if (maxagg_value_ptr != nullptr) {
                ai.firing.maximum_firing_angle.maxagg = boost::json::value_to<int>(*maxagg_value_ptr);
            }

        }


        }


        const boost::json::value * targeting_value_ptr = ai_object.if_contains("targeting");
        if (targeting_value_ptr != nullptr) {
            boost::json::object targeting_object = targeting_value_ptr->get_object();
            const boost::json::value * assign_point_def_value_ptr = targeting_object.if_contains("assign_point_def");
            if (assign_point_def_value_ptr != nullptr) {
                ai.targeting.assign_point_def = boost::json::value_to<bool>(*assign_point_def_value_ptr);
            }

            const boost::json::value * escort_distance_value_ptr = targeting_object.if_contains("escort_distance");
            if (escort_distance_value_ptr != nullptr) {
                ai.targeting.escort_distance = boost::json::value_to<double>(*escort_distance_value_ptr);
            }

            const boost::json::value * mass_inertial_priority_cutoff_value_ptr = targeting_object.if_contains("mass_inertial_priority_cutoff");
            if (mass_inertial_priority_cutoff_value_ptr != nullptr) {
                ai.targeting.mass_inertial_priority_cutoff = boost::json::value_to<double>(*mass_inertial_priority_cutoff_value_ptr);
            }

            const boost::json::value * mass_inertial_priority_scale_value_ptr = targeting_object.if_contains("mass_inertial_priority_scale");
            if (mass_inertial_priority_scale_value_ptr != nullptr) {
                ai.targeting.mass_inertial_priority_scale = boost::json::value_to<double>(*mass_inertial_priority_scale_value_ptr);
            }

            const boost::json::value * max_number_of_pollers_per_frame_value_ptr = targeting_object.if_contains("max_number_of_pollers_per_frame");
            if (max_number_of_pollers_per_frame_value_ptr != nullptr) {
                ai.targeting.max_number_of_pollers_per_frame = boost::json::value_to<int>(*max_number_of_pollers_per_frame_value_ptr);
            }

            const boost::json::value * min_number_of_pollers_per_frame_value_ptr = targeting_object.if_contains("min_number_of_pollers_per_frame");
            if (min_number_of_pollers_per_frame_value_ptr != nullptr) {
                ai.targeting.min_number_of_pollers_per_frame = boost::json::value_to<int>(*min_number_of_pollers_per_frame_value_ptr);
            }

            const boost::json::value * min_rechoose_interval_value_ptr = targeting_object.if_contains("min_rechoose_interval");
            if (min_rechoose_interval_value_ptr != nullptr) {
                ai.targeting.min_rechoose_interval = boost::json::value_to<int>(*min_rechoose_interval_value_ptr);
            }

            const boost::json::value * min_time_to_switch_targets_value_ptr = targeting_object.if_contains("min_time_to_switch_targets");
            if (min_time_to_switch_targets_value_ptr != nullptr) {
                ai.targeting.min_time_to_switch_targets = boost::json::value_to<double>(*min_time_to_switch_targets_value_ptr);
            }

            const boost::json::value * min_null_time_to_switch_targets_value_ptr = targeting_object.if_contains("min_null_time_to_switch_targets");
            if (min_null_time_to_switch_targets_value_ptr != nullptr) {
                ai.targeting.min_null_time_to_switch_targets = boost::json::value_to<double>(*min_null_time_to_switch_targets_value_ptr);
            }

            const boost::json::value * mountless_gun_range_value_ptr = targeting_object.if_contains("mountless_gun_range");
            if (mountless_gun_range_value_ptr != nullptr) {
                ai.targeting.mountless_gun_range = boost::json::value_to<double>(*mountless_gun_range_value_ptr);
            }

            const boost::json::value * obedience_value_ptr = targeting_object.if_contains("obedience");
            if (obedience_value_ptr != nullptr) {
                ai.targeting.obedience = boost::json::value_to<double>(*obedience_value_ptr);
            }

            const boost::json::value * search_extra_radius_value_ptr = targeting_object.if_contains("search_extra_radius");
            if (search_extra_radius_value_ptr != nullptr) {
                ai.targeting.search_extra_radius = boost::json::value_to<double>(*search_extra_radius_value_ptr);
            }

            const boost::json::value * search_max_candidates_value_ptr = targeting_object.if_contains("search_max_candidates");
            if (search_max_candidates_value_ptr != nullptr) {
                ai.targeting.search_max_candidates = boost::json::value_to<int>(*search_max_candidates_value_ptr);
            }

            const boost::json::value * search_max_role_priority_value_ptr = targeting_object.if_contains("search_max_role_priority");
            if (search_max_role_priority_value_ptr != nullptr) {
                ai.targeting.search_max_role_priority = boost::json::value_to<int>(*search_max_role_priority_value_ptr);
            }

            const boost::json::value * threat_weight_value_ptr = targeting_object.if_contains("threat_weight");
            if (threat_weight_value_ptr != nullptr) {
                ai.targeting.threat_weight = boost::json::value_to<double>(*threat_weight_value_ptr);
            }

            const boost::json::value * time_to_recommand_wing_value_ptr = targeting_object.if_contains("time_to_recommand_wing");
            if (time_to_recommand_wing_value_ptr != nullptr) {
                ai.targeting.time_to_recommand_wing = boost::json::value_to<double>(*time_to_recommand_wing_value_ptr);
            }

            const boost::json::value * time_until_switch_value_ptr = targeting_object.if_contains("time_until_switch");
            if (time_until_switch_value_ptr != nullptr) {
                ai.targeting.time_until_switch = boost::json::value_to<double>(*time_until_switch_value_ptr);
            }

            const boost::json::value * turn_leader_distance_value_ptr = targeting_object.if_contains("turn_leader_distance");
            if (turn_leader_distance_value_ptr != nullptr) {
                ai.targeting.turn_leader_distance = boost::json::value_to<double>(*turn_leader_distance_value_ptr);
            }

        }


        }


        const boost::json::value * audio_value_ptr = root_object.if_contains("audio");
        if (audio_value_ptr != nullptr) {
            boost::json::object audio_object = audio_value_ptr->get_object();
            const boost::json::value * afterburner_gain_value_ptr = audio_object.if_contains("afterburner_gain");
            if (afterburner_gain_value_ptr != nullptr) {
                audio.afterburner_gain = boost::json::value_to<double>(*afterburner_gain_value_ptr);
            }

            const boost::json::value * ai_high_quality_weapon_value_ptr = audio_object.if_contains("ai_high_quality_weapon");
            if (ai_high_quality_weapon_value_ptr != nullptr) {
                audio.ai_high_quality_weapon = boost::json::value_to<bool>(*ai_high_quality_weapon_value_ptr);
            }

            const boost::json::value * ai_sound_value_ptr = audio_object.if_contains("ai_sound");
            if (ai_sound_value_ptr != nullptr) {
                audio.ai_sound = boost::json::value_to<bool>(*ai_sound_value_ptr);
            }

            const boost::json::value * audio_max_distance_value_ptr = audio_object.if_contains("audio_max_distance");
            if (audio_max_distance_value_ptr != nullptr) {
                audio.audio_max_distance = boost::json::value_to<double>(*audio_max_distance_value_ptr);
            }

            const boost::json::value * audio_ref_distance_value_ptr = audio_object.if_contains("audio_ref_distance");
            if (audio_ref_distance_value_ptr != nullptr) {
                audio.audio_ref_distance = boost::json::value_to<double>(*audio_ref_distance_value_ptr);
            }

            const boost::json::value * automatic_docking_zone_value_ptr = audio_object.if_contains("automatic_docking_zone");
            if (automatic_docking_zone_value_ptr != nullptr) {
                audio.automatic_docking_zone = boost::json::value_to<std::string>(*automatic_docking_zone_value_ptr);
            }

            const boost::json::value * battle_playlist_value_ptr = audio_object.if_contains("battle_playlist");
            if (battle_playlist_value_ptr != nullptr) {
                audio.battle_playlist = boost::json::value_to<std::string>(*battle_playlist_value_ptr);
            }

            const boost::json::value * buzzing_distance_value_ptr = audio_object.if_contains("buzzing_distance");
            if (buzzing_distance_value_ptr != nullptr) {
                audio.buzzing_distance = boost::json::value_to<double>(*buzzing_distance_value_ptr);
            }

            const boost::json::value * buzzing_needs_afterburner_value_ptr = audio_object.if_contains("buzzing_needs_afterburner");
            if (buzzing_needs_afterburner_value_ptr != nullptr) {
                audio.buzzing_needs_afterburner = boost::json::value_to<bool>(*buzzing_needs_afterburner_value_ptr);
            }

            const boost::json::value * buzzing_time_value_ptr = audio_object.if_contains("buzzing_time");
            if (buzzing_time_value_ptr != nullptr) {
                audio.buzzing_time = boost::json::value_to<double>(*buzzing_time_value_ptr);
            }

            const boost::json::value * cache_songs_value_ptr = audio_object.if_contains("cache_songs");
            if (cache_songs_value_ptr != nullptr) {
                audio.cache_songs = boost::json::value_to<std::string>(*cache_songs_value_ptr);
            }

            const boost::json::value * cross_fade_music_value_ptr = audio_object.if_contains("cross_fade_music");
            if (cross_fade_music_value_ptr != nullptr) {
                audio.cross_fade_music = boost::json::value_to<bool>(*cross_fade_music_value_ptr);
            }

            const boost::json::value * dj_script_value_ptr = audio_object.if_contains("dj_script");
            if (dj_script_value_ptr != nullptr) {
                audio.dj_script = boost::json::value_to<std::string>(*dj_script_value_ptr);
            }

            const boost::json::value * doppler_value_ptr = audio_object.if_contains("doppler");
            if (doppler_value_ptr != nullptr) {
                audio.doppler = boost::json::value_to<bool>(*doppler_value_ptr);
            }

            const boost::json::value * doppler_scale_value_ptr = audio_object.if_contains("doppler_scale");
            if (doppler_scale_value_ptr != nullptr) {
                audio.doppler_scale = boost::json::value_to<double>(*doppler_scale_value_ptr);
            }

            const boost::json::value * every_other_mount_value_ptr = audio_object.if_contains("every_other_mount");
            if (every_other_mount_value_ptr != nullptr) {
                audio.every_other_mount = boost::json::value_to<bool>(*every_other_mount_value_ptr);
            }

            const boost::json::value * explosion_closeness_value_ptr = audio_object.if_contains("explosion_closeness");
            if (explosion_closeness_value_ptr != nullptr) {
                audio.explosion_closeness = boost::json::value_to<double>(*explosion_closeness_value_ptr);
            }

            const boost::json::value * exterior_weapon_gain_value_ptr = audio_object.if_contains("exterior_weapon_gain");
            if (exterior_weapon_gain_value_ptr != nullptr) {
                audio.exterior_weapon_gain = boost::json::value_to<double>(*exterior_weapon_gain_value_ptr);
            }

            const boost::json::value * frequency_value_ptr = audio_object.if_contains("frequency");
            if (frequency_value_ptr != nullptr) {
                audio.frequency = boost::json::value_to<int>(*frequency_value_ptr);
            }

            const boost::json::value * high_quality_weapon_value_ptr = audio_object.if_contains("high_quality_weapon");
            if (high_quality_weapon_value_ptr != nullptr) {
                audio.high_quality_weapon = boost::json::value_to<bool>(*high_quality_weapon_value_ptr);
            }

            const boost::json::value * loading_sound_value_ptr = audio_object.if_contains("loading_sound");
            if (loading_sound_value_ptr != nullptr) {
                audio.loading_sound = boost::json::value_to<std::string>(*loading_sound_value_ptr);
            }

            const boost::json::value * loss_playlist_value_ptr = audio_object.if_contains("loss_playlist");
            if (loss_playlist_value_ptr != nullptr) {
                audio.loss_playlist = boost::json::value_to<std::string>(*loss_playlist_value_ptr);
            }

            const boost::json::value * loss_relationship_value_ptr = audio_object.if_contains("loss_relationship");
            if (loss_relationship_value_ptr != nullptr) {
                audio.loss_relationship = boost::json::value_to<double>(*loss_relationship_value_ptr);
            }

            const boost::json::value * max_single_sounds_value_ptr = audio_object.if_contains("max_single_sounds");
            if (max_single_sounds_value_ptr != nullptr) {
                audio.max_single_sounds = boost::json::value_to<int>(*max_single_sounds_value_ptr);
            }

            const boost::json::value * max_total_sounds_value_ptr = audio_object.if_contains("max_total_sounds");
            if (max_total_sounds_value_ptr != nullptr) {
                audio.max_total_sounds = boost::json::value_to<int>(*max_total_sounds_value_ptr);
            }

            const boost::json::value * max_range_to_hear_weapon_fire_value_ptr = audio_object.if_contains("max_range_to_hear_weapon_fire");
            if (max_range_to_hear_weapon_fire_value_ptr != nullptr) {
                audio.max_range_to_hear_weapon_fire = boost::json::value_to<double>(*max_range_to_hear_weapon_fire_value_ptr);
            }

            const boost::json::value * min_weapon_sound_refire_value_ptr = audio_object.if_contains("min_weapon_sound_refire");
            if (min_weapon_sound_refire_value_ptr != nullptr) {
                audio.min_weapon_sound_refire = boost::json::value_to<double>(*min_weapon_sound_refire_value_ptr);
            }

            const boost::json::value * mission_victory_song_value_ptr = audio_object.if_contains("mission_victory_song");
            if (mission_victory_song_value_ptr != nullptr) {
                audio.mission_victory_song = boost::json::value_to<std::string>(*mission_victory_song_value_ptr);
            }

            const boost::json::value * music_value_ptr = audio_object.if_contains("music");
            if (music_value_ptr != nullptr) {
                audio.music = boost::json::value_to<bool>(*music_value_ptr);
            }

            const boost::json::value * music_layers_value_ptr = audio_object.if_contains("music_layers");
            if (music_layers_value_ptr != nullptr) {
                audio.music_layers = boost::json::value_to<int>(*music_layers_value_ptr);
            }

            const boost::json::value * music_muting_fade_in_value_ptr = audio_object.if_contains("music_muting_fade_in");
            if (music_muting_fade_in_value_ptr != nullptr) {
                audio.music_muting_fade_in = boost::json::value_to<double>(*music_muting_fade_in_value_ptr);
            }

            const boost::json::value * music_muting_fade_out_value_ptr = audio_object.if_contains("music_muting_fade_out");
            if (music_muting_fade_out_value_ptr != nullptr) {
                audio.music_muting_fade_out = boost::json::value_to<double>(*music_muting_fade_out_value_ptr);
            }

            const boost::json::value * music_volume_value_ptr = audio_object.if_contains("music_volume");
            if (music_volume_value_ptr != nullptr) {
                audio.music_volume = boost::json::value_to<double>(*music_volume_value_ptr);
            }

            const boost::json::value * music_volume_down_latency_value_ptr = audio_object.if_contains("music_volume_down_latency");
            if (music_volume_down_latency_value_ptr != nullptr) {
                audio.music_volume_down_latency = boost::json::value_to<double>(*music_volume_down_latency_value_ptr);
            }

            const boost::json::value * music_volume_up_latency_value_ptr = audio_object.if_contains("music_volume_up_latency");
            if (music_volume_up_latency_value_ptr != nullptr) {
                audio.music_volume_up_latency = boost::json::value_to<double>(*music_volume_up_latency_value_ptr);
            }

            const boost::json::value * news_song_value_ptr = audio_object.if_contains("news_song");
            if (news_song_value_ptr != nullptr) {
                audio.news_song = boost::json::value_to<std::string>(*news_song_value_ptr);
            }

            const boost::json::value * panic_playlist_value_ptr = audio_object.if_contains("panic_playlist");
            if (panic_playlist_value_ptr != nullptr) {
                audio.panic_playlist = boost::json::value_to<std::string>(*panic_playlist_value_ptr);
            }

            const boost::json::value * peace_playlist_value_ptr = audio_object.if_contains("peace_playlist");
            if (peace_playlist_value_ptr != nullptr) {
                audio.peace_playlist = boost::json::value_to<std::string>(*peace_playlist_value_ptr);
            }

            const boost::json::value * positional_value_ptr = audio_object.if_contains("positional");
            if (positional_value_ptr != nullptr) {
                audio.positional = boost::json::value_to<bool>(*positional_value_ptr);
            }

            const boost::json::value * shuffle_songs_value_ptr = audio_object.if_contains("shuffle_songs");
            if (shuffle_songs_value_ptr != nullptr) {
                audio.shuffle_songs = boost::json::value_to<bool>(*shuffle_songs_value_ptr);
            }

            const boost::json::value * sound_value_ptr = audio_object.if_contains("sound");
            if (sound_value_ptr != nullptr) {
                audio.sound = boost::json::value_to<bool>(*sound_value_ptr);
            }

            const boost::json::value * sound_gain_value_ptr = audio_object.if_contains("sound_gain");
            if (sound_gain_value_ptr != nullptr) {
                audio.sound_gain = boost::json::value_to<double>(*sound_gain_value_ptr);
            }

            const boost::json::value * thread_time_value_ptr = audio_object.if_contains("thread_time");
            if (thread_time_value_ptr != nullptr) {
                audio.thread_time = boost::json::value_to<int>(*thread_time_value_ptr);
            }

            const boost::json::value * time_between_music_value_ptr = audio_object.if_contains("time_between_music");
            if (time_between_music_value_ptr != nullptr) {
                audio.time_between_music = boost::json::value_to<double>(*time_between_music_value_ptr);
            }

            const boost::json::value * victory_playlist_value_ptr = audio_object.if_contains("victory_playlist");
            if (victory_playlist_value_ptr != nullptr) {
                audio.victory_playlist = boost::json::value_to<std::string>(*victory_playlist_value_ptr);
            }

            const boost::json::value * victory_relationship_value_ptr = audio_object.if_contains("victory_relationship");
            if (victory_relationship_value_ptr != nullptr) {
                audio.victory_relationship = boost::json::value_to<double>(*victory_relationship_value_ptr);
            }

            const boost::json::value * volume_value_ptr = audio_object.if_contains("volume");
            if (volume_value_ptr != nullptr) {
                audio.volume = boost::json::value_to<double>(*volume_value_ptr);
            }

            const boost::json::value * weapon_gain_value_ptr = audio_object.if_contains("weapon_gain");
            if (weapon_gain_value_ptr != nullptr) {
                audio.weapon_gain = boost::json::value_to<double>(*weapon_gain_value_ptr);
            }

        const boost::json::value * shuffle_songs_section_value_ptr = audio_object.if_contains("shuffle_songs_section");
        if (shuffle_songs_section_value_ptr != nullptr) {
            boost::json::object shuffle_songs_section_object = shuffle_songs_section_value_ptr->get_object();
            const boost::json::value * clear_history_on_list_change_value_ptr = shuffle_songs_section_object.if_contains("clear_history_on_list_change");
            if (clear_history_on_list_change_value_ptr != nullptr) {
                audio.shuffle_songs_section.clear_history_on_list_change = boost::json::value_to<bool>(*clear_history_on_list_change_value_ptr);
            }

            const boost::json::value * history_depth_value_ptr = shuffle_songs_section_object.if_contains("history_depth");
            if (history_depth_value_ptr != nullptr) {
                audio.shuffle_songs_section.history_depth = boost::json::value_to<int>(*history_depth_value_ptr);
            }

        }


        const boost::json::value * unit_audio_value_ptr = audio_object.if_contains("unit_audio");
        if (unit_audio_value_ptr != nullptr) {
            boost::json::object unit_audio_object = unit_audio_value_ptr->get_object();
            const boost::json::value * afterburner_value_ptr = unit_audio_object.if_contains("afterburner");
            if (afterburner_value_ptr != nullptr) {
                audio.unit_audio.afterburner = boost::json::value_to<std::string>(*afterburner_value_ptr);
            }

            const boost::json::value * armor_value_ptr = unit_audio_object.if_contains("armor");
            if (armor_value_ptr != nullptr) {
                audio.unit_audio.armor = boost::json::value_to<std::string>(*armor_value_ptr);
            }

            const boost::json::value * cloak_value_ptr = unit_audio_object.if_contains("cloak");
            if (cloak_value_ptr != nullptr) {
                audio.unit_audio.cloak = boost::json::value_to<std::string>(*cloak_value_ptr);
            }

            const boost::json::value * explode_value_ptr = unit_audio_object.if_contains("explode");
            if (explode_value_ptr != nullptr) {
                audio.unit_audio.explode = boost::json::value_to<std::string>(*explode_value_ptr);
            }

            const boost::json::value * hull_value_ptr = unit_audio_object.if_contains("hull");
            if (hull_value_ptr != nullptr) {
                audio.unit_audio.hull = boost::json::value_to<std::string>(*hull_value_ptr);
            }

            const boost::json::value * jump_arrive_value_ptr = unit_audio_object.if_contains("jump_arrive");
            if (jump_arrive_value_ptr != nullptr) {
                audio.unit_audio.jump_arrive = boost::json::value_to<std::string>(*jump_arrive_value_ptr);
            }

            const boost::json::value * jump_leave_value_ptr = unit_audio_object.if_contains("jump_leave");
            if (jump_leave_value_ptr != nullptr) {
                audio.unit_audio.jump_leave = boost::json::value_to<std::string>(*jump_leave_value_ptr);
            }

            const boost::json::value * locked_value_ptr = unit_audio_object.if_contains("locked");
            if (locked_value_ptr != nullptr) {
                audio.unit_audio.locked = boost::json::value_to<std::string>(*locked_value_ptr);
            }

            const boost::json::value * locking_value_ptr = unit_audio_object.if_contains("locking");
            if (locking_value_ptr != nullptr) {
                audio.unit_audio.locking = boost::json::value_to<std::string>(*locking_value_ptr);
            }

            const boost::json::value * locking_torp_value_ptr = unit_audio_object.if_contains("locking_torp");
            if (locking_torp_value_ptr != nullptr) {
                audio.unit_audio.locking_torp = boost::json::value_to<std::string>(*locking_torp_value_ptr);
            }

            const boost::json::value * locking_torp_trumps_music_value_ptr = unit_audio_object.if_contains("locking_torp_trumps_music");
            if (locking_torp_trumps_music_value_ptr != nullptr) {
                audio.unit_audio.locking_torp_trumps_music = boost::json::value_to<bool>(*locking_torp_trumps_music_value_ptr);
            }

            const boost::json::value * locking_trumps_music_value_ptr = unit_audio_object.if_contains("locking_trumps_music");
            if (locking_trumps_music_value_ptr != nullptr) {
                audio.unit_audio.locking_trumps_music = boost::json::value_to<bool>(*locking_trumps_music_value_ptr);
            }

            const boost::json::value * player_afterburner_value_ptr = unit_audio_object.if_contains("player_afterburner");
            if (player_afterburner_value_ptr != nullptr) {
                audio.unit_audio.player_afterburner = boost::json::value_to<std::string>(*player_afterburner_value_ptr);
            }

            const boost::json::value * player_armor_hit_value_ptr = unit_audio_object.if_contains("player_armor_hit");
            if (player_armor_hit_value_ptr != nullptr) {
                audio.unit_audio.player_armor_hit = boost::json::value_to<std::string>(*player_armor_hit_value_ptr);
            }

            const boost::json::value * player_hull_hit_value_ptr = unit_audio_object.if_contains("player_hull_hit");
            if (player_hull_hit_value_ptr != nullptr) {
                audio.unit_audio.player_hull_hit = boost::json::value_to<std::string>(*player_hull_hit_value_ptr);
            }

            const boost::json::value * player_shield_hit_value_ptr = unit_audio_object.if_contains("player_shield_hit");
            if (player_shield_hit_value_ptr != nullptr) {
                audio.unit_audio.player_shield_hit = boost::json::value_to<std::string>(*player_shield_hit_value_ptr);
            }

            const boost::json::value * player_tractor_cargo_value_ptr = unit_audio_object.if_contains("player_tractor_cargo");
            if (player_tractor_cargo_value_ptr != nullptr) {
                audio.unit_audio.player_tractor_cargo = boost::json::value_to<std::string>(*player_tractor_cargo_value_ptr);
            }

            const boost::json::value * player_tractor_cargo_fromturret_value_ptr = unit_audio_object.if_contains("player_tractor_cargo_fromturret");
            if (player_tractor_cargo_fromturret_value_ptr != nullptr) {
                audio.unit_audio.player_tractor_cargo_fromturret = boost::json::value_to<std::string>(*player_tractor_cargo_fromturret_value_ptr);
            }

            const boost::json::value * shield_value_ptr = unit_audio_object.if_contains("shield");
            if (shield_value_ptr != nullptr) {
                audio.unit_audio.shield = boost::json::value_to<std::string>(*shield_value_ptr);
            }

        }


        }


        const boost::json::value * cargo_value_ptr = root_object.if_contains("cargo");
        if (cargo_value_ptr != nullptr) {
            boost::json::object cargo_object = cargo_value_ptr->get_object();
            const boost::json::value * filter_expensive_cargo_value_ptr = cargo_object.if_contains("filter_expensive_cargo");
            if (filter_expensive_cargo_value_ptr != nullptr) {
                cargo.filter_expensive_cargo = boost::json::value_to<bool>(*filter_expensive_cargo_value_ptr);
            }

            const boost::json::value * junk_starship_mass_value_ptr = cargo_object.if_contains("junk_starship_mass");
            if (junk_starship_mass_value_ptr != nullptr) {
                cargo.junk_starship_mass = boost::json::value_to<double>(*junk_starship_mass_value_ptr);
            }

            const boost::json::value * junk_starship_price_value_ptr = cargo_object.if_contains("junk_starship_price");
            if (junk_starship_price_value_ptr != nullptr) {
                cargo.junk_starship_price = boost::json::value_to<double>(*junk_starship_price_value_ptr);
            }

            const boost::json::value * junk_starship_volume_value_ptr = cargo_object.if_contains("junk_starship_volume");
            if (junk_starship_volume_value_ptr != nullptr) {
                cargo.junk_starship_volume = boost::json::value_to<double>(*junk_starship_volume_value_ptr);
            }

            const boost::json::value * max_price_quant_adj_value_ptr = cargo_object.if_contains("max_price_quant_adj");
            if (max_price_quant_adj_value_ptr != nullptr) {
                cargo.max_price_quant_adj = boost::json::value_to<double>(*max_price_quant_adj_value_ptr);
            }

            const boost::json::value * min_cargo_price_value_ptr = cargo_object.if_contains("min_cargo_price");
            if (min_cargo_price_value_ptr != nullptr) {
                cargo.min_cargo_price = boost::json::value_to<double>(*min_cargo_price_value_ptr);
            }

            const boost::json::value * min_price_quant_adj_value_ptr = cargo_object.if_contains("min_price_quant_adj");
            if (min_price_quant_adj_value_ptr != nullptr) {
                cargo.min_price_quant_adj = boost::json::value_to<double>(*min_price_quant_adj_value_ptr);
            }

            const boost::json::value * news_from_cargo_list_value_ptr = cargo_object.if_contains("news_from_cargo_list");
            if (news_from_cargo_list_value_ptr != nullptr) {
                cargo.news_from_cargo_list = boost::json::value_to<bool>(*news_from_cargo_list_value_ptr);
            }

            const boost::json::value * price_quant_adj_power_value_ptr = cargo_object.if_contains("price_quant_adj_power");
            if (price_quant_adj_power_value_ptr != nullptr) {
                cargo.price_quant_adj_power = boost::json::value_to<double>(*price_quant_adj_power_value_ptr);
            }

            const boost::json::value * price_recenter_factor_value_ptr = cargo_object.if_contains("price_recenter_factor");
            if (price_recenter_factor_value_ptr != nullptr) {
                cargo.price_recenter_factor = boost::json::value_to<double>(*price_recenter_factor_value_ptr);
            }

            const boost::json::value * space_junk_price_value_ptr = cargo_object.if_contains("space_junk_price");
            if (space_junk_price_value_ptr != nullptr) {
                cargo.space_junk_price = boost::json::value_to<double>(*space_junk_price_value_ptr);
            }

        }


        const boost::json::value * cockpit_audio_value_ptr = root_object.if_contains("cockpit_audio");
        if (cockpit_audio_value_ptr != nullptr) {
            boost::json::object cockpit_audio_object = cockpit_audio_value_ptr->get_object();
            const boost::json::value * automatic_landing_zone_value_ptr = cockpit_audio_object.if_contains("automatic_landing_zone");
            if (automatic_landing_zone_value_ptr != nullptr) {
                cockpit_audio.automatic_landing_zone = boost::json::value_to<std::string>(*automatic_landing_zone_value_ptr);
            }

            const boost::json::value * automatic_landing_zone1_value_ptr = cockpit_audio_object.if_contains("automatic_landing_zone1");
            if (automatic_landing_zone1_value_ptr != nullptr) {
                cockpit_audio.automatic_landing_zone1 = boost::json::value_to<std::string>(*automatic_landing_zone1_value_ptr);
            }

            const boost::json::value * automatic_landing_zone2_value_ptr = cockpit_audio_object.if_contains("automatic_landing_zone2");
            if (automatic_landing_zone2_value_ptr != nullptr) {
                cockpit_audio.automatic_landing_zone2 = boost::json::value_to<std::string>(*automatic_landing_zone2_value_ptr);
            }

            const boost::json::value * autopilot_available_value_ptr = cockpit_audio_object.if_contains("autopilot_available");
            if (autopilot_available_value_ptr != nullptr) {
                cockpit_audio.autopilot_available = boost::json::value_to<std::string>(*autopilot_available_value_ptr);
            }

            const boost::json::value * autopilot_disabled_value_ptr = cockpit_audio_object.if_contains("autopilot_disabled");
            if (autopilot_disabled_value_ptr != nullptr) {
                cockpit_audio.autopilot_disabled = boost::json::value_to<std::string>(*autopilot_disabled_value_ptr);
            }

            const boost::json::value * autopilot_enabled_value_ptr = cockpit_audio_object.if_contains("autopilot_enabled");
            if (autopilot_enabled_value_ptr != nullptr) {
                cockpit_audio.autopilot_enabled = boost::json::value_to<std::string>(*autopilot_enabled_value_ptr);
            }

            const boost::json::value * autopilot_unavailable_value_ptr = cockpit_audio_object.if_contains("autopilot_unavailable");
            if (autopilot_unavailable_value_ptr != nullptr) {
                cockpit_audio.autopilot_unavailable = boost::json::value_to<std::string>(*autopilot_unavailable_value_ptr);
            }

            const boost::json::value * comm_value_ptr = cockpit_audio_object.if_contains("comm");
            if (comm_value_ptr != nullptr) {
                cockpit_audio.comm = boost::json::value_to<std::string>(*comm_value_ptr);
            }

            const boost::json::value * comm_preload_value_ptr = cockpit_audio_object.if_contains("comm_preload");
            if (comm_preload_value_ptr != nullptr) {
                cockpit_audio.comm_preload = boost::json::value_to<bool>(*comm_preload_value_ptr);
            }

            const boost::json::value * compress_change_value_ptr = cockpit_audio_object.if_contains("compress_change");
            if (compress_change_value_ptr != nullptr) {
                cockpit_audio.compress_change = boost::json::value_to<std::string>(*compress_change_value_ptr);
            }

            const boost::json::value * compress_interval_value_ptr = cockpit_audio_object.if_contains("compress_interval");
            if (compress_interval_value_ptr != nullptr) {
                cockpit_audio.compress_interval = boost::json::value_to<int>(*compress_interval_value_ptr);
            }

            const boost::json::value * compress_loop_value_ptr = cockpit_audio_object.if_contains("compress_loop");
            if (compress_loop_value_ptr != nullptr) {
                cockpit_audio.compress_loop = boost::json::value_to<std::string>(*compress_loop_value_ptr);
            }

            const boost::json::value * compress_max_value_ptr = cockpit_audio_object.if_contains("compress_max");
            if (compress_max_value_ptr != nullptr) {
                cockpit_audio.compress_max = boost::json::value_to<int>(*compress_max_value_ptr);
            }

            const boost::json::value * compress_stop_value_ptr = cockpit_audio_object.if_contains("compress_stop");
            if (compress_stop_value_ptr != nullptr) {
                cockpit_audio.compress_stop = boost::json::value_to<std::string>(*compress_stop_value_ptr);
            }

            const boost::json::value * docking_complete_value_ptr = cockpit_audio_object.if_contains("docking_complete");
            if (docking_complete_value_ptr != nullptr) {
                cockpit_audio.docking_complete = boost::json::value_to<std::string>(*docking_complete_value_ptr);
            }

            const boost::json::value * docking_denied_value_ptr = cockpit_audio_object.if_contains("docking_denied");
            if (docking_denied_value_ptr != nullptr) {
                cockpit_audio.docking_denied = boost::json::value_to<std::string>(*docking_denied_value_ptr);
            }

            const boost::json::value * docking_failed_value_ptr = cockpit_audio_object.if_contains("docking_failed");
            if (docking_failed_value_ptr != nullptr) {
                cockpit_audio.docking_failed = boost::json::value_to<std::string>(*docking_failed_value_ptr);
            }

            const boost::json::value * docking_granted_value_ptr = cockpit_audio_object.if_contains("docking_granted");
            if (docking_granted_value_ptr != nullptr) {
                cockpit_audio.docking_granted = boost::json::value_to<std::string>(*docking_granted_value_ptr);
            }

            const boost::json::value * examine_value_ptr = cockpit_audio_object.if_contains("examine");
            if (examine_value_ptr != nullptr) {
                cockpit_audio.examine = boost::json::value_to<std::string>(*examine_value_ptr);
            }

            const boost::json::value * jump_engaged_value_ptr = cockpit_audio_object.if_contains("jump_engaged");
            if (jump_engaged_value_ptr != nullptr) {
                cockpit_audio.jump_engaged = boost::json::value_to<std::string>(*jump_engaged_value_ptr);
            }

            const boost::json::value * manifest_value_ptr = cockpit_audio_object.if_contains("manifest");
            if (manifest_value_ptr != nullptr) {
                cockpit_audio.manifest = boost::json::value_to<std::string>(*manifest_value_ptr);
            }

            const boost::json::value * missile_switch_value_ptr = cockpit_audio_object.if_contains("missile_switch");
            if (missile_switch_value_ptr != nullptr) {
                cockpit_audio.missile_switch = boost::json::value_to<std::string>(*missile_switch_value_ptr);
            }

            const boost::json::value * objective_value_ptr = cockpit_audio_object.if_contains("objective");
            if (objective_value_ptr != nullptr) {
                cockpit_audio.objective = boost::json::value_to<std::string>(*objective_value_ptr);
            }

            const boost::json::value * overload_value_ptr = cockpit_audio_object.if_contains("overload");
            if (overload_value_ptr != nullptr) {
                cockpit_audio.overload = boost::json::value_to<std::string>(*overload_value_ptr);
            }

            const boost::json::value * overload_stopped_value_ptr = cockpit_audio_object.if_contains("overload_stopped");
            if (overload_stopped_value_ptr != nullptr) {
                cockpit_audio.overload_stopped = boost::json::value_to<std::string>(*overload_stopped_value_ptr);
            }

            const boost::json::value * repair_value_ptr = cockpit_audio_object.if_contains("repair");
            if (repair_value_ptr != nullptr) {
                cockpit_audio.repair = boost::json::value_to<std::string>(*repair_value_ptr);
            }

            const boost::json::value * scanning_value_ptr = cockpit_audio_object.if_contains("scanning");
            if (scanning_value_ptr != nullptr) {
                cockpit_audio.scanning = boost::json::value_to<std::string>(*scanning_value_ptr);
            }

            const boost::json::value * shield_value_ptr = cockpit_audio_object.if_contains("shield");
            if (shield_value_ptr != nullptr) {
                cockpit_audio.shield = boost::json::value_to<std::string>(*shield_value_ptr);
            }

            const boost::json::value * sounds_extension_1_value_ptr = cockpit_audio_object.if_contains("sounds_extension_1");
            if (sounds_extension_1_value_ptr != nullptr) {
                cockpit_audio.sounds_extension_1 = boost::json::value_to<std::string>(*sounds_extension_1_value_ptr);
            }

            const boost::json::value * sounds_extension_2_value_ptr = cockpit_audio_object.if_contains("sounds_extension_2");
            if (sounds_extension_2_value_ptr != nullptr) {
                cockpit_audio.sounds_extension_2 = boost::json::value_to<std::string>(*sounds_extension_2_value_ptr);
            }

            const boost::json::value * sounds_extension_3_value_ptr = cockpit_audio_object.if_contains("sounds_extension_3");
            if (sounds_extension_3_value_ptr != nullptr) {
                cockpit_audio.sounds_extension_3 = boost::json::value_to<std::string>(*sounds_extension_3_value_ptr);
            }

            const boost::json::value * sounds_extension_4_value_ptr = cockpit_audio_object.if_contains("sounds_extension_4");
            if (sounds_extension_4_value_ptr != nullptr) {
                cockpit_audio.sounds_extension_4 = boost::json::value_to<std::string>(*sounds_extension_4_value_ptr);
            }

            const boost::json::value * sounds_extension_5_value_ptr = cockpit_audio_object.if_contains("sounds_extension_5");
            if (sounds_extension_5_value_ptr != nullptr) {
                cockpit_audio.sounds_extension_5 = boost::json::value_to<std::string>(*sounds_extension_5_value_ptr);
            }

            const boost::json::value * sounds_extension_6_value_ptr = cockpit_audio_object.if_contains("sounds_extension_6");
            if (sounds_extension_6_value_ptr != nullptr) {
                cockpit_audio.sounds_extension_6 = boost::json::value_to<std::string>(*sounds_extension_6_value_ptr);
            }

            const boost::json::value * sounds_extension_7_value_ptr = cockpit_audio_object.if_contains("sounds_extension_7");
            if (sounds_extension_7_value_ptr != nullptr) {
                cockpit_audio.sounds_extension_7 = boost::json::value_to<std::string>(*sounds_extension_7_value_ptr);
            }

            const boost::json::value * sounds_extension_8_value_ptr = cockpit_audio_object.if_contains("sounds_extension_8");
            if (sounds_extension_8_value_ptr != nullptr) {
                cockpit_audio.sounds_extension_8 = boost::json::value_to<std::string>(*sounds_extension_8_value_ptr);
            }

            const boost::json::value * sounds_extension_9_value_ptr = cockpit_audio_object.if_contains("sounds_extension_9");
            if (sounds_extension_9_value_ptr != nullptr) {
                cockpit_audio.sounds_extension_9 = boost::json::value_to<std::string>(*sounds_extension_9_value_ptr);
            }

            const boost::json::value * target_value_ptr = cockpit_audio_object.if_contains("target");
            if (target_value_ptr != nullptr) {
                cockpit_audio.target = boost::json::value_to<std::string>(*target_value_ptr);
            }

            const boost::json::value * target_reverse_value_ptr = cockpit_audio_object.if_contains("target_reverse");
            if (target_reverse_value_ptr != nullptr) {
                cockpit_audio.target_reverse = boost::json::value_to<std::string>(*target_reverse_value_ptr);
            }

            const boost::json::value * undocking_complete_value_ptr = cockpit_audio_object.if_contains("undocking_complete");
            if (undocking_complete_value_ptr != nullptr) {
                cockpit_audio.undocking_complete = boost::json::value_to<std::string>(*undocking_complete_value_ptr);
            }

            const boost::json::value * undocking_failed_value_ptr = cockpit_audio_object.if_contains("undocking_failed");
            if (undocking_failed_value_ptr != nullptr) {
                cockpit_audio.undocking_failed = boost::json::value_to<std::string>(*undocking_failed_value_ptr);
            }

            const boost::json::value * vdu_static_value_ptr = cockpit_audio_object.if_contains("vdu_static");
            if (vdu_static_value_ptr != nullptr) {
                cockpit_audio.vdu_static = boost::json::value_to<std::string>(*vdu_static_value_ptr);
            }

            const boost::json::value * view_value_ptr = cockpit_audio_object.if_contains("view");
            if (view_value_ptr != nullptr) {
                cockpit_audio.view = boost::json::value_to<std::string>(*view_value_ptr);
            }

            const boost::json::value * weapon_switch_value_ptr = cockpit_audio_object.if_contains("weapon_switch");
            if (weapon_switch_value_ptr != nullptr) {
                cockpit_audio.weapon_switch = boost::json::value_to<std::string>(*weapon_switch_value_ptr);
            }

        }


        const boost::json::value * collision_hacks_value_ptr = root_object.if_contains("collision_hacks");
        if (collision_hacks_value_ptr != nullptr) {
            boost::json::object collision_hacks_object = collision_hacks_value_ptr->get_object();
            const boost::json::value * collision_hack_distance_value_ptr = collision_hacks_object.if_contains("collision_hack_distance");
            if (collision_hack_distance_value_ptr != nullptr) {
                collision_hacks.collision_hack_distance = boost::json::value_to<double>(*collision_hack_distance_value_ptr);
            }

            const boost::json::value * collision_damage_to_ai_value_ptr = collision_hacks_object.if_contains("collision_damage_to_ai");
            if (collision_damage_to_ai_value_ptr != nullptr) {
                collision_hacks.collision_damage_to_ai = boost::json::value_to<bool>(*collision_damage_to_ai_value_ptr);
            }

            const boost::json::value * crash_dock_hangar_value_ptr = collision_hacks_object.if_contains("crash_dock_hangar");
            if (crash_dock_hangar_value_ptr != nullptr) {
                collision_hacks.crash_dock_hangar = boost::json::value_to<bool>(*crash_dock_hangar_value_ptr);
            }

            const boost::json::value * crash_dock_unit_value_ptr = collision_hacks_object.if_contains("crash_dock_unit");
            if (crash_dock_unit_value_ptr != nullptr) {
                collision_hacks.crash_dock_unit = boost::json::value_to<bool>(*crash_dock_unit_value_ptr);
            }

            const boost::json::value * front_collision_hack_angle_value_ptr = collision_hacks_object.if_contains("front_collision_hack_angle");
            if (front_collision_hack_angle_value_ptr != nullptr) {
                collision_hacks.front_collision_hack_angle = boost::json::value_to<double>(*front_collision_hack_angle_value_ptr);
            }

            const boost::json::value * front_collision_hack_distance_value_ptr = collision_hacks_object.if_contains("front_collision_hack_distance");
            if (front_collision_hack_distance_value_ptr != nullptr) {
                collision_hacks.front_collision_hack_distance = boost::json::value_to<double>(*front_collision_hack_distance_value_ptr);
            }

            const boost::json::value * cargo_deals_collide_damage_value_ptr = collision_hacks_object.if_contains("cargo_deals_collide_damage");
            if (cargo_deals_collide_damage_value_ptr != nullptr) {
                collision_hacks.cargo_deals_collide_damage = boost::json::value_to<bool>(*cargo_deals_collide_damage_value_ptr);
            }

        }


        const boost::json::value * economics_value_ptr = root_object.if_contains("economics");
        if (economics_value_ptr != nullptr) {
            boost::json::object economics_object = economics_value_ptr->get_object();
            const boost::json::value * repair_price_value_ptr = economics_object.if_contains("repair_price");
            if (repair_price_value_ptr != nullptr) {
                economics.repair_price = boost::json::value_to<double>(*repair_price_value_ptr);
            }

            const boost::json::value * sellback_shipping_price_value_ptr = economics_object.if_contains("sellback_shipping_price");
            if (sellback_shipping_price_value_ptr != nullptr) {
                economics.sellback_shipping_price = boost::json::value_to<double>(*sellback_shipping_price_value_ptr);
            }

            const boost::json::value * ship_sellback_price_value_ptr = economics_object.if_contains("ship_sellback_price");
            if (ship_sellback_price_value_ptr != nullptr) {
                economics.ship_sellback_price = boost::json::value_to<double>(*ship_sellback_price_value_ptr);
            }

            const boost::json::value * shipping_price_base_value_ptr = economics_object.if_contains("shipping_price_base");
            if (shipping_price_base_value_ptr != nullptr) {
                economics.shipping_price_base = boost::json::value_to<double>(*shipping_price_base_value_ptr);
            }

            const boost::json::value * shipping_price_insys_value_ptr = economics_object.if_contains("shipping_price_insys");
            if (shipping_price_insys_value_ptr != nullptr) {
                economics.shipping_price_insys = boost::json::value_to<double>(*shipping_price_insys_value_ptr);
            }

            const boost::json::value * shipping_price_perjump_value_ptr = economics_object.if_contains("shipping_price_perjump");
            if (shipping_price_perjump_value_ptr != nullptr) {
                economics.shipping_price_perjump = boost::json::value_to<double>(*shipping_price_perjump_value_ptr);
            }

        }


        const boost::json::value * flight_value_ptr = root_object.if_contains("flight");
        if (flight_value_ptr != nullptr) {
            boost::json::object flight_object = flight_value_ptr->get_object();
        const boost::json::value * inertial_value_ptr = flight_object.if_contains("inertial");
        if (inertial_value_ptr != nullptr) {
            boost::json::object inertial_object = inertial_value_ptr->get_object();
            const boost::json::value * initial_value_ptr = inertial_object.if_contains("initial");
            if (initial_value_ptr != nullptr) {
                flight.inertial.initial = boost::json::value_to<bool>(*initial_value_ptr);
            }

            const boost::json::value * enable_value_ptr = inertial_object.if_contains("enable");
            if (enable_value_ptr != nullptr) {
                flight.inertial.enable = boost::json::value_to<bool>(*enable_value_ptr);
            }

        }


        }


        const boost::json::value * galaxy_value_ptr = root_object.if_contains("galaxy");
        if (galaxy_value_ptr != nullptr) {
            boost::json::object galaxy_object = galaxy_value_ptr->get_object();
            const boost::json::value * ambient_light_factor_value_ptr = galaxy_object.if_contains("ambient_light_factor");
            if (ambient_light_factor_value_ptr != nullptr) {
                galaxy.ambient_light_factor = boost::json::value_to<double>(*ambient_light_factor_value_ptr);
            }

            const boost::json::value * atmosphere_probability_value_ptr = galaxy_object.if_contains("atmosphere_probability");
            if (atmosphere_probability_value_ptr != nullptr) {
                galaxy.atmosphere_probability = boost::json::value_to<double>(*atmosphere_probability_value_ptr);
            }

            const boost::json::value * compactness_scale_value_ptr = galaxy_object.if_contains("compactness_scale");
            if (compactness_scale_value_ptr != nullptr) {
                galaxy.compactness_scale = boost::json::value_to<double>(*compactness_scale_value_ptr);
            }

            const boost::json::value * default_atmosphere_texture_value_ptr = galaxy_object.if_contains("default_atmosphere_texture");
            if (default_atmosphere_texture_value_ptr != nullptr) {
                galaxy.default_atmosphere_texture = boost::json::value_to<std::string>(*default_atmosphere_texture_value_ptr);
            }

            const boost::json::value * default_ring_texture_value_ptr = galaxy_object.if_contains("default_ring_texture");
            if (default_ring_texture_value_ptr != nullptr) {
                galaxy.default_ring_texture = boost::json::value_to<std::string>(*default_ring_texture_value_ptr);
            }

            const boost::json::value * double_ring_probability_value_ptr = galaxy_object.if_contains("double_ring_probability");
            if (double_ring_probability_value_ptr != nullptr) {
                galaxy.double_ring_probability = boost::json::value_to<double>(*double_ring_probability_value_ptr);
            }

            const boost::json::value * inner_ring_radius_value_ptr = galaxy_object.if_contains("inner_ring_radius");
            if (inner_ring_radius_value_ptr != nullptr) {
                galaxy.inner_ring_radius = boost::json::value_to<double>(*inner_ring_radius_value_ptr);
            }

            const boost::json::value * jump_compactness_scale_value_ptr = galaxy_object.if_contains("jump_compactness_scale");
            if (jump_compactness_scale_value_ptr != nullptr) {
                galaxy.jump_compactness_scale = boost::json::value_to<double>(*jump_compactness_scale_value_ptr);
            }

            const boost::json::value * mean_natural_phenomena_value_ptr = galaxy_object.if_contains("mean_natural_phenomena");
            if (mean_natural_phenomena_value_ptr != nullptr) {
                galaxy.mean_natural_phenomena = boost::json::value_to<int>(*mean_natural_phenomena_value_ptr);
            }

            const boost::json::value * mean_star_bases_value_ptr = galaxy_object.if_contains("mean_star_bases");
            if (mean_star_bases_value_ptr != nullptr) {
                galaxy.mean_star_bases = boost::json::value_to<int>(*mean_star_bases_value_ptr);
            }

            const boost::json::value * moon_relative_to_moon_value_ptr = galaxy_object.if_contains("moon_relative_to_moon");
            if (moon_relative_to_moon_value_ptr != nullptr) {
                galaxy.moon_relative_to_moon = boost::json::value_to<double>(*moon_relative_to_moon_value_ptr);
            }

            const boost::json::value * moon_relative_to_planet_value_ptr = galaxy_object.if_contains("moon_relative_to_planet");
            if (moon_relative_to_planet_value_ptr != nullptr) {
                galaxy.moon_relative_to_planet = boost::json::value_to<double>(*moon_relative_to_planet_value_ptr);
            }

            const boost::json::value * outer_ring_radius_value_ptr = galaxy_object.if_contains("outer_ring_radius");
            if (outer_ring_radius_value_ptr != nullptr) {
                galaxy.outer_ring_radius = boost::json::value_to<double>(*outer_ring_radius_value_ptr);
            }

            const boost::json::value * police_faction_value_ptr = galaxy_object.if_contains("police_faction");
            if (police_faction_value_ptr != nullptr) {
                galaxy.police_faction = boost::json::value_to<std::string>(*police_faction_value_ptr);
            }

            const boost::json::value * prison_system_value_ptr = galaxy_object.if_contains("prison_system");
            if (prison_system_value_ptr != nullptr) {
                galaxy.prison_system = boost::json::value_to<std::string>(*prison_system_value_ptr);
            }

            const boost::json::value * push_values_to_mean_value_ptr = galaxy_object.if_contains("push_values_to_mean");
            if (push_values_to_mean_value_ptr != nullptr) {
                galaxy.push_values_to_mean = boost::json::value_to<bool>(*push_values_to_mean_value_ptr);
            }

            const boost::json::value * ring_probability_value_ptr = galaxy_object.if_contains("ring_probability");
            if (ring_probability_value_ptr != nullptr) {
                galaxy.ring_probability = boost::json::value_to<double>(*ring_probability_value_ptr);
            }

            const boost::json::value * rocky_relative_to_primary_value_ptr = galaxy_object.if_contains("rocky_relative_to_primary");
            if (rocky_relative_to_primary_value_ptr != nullptr) {
                galaxy.rocky_relative_to_primary = boost::json::value_to<double>(*rocky_relative_to_primary_value_ptr);
            }

            const boost::json::value * second_ring_difference_value_ptr = galaxy_object.if_contains("second_ring_difference");
            if (second_ring_difference_value_ptr != nullptr) {
                galaxy.second_ring_difference = boost::json::value_to<double>(*second_ring_difference_value_ptr);
            }

            const boost::json::value * star_radius_scale_value_ptr = galaxy_object.if_contains("star_radius_scale");
            if (star_radius_scale_value_ptr != nullptr) {
                galaxy.star_radius_scale = boost::json::value_to<double>(*star_radius_scale_value_ptr);
            }

        }


        const boost::json::value * interpreter_value_ptr = root_object.if_contains("interpreter");
        if (interpreter_value_ptr != nullptr) {
            boost::json::object interpreter_object = interpreter_value_ptr->get_object();
            const boost::json::value * debug_level_value_ptr = interpreter_object.if_contains("debug_level");
            if (debug_level_value_ptr != nullptr) {
                interpreter.debug_level = boost::json::value_to<int>(*debug_level_value_ptr);
            }

            const boost::json::value * start_game_value_ptr = interpreter_object.if_contains("start_game");
            if (start_game_value_ptr != nullptr) {
                interpreter.start_game = boost::json::value_to<bool>(*start_game_value_ptr);
            }

            const boost::json::value * trace_value_ptr = interpreter_object.if_contains("trace");
            if (trace_value_ptr != nullptr) {
                interpreter.trace = boost::json::value_to<bool>(*trace_value_ptr);
            }

        }


        const boost::json::value * joystick_value_ptr = root_object.if_contains("joystick");
        if (joystick_value_ptr != nullptr) {
            boost::json::object joystick_object = joystick_value_ptr->get_object();
            const boost::json::value * deadband_value_ptr = joystick_object.if_contains("deadband");
            if (deadband_value_ptr != nullptr) {
                joystick.deadband = boost::json::value_to<double>(*deadband_value_ptr);
            }

            const boost::json::value * debug_digital_hatswitch_value_ptr = joystick_object.if_contains("debug_digital_hatswitch");
            if (debug_digital_hatswitch_value_ptr != nullptr) {
                joystick.debug_digital_hatswitch = boost::json::value_to<bool>(*debug_digital_hatswitch_value_ptr);
            }

            const boost::json::value * double_mouse_position_value_ptr = joystick_object.if_contains("double_mouse_position");
            if (double_mouse_position_value_ptr != nullptr) {
                joystick.double_mouse_position = boost::json::value_to<int>(*double_mouse_position_value_ptr);
            }

            const boost::json::value * double_mouse_factor_value_ptr = joystick_object.if_contains("double_mouse_factor");
            if (double_mouse_factor_value_ptr != nullptr) {
                joystick.double_mouse_factor = boost::json::value_to<double>(*double_mouse_factor_value_ptr);
            }

            const boost::json::value * ff_device_value_ptr = joystick_object.if_contains("ff_device");
            if (ff_device_value_ptr != nullptr) {
                joystick.ff_device = boost::json::value_to<int>(*ff_device_value_ptr);
            }

            const boost::json::value * force_feedback_value_ptr = joystick_object.if_contains("force_feedback");
            if (force_feedback_value_ptr != nullptr) {
                joystick.force_feedback = boost::json::value_to<bool>(*force_feedback_value_ptr);
            }

            const boost::json::value * force_use_of_joystick_value_ptr = joystick_object.if_contains("force_use_of_joystick");
            if (force_use_of_joystick_value_ptr != nullptr) {
                joystick.force_use_of_joystick = boost::json::value_to<bool>(*force_use_of_joystick_value_ptr);
            }

            const boost::json::value * initial_mode_value_ptr = joystick_object.if_contains("initial_mode");
            if (initial_mode_value_ptr != nullptr) {
                joystick.initial_mode = boost::json::value_to<std::string>(*initial_mode_value_ptr);
            }

            const boost::json::value * joystick_exponent_value_ptr = joystick_object.if_contains("joystick_exponent");
            if (joystick_exponent_value_ptr != nullptr) {
                joystick.joystick_exponent = boost::json::value_to<double>(*joystick_exponent_value_ptr);
            }

            const boost::json::value * mouse_blur_value_ptr = joystick_object.if_contains("mouse_blur");
            if (mouse_blur_value_ptr != nullptr) {
                joystick.mouse_blur = boost::json::value_to<double>(*mouse_blur_value_ptr);
            }

            const boost::json::value * mouse_crosshair_value_ptr = joystick_object.if_contains("mouse_crosshair");
            if (mouse_crosshair_value_ptr != nullptr) {
                joystick.mouse_crosshair = boost::json::value_to<std::string>(*mouse_crosshair_value_ptr);
            }

            const boost::json::value * mouse_cursor_value_ptr = joystick_object.if_contains("mouse_cursor");
            if (mouse_cursor_value_ptr != nullptr) {
                joystick.mouse_cursor = boost::json::value_to<bool>(*mouse_cursor_value_ptr);
            }

            const boost::json::value * mouse_cursor_chasecam_value_ptr = joystick_object.if_contains("mouse_cursor_chasecam");
            if (mouse_cursor_chasecam_value_ptr != nullptr) {
                joystick.mouse_cursor_chasecam = boost::json::value_to<bool>(*mouse_cursor_chasecam_value_ptr);
            }

            const boost::json::value * mouse_cursor_pancam_value_ptr = joystick_object.if_contains("mouse_cursor_pancam");
            if (mouse_cursor_pancam_value_ptr != nullptr) {
                joystick.mouse_cursor_pancam = boost::json::value_to<bool>(*mouse_cursor_pancam_value_ptr);
            }

            const boost::json::value * mouse_cursor_pantgt_value_ptr = joystick_object.if_contains("mouse_cursor_pantgt");
            if (mouse_cursor_pantgt_value_ptr != nullptr) {
                joystick.mouse_cursor_pantgt = boost::json::value_to<bool>(*mouse_cursor_pantgt_value_ptr);
            }

            const boost::json::value * mouse_deadband_value_ptr = joystick_object.if_contains("mouse_deadband");
            if (mouse_deadband_value_ptr != nullptr) {
                joystick.mouse_deadband = boost::json::value_to<double>(*mouse_deadband_value_ptr);
            }

            const boost::json::value * mouse_exponent_value_ptr = joystick_object.if_contains("mouse_exponent");
            if (mouse_exponent_value_ptr != nullptr) {
                joystick.mouse_exponent = boost::json::value_to<double>(*mouse_exponent_value_ptr);
            }

            const boost::json::value * mouse_sensitivity_value_ptr = joystick_object.if_contains("mouse_sensitivity");
            if (mouse_sensitivity_value_ptr != nullptr) {
                joystick.mouse_sensitivity = boost::json::value_to<double>(*mouse_sensitivity_value_ptr);
            }

            const boost::json::value * polling_rate_value_ptr = joystick_object.if_contains("polling_rate");
            if (polling_rate_value_ptr != nullptr) {
                joystick.polling_rate = boost::json::value_to<int>(*polling_rate_value_ptr);
            }

            const boost::json::value * reverse_mouse_spr_value_ptr = joystick_object.if_contains("reverse_mouse_spr");
            if (reverse_mouse_spr_value_ptr != nullptr) {
                joystick.reverse_mouse_spr = boost::json::value_to<bool>(*reverse_mouse_spr_value_ptr);
            }

            const boost::json::value * warp_mouse_value_ptr = joystick_object.if_contains("warp_mouse");
            if (warp_mouse_value_ptr != nullptr) {
                joystick.warp_mouse = boost::json::value_to<bool>(*warp_mouse_value_ptr);
            }

            const boost::json::value * warp_mouse_zone_value_ptr = joystick_object.if_contains("warp_mouse_zone");
            if (warp_mouse_zone_value_ptr != nullptr) {
                joystick.warp_mouse_zone = boost::json::value_to<int>(*warp_mouse_zone_value_ptr);
            }

        }


        const boost::json::value * keyboard_value_ptr = root_object.if_contains("keyboard");
        if (keyboard_value_ptr != nullptr) {
            boost::json::object keyboard_object = keyboard_value_ptr->get_object();
            const boost::json::value * enable_unicode_value_ptr = keyboard_object.if_contains("enable_unicode");
            if (enable_unicode_value_ptr != nullptr) {
                keyboard.enable_unicode = boost::json::value_to<bool>(*enable_unicode_value_ptr);
            }

        }


        const boost::json::value * logging_value_ptr = root_object.if_contains("logging");
        if (logging_value_ptr != nullptr) {
            boost::json::object logging_object = logging_value_ptr->get_object();
            const boost::json::value * vsdebug_value_ptr = logging_object.if_contains("vsdebug");
            if (vsdebug_value_ptr != nullptr) {
                logging.vsdebug = boost::json::value_to<int>(*vsdebug_value_ptr);
            }

            const boost::json::value * verbose_debug_value_ptr = logging_object.if_contains("verbose_debug");
            if (verbose_debug_value_ptr != nullptr) {
                logging.verbose_debug = boost::json::value_to<bool>(*verbose_debug_value_ptr);
            }

        }


        const boost::json::value * network_value_ptr = root_object.if_contains("network");
        if (network_value_ptr != nullptr) {
            boost::json::object network_object = network_value_ptr->get_object();
            const boost::json::value * account_server_url_value_ptr = network_object.if_contains("account_server_url");
            if (account_server_url_value_ptr != nullptr) {
                network.account_server_url = boost::json::value_to<std::string>(*account_server_url_value_ptr);
            }

            const boost::json::value * chat_only_in_network_value_ptr = network_object.if_contains("chat_only_in_network");
            if (chat_only_in_network_value_ptr != nullptr) {
                network.chat_only_in_network = boost::json::value_to<bool>(*chat_only_in_network_value_ptr);
            }

            const boost::json::value * force_client_connect_value_ptr = network_object.if_contains("force_client_connect");
            if (force_client_connect_value_ptr != nullptr) {
                network.force_client_connect = boost::json::value_to<bool>(*force_client_connect_value_ptr);
            }

            const boost::json::value * server_ip_value_ptr = network_object.if_contains("server_ip");
            if (server_ip_value_ptr != nullptr) {
                network.server_ip = boost::json::value_to<std::string>(*server_ip_value_ptr);
            }

            const boost::json::value * server_port_value_ptr = network_object.if_contains("server_port");
            if (server_port_value_ptr != nullptr) {
                network.server_port = boost::json::value_to<std::string>(*server_port_value_ptr);
            }

            const boost::json::value * use_account_server_value_ptr = network_object.if_contains("use_account_server");
            if (use_account_server_value_ptr != nullptr) {
                network.use_account_server = boost::json::value_to<bool>(*use_account_server_value_ptr);
            }

        }


        const boost::json::value * physics_value_ptr = root_object.if_contains("physics");
        if (physics_value_ptr != nullptr) {
            boost::json::object physics_object = physics_value_ptr->get_object();
            const boost::json::value * ai_lock_cheat_value_ptr = physics_object.if_contains("ai_lock_cheat");
            if (ai_lock_cheat_value_ptr != nullptr) {
                physics.ai_lock_cheat = boost::json::value_to<bool>(*ai_lock_cheat_value_ptr);
            }

            const boost::json::value * ai_pilot_when_in_turret_value_ptr = physics_object.if_contains("ai_pilot_when_in_turret");
            if (ai_pilot_when_in_turret_value_ptr != nullptr) {
                physics.ai_pilot_when_in_turret = boost::json::value_to<bool>(*ai_pilot_when_in_turret_value_ptr);
            }

            const boost::json::value * allow_mission_abort_value_ptr = physics_object.if_contains("allow_mission_abort");
            if (allow_mission_abort_value_ptr != nullptr) {
                physics.allow_mission_abort = boost::json::value_to<bool>(*allow_mission_abort_value_ptr);
            }

            const boost::json::value * allow_special_and_normal_gun_combo_value_ptr = physics_object.if_contains("allow_special_and_normal_gun_combo");
            if (allow_special_and_normal_gun_combo_value_ptr != nullptr) {
                physics.allow_special_and_normal_gun_combo = boost::json::value_to<bool>(*allow_special_and_normal_gun_combo_value_ptr);
            }

            const boost::json::value * asteroid_difficulty_value_ptr = physics_object.if_contains("asteroid_difficulty");
            if (asteroid_difficulty_value_ptr != nullptr) {
                physics.asteroid_difficulty = boost::json::value_to<double>(*asteroid_difficulty_value_ptr);
            }

            const boost::json::value * asteroid_weapon_collision_value_ptr = physics_object.if_contains("asteroid_weapon_collision");
            if (asteroid_weapon_collision_value_ptr != nullptr) {
                physics.asteroid_weapon_collision = boost::json::value_to<bool>(*asteroid_weapon_collision_value_ptr);
            }

            const boost::json::value * auto_docking_speed_boost_value_ptr = physics_object.if_contains("auto_docking_speed_boost");
            if (auto_docking_speed_boost_value_ptr != nullptr) {
                physics.auto_docking_speed_boost = boost::json::value_to<double>(*auto_docking_speed_boost_value_ptr);
            }

            const boost::json::value * auto_landing_displace_distance_value_ptr = physics_object.if_contains("auto_landing_displace_distance");
            if (auto_landing_displace_distance_value_ptr != nullptr) {
                physics.auto_landing_displace_distance = boost::json::value_to<double>(*auto_landing_displace_distance_value_ptr);
            }

            const boost::json::value * auto_landing_dock_distance_value_ptr = physics_object.if_contains("auto_landing_dock_distance");
            if (auto_landing_dock_distance_value_ptr != nullptr) {
                physics.auto_landing_dock_distance = boost::json::value_to<double>(*auto_landing_dock_distance_value_ptr);
            }

            const boost::json::value * auto_landing_enable_value_ptr = physics_object.if_contains("auto_landing_enable");
            if (auto_landing_enable_value_ptr != nullptr) {
                physics.auto_landing_enable = boost::json::value_to<bool>(*auto_landing_enable_value_ptr);
            }

            const boost::json::value * auto_landing_move_distance_value_ptr = physics_object.if_contains("auto_landing_move_distance");
            if (auto_landing_move_distance_value_ptr != nullptr) {
                physics.auto_landing_move_distance = boost::json::value_to<double>(*auto_landing_move_distance_value_ptr);
            }

            const boost::json::value * auto_landing_port_unclamped_seconds_value_ptr = physics_object.if_contains("auto_landing_port_unclamped_seconds");
            if (auto_landing_port_unclamped_seconds_value_ptr != nullptr) {
                physics.auto_landing_port_unclamped_seconds = boost::json::value_to<double>(*auto_landing_port_unclamped_seconds_value_ptr);
            }

            const boost::json::value * auto_landing_warning_distance_value_ptr = physics_object.if_contains("auto_landing_warning_distance");
            if (auto_landing_warning_distance_value_ptr != nullptr) {
                physics.auto_landing_warning_distance = boost::json::value_to<double>(*auto_landing_warning_distance_value_ptr);
            }

            const boost::json::value * auto_pilot_compensate_for_interdiction_value_ptr = physics_object.if_contains("auto_pilot_compensate_for_interdiction");
            if (auto_pilot_compensate_for_interdiction_value_ptr != nullptr) {
                physics.auto_pilot_compensate_for_interdiction = boost::json::value_to<bool>(*auto_pilot_compensate_for_interdiction_value_ptr);
            }

            const boost::json::value * auto_pilot_no_enemies_distance_multiplier_value_ptr = physics_object.if_contains("auto_pilot_no_enemies_distance_multiplier");
            if (auto_pilot_no_enemies_distance_multiplier_value_ptr != nullptr) {
                physics.auto_pilot_no_enemies_distance_multiplier = boost::json::value_to<double>(*auto_pilot_no_enemies_distance_multiplier_value_ptr);
            }

            const boost::json::value * auto_pilot_planet_radius_percent_value_ptr = physics_object.if_contains("auto_pilot_planet_radius_percent");
            if (auto_pilot_planet_radius_percent_value_ptr != nullptr) {
                physics.auto_pilot_planet_radius_percent = boost::json::value_to<double>(*auto_pilot_planet_radius_percent_value_ptr);
            }

            const boost::json::value * auto_pilot_spec_lining_up_angle_value_ptr = physics_object.if_contains("auto_pilot_spec_lining_up_angle");
            if (auto_pilot_spec_lining_up_angle_value_ptr != nullptr) {
                physics.auto_pilot_spec_lining_up_angle = boost::json::value_to<double>(*auto_pilot_spec_lining_up_angle_value_ptr);
            }

            const boost::json::value * auto_pilot_terminate_value_ptr = physics_object.if_contains("auto_pilot_terminate");
            if (auto_pilot_terminate_value_ptr != nullptr) {
                physics.auto_pilot_terminate = boost::json::value_to<bool>(*auto_pilot_terminate_value_ptr);
            }

            const boost::json::value * auto_pilot_termination_distance_value_ptr = physics_object.if_contains("auto_pilot_termination_distance");
            if (auto_pilot_termination_distance_value_ptr != nullptr) {
                physics.auto_pilot_termination_distance = boost::json::value_to<double>(*auto_pilot_termination_distance_value_ptr);
            }

            const boost::json::value * auto_pilot_termination_distance_enemy_value_ptr = physics_object.if_contains("auto_pilot_termination_distance_enemy");
            if (auto_pilot_termination_distance_enemy_value_ptr != nullptr) {
                physics.auto_pilot_termination_distance_enemy = boost::json::value_to<double>(*auto_pilot_termination_distance_enemy_value_ptr);
            }

            const boost::json::value * auto_pilot_termination_distance_no_enemies_value_ptr = physics_object.if_contains("auto_pilot_termination_distance_no_enemies");
            if (auto_pilot_termination_distance_no_enemies_value_ptr != nullptr) {
                physics.auto_pilot_termination_distance_no_enemies = boost::json::value_to<double>(*auto_pilot_termination_distance_no_enemies_value_ptr);
            }

            const boost::json::value * auto_pilot_ramp_warp_down_value_ptr = physics_object.if_contains("auto_pilot_ramp_warp_down");
            if (auto_pilot_ramp_warp_down_value_ptr != nullptr) {
                physics.auto_pilot_ramp_warp_down = boost::json::value_to<bool>(*auto_pilot_ramp_warp_down_value_ptr);
            }

            const boost::json::value * auto_landing_exclude_list_value_ptr = physics_object.if_contains("auto_landing_exclude_list");
            if (auto_landing_exclude_list_value_ptr != nullptr) {
                physics.auto_landing_exclude_list = boost::json::value_to<std::string>(*auto_landing_exclude_list_value_ptr);
            }

            const boost::json::value * auto_landing_exclude_warning_list_value_ptr = physics_object.if_contains("auto_landing_exclude_warning_list");
            if (auto_landing_exclude_warning_list_value_ptr != nullptr) {
                physics.auto_landing_exclude_warning_list = boost::json::value_to<std::string>(*auto_landing_exclude_warning_list_value_ptr);
            }

            const boost::json::value * auto_time_in_seconds_value_ptr = physics_object.if_contains("auto_time_in_seconds");
            if (auto_time_in_seconds_value_ptr != nullptr) {
                physics.auto_time_in_seconds = boost::json::value_to<double>(*auto_time_in_seconds_value_ptr);
            }

            const boost::json::value * auto_turn_towards_value_ptr = physics_object.if_contains("auto_turn_towards");
            if (auto_turn_towards_value_ptr != nullptr) {
                physics.auto_turn_towards = boost::json::value_to<bool>(*auto_turn_towards_value_ptr);
            }

            const boost::json::value * auto_undock_value_ptr = physics_object.if_contains("auto_undock");
            if (auto_undock_value_ptr != nullptr) {
                physics.auto_undock = boost::json::value_to<bool>(*auto_undock_value_ptr);
            }

            const boost::json::value * autogen_compactness_value_ptr = physics_object.if_contains("autogen_compactness");
            if (autogen_compactness_value_ptr != nullptr) {
                physics.autogen_compactness = boost::json::value_to<double>(*autogen_compactness_value_ptr);
            }

            const boost::json::value * autotracking_value_ptr = physics_object.if_contains("autotracking");
            if (autotracking_value_ptr != nullptr) {
                physics.autotracking = boost::json::value_to<double>(*autotracking_value_ptr);
            }

            const boost::json::value * can_auto_through_planets_value_ptr = physics_object.if_contains("can_auto_through_planets");
            if (can_auto_through_planets_value_ptr != nullptr) {
                physics.can_auto_through_planets = boost::json::value_to<bool>(*can_auto_through_planets_value_ptr);
            }

            const boost::json::value * capship_size_value_ptr = physics_object.if_contains("capship_size");
            if (capship_size_value_ptr != nullptr) {
                physics.capship_size = boost::json::value_to<double>(*capship_size_value_ptr);
            }

            const boost::json::value * car_control_value_ptr = physics_object.if_contains("car_control");
            if (car_control_value_ptr != nullptr) {
                physics.car_control = boost::json::value_to<bool>(*car_control_value_ptr);
            }

            const boost::json::value * cargo_live_time_value_ptr = physics_object.if_contains("cargo_live_time");
            if (cargo_live_time_value_ptr != nullptr) {
                physics.cargo_live_time = boost::json::value_to<double>(*cargo_live_time_value_ptr);
            }

            const boost::json::value * cargo_wingmen_only_with_dockport_value_ptr = physics_object.if_contains("cargo_wingmen_only_with_dockport");
            if (cargo_wingmen_only_with_dockport_value_ptr != nullptr) {
                physics.cargo_wingmen_only_with_dockport = boost::json::value_to<bool>(*cargo_wingmen_only_with_dockport_value_ptr);
            }

            const boost::json::value * change_docking_orientation_value_ptr = physics_object.if_contains("change_docking_orientation");
            if (change_docking_orientation_value_ptr != nullptr) {
                physics.change_docking_orientation = boost::json::value_to<bool>(*change_docking_orientation_value_ptr);
            }

            const boost::json::value * close_enough_to_autotrack_value_ptr = physics_object.if_contains("close_enough_to_autotrack");
            if (close_enough_to_autotrack_value_ptr != nullptr) {
                physics.close_enough_to_autotrack = boost::json::value_to<double>(*close_enough_to_autotrack_value_ptr);
            }

            const boost::json::value * collidemap_sanity_check_value_ptr = physics_object.if_contains("collidemap_sanity_check");
            if (collidemap_sanity_check_value_ptr != nullptr) {
                physics.collidemap_sanity_check = boost::json::value_to<bool>(*collidemap_sanity_check_value_ptr);
            }

            const boost::json::value * collision_inertial_time_value_ptr = physics_object.if_contains("collision_inertial_time");
            if (collision_inertial_time_value_ptr != nullptr) {
                physics.collision_inertial_time = boost::json::value_to<double>(*collision_inertial_time_value_ptr);
            }

            const boost::json::value * collision_scale_factor_value_ptr = physics_object.if_contains("collision_scale_factor");
            if (collision_scale_factor_value_ptr != nullptr) {
                physics.collision_scale_factor = boost::json::value_to<double>(*collision_scale_factor_value_ptr);
            }

            const boost::json::value * component_based_upgrades_value_ptr = physics_object.if_contains("component_based_upgrades");
            if (component_based_upgrades_value_ptr != nullptr) {
                physics.component_based_upgrades = boost::json::value_to<bool>(*component_based_upgrades_value_ptr);
            }

            const boost::json::value * computer_warp_ramp_up_time_value_ptr = physics_object.if_contains("computer_warp_ramp_up_time");
            if (computer_warp_ramp_up_time_value_ptr != nullptr) {
                physics.computer_warp_ramp_up_time = boost::json::value_to<double>(*computer_warp_ramp_up_time_value_ptr);
            }

            const boost::json::value * contraband_assist_range_value_ptr = physics_object.if_contains("contraband_assist_range");
            if (contraband_assist_range_value_ptr != nullptr) {
                physics.contraband_assist_range = boost::json::value_to<double>(*contraband_assist_range_value_ptr);
            }

            const boost::json::value * damage_chance_value_ptr = physics_object.if_contains("damage_chance");
            if (damage_chance_value_ptr != nullptr) {
                physics.damage_chance = boost::json::value_to<double>(*damage_chance_value_ptr);
            }

            const boost::json::value * damage_player_chance_value_ptr = physics_object.if_contains("damage_player_chance");
            if (damage_player_chance_value_ptr != nullptr) {
                physics.damage_player_chance = boost::json::value_to<double>(*damage_player_chance_value_ptr);
            }

            const boost::json::value * debris_mass_value_ptr = physics_object.if_contains("debris_mass");
            if (debris_mass_value_ptr != nullptr) {
                physics.debris_mass = boost::json::value_to<double>(*debris_mass_value_ptr);
            }

            const boost::json::value * debris_time_value_ptr = physics_object.if_contains("debris_time");
            if (debris_time_value_ptr != nullptr) {
                physics.debris_time = boost::json::value_to<double>(*debris_time_value_ptr);
            }

            const boost::json::value * default_interdiction_value_ptr = physics_object.if_contains("default_interdiction");
            if (default_interdiction_value_ptr != nullptr) {
                physics.default_interdiction = boost::json::value_to<double>(*default_interdiction_value_ptr);
            }

            const boost::json::value * default_shield_tightness_value_ptr = physics_object.if_contains("default_shield_tightness");
            if (default_shield_tightness_value_ptr != nullptr) {
                physics.default_shield_tightness = boost::json::value_to<double>(*default_shield_tightness_value_ptr);
            }

            const boost::json::value * definite_damage_chance_value_ptr = physics_object.if_contains("definite_damage_chance");
            if (definite_damage_chance_value_ptr != nullptr) {
                physics.definite_damage_chance = boost::json::value_to<double>(*definite_damage_chance_value_ptr);
            }

            const boost::json::value * density_of_jump_point_value_ptr = physics_object.if_contains("density_of_jump_point");
            if (density_of_jump_point_value_ptr != nullptr) {
                physics.density_of_jump_point = boost::json::value_to<double>(*density_of_jump_point_value_ptr);
            }

            const boost::json::value * density_of_rock_value_ptr = physics_object.if_contains("density_of_rock");
            if (density_of_rock_value_ptr != nullptr) {
                physics.density_of_rock = boost::json::value_to<double>(*density_of_rock_value_ptr);
            }

            const boost::json::value * difficulty_based_enemy_damage_value_ptr = physics_object.if_contains("difficulty_based_enemy_damage");
            if (difficulty_based_enemy_damage_value_ptr != nullptr) {
                physics.difficulty_based_enemy_damage = boost::json::value_to<bool>(*difficulty_based_enemy_damage_value_ptr);
            }

            const boost::json::value * difficulty_speed_exponent_value_ptr = physics_object.if_contains("difficulty_speed_exponent");
            if (difficulty_speed_exponent_value_ptr != nullptr) {
                physics.difficulty_speed_exponent = boost::json::value_to<double>(*difficulty_speed_exponent_value_ptr);
            }

            const boost::json::value * difficulty_based_shield_recharge_value_ptr = physics_object.if_contains("difficulty_based_shield_recharge");
            if (difficulty_based_shield_recharge_value_ptr != nullptr) {
                physics.difficulty_based_shield_recharge = boost::json::value_to<bool>(*difficulty_based_shield_recharge_value_ptr);
            }

            const boost::json::value * direct_interstellar_journey_value_ptr = physics_object.if_contains("direct_interstellar_journey");
            if (direct_interstellar_journey_value_ptr != nullptr) {
                physics.direct_interstellar_journey = boost::json::value_to<bool>(*direct_interstellar_journey_value_ptr);
            }

            const boost::json::value * display_in_meters_value_ptr = physics_object.if_contains("display_in_meters");
            if (display_in_meters_value_ptr != nullptr) {
                physics.display_in_meters = boost::json::value_to<bool>(*display_in_meters_value_ptr);
            }

            const boost::json::value * distance_to_warp_value_ptr = physics_object.if_contains("distance_to_warp");
            if (distance_to_warp_value_ptr != nullptr) {
                physics.distance_to_warp = boost::json::value_to<double>(*distance_to_warp_value_ptr);
            }

            const boost::json::value * dock_with_clear_planets_value_ptr = physics_object.if_contains("dock_with_clear_planets");
            if (dock_with_clear_planets_value_ptr != nullptr) {
                physics.dock_with_clear_planets = boost::json::value_to<bool>(*dock_with_clear_planets_value_ptr);
            }

            const boost::json::value * dock_within_base_shield_value_ptr = physics_object.if_contains("dock_within_base_shield");
            if (dock_within_base_shield_value_ptr != nullptr) {
                physics.dock_within_base_shield = boost::json::value_to<bool>(*dock_within_base_shield_value_ptr);
            }

            const boost::json::value * docking_time_value_ptr = physics_object.if_contains("docking_time");
            if (docking_time_value_ptr != nullptr) {
                physics.docking_time = boost::json::value_to<double>(*docking_time_value_ptr);
            }

            const boost::json::value * dockport_allows_upgrade_storage_value_ptr = physics_object.if_contains("dockport_allows_upgrade_storage");
            if (dockport_allows_upgrade_storage_value_ptr != nullptr) {
                physics.dockport_allows_upgrade_storage = boost::json::value_to<bool>(*dockport_allows_upgrade_storage_value_ptr);
            }

            const boost::json::value * does_missile_bounce_value_ptr = physics_object.if_contains("does_missile_bounce");
            if (does_missile_bounce_value_ptr != nullptr) {
                physics.does_missile_bounce = boost::json::value_to<bool>(*does_missile_bounce_value_ptr);
            }

            const boost::json::value * drone_value_ptr = physics_object.if_contains("drone");
            if (drone_value_ptr != nullptr) {
                physics.drone = boost::json::value_to<std::string>(*drone_value_ptr);
            }

            const boost::json::value * eject_cargo_on_blowup_value_ptr = physics_object.if_contains("eject_cargo_on_blowup");
            if (eject_cargo_on_blowup_value_ptr != nullptr) {
                physics.eject_cargo_on_blowup = boost::json::value_to<int>(*eject_cargo_on_blowup_value_ptr);
            }

            const boost::json::value * eject_cargo_speed_value_ptr = physics_object.if_contains("eject_cargo_speed");
            if (eject_cargo_speed_value_ptr != nullptr) {
                physics.eject_cargo_speed = boost::json::value_to<double>(*eject_cargo_speed_value_ptr);
            }

            const boost::json::value * eject_distance_value_ptr = physics_object.if_contains("eject_distance");
            if (eject_distance_value_ptr != nullptr) {
                physics.eject_distance = boost::json::value_to<double>(*eject_distance_value_ptr);
            }

            const boost::json::value * eject_live_time_value_ptr = physics_object.if_contains("eject_live_time");
            if (eject_live_time_value_ptr != nullptr) {
                physics.eject_live_time = boost::json::value_to<double>(*eject_live_time_value_ptr);
            }

            const boost::json::value * engine_energy_takes_priority_value_ptr = physics_object.if_contains("engine_energy_takes_priority");
            if (engine_energy_takes_priority_value_ptr != nullptr) {
                physics.engine_energy_takes_priority = boost::json::value_to<bool>(*engine_energy_takes_priority_value_ptr);
            }

            const boost::json::value * enough_warp_for_cruise_value_ptr = physics_object.if_contains("enough_warp_for_cruise");
            if (enough_warp_for_cruise_value_ptr != nullptr) {
                physics.enough_warp_for_cruise = boost::json::value_to<double>(*enough_warp_for_cruise_value_ptr);
            }

            const boost::json::value * explosion_damage_center_value_ptr = physics_object.if_contains("explosion_damage_center");
            if (explosion_damage_center_value_ptr != nullptr) {
                physics.explosion_damage_center = boost::json::value_to<double>(*explosion_damage_center_value_ptr);
            }

            const boost::json::value * explosion_damage_edge_value_ptr = physics_object.if_contains("explosion_damage_edge");
            if (explosion_damage_edge_value_ptr != nullptr) {
                physics.explosion_damage_edge = boost::json::value_to<double>(*explosion_damage_edge_value_ptr);
            }

            const boost::json::value * extra_space_drag_for_cargo_value_ptr = physics_object.if_contains("extra_space_drag_for_cargo");
            if (extra_space_drag_for_cargo_value_ptr != nullptr) {
                physics.extra_space_drag_for_cargo = boost::json::value_to<double>(*extra_space_drag_for_cargo_value_ptr);
            }

            const boost::json::value * face_target_on_auto_value_ptr = physics_object.if_contains("face_target_on_auto");
            if (face_target_on_auto_value_ptr != nullptr) {
                physics.face_target_on_auto = boost::json::value_to<bool>(*face_target_on_auto_value_ptr);
            }

            const boost::json::value * fire_missing_autotrackers_value_ptr = physics_object.if_contains("fire_missing_autotrackers");
            if (fire_missing_autotrackers_value_ptr != nullptr) {
                physics.fire_missing_autotrackers = boost::json::value_to<bool>(*fire_missing_autotrackers_value_ptr);
            }

            const boost::json::value * friendly_auto_radius_value_ptr = physics_object.if_contains("friendly_auto_radius");
            if (friendly_auto_radius_value_ptr != nullptr) {
                physics.friendly_auto_radius = boost::json::value_to<double>(*friendly_auto_radius_value_ptr);
            }

            const boost::json::value * game_accel_value_ptr = physics_object.if_contains("game_accel");
            if (game_accel_value_ptr != nullptr) {
                physics.game_accel = boost::json::value_to<double>(*game_accel_value_ptr);
            }

            const boost::json::value * game_speed_value_ptr = physics_object.if_contains("game_speed");
            if (game_speed_value_ptr != nullptr) {
                physics.game_speed = boost::json::value_to<double>(*game_speed_value_ptr);
            }

            const boost::json::value * game_speed_affects_autogen_systems_value_ptr = physics_object.if_contains("game_speed_affects_autogen_systems");
            if (game_speed_affects_autogen_systems_value_ptr != nullptr) {
                physics.game_speed_affects_autogen_systems = boost::json::value_to<bool>(*game_speed_affects_autogen_systems_value_ptr);
            }

            const boost::json::value * game_speed_lying_value_ptr = physics_object.if_contains("game_speed_lying");
            if (game_speed_lying_value_ptr != nullptr) {
                physics.game_speed_lying = boost::json::value_to<bool>(*game_speed_lying_value_ptr);
            }

            const boost::json::value * gun_speed_value_ptr = physics_object.if_contains("gun_speed");
            if (gun_speed_value_ptr != nullptr) {
                physics.gun_speed = boost::json::value_to<double>(*gun_speed_value_ptr);
            }

            const boost::json::value * gun_speed_adjusted_game_speed_value_ptr = physics_object.if_contains("gun_speed_adjusted_game_speed");
            if (gun_speed_adjusted_game_speed_value_ptr != nullptr) {
                physics.gun_speed_adjusted_game_speed = boost::json::value_to<bool>(*gun_speed_adjusted_game_speed_value_ptr);
            }

            const boost::json::value * hardware_cursor_value_ptr = physics_object.if_contains("hardware_cursor");
            if (hardware_cursor_value_ptr != nullptr) {
                physics.hardware_cursor = boost::json::value_to<bool>(*hardware_cursor_value_ptr);
            }

            const boost::json::value * hostile_auto_radius_value_ptr = physics_object.if_contains("hostile_auto_radius");
            if (hostile_auto_radius_value_ptr != nullptr) {
                physics.hostile_auto_radius = boost::json::value_to<double>(*hostile_auto_radius_value_ptr);
            }

            const boost::json::value * hud_repair_unit_value_ptr = physics_object.if_contains("hud_repair_unit");
            if (hud_repair_unit_value_ptr != nullptr) {
                physics.hud_repair_unit = boost::json::value_to<double>(*hud_repair_unit_value_ptr);
            }

            const boost::json::value * in_system_jump_or_timeless_auto_pilot_value_ptr = physics_object.if_contains("in_system_jump_or_timeless_auto_pilot");
            if (in_system_jump_or_timeless_auto_pilot_value_ptr != nullptr) {
                physics.in_system_jump_or_timeless_auto_pilot = boost::json::value_to<bool>(*in_system_jump_or_timeless_auto_pilot_value_ptr);
            }

            const boost::json::value * inactive_system_time_value_ptr = physics_object.if_contains("inactive_system_time");
            if (inactive_system_time_value_ptr != nullptr) {
                physics.inactive_system_time = boost::json::value_to<double>(*inactive_system_time_value_ptr);
            }

            const boost::json::value * indestructible_cargo_items_value_ptr = physics_object.if_contains("indestructible_cargo_items");
            if (indestructible_cargo_items_value_ptr != nullptr) {
                physics.indestructible_cargo_items = boost::json::value_to<std::string>(*indestructible_cargo_items_value_ptr);
            }

            const boost::json::value * indiscriminate_system_destruction_value_ptr = physics_object.if_contains("indiscriminate_system_destruction");
            if (indiscriminate_system_destruction_value_ptr != nullptr) {
                physics.indiscriminate_system_destruction = boost::json::value_to<double>(*indiscriminate_system_destruction_value_ptr);
            }

            const boost::json::value * inelastic_scale_value_ptr = physics_object.if_contains("inelastic_scale");
            if (inelastic_scale_value_ptr != nullptr) {
                physics.inelastic_scale = boost::json::value_to<double>(*inelastic_scale_value_ptr);
            }

            const boost::json::value * jump_damage_multiplier_value_ptr = physics_object.if_contains("jump_damage_multiplier");
            if (jump_damage_multiplier_value_ptr != nullptr) {
                physics.jump_damage_multiplier = boost::json::value_to<double>(*jump_damage_multiplier_value_ptr);
            }

            const boost::json::value * jump_disables_shields_value_ptr = physics_object.if_contains("jump_disables_shields");
            if (jump_disables_shields_value_ptr != nullptr) {
                physics.jump_disables_shields = boost::json::value_to<bool>(*jump_disables_shields_value_ptr);
            }

            const boost::json::value * jump_mesh_radius_scale_value_ptr = physics_object.if_contains("jump_mesh_radius_scale");
            if (jump_mesh_radius_scale_value_ptr != nullptr) {
                physics.jump_mesh_radius_scale = boost::json::value_to<double>(*jump_mesh_radius_scale_value_ptr);
            }

            const boost::json::value * jump_radius_scale_value_ptr = physics_object.if_contains("jump_radius_scale");
            if (jump_radius_scale_value_ptr != nullptr) {
                physics.jump_radius_scale = boost::json::value_to<double>(*jump_radius_scale_value_ptr);
            }

            const boost::json::value * jump_weapon_collision_value_ptr = physics_object.if_contains("jump_weapon_collision");
            if (jump_weapon_collision_value_ptr != nullptr) {
                physics.jump_weapon_collision = boost::json::value_to<bool>(*jump_weapon_collision_value_ptr);
            }

            const boost::json::value * launch_speed_value_ptr = physics_object.if_contains("launch_speed");
            if (launch_speed_value_ptr != nullptr) {
                physics.launch_speed = boost::json::value_to<double>(*launch_speed_value_ptr);
            }

            const boost::json::value * lock_cone_value_ptr = physics_object.if_contains("lock_cone");
            if (lock_cone_value_ptr != nullptr) {
                physics.lock_cone = boost::json::value_to<double>(*lock_cone_value_ptr);
            }

            const boost::json::value * match_speed_with_target_value_ptr = physics_object.if_contains("match_speed_with_target");
            if (match_speed_with_target_value_ptr != nullptr) {
                physics.match_speed_with_target = boost::json::value_to<bool>(*match_speed_with_target_value_ptr);
            }

            const boost::json::value * max_accel_for_smooth_autopilot_value_ptr = physics_object.if_contains("max_accel_for_smooth_autopilot");
            if (max_accel_for_smooth_autopilot_value_ptr != nullptr) {
                physics.max_accel_for_smooth_autopilot = boost::json::value_to<double>(*max_accel_for_smooth_autopilot_value_ptr);
            }

            const boost::json::value * max_collide_trees_value_ptr = physics_object.if_contains("max_collide_trees");
            if (max_collide_trees_value_ptr != nullptr) {
                physics.max_collide_trees = boost::json::value_to<int>(*max_collide_trees_value_ptr);
            }

            const boost::json::value * max_damage_value_ptr = physics_object.if_contains("max_damage");
            if (max_damage_value_ptr != nullptr) {
                physics.max_damage = boost::json::value_to<double>(*max_damage_value_ptr);
            }

            const boost::json::value * max_ecm_value_ptr = physics_object.if_contains("max_ecm");
            if (max_ecm_value_ptr != nullptr) {
                physics.max_ecm = boost::json::value_to<int>(*max_ecm_value_ptr);
            }

            const boost::json::value * max_force_multiplier_value_ptr = physics_object.if_contains("max_force_multiplier");
            if (max_force_multiplier_value_ptr != nullptr) {
                physics.max_force_multiplier = boost::json::value_to<double>(*max_force_multiplier_value_ptr);
            }

            const boost::json::value * max_jump_damage_value_ptr = physics_object.if_contains("max_jump_damage");
            if (max_jump_damage_value_ptr != nullptr) {
                physics.max_jump_damage = boost::json::value_to<double>(*max_jump_damage_value_ptr);
            }

            const boost::json::value * max_lost_target_live_time_value_ptr = physics_object.if_contains("max_lost_target_live_time");
            if (max_lost_target_live_time_value_ptr != nullptr) {
                physics.max_lost_target_live_time = boost::json::value_to<double>(*max_lost_target_live_time_value_ptr);
            }

            const boost::json::value * max_missions_value_ptr = physics_object.if_contains("max_missions");
            if (max_missions_value_ptr != nullptr) {
                physics.max_missions = boost::json::value_to<int>(*max_missions_value_ptr);
            }

            const boost::json::value * max_non_player_rotation_rate_value_ptr = physics_object.if_contains("max_non_player_rotation_rate");
            if (max_non_player_rotation_rate_value_ptr != nullptr) {
                physics.max_non_player_rotation_rate = boost::json::value_to<double>(*max_non_player_rotation_rate_value_ptr);
            }

            const boost::json::value * max_over_combat_speed_for_smooth_autopilot_value_ptr = physics_object.if_contains("max_over_combat_speed_for_smooth_autopilot");
            if (max_over_combat_speed_for_smooth_autopilot_value_ptr != nullptr) {
                physics.max_over_combat_speed_for_smooth_autopilot = boost::json::value_to<double>(*max_over_combat_speed_for_smooth_autopilot_value_ptr);
            }

            const boost::json::value * max_player_rotation_rate_value_ptr = physics_object.if_contains("max_player_rotation_rate");
            if (max_player_rotation_rate_value_ptr != nullptr) {
                physics.max_player_rotation_rate = boost::json::value_to<double>(*max_player_rotation_rate_value_ptr);
            }

            const boost::json::value * max_radar_cone_damage_value_ptr = physics_object.if_contains("max_radar_cone_damage");
            if (max_radar_cone_damage_value_ptr != nullptr) {
                physics.max_radar_cone_damage = boost::json::value_to<double>(*max_radar_cone_damage_value_ptr);
            }

            const boost::json::value * max_radar_lock_cone_damage_value_ptr = physics_object.if_contains("max_radar_lock_cone_damage");
            if (max_radar_lock_cone_damage_value_ptr != nullptr) {
                physics.max_radar_lock_cone_damage = boost::json::value_to<double>(*max_radar_lock_cone_damage_value_ptr);
            }

            const boost::json::value * max_radar_track_cone_damage_value_ptr = physics_object.if_contains("max_radar_track_cone_damage");
            if (max_radar_track_cone_damage_value_ptr != nullptr) {
                physics.max_radar_track_cone_damage = boost::json::value_to<double>(*max_radar_track_cone_damage_value_ptr);
            }

            const boost::json::value * max_shield_lowers_capacitance_value_ptr = physics_object.if_contains("max_shield_lowers_capacitance");
            if (max_shield_lowers_capacitance_value_ptr != nullptr) {
                physics.max_shield_lowers_capacitance = boost::json::value_to<bool>(*max_shield_lowers_capacitance_value_ptr);
            }

            const boost::json::value * max_torque_multiplier_value_ptr = physics_object.if_contains("max_torque_multiplier");
            if (max_torque_multiplier_value_ptr != nullptr) {
                physics.max_torque_multiplier = boost::json::value_to<double>(*max_torque_multiplier_value_ptr);
            }

            const boost::json::value * max_warp_effect_size_value_ptr = physics_object.if_contains("max_warp_effect_size");
            if (max_warp_effect_size_value_ptr != nullptr) {
                physics.max_warp_effect_size = boost::json::value_to<double>(*max_warp_effect_size_value_ptr);
            }

            const boost::json::value * min_asteroid_distance_value_ptr = physics_object.if_contains("min_asteroid_distance");
            if (min_asteroid_distance_value_ptr != nullptr) {
                physics.min_asteroid_distance = boost::json::value_to<double>(*min_asteroid_distance_value_ptr);
            }

            const boost::json::value * min_damage_value_ptr = physics_object.if_contains("min_damage");
            if (min_damage_value_ptr != nullptr) {
                physics.min_damage = boost::json::value_to<double>(*min_damage_value_ptr);
            }

            const boost::json::value * min_maxenergy_shot_damage_value_ptr = physics_object.if_contains("min_maxenergy_shot_damage");
            if (min_maxenergy_shot_damage_value_ptr != nullptr) {
                physics.min_maxenergy_shot_damage = boost::json::value_to<double>(*min_maxenergy_shot_damage_value_ptr);
            }

            const boost::json::value * min_shield_speeding_discharge_value_ptr = physics_object.if_contains("min_shield_speeding_discharge");
            if (min_shield_speeding_discharge_value_ptr != nullptr) {
                physics.min_shield_speeding_discharge = boost::json::value_to<double>(*min_shield_speeding_discharge_value_ptr);
            }

            const boost::json::value * min_spec_interdiction_for_jittery_autopilot_value_ptr = physics_object.if_contains("min_spec_interdiction_for_jittery_autopilot");
            if (min_spec_interdiction_for_jittery_autopilot_value_ptr != nullptr) {
                physics.min_spec_interdiction_for_jittery_autopilot = boost::json::value_to<double>(*min_spec_interdiction_for_jittery_autopilot_value_ptr);
            }

            const boost::json::value * min_warp_effect_size_value_ptr = physics_object.if_contains("min_warp_effect_size");
            if (min_warp_effect_size_value_ptr != nullptr) {
                physics.min_warp_effect_size = boost::json::value_to<double>(*min_warp_effect_size_value_ptr);
            }

            const boost::json::value * min_warp_orbit_radius_value_ptr = physics_object.if_contains("min_warp_orbit_radius");
            if (min_warp_orbit_radius_value_ptr != nullptr) {
                physics.min_warp_orbit_radius = boost::json::value_to<double>(*min_warp_orbit_radius_value_ptr);
            }

            const boost::json::value * minimum_mass_value_ptr = physics_object.if_contains("minimum_mass");
            if (minimum_mass_value_ptr != nullptr) {
                physics.minimum_mass = boost::json::value_to<double>(*minimum_mass_value_ptr);
            }

            const boost::json::value * minimum_time_value_ptr = physics_object.if_contains("minimum_time");
            if (minimum_time_value_ptr != nullptr) {
                physics.minimum_time = boost::json::value_to<double>(*minimum_time_value_ptr);
            }

            const boost::json::value * mount_repair_unit_value_ptr = physics_object.if_contains("mount_repair_unit");
            if (mount_repair_unit_value_ptr != nullptr) {
                physics.mount_repair_unit = boost::json::value_to<double>(*mount_repair_unit_value_ptr);
            }

            const boost::json::value * must_lock_to_autotrack_value_ptr = physics_object.if_contains("must_lock_to_autotrack");
            if (must_lock_to_autotrack_value_ptr != nullptr) {
                physics.must_lock_to_autotrack = boost::json::value_to<bool>(*must_lock_to_autotrack_value_ptr);
            }

            const boost::json::value * must_repair_to_sell_value_ptr = physics_object.if_contains("must_repair_to_sell");
            if (must_repair_to_sell_value_ptr != nullptr) {
                physics.must_repair_to_sell = boost::json::value_to<bool>(*must_repair_to_sell_value_ptr);
            }

            const boost::json::value * near_autotrack_cone_value_ptr = physics_object.if_contains("near_autotrack_cone");
            if (near_autotrack_cone_value_ptr != nullptr) {
                physics.near_autotrack_cone = boost::json::value_to<double>(*near_autotrack_cone_value_ptr);
            }

            const boost::json::value * nebula_shield_recharge_value_ptr = physics_object.if_contains("nebula_shield_recharge");
            if (nebula_shield_recharge_value_ptr != nullptr) {
                physics.nebula_shield_recharge = boost::json::value_to<double>(*nebula_shield_recharge_value_ptr);
            }

            const boost::json::value * nebula_space_drag_value_ptr = physics_object.if_contains("nebula_space_drag");
            if (nebula_space_drag_value_ptr != nullptr) {
                physics.nebula_space_drag = boost::json::value_to<double>(*nebula_space_drag_value_ptr);
            }

            const boost::json::value * neutral_auto_radius_value_ptr = physics_object.if_contains("neutral_auto_radius");
            if (neutral_auto_radius_value_ptr != nullptr) {
                physics.neutral_auto_radius = boost::json::value_to<double>(*neutral_auto_radius_value_ptr);
            }

            const boost::json::value * no_ai_jump_points_value_ptr = physics_object.if_contains("no_ai_jump_points");
            if (no_ai_jump_points_value_ptr != nullptr) {
                physics.no_ai_jump_points = boost::json::value_to<bool>(*no_ai_jump_points_value_ptr);
            }

            const boost::json::value * no_auto_light_below_value_ptr = physics_object.if_contains("no_auto_light_below");
            if (no_auto_light_below_value_ptr != nullptr) {
                physics.no_auto_light_below = boost::json::value_to<double>(*no_auto_light_below_value_ptr);
            }

            const boost::json::value * no_damage_to_docked_ships_value_ptr = physics_object.if_contains("no_damage_to_docked_ships");
            if (no_damage_to_docked_ships_value_ptr != nullptr) {
                physics.no_damage_to_docked_ships = boost::json::value_to<bool>(*no_damage_to_docked_ships_value_ptr);
            }

            const boost::json::value * no_spec_jump_value_ptr = physics_object.if_contains("no_spec_jump");
            if (no_spec_jump_value_ptr != nullptr) {
                physics.no_spec_jump = boost::json::value_to<bool>(*no_spec_jump_value_ptr);
            }

            const boost::json::value * no_unit_collisions_value_ptr = physics_object.if_contains("no_unit_collisions");
            if (no_unit_collisions_value_ptr != nullptr) {
                physics.no_unit_collisions = boost::json::value_to<bool>(*no_unit_collisions_value_ptr);
            }

            const boost::json::value * num_running_systems_value_ptr = physics_object.if_contains("num_running_systems");
            if (num_running_systems_value_ptr != nullptr) {
                physics.num_running_systems = boost::json::value_to<int>(*num_running_systems_value_ptr);
            }

            const boost::json::value * num_times_to_simulate_new_star_system_value_ptr = physics_object.if_contains("num_times_to_simulate_new_star_system");
            if (num_times_to_simulate_new_star_system_value_ptr != nullptr) {
                physics.num_times_to_simulate_new_star_system = boost::json::value_to<int>(*num_times_to_simulate_new_star_system_value_ptr);
            }

            const boost::json::value * only_show_best_downgrade_value_ptr = physics_object.if_contains("only_show_best_downgrade");
            if (only_show_best_downgrade_value_ptr != nullptr) {
                physics.only_show_best_downgrade = boost::json::value_to<bool>(*only_show_best_downgrade_value_ptr);
            }

            const boost::json::value * orbit_averaging_value_ptr = physics_object.if_contains("orbit_averaging");
            if (orbit_averaging_value_ptr != nullptr) {
                physics.orbit_averaging = boost::json::value_to<double>(*orbit_averaging_value_ptr);
            }

            const boost::json::value * out_of_arc_fire_disrupts_lock_value_ptr = physics_object.if_contains("out_of_arc_fire_disrupts_lock");
            if (out_of_arc_fire_disrupts_lock_value_ptr != nullptr) {
                physics.out_of_arc_fire_disrupts_lock = boost::json::value_to<bool>(*out_of_arc_fire_disrupts_lock_value_ptr);
            }

            const boost::json::value * percent_missile_match_target_velocity_value_ptr = physics_object.if_contains("percent_missile_match_target_velocity");
            if (percent_missile_match_target_velocity_value_ptr != nullptr) {
                physics.percent_missile_match_target_velocity = boost::json::value_to<double>(*percent_missile_match_target_velocity_value_ptr);
            }

            const boost::json::value * persistent_on_load_value_ptr = physics_object.if_contains("persistent_on_load");
            if (persistent_on_load_value_ptr != nullptr) {
                physics.persistent_on_load = boost::json::value_to<bool>(*persistent_on_load_value_ptr);
            }

            const boost::json::value * planet_dock_min_port_size_value_ptr = physics_object.if_contains("planet_dock_min_port_size");
            if (planet_dock_min_port_size_value_ptr != nullptr) {
                physics.planet_dock_min_port_size = boost::json::value_to<double>(*planet_dock_min_port_size_value_ptr);
            }

            const boost::json::value * planet_dock_port_size_value_ptr = physics_object.if_contains("planet_dock_port_size");
            if (planet_dock_port_size_value_ptr != nullptr) {
                physics.planet_dock_port_size = boost::json::value_to<double>(*planet_dock_port_size_value_ptr);
            }

            const boost::json::value * planet_ejection_stophack_value_ptr = physics_object.if_contains("planet_ejection_stophack");
            if (planet_ejection_stophack_value_ptr != nullptr) {
                physics.planet_ejection_stophack = boost::json::value_to<double>(*planet_ejection_stophack_value_ptr);
            }

            const boost::json::value * planets_always_neutral_value_ptr = physics_object.if_contains("planets_always_neutral");
            if (planets_always_neutral_value_ptr != nullptr) {
                physics.planets_always_neutral = boost::json::value_to<bool>(*planets_always_neutral_value_ptr);
            }

            const boost::json::value * planets_can_have_subunits_value_ptr = physics_object.if_contains("planets_can_have_subunits");
            if (planets_can_have_subunits_value_ptr != nullptr) {
                physics.planets_can_have_subunits = boost::json::value_to<bool>(*planets_can_have_subunits_value_ptr);
            }

            const boost::json::value * player_godliness_value_ptr = physics_object.if_contains("player_godliness");
            if (player_godliness_value_ptr != nullptr) {
                physics.player_godliness = boost::json::value_to<double>(*player_godliness_value_ptr);
            }

            const boost::json::value * player_survival_chance_on_base_death_value_ptr = physics_object.if_contains("player_survival_chance_on_base_death");
            if (player_survival_chance_on_base_death_value_ptr != nullptr) {
                physics.player_survival_chance_on_base_death = boost::json::value_to<double>(*player_survival_chance_on_base_death_value_ptr);
            }

            const boost::json::value * player_tractorability_mask_value_ptr = physics_object.if_contains("player_tractorability_mask");
            if (player_tractorability_mask_value_ptr != nullptr) {
                physics.player_tractorability_mask = boost::json::value_to<std::string>(*player_tractorability_mask_value_ptr);
            }

            const boost::json::value * reduce_beam_ammo_value_ptr = physics_object.if_contains("reduce_beam_ammo");
            if (reduce_beam_ammo_value_ptr != nullptr) {
                physics.reduce_beam_ammo = boost::json::value_to<bool>(*reduce_beam_ammo_value_ptr);
            }

            const boost::json::value * refire_difficulty_scaling_value_ptr = physics_object.if_contains("refire_difficulty_scaling");
            if (refire_difficulty_scaling_value_ptr != nullptr) {
                physics.refire_difficulty_scaling = boost::json::value_to<double>(*refire_difficulty_scaling_value_ptr);
            }

            const boost::json::value * repair_droid_check_time_value_ptr = physics_object.if_contains("repair_droid_check_time");
            if (repair_droid_check_time_value_ptr != nullptr) {
                physics.repair_droid_check_time = boost::json::value_to<double>(*repair_droid_check_time_value_ptr);
            }

            const boost::json::value * repair_droid_time_value_ptr = physics_object.if_contains("repair_droid_time");
            if (repair_droid_time_value_ptr != nullptr) {
                physics.repair_droid_time = boost::json::value_to<double>(*repair_droid_time_value_ptr);
            }

            const boost::json::value * respawn_unit_size_value_ptr = physics_object.if_contains("respawn_unit_size");
            if (respawn_unit_size_value_ptr != nullptr) {
                physics.respawn_unit_size = boost::json::value_to<double>(*respawn_unit_size_value_ptr);
            }

            const boost::json::value * reverse_speed_limit_value_ptr = physics_object.if_contains("reverse_speed_limit");
            if (reverse_speed_limit_value_ptr != nullptr) {
                physics.reverse_speed_limit = boost::json::value_to<double>(*reverse_speed_limit_value_ptr);
            }

            const boost::json::value * runtime_compactness_value_ptr = physics_object.if_contains("runtime_compactness");
            if (runtime_compactness_value_ptr != nullptr) {
                physics.runtime_compactness = boost::json::value_to<double>(*runtime_compactness_value_ptr);
            }

            const boost::json::value * seconds_per_parsec_value_ptr = physics_object.if_contains("seconds_per_parsec");
            if (seconds_per_parsec_value_ptr != nullptr) {
                physics.seconds_per_parsec = boost::json::value_to<double>(*seconds_per_parsec_value_ptr);
            }

            const boost::json::value * separate_system_flakiness_component_value_ptr = physics_object.if_contains("separate_system_flakiness_component");
            if (separate_system_flakiness_component_value_ptr != nullptr) {
                physics.separate_system_flakiness_component = boost::json::value_to<bool>(*separate_system_flakiness_component_value_ptr);
            }

            const boost::json::value * shield_energy_capacitance_value_ptr = physics_object.if_contains("shield_energy_capacitance");
            if (shield_energy_capacitance_value_ptr != nullptr) {
                physics.shield_energy_capacitance = boost::json::value_to<double>(*shield_energy_capacitance_value_ptr);
            }

            const boost::json::value * shield_maintenance_charge_value_ptr = physics_object.if_contains("shield_maintenance_charge");
            if (shield_maintenance_charge_value_ptr != nullptr) {
                physics.shield_maintenance_charge = boost::json::value_to<double>(*shield_maintenance_charge_value_ptr);
            }

            const boost::json::value * shields_in_spec_value_ptr = physics_object.if_contains("shields_in_spec");
            if (shields_in_spec_value_ptr != nullptr) {
                physics.shields_in_spec = boost::json::value_to<bool>(*shields_in_spec_value_ptr);
            }

            const boost::json::value * simulate_while_docked_value_ptr = physics_object.if_contains("simulate_while_docked");
            if (simulate_while_docked_value_ptr != nullptr) {
                physics.simulate_while_docked = boost::json::value_to<bool>(*simulate_while_docked_value_ptr);
            }

            const boost::json::value * slide_end_value_ptr = physics_object.if_contains("slide_end");
            if (slide_end_value_ptr != nullptr) {
                physics.slide_end = boost::json::value_to<int>(*slide_end_value_ptr);
            }

            const boost::json::value * slide_start_value_ptr = physics_object.if_contains("slide_start");
            if (slide_start_value_ptr != nullptr) {
                physics.slide_start = boost::json::value_to<int>(*slide_start_value_ptr);
            }

            const boost::json::value * smallest_subunit_to_collide_value_ptr = physics_object.if_contains("smallest_subunit_to_collide");
            if (smallest_subunit_to_collide_value_ptr != nullptr) {
                physics.smallest_subunit_to_collide = boost::json::value_to<double>(*smallest_subunit_to_collide_value_ptr);
            }

            const boost::json::value * speeding_discharge_value_ptr = physics_object.if_contains("speeding_discharge");
            if (speeding_discharge_value_ptr != nullptr) {
                physics.speeding_discharge = boost::json::value_to<double>(*speeding_discharge_value_ptr);
            }

            const boost::json::value * sphere_collision_value_ptr = physics_object.if_contains("sphere_collision");
            if (sphere_collision_value_ptr != nullptr) {
                physics.sphere_collision = boost::json::value_to<bool>(*sphere_collision_value_ptr);
            }

            const boost::json::value * star_system_scale_value_ptr = physics_object.if_contains("star_system_scale");
            if (star_system_scale_value_ptr != nullptr) {
                physics.star_system_scale = boost::json::value_to<double>(*star_system_scale_value_ptr);
            }

            const boost::json::value * starships_as_cargo_value_ptr = physics_object.if_contains("starships_as_cargo");
            if (starships_as_cargo_value_ptr != nullptr) {
                physics.starships_as_cargo = boost::json::value_to<bool>(*starships_as_cargo_value_ptr);
            }

            const boost::json::value * steady_itts_value_ptr = physics_object.if_contains("steady_itts");
            if (steady_itts_value_ptr != nullptr) {
                physics.steady_itts = boost::json::value_to<bool>(*steady_itts_value_ptr);
            }

            const boost::json::value * survival_chance_on_base_death_value_ptr = physics_object.if_contains("survival_chance_on_base_death");
            if (survival_chance_on_base_death_value_ptr != nullptr) {
                physics.survival_chance_on_base_death = boost::json::value_to<double>(*survival_chance_on_base_death_value_ptr);
            }

            const boost::json::value * system_damage_on_armor_value_ptr = physics_object.if_contains("system_damage_on_armor");
            if (system_damage_on_armor_value_ptr != nullptr) {
                physics.system_damage_on_armor = boost::json::value_to<bool>(*system_damage_on_armor_value_ptr);
            }

            const boost::json::value * target_distance_to_warp_bonus_value_ptr = physics_object.if_contains("target_distance_to_warp_bonus");
            if (target_distance_to_warp_bonus_value_ptr != nullptr) {
                physics.target_distance_to_warp_bonus = boost::json::value_to<double>(*target_distance_to_warp_bonus_value_ptr);
            }

            const boost::json::value * teleport_autopilot_value_ptr = physics_object.if_contains("teleport_autopilot");
            if (teleport_autopilot_value_ptr != nullptr) {
                physics.teleport_autopilot = boost::json::value_to<bool>(*teleport_autopilot_value_ptr);
            }

            const boost::json::value * thruster_hit_chance_value_ptr = physics_object.if_contains("thruster_hit_chance");
            if (thruster_hit_chance_value_ptr != nullptr) {
                physics.thruster_hit_chance = boost::json::value_to<double>(*thruster_hit_chance_value_ptr);
            }

            const boost::json::value * un_docking_time_value_ptr = physics_object.if_contains("un_docking_time");
            if (un_docking_time_value_ptr != nullptr) {
                physics.un_docking_time = boost::json::value_to<double>(*un_docking_time_value_ptr);
            }

            const boost::json::value * undock_turn_away_value_ptr = physics_object.if_contains("undock_turn_away");
            if (undock_turn_away_value_ptr != nullptr) {
                physics.undock_turn_away = boost::json::value_to<bool>(*undock_turn_away_value_ptr);
            }

            const boost::json::value * unit_default_autodock_radius_value_ptr = physics_object.if_contains("unit_default_autodock_radius");
            if (unit_default_autodock_radius_value_ptr != nullptr) {
                physics.unit_default_autodock_radius = boost::json::value_to<double>(*unit_default_autodock_radius_value_ptr);
            }

            const boost::json::value * unit_space_drag_value_ptr = physics_object.if_contains("unit_space_drag");
            if (unit_space_drag_value_ptr != nullptr) {
                physics.unit_space_drag = boost::json::value_to<double>(*unit_space_drag_value_ptr);
            }

            const boost::json::value * use_cargo_mass_value_ptr = physics_object.if_contains("use_cargo_mass");
            if (use_cargo_mass_value_ptr != nullptr) {
                physics.use_cargo_mass = boost::json::value_to<bool>(*use_cargo_mass_value_ptr);
            }

            const boost::json::value * use_hidden_cargo_space_value_ptr = physics_object.if_contains("use_hidden_cargo_space");
            if (use_hidden_cargo_space_value_ptr != nullptr) {
                physics.use_hidden_cargo_space = boost::json::value_to<bool>(*use_hidden_cargo_space_value_ptr);
            }

            const boost::json::value * use_unit_autodock_radius_value_ptr = physics_object.if_contains("use_unit_autodock_radius");
            if (use_unit_autodock_radius_value_ptr != nullptr) {
                physics.use_unit_autodock_radius = boost::json::value_to<bool>(*use_unit_autodock_radius_value_ptr);
            }

            const boost::json::value * use_upgrade_template_maxrange_value_ptr = physics_object.if_contains("use_upgrade_template_maxrange");
            if (use_upgrade_template_maxrange_value_ptr != nullptr) {
                physics.use_upgrade_template_maxrange = boost::json::value_to<bool>(*use_upgrade_template_maxrange_value_ptr);
            }

            const boost::json::value * use_max_shield_energy_usage_value_ptr = physics_object.if_contains("use_max_shield_energy_usage");
            if (use_max_shield_energy_usage_value_ptr != nullptr) {
                physics.use_max_shield_energy_usage = boost::json::value_to<bool>(*use_max_shield_energy_usage_value_ptr);
            }

            const boost::json::value * velocity_max_value_ptr = physics_object.if_contains("velocity_max");
            if (velocity_max_value_ptr != nullptr) {
                physics.velocity_max = boost::json::value_to<double>(*velocity_max_value_ptr);
            }

            const boost::json::value * warp_behind_angle_value_ptr = physics_object.if_contains("warp_behind_angle");
            if (warp_behind_angle_value_ptr != nullptr) {
                physics.warp_behind_angle = boost::json::value_to<double>(*warp_behind_angle_value_ptr);
            }

            const boost::json::value * warp_cruise_mult_value_ptr = physics_object.if_contains("warp_cruise_mult");
            if (warp_cruise_mult_value_ptr != nullptr) {
                physics.warp_cruise_mult = boost::json::value_to<double>(*warp_cruise_mult_value_ptr);
            }

            const boost::json::value * warp_curve_degree_value_ptr = physics_object.if_contains("warp_curve_degree");
            if (warp_curve_degree_value_ptr != nullptr) {
                physics.warp_curve_degree = boost::json::value_to<double>(*warp_curve_degree_value_ptr);
            }

            const boost::json::value * warp_is_interstellar_value_ptr = physics_object.if_contains("warp_is_interstellar");
            if (warp_is_interstellar_value_ptr != nullptr) {
                physics.warp_is_interstellar = boost::json::value_to<bool>(*warp_is_interstellar_value_ptr);
            }

            const boost::json::value * warp_orbit_multiplier_value_ptr = physics_object.if_contains("warp_orbit_multiplier");
            if (warp_orbit_multiplier_value_ptr != nullptr) {
                physics.warp_orbit_multiplier = boost::json::value_to<double>(*warp_orbit_multiplier_value_ptr);
            }

            const boost::json::value * warp_perpendicular_value_ptr = physics_object.if_contains("warp_perpendicular");
            if (warp_perpendicular_value_ptr != nullptr) {
                physics.warp_perpendicular = boost::json::value_to<double>(*warp_perpendicular_value_ptr);
            }

            const boost::json::value * warp_ramp_down_time_value_ptr = physics_object.if_contains("warp_ramp_down_time");
            if (warp_ramp_down_time_value_ptr != nullptr) {
                physics.warp_ramp_down_time = boost::json::value_to<double>(*warp_ramp_down_time_value_ptr);
            }

            const boost::json::value * warp_region0_value_ptr = physics_object.if_contains("warp_region0");
            if (warp_region0_value_ptr != nullptr) {
                physics.warp_region0 = boost::json::value_to<double>(*warp_region0_value_ptr);
            }

            const boost::json::value * warp_region1_value_ptr = physics_object.if_contains("warp_region1");
            if (warp_region1_value_ptr != nullptr) {
                physics.warp_region1 = boost::json::value_to<double>(*warp_region1_value_ptr);
            }

            const boost::json::value * weapon_damage_efficiency_value_ptr = physics_object.if_contains("weapon_damage_efficiency");
            if (weapon_damage_efficiency_value_ptr != nullptr) {
                physics.weapon_damage_efficiency = boost::json::value_to<double>(*weapon_damage_efficiency_value_ptr);
            }

            const boost::json::value * year_scale_value_ptr = physics_object.if_contains("year_scale");
            if (year_scale_value_ptr != nullptr) {
                physics.year_scale = boost::json::value_to<double>(*year_scale_value_ptr);
            }

        const boost::json::value * ejection_value_ptr = physics_object.if_contains("ejection");
        if (ejection_value_ptr != nullptr) {
            boost::json::object ejection_object = ejection_value_ptr->get_object();
            const boost::json::value * auto_eject_percent_value_ptr = ejection_object.if_contains("auto_eject_percent");
            if (auto_eject_percent_value_ptr != nullptr) {
                physics.ejection.auto_eject_percent = boost::json::value_to<double>(*auto_eject_percent_value_ptr);
            }

            const boost::json::value * eject_cargo_percent_value_ptr = ejection_object.if_contains("eject_cargo_percent");
            if (eject_cargo_percent_value_ptr != nullptr) {
                physics.ejection.eject_cargo_percent = boost::json::value_to<double>(*eject_cargo_percent_value_ptr);
            }

            const boost::json::value * max_dumped_cargo_value_ptr = ejection_object.if_contains("max_dumped_cargo");
            if (max_dumped_cargo_value_ptr != nullptr) {
                physics.ejection.max_dumped_cargo = boost::json::value_to<int>(*max_dumped_cargo_value_ptr);
            }

            const boost::json::value * hull_damage_to_eject_value_ptr = ejection_object.if_contains("hull_damage_to_eject");
            if (hull_damage_to_eject_value_ptr != nullptr) {
                physics.ejection.hull_damage_to_eject = boost::json::value_to<double>(*hull_damage_to_eject_value_ptr);
            }

            const boost::json::value * player_auto_eject_value_ptr = ejection_object.if_contains("player_auto_eject");
            if (player_auto_eject_value_ptr != nullptr) {
                physics.ejection.player_auto_eject = boost::json::value_to<bool>(*player_auto_eject_value_ptr);
            }

        }


        const boost::json::value * priorities_value_ptr = physics_object.if_contains("priorities");
        if (priorities_value_ptr != nullptr) {
            boost::json::object priorities_object = priorities_value_ptr->get_object();
            const boost::json::value * asteroid_high_value_ptr = priorities_object.if_contains("asteroid_high");
            if (asteroid_high_value_ptr != nullptr) {
                physics.priorities.asteroid_high = boost::json::value_to<int>(*asteroid_high_value_ptr);
            }

            const boost::json::value * asteroid_low_value_ptr = priorities_object.if_contains("asteroid_low");
            if (asteroid_low_value_ptr != nullptr) {
                physics.priorities.asteroid_low = boost::json::value_to<int>(*asteroid_low_value_ptr);
            }

            const boost::json::value * asteroid_parent_value_ptr = priorities_object.if_contains("asteroid_parent");
            if (asteroid_parent_value_ptr != nullptr) {
                physics.priorities.asteroid_parent = boost::json::value_to<int>(*asteroid_parent_value_ptr);
            }

            const boost::json::value * dockable_value_ptr = priorities_object.if_contains("dockable");
            if (dockable_value_ptr != nullptr) {
                physics.priorities.dockable = boost::json::value_to<int>(*dockable_value_ptr);
            }

            const boost::json::value * force_top_priority_value_ptr = priorities_object.if_contains("force_top_priority");
            if (force_top_priority_value_ptr != nullptr) {
                physics.priorities.force_top_priority = boost::json::value_to<bool>(*force_top_priority_value_ptr);
            }

            const boost::json::value * high_value_ptr = priorities_object.if_contains("high");
            if (high_value_ptr != nullptr) {
                physics.priorities.high = boost::json::value_to<int>(*high_value_ptr);
            }

            const boost::json::value * inert_value_ptr = priorities_object.if_contains("inert");
            if (inert_value_ptr != nullptr) {
                physics.priorities.inert = boost::json::value_to<int>(*inert_value_ptr);
            }

            const boost::json::value * low_value_ptr = priorities_object.if_contains("low");
            if (low_value_ptr != nullptr) {
                physics.priorities.low = boost::json::value_to<int>(*low_value_ptr);
            }

            const boost::json::value * medium_value_ptr = priorities_object.if_contains("medium");
            if (medium_value_ptr != nullptr) {
                physics.priorities.medium = boost::json::value_to<int>(*medium_value_ptr);
            }

            const boost::json::value * medium_high_value_ptr = priorities_object.if_contains("medium_high");
            if (medium_high_value_ptr != nullptr) {
                physics.priorities.medium_high = boost::json::value_to<int>(*medium_high_value_ptr);
            }

            const boost::json::value * min_asteroid_distance_value_ptr = priorities_object.if_contains("min_asteroid_distance");
            if (min_asteroid_distance_value_ptr != nullptr) {
                physics.priorities.min_asteroid_distance = boost::json::value_to<std::string>(*min_asteroid_distance_value_ptr);
            }

            const boost::json::value * missile_value_ptr = priorities_object.if_contains("missile");
            if (missile_value_ptr != nullptr) {
                physics.priorities.missile = boost::json::value_to<int>(*missile_value_ptr);
            }

            const boost::json::value * no_enemies_value_ptr = priorities_object.if_contains("no_enemies");
            if (no_enemies_value_ptr != nullptr) {
                physics.priorities.no_enemies = boost::json::value_to<int>(*no_enemies_value_ptr);
            }

            const boost::json::value * not_visible_combat_high_value_ptr = priorities_object.if_contains("not_visible_combat_high");
            if (not_visible_combat_high_value_ptr != nullptr) {
                physics.priorities.not_visible_combat_high = boost::json::value_to<int>(*not_visible_combat_high_value_ptr);
            }

            const boost::json::value * not_visible_combat_low_value_ptr = priorities_object.if_contains("not_visible_combat_low");
            if (not_visible_combat_low_value_ptr != nullptr) {
                physics.priorities.not_visible_combat_low = boost::json::value_to<int>(*not_visible_combat_low_value_ptr);
            }

            const boost::json::value * not_visible_combat_medium_value_ptr = priorities_object.if_contains("not_visible_combat_medium");
            if (not_visible_combat_medium_value_ptr != nullptr) {
                physics.priorities.not_visible_combat_medium = boost::json::value_to<int>(*not_visible_combat_medium_value_ptr);
            }

            const boost::json::value * player_value_ptr = priorities_object.if_contains("player");
            if (player_value_ptr != nullptr) {
                physics.priorities.player = boost::json::value_to<int>(*player_value_ptr);
            }

            const boost::json::value * player_threat_distance_factor_value_ptr = priorities_object.if_contains("player_threat_distance_factor");
            if (player_threat_distance_factor_value_ptr != nullptr) {
                physics.priorities.player_threat_distance_factor = boost::json::value_to<double>(*player_threat_distance_factor_value_ptr);
            }

            const boost::json::value * threat_distance_factor_value_ptr = priorities_object.if_contains("threat_distance_factor");
            if (threat_distance_factor_value_ptr != nullptr) {
                physics.priorities.threat_distance_factor = boost::json::value_to<double>(*threat_distance_factor_value_ptr);
            }

            const boost::json::value * system_installation_value_ptr = priorities_object.if_contains("system_installation");
            if (system_installation_value_ptr != nullptr) {
                physics.priorities.system_installation = boost::json::value_to<int>(*system_installation_value_ptr);
            }

        const boost::json::value * dynamic_throttle_value_ptr = priorities_object.if_contains("dynamic_throttle");
        if (dynamic_throttle_value_ptr != nullptr) {
            boost::json::object dynamic_throttle_object = dynamic_throttle_value_ptr->get_object();
            const boost::json::value * max_distance_factor_value_ptr = dynamic_throttle_object.if_contains("max_distance_factor");
            if (max_distance_factor_value_ptr != nullptr) {
                physics.priorities.dynamic_throttle.max_distance_factor = boost::json::value_to<double>(*max_distance_factor_value_ptr);
            }

            const boost::json::value * min_distance_factor_value_ptr = dynamic_throttle_object.if_contains("min_distance_factor");
            if (min_distance_factor_value_ptr != nullptr) {
                physics.priorities.dynamic_throttle.min_distance_factor = boost::json::value_to<double>(*min_distance_factor_value_ptr);
            }

            const boost::json::value * target_fps_value_ptr = dynamic_throttle_object.if_contains("target_fps");
            if (target_fps_value_ptr != nullptr) {
                physics.priorities.dynamic_throttle.target_fps = boost::json::value_to<double>(*target_fps_value_ptr);
            }

        }


        }


        const boost::json::value * tractor_value_ptr = physics_object.if_contains("tractor");
        if (tractor_value_ptr != nullptr) {
            boost::json::object tractor_object = tractor_value_ptr->get_object();
            const boost::json::value * max_relative_speed_value_ptr = tractor_object.if_contains("max_relative_speed");
            if (max_relative_speed_value_ptr != nullptr) {
                physics.tractor.max_relative_speed = boost::json::value_to<int>(*max_relative_speed_value_ptr);
            }

            const boost::json::value * percent_to_tractor_value_ptr = tractor_object.if_contains("percent_to_tractor");
            if (percent_to_tractor_value_ptr != nullptr) {
                physics.tractor.percent_to_tractor = boost::json::value_to<double>(*percent_to_tractor_value_ptr);
            }

            const boost::json::value * scoop_value_ptr = tractor_object.if_contains("scoop");
            if (scoop_value_ptr != nullptr) {
                physics.tractor.scoop = boost::json::value_to<bool>(*scoop_value_ptr);
            }

            const boost::json::value * scoop_alpha_multiplier_value_ptr = tractor_object.if_contains("scoop_alpha_multiplier");
            if (scoop_alpha_multiplier_value_ptr != nullptr) {
                physics.tractor.scoop_alpha_multiplier = boost::json::value_to<double>(*scoop_alpha_multiplier_value_ptr);
            }

            const boost::json::value * scoop_angle_value_ptr = tractor_object.if_contains("scoop_angle");
            if (scoop_angle_value_ptr != nullptr) {
                physics.tractor.scoop_angle = boost::json::value_to<double>(*scoop_angle_value_ptr);
            }

            const boost::json::value * scoop_fov_value_ptr = tractor_object.if_contains("scoop_fov");
            if (scoop_fov_value_ptr != nullptr) {
                physics.tractor.scoop_fov = boost::json::value_to<double>(*scoop_fov_value_ptr);
            }

            const boost::json::value * scoop_long_slices_value_ptr = tractor_object.if_contains("scoop_long_slices");
            if (scoop_long_slices_value_ptr != nullptr) {
                physics.tractor.scoop_long_slices = boost::json::value_to<int>(*scoop_long_slices_value_ptr);
            }

            const boost::json::value * scoop_rad_slices_value_ptr = tractor_object.if_contains("scoop_rad_slices");
            if (scoop_rad_slices_value_ptr != nullptr) {
                physics.tractor.scoop_rad_slices = boost::json::value_to<int>(*scoop_rad_slices_value_ptr);
            }

        const boost::json::value * cargo_value_ptr = tractor_object.if_contains("cargo");
        if (cargo_value_ptr != nullptr) {
            boost::json::object cargo_object = cargo_value_ptr->get_object();
            const boost::json::value * distance_value_ptr = cargo_object.if_contains("distance");
            if (distance_value_ptr != nullptr) {
                physics.tractor.cargo.distance = boost::json::value_to<double>(*distance_value_ptr);
            }

            const boost::json::value * distance_own_rsize_value_ptr = cargo_object.if_contains("distance_own_rsize");
            if (distance_own_rsize_value_ptr != nullptr) {
                physics.tractor.cargo.distance_own_rsize = boost::json::value_to<double>(*distance_own_rsize_value_ptr);
            }

            const boost::json::value * distance_tgt_rsize_value_ptr = cargo_object.if_contains("distance_tgt_rsize");
            if (distance_tgt_rsize_value_ptr != nullptr) {
                physics.tractor.cargo.distance_tgt_rsize = boost::json::value_to<double>(*distance_tgt_rsize_value_ptr);
            }

            const boost::json::value * force_push_value_ptr = cargo_object.if_contains("force_push");
            if (force_push_value_ptr != nullptr) {
                physics.tractor.cargo.force_push = boost::json::value_to<bool>(*force_push_value_ptr);
            }

            const boost::json::value * force_in_value_ptr = cargo_object.if_contains("force_in");
            if (force_in_value_ptr != nullptr) {
                physics.tractor.cargo.force_in = boost::json::value_to<bool>(*force_in_value_ptr);
            }

        }


        const boost::json::value * disabled_value_ptr = tractor_object.if_contains("disabled");
        if (disabled_value_ptr != nullptr) {
            boost::json::object disabled_object = disabled_value_ptr->get_object();
            const boost::json::value * distance_value_ptr = disabled_object.if_contains("distance");
            if (distance_value_ptr != nullptr) {
                physics.tractor.disabled.distance = boost::json::value_to<double>(*distance_value_ptr);
            }

            const boost::json::value * distance_own_rsize_value_ptr = disabled_object.if_contains("distance_own_rsize");
            if (distance_own_rsize_value_ptr != nullptr) {
                physics.tractor.disabled.distance_own_rsize = boost::json::value_to<double>(*distance_own_rsize_value_ptr);
            }

            const boost::json::value * distance_tgt_rsize_value_ptr = disabled_object.if_contains("distance_tgt_rsize");
            if (distance_tgt_rsize_value_ptr != nullptr) {
                physics.tractor.disabled.distance_tgt_rsize = boost::json::value_to<double>(*distance_tgt_rsize_value_ptr);
            }

            const boost::json::value * force_push_value_ptr = disabled_object.if_contains("force_push");
            if (force_push_value_ptr != nullptr) {
                physics.tractor.disabled.force_push = boost::json::value_to<bool>(*force_push_value_ptr);
            }

            const boost::json::value * force_in_value_ptr = disabled_object.if_contains("force_in");
            if (force_in_value_ptr != nullptr) {
                physics.tractor.disabled.force_in = boost::json::value_to<bool>(*force_in_value_ptr);
            }

        }


        const boost::json::value * faction_value_ptr = tractor_object.if_contains("faction");
        if (faction_value_ptr != nullptr) {
            boost::json::object faction_object = faction_value_ptr->get_object();
            const boost::json::value * distance_value_ptr = faction_object.if_contains("distance");
            if (distance_value_ptr != nullptr) {
                physics.tractor.faction.distance = boost::json::value_to<double>(*distance_value_ptr);
            }

            const boost::json::value * distance_own_rsize_value_ptr = faction_object.if_contains("distance_own_rsize");
            if (distance_own_rsize_value_ptr != nullptr) {
                physics.tractor.faction.distance_own_rsize = boost::json::value_to<double>(*distance_own_rsize_value_ptr);
            }

            const boost::json::value * distance_tgt_rsize_value_ptr = faction_object.if_contains("distance_tgt_rsize");
            if (distance_tgt_rsize_value_ptr != nullptr) {
                physics.tractor.faction.distance_tgt_rsize = boost::json::value_to<double>(*distance_tgt_rsize_value_ptr);
            }

            const boost::json::value * force_push_value_ptr = faction_object.if_contains("force_push");
            if (force_push_value_ptr != nullptr) {
                physics.tractor.faction.force_push = boost::json::value_to<bool>(*force_push_value_ptr);
            }

            const boost::json::value * force_in_value_ptr = faction_object.if_contains("force_in");
            if (force_in_value_ptr != nullptr) {
                physics.tractor.faction.force_in = boost::json::value_to<bool>(*force_in_value_ptr);
            }

        }


        const boost::json::value * upgrade_value_ptr = tractor_object.if_contains("upgrade");
        if (upgrade_value_ptr != nullptr) {
            boost::json::object upgrade_object = upgrade_value_ptr->get_object();
            const boost::json::value * distance_value_ptr = upgrade_object.if_contains("distance");
            if (distance_value_ptr != nullptr) {
                physics.tractor.upgrade.distance = boost::json::value_to<double>(*distance_value_ptr);
            }

            const boost::json::value * distance_own_rsize_value_ptr = upgrade_object.if_contains("distance_own_rsize");
            if (distance_own_rsize_value_ptr != nullptr) {
                physics.tractor.upgrade.distance_own_rsize = boost::json::value_to<double>(*distance_own_rsize_value_ptr);
            }

            const boost::json::value * distance_tgt_rsize_value_ptr = upgrade_object.if_contains("distance_tgt_rsize");
            if (distance_tgt_rsize_value_ptr != nullptr) {
                physics.tractor.upgrade.distance_tgt_rsize = boost::json::value_to<double>(*distance_tgt_rsize_value_ptr);
            }

            const boost::json::value * force_push_value_ptr = upgrade_object.if_contains("force_push");
            if (force_push_value_ptr != nullptr) {
                physics.tractor.upgrade.force_push = boost::json::value_to<bool>(*force_push_value_ptr);
            }

            const boost::json::value * force_in_value_ptr = upgrade_object.if_contains("force_in");
            if (force_in_value_ptr != nullptr) {
                physics.tractor.upgrade.force_in = boost::json::value_to<bool>(*force_in_value_ptr);
            }

        }


        const boost::json::value * others_value_ptr = tractor_object.if_contains("others");
        if (others_value_ptr != nullptr) {
            boost::json::object others_object = others_value_ptr->get_object();
            const boost::json::value * distance_value_ptr = others_object.if_contains("distance");
            if (distance_value_ptr != nullptr) {
                physics.tractor.others.distance = boost::json::value_to<double>(*distance_value_ptr);
            }

            const boost::json::value * distance_own_rsize_value_ptr = others_object.if_contains("distance_own_rsize");
            if (distance_own_rsize_value_ptr != nullptr) {
                physics.tractor.others.distance_own_rsize = boost::json::value_to<double>(*distance_own_rsize_value_ptr);
            }

            const boost::json::value * distance_tgt_rsize_value_ptr = others_object.if_contains("distance_tgt_rsize");
            if (distance_tgt_rsize_value_ptr != nullptr) {
                physics.tractor.others.distance_tgt_rsize = boost::json::value_to<double>(*distance_tgt_rsize_value_ptr);
            }

            const boost::json::value * force_push_value_ptr = others_object.if_contains("force_push");
            if (force_push_value_ptr != nullptr) {
                physics.tractor.others.force_push = boost::json::value_to<bool>(*force_push_value_ptr);
            }

            const boost::json::value * force_in_value_ptr = others_object.if_contains("force_in");
            if (force_in_value_ptr != nullptr) {
                physics.tractor.others.force_in = boost::json::value_to<bool>(*force_in_value_ptr);
            }

        }


        }


        }


        const boost::json::value * player_value_ptr = root_object.if_contains("player");
        if (player_value_ptr != nullptr) {
            boost::json::object player_object = player_value_ptr->get_object();
            const boost::json::value * callsign_value_ptr = player_object.if_contains("callsign");
            if (callsign_value_ptr != nullptr) {
                player.callsign = boost::json::value_to<std::string>(*callsign_value_ptr);
            }

            const boost::json::value * password_value_ptr = player_object.if_contains("password");
            if (password_value_ptr != nullptr) {
                player.password = boost::json::value_to<std::string>(*password_value_ptr);
            }

        }


        const boost::json::value * splash_value_ptr = root_object.if_contains("splash");
        if (splash_value_ptr != nullptr) {
            boost::json::object splash_object = splash_value_ptr->get_object();
            const boost::json::value * auto_hide_value_ptr = splash_object.if_contains("auto_hide");
            if (auto_hide_value_ptr != nullptr) {
                splash.auto_hide = boost::json::value_to<bool>(*auto_hide_value_ptr);
            }

        }


        const boost::json::value * terrain_value_ptr = root_object.if_contains("terrain");
        if (terrain_value_ptr != nullptr) {
            boost::json::object terrain_object = terrain_value_ptr->get_object();
            const boost::json::value * mass_value_ptr = terrain_object.if_contains("mass");
            if (mass_value_ptr != nullptr) {
                terrain.mass = boost::json::value_to<double>(*mass_value_ptr);
            }

            const boost::json::value * radius_value_ptr = terrain_object.if_contains("radius");
            if (radius_value_ptr != nullptr) {
                terrain.radius = boost::json::value_to<double>(*radius_value_ptr);
            }

            const boost::json::value * xscale_value_ptr = terrain_object.if_contains("xscale");
            if (xscale_value_ptr != nullptr) {
                terrain.xscale = boost::json::value_to<double>(*xscale_value_ptr);
            }

            const boost::json::value * yscale_value_ptr = terrain_object.if_contains("yscale");
            if (yscale_value_ptr != nullptr) {
                terrain.yscale = boost::json::value_to<double>(*yscale_value_ptr);
            }

            const boost::json::value * zscale_value_ptr = terrain_object.if_contains("zscale");
            if (zscale_value_ptr != nullptr) {
                terrain.zscale = boost::json::value_to<double>(*zscale_value_ptr);
            }

        }


        const boost::json::value * unit_value_ptr = root_object.if_contains("unit");
        if (unit_value_ptr != nullptr) {
            boost::json::object unit_object = unit_value_ptr->get_object();
            const boost::json::value * default_aggressivity_value_ptr = unit_object.if_contains("default_aggressivity");
            if (default_aggressivity_value_ptr != nullptr) {
                unit.default_aggressivity = boost::json::value_to<double>(*default_aggressivity_value_ptr);
            }

        }


        const boost::json::value * warp_value_ptr = root_object.if_contains("warp");
        if (warp_value_ptr != nullptr) {
            boost::json::object warp_object = warp_value_ptr->get_object();
            const boost::json::value * bleed_factor_value_ptr = warp_object.if_contains("bleed_factor");
            if (bleed_factor_value_ptr != nullptr) {
                warp.bleed_factor = boost::json::value_to<double>(*bleed_factor_value_ptr);
            }

            const boost::json::value * computer_warp_ramp_up_time_value_ptr = warp_object.if_contains("computer_warp_ramp_up_time");
            if (computer_warp_ramp_up_time_value_ptr != nullptr) {
                warp.computer_warp_ramp_up_time = boost::json::value_to<double>(*computer_warp_ramp_up_time_value_ptr);
            }

            const boost::json::value * in_system_jump_cost_value_ptr = warp_object.if_contains("in_system_jump_cost");
            if (in_system_jump_cost_value_ptr != nullptr) {
                warp.in_system_jump_cost = boost::json::value_to<double>(*in_system_jump_cost_value_ptr);
            }

            const boost::json::value * max_effective_velocity_value_ptr = warp_object.if_contains("max_effective_velocity");
            if (max_effective_velocity_value_ptr != nullptr) {
                warp.max_effective_velocity = boost::json::value_to<double>(*max_effective_velocity_value_ptr);
            }

            const boost::json::value * player_warp_energy_multiplier_value_ptr = warp_object.if_contains("player_warp_energy_multiplier");
            if (player_warp_energy_multiplier_value_ptr != nullptr) {
                warp.player_warp_energy_multiplier = boost::json::value_to<double>(*player_warp_energy_multiplier_value_ptr);
            }

            const boost::json::value * use_warp_energy_for_cloak_value_ptr = warp_object.if_contains("use_warp_energy_for_cloak");
            if (use_warp_energy_for_cloak_value_ptr != nullptr) {
                warp.use_warp_energy_for_cloak = boost::json::value_to<bool>(*use_warp_energy_for_cloak_value_ptr);
            }

            const boost::json::value * warp_energy_multiplier_value_ptr = warp_object.if_contains("warp_energy_multiplier");
            if (warp_energy_multiplier_value_ptr != nullptr) {
                warp.warp_energy_multiplier = boost::json::value_to<double>(*warp_energy_multiplier_value_ptr);
            }

            const boost::json::value * warp_memory_effect_value_ptr = warp_object.if_contains("warp_memory_effect");
            if (warp_memory_effect_value_ptr != nullptr) {
                warp.warp_memory_effect = boost::json::value_to<double>(*warp_memory_effect_value_ptr);
            }

            const boost::json::value * warp_multiplier_max_value_ptr = warp_object.if_contains("warp_multiplier_max");
            if (warp_multiplier_max_value_ptr != nullptr) {
                warp.warp_multiplier_max = boost::json::value_to<double>(*warp_multiplier_max_value_ptr);
            }

            const boost::json::value * warp_multiplier_min_value_ptr = warp_object.if_contains("warp_multiplier_min");
            if (warp_multiplier_min_value_ptr != nullptr) {
                warp.warp_multiplier_min = boost::json::value_to<double>(*warp_multiplier_min_value_ptr);
            }

            const boost::json::value * warp_ramp_down_time_value_ptr = warp_object.if_contains("warp_ramp_down_time");
            if (warp_ramp_down_time_value_ptr != nullptr) {
                warp.warp_ramp_down_time = boost::json::value_to<double>(*warp_ramp_down_time_value_ptr);
            }

            const boost::json::value * warp_ramp_up_time_value_ptr = warp_object.if_contains("warp_ramp_up_time");
            if (warp_ramp_up_time_value_ptr != nullptr) {
                warp.warp_ramp_up_time = boost::json::value_to<double>(*warp_ramp_up_time_value_ptr);
            }

            const boost::json::value * warp_stretch_cutoff_value_ptr = warp_object.if_contains("warp_stretch_cutoff");
            if (warp_stretch_cutoff_value_ptr != nullptr) {
                warp.warp_stretch_cutoff = boost::json::value_to<double>(*warp_stretch_cutoff_value_ptr);
            }

            const boost::json::value * warp_stretch_decel_cutoff_value_ptr = warp_object.if_contains("warp_stretch_decel_cutoff");
            if (warp_stretch_decel_cutoff_value_ptr != nullptr) {
                warp.warp_stretch_decel_cutoff = boost::json::value_to<double>(*warp_stretch_decel_cutoff_value_ptr);
            }

        const boost::json::value * fov_link_value_ptr = warp_object.if_contains("fov_link");
        if (fov_link_value_ptr != nullptr) {
            boost::json::object fov_link_object = fov_link_value_ptr->get_object();
            const boost::json::value * reference_kps_value_ptr = fov_link_object.if_contains("reference_kps");
            if (reference_kps_value_ptr != nullptr) {
                warp.fov_link.reference_kps = boost::json::value_to<double>(*reference_kps_value_ptr);
            }

            const boost::json::value * smoothing_value_ptr = fov_link_object.if_contains("smoothing");
            if (smoothing_value_ptr != nullptr) {
                warp.fov_link.smoothing = boost::json::value_to<double>(*smoothing_value_ptr);
            }

        const boost::json::value * shake_value_ptr = fov_link_object.if_contains("shake");
        if (shake_value_ptr != nullptr) {
            boost::json::object shake_object = shake_value_ptr->get_object();
            const boost::json::value * asymptotic_value_ptr = shake_object.if_contains("asymptotic");
            if (asymptotic_value_ptr != nullptr) {
                warp.fov_link.shake.asymptotic = boost::json::value_to<bool>(*asymptotic_value_ptr);
            }

            const boost::json::value * exp_value_ptr = shake_object.if_contains("exp");
            if (exp_value_ptr != nullptr) {
                warp.fov_link.shake.exp = boost::json::value_to<double>(*exp_value_ptr);
            }

            const boost::json::value * high_ref_value_ptr = shake_object.if_contains("high_ref");
            if (high_ref_value_ptr != nullptr) {
                warp.fov_link.shake.high_ref = boost::json::value_to<double>(*high_ref_value_ptr);
            }

            const boost::json::value * low_ref_value_ptr = shake_object.if_contains("low_ref");
            if (low_ref_value_ptr != nullptr) {
                warp.fov_link.shake.low_ref = boost::json::value_to<double>(*low_ref_value_ptr);
            }

            const boost::json::value * speed_value_ptr = shake_object.if_contains("speed");
            if (speed_value_ptr != nullptr) {
                warp.fov_link.shake.speed = boost::json::value_to<double>(*speed_value_ptr);
            }

        const boost::json::value * offset_value_ptr = shake_object.if_contains("offset");
        if (offset_value_ptr != nullptr) {
            boost::json::object offset_object = offset_value_ptr->get_object();
            const boost::json::value * back_value_ptr = offset_object.if_contains("back");
            if (back_value_ptr != nullptr) {
                warp.fov_link.shake.offset.back = boost::json::value_to<double>(*back_value_ptr);
            }

            const boost::json::value * front_value_ptr = offset_object.if_contains("front");
            if (front_value_ptr != nullptr) {
                warp.fov_link.shake.offset.front = boost::json::value_to<double>(*front_value_ptr);
            }

            const boost::json::value * perpendicular_value_ptr = offset_object.if_contains("perpendicular");
            if (perpendicular_value_ptr != nullptr) {
                warp.fov_link.shake.offset.perpendicular = boost::json::value_to<double>(*perpendicular_value_ptr);
            }

        }


        const boost::json::value * multiplier_value_ptr = shake_object.if_contains("multiplier");
        if (multiplier_value_ptr != nullptr) {
            boost::json::object multiplier_object = multiplier_value_ptr->get_object();
            const boost::json::value * back_value_ptr = multiplier_object.if_contains("back");
            if (back_value_ptr != nullptr) {
                warp.fov_link.shake.multiplier.back = boost::json::value_to<double>(*back_value_ptr);
            }

            const boost::json::value * front_value_ptr = multiplier_object.if_contains("front");
            if (front_value_ptr != nullptr) {
                warp.fov_link.shake.multiplier.front = boost::json::value_to<double>(*front_value_ptr);
            }

            const boost::json::value * perpendicular_value_ptr = multiplier_object.if_contains("perpendicular");
            if (perpendicular_value_ptr != nullptr) {
                warp.fov_link.shake.multiplier.perpendicular = boost::json::value_to<double>(*perpendicular_value_ptr);
            }

        }


        }


        const boost::json::value * stable_value_ptr = fov_link_object.if_contains("stable");
        if (stable_value_ptr != nullptr) {
            boost::json::object stable_object = stable_value_ptr->get_object();
            const boost::json::value * asymptotic_value_ptr = stable_object.if_contains("asymptotic");
            if (asymptotic_value_ptr != nullptr) {
                warp.fov_link.stable.asymptotic = boost::json::value_to<bool>(*asymptotic_value_ptr);
            }

            const boost::json::value * exp_value_ptr = stable_object.if_contains("exp");
            if (exp_value_ptr != nullptr) {
                warp.fov_link.stable.exp = boost::json::value_to<double>(*exp_value_ptr);
            }

            const boost::json::value * high_ref_value_ptr = stable_object.if_contains("high_ref");
            if (high_ref_value_ptr != nullptr) {
                warp.fov_link.stable.high_ref = boost::json::value_to<double>(*high_ref_value_ptr);
            }

            const boost::json::value * low_ref_value_ptr = stable_object.if_contains("low_ref");
            if (low_ref_value_ptr != nullptr) {
                warp.fov_link.stable.low_ref = boost::json::value_to<double>(*low_ref_value_ptr);
            }

        const boost::json::value * offset_value_ptr = stable_object.if_contains("offset");
        if (offset_value_ptr != nullptr) {
            boost::json::object offset_object = offset_value_ptr->get_object();
            const boost::json::value * back_value_ptr = offset_object.if_contains("back");
            if (back_value_ptr != nullptr) {
                warp.fov_link.stable.offset.back = boost::json::value_to<double>(*back_value_ptr);
            }

            const boost::json::value * front_value_ptr = offset_object.if_contains("front");
            if (front_value_ptr != nullptr) {
                warp.fov_link.stable.offset.front = boost::json::value_to<double>(*front_value_ptr);
            }

            const boost::json::value * perpendicular_value_ptr = offset_object.if_contains("perpendicular");
            if (perpendicular_value_ptr != nullptr) {
                warp.fov_link.stable.offset.perpendicular = boost::json::value_to<double>(*perpendicular_value_ptr);
            }

        }


        const boost::json::value * multiplier_value_ptr = stable_object.if_contains("multiplier");
        if (multiplier_value_ptr != nullptr) {
            boost::json::object multiplier_object = multiplier_value_ptr->get_object();
            const boost::json::value * back_value_ptr = multiplier_object.if_contains("back");
            if (back_value_ptr != nullptr) {
                warp.fov_link.stable.multiplier.back = boost::json::value_to<double>(*back_value_ptr);
            }

            const boost::json::value * front_value_ptr = multiplier_object.if_contains("front");
            if (front_value_ptr != nullptr) {
                warp.fov_link.stable.multiplier.front = boost::json::value_to<double>(*front_value_ptr);
            }

            const boost::json::value * perpendicular_value_ptr = multiplier_object.if_contains("perpendicular");
            if (perpendicular_value_ptr != nullptr) {
                warp.fov_link.stable.multiplier.perpendicular = boost::json::value_to<double>(*perpendicular_value_ptr);
            }

        }


        }


        }


        }


        const boost::json::value * weapons_value_ptr = root_object.if_contains("weapons");
        if (weapons_value_ptr != nullptr) {
            boost::json::object weapons_object = weapons_value_ptr->get_object();
            const boost::json::value * can_fire_in_cloak_value_ptr = weapons_object.if_contains("can_fire_in_cloak");
            if (can_fire_in_cloak_value_ptr != nullptr) {
                weapons.can_fire_in_cloak = boost::json::value_to<bool>(*can_fire_in_cloak_value_ptr);
            }

            const boost::json::value * can_fire_in_spec_value_ptr = weapons_object.if_contains("can_fire_in_spec");
            if (can_fire_in_spec_value_ptr != nullptr) {
                weapons.can_fire_in_spec = boost::json::value_to<bool>(*can_fire_in_spec_value_ptr);
            }

        }


    }
    catch (std::exception const& e)
    {
        VS_LOG(error, (boost::format("Error in %1%: '%2%'") % __FUNCTION__ % e.what()));
    }
}
