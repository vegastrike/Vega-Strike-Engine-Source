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

            const boost::json::value * docking_time_value_ptr = general_object.if_contains("docking_time");
            if (docking_time_value_ptr != nullptr) {
                general.docking_time = boost::json::value_to<double>(*docking_time_value_ptr);
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
            const boost::json::value * resolution_x_value_ptr = graphics_object.if_contains("resolution_x");
            if (resolution_x_value_ptr != nullptr) {
                graphics.resolution_x = boost::json::value_to<int>(*resolution_x_value_ptr);
            }

            const boost::json::value * resolution_y_value_ptr = graphics_object.if_contains("resolution_y");
            if (resolution_y_value_ptr != nullptr) {
                graphics.resolution_y = boost::json::value_to<int>(*resolution_y_value_ptr);
            }

            const boost::json::value * screen_value_ptr = graphics_object.if_contains("screen");
            if (screen_value_ptr != nullptr) {
                graphics.screen = boost::json::value_to<int>(*screen_value_ptr);
            }

            const boost::json::value * atmosphere_emissive_value_ptr = graphics_object.if_contains("atmosphere_emissive");
            if (atmosphere_emissive_value_ptr != nullptr) {
                graphics.atmosphere_emissive = boost::json::value_to<double>(*atmosphere_emissive_value_ptr);
            }

            const boost::json::value * atmosphere_diffuse_value_ptr = graphics_object.if_contains("atmosphere_diffuse");
            if (atmosphere_diffuse_value_ptr != nullptr) {
                graphics.atmosphere_diffuse = boost::json::value_to<double>(*atmosphere_diffuse_value_ptr);
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

            const boost::json::value * city_light_strength_value_ptr = graphics_object.if_contains("city_light_strength");
            if (city_light_strength_value_ptr != nullptr) {
                graphics.city_light_strength = boost::json::value_to<double>(*city_light_strength_value_ptr);
            }

            const boost::json::value * day_city_light_strength_value_ptr = graphics_object.if_contains("day_city_light_strength");
            if (day_city_light_strength_value_ptr != nullptr) {
                graphics.day_city_light_strength = boost::json::value_to<double>(*day_city_light_strength_value_ptr);
            }

            const boost::json::value * default_engine_activation_value_ptr = graphics_object.if_contains("default_engine_activation");
            if (default_engine_activation_value_ptr != nullptr) {
                graphics.default_engine_activation = boost::json::value_to<double>(*default_engine_activation_value_ptr);
            }

            const boost::json::value * draw_heading_marker_value_ptr = graphics_object.if_contains("draw_heading_marker");
            if (draw_heading_marker_value_ptr != nullptr) {
                graphics.draw_heading_marker = boost::json::value_to<bool>(*draw_heading_marker_value_ptr);
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

            const boost::json::value * explosion_animation_value_ptr = graphics_object.if_contains("explosion_animation");
            if (explosion_animation_value_ptr != nullptr) {
                graphics.explosion_animation = boost::json::value_to<std::string>(*explosion_animation_value_ptr);
            }

            const boost::json::value * fov_value_ptr = graphics_object.if_contains("fov");
            if (fov_value_ptr != nullptr) {
                graphics.fov = boost::json::value_to<double>(*fov_value_ptr);
            }

            const boost::json::value * glow_ambient_star_light_value_ptr = graphics_object.if_contains("glow_ambient_star_light");
            if (glow_ambient_star_light_value_ptr != nullptr) {
                graphics.glow_ambient_star_light = boost::json::value_to<bool>(*glow_ambient_star_light_value_ptr);
            }

            const boost::json::value * glow_diffuse_star_light_value_ptr = graphics_object.if_contains("glow_diffuse_star_light");
            if (glow_diffuse_star_light_value_ptr != nullptr) {
                graphics.glow_diffuse_star_light = boost::json::value_to<bool>(*glow_diffuse_star_light_value_ptr);
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

            const boost::json::value * missile_explosion_radius_mult_value_ptr = graphics_object.if_contains("missile_explosion_radius_mult");
            if (missile_explosion_radius_mult_value_ptr != nullptr) {
                graphics.missile_explosion_radius_mult = boost::json::value_to<double>(*missile_explosion_radius_mult_value_ptr);
            }

            const boost::json::value * missile_sparkle_value_ptr = graphics_object.if_contains("missile_sparkle");
            if (missile_sparkle_value_ptr != nullptr) {
                graphics.missile_sparkle = boost::json::value_to<bool>(*missile_sparkle_value_ptr);
            }

            const boost::json::value * num_times_to_draw_shine_value_ptr = graphics_object.if_contains("num_times_to_draw_shine");
            if (num_times_to_draw_shine_value_ptr != nullptr) {
                graphics.num_times_to_draw_shine = boost::json::value_to<int>(*num_times_to_draw_shine_value_ptr);
            }

            const boost::json::value * planet_detail_stack_count_value_ptr = graphics_object.if_contains("planet_detail_stack_count");
            if (planet_detail_stack_count_value_ptr != nullptr) {
                graphics.planet_detail_stack_count = boost::json::value_to<int>(*planet_detail_stack_count_value_ptr);
            }

            const boost::json::value * reduced_vdus_width_value_ptr = graphics_object.if_contains("reduced_vdus_width");
            if (reduced_vdus_width_value_ptr != nullptr) {
                graphics.reduced_vdus_width = boost::json::value_to<double>(*reduced_vdus_width_value_ptr);
            }

            const boost::json::value * reduced_vdus_height_value_ptr = graphics_object.if_contains("reduced_vdus_height");
            if (reduced_vdus_height_value_ptr != nullptr) {
                graphics.reduced_vdus_height = boost::json::value_to<double>(*reduced_vdus_height_value_ptr);
            }

            const boost::json::value * shield_detail_stack_count_value_ptr = graphics_object.if_contains("shield_detail_stack_count");
            if (shield_detail_stack_count_value_ptr != nullptr) {
                graphics.shield_detail_stack_count = boost::json::value_to<int>(*shield_detail_stack_count_value_ptr);
            }

            const boost::json::value * shield_technique_value_ptr = graphics_object.if_contains("shield_technique");
            if (shield_technique_value_ptr != nullptr) {
                graphics.shield_technique = boost::json::value_to<std::string>(*shield_technique_value_ptr);
            }

            const boost::json::value * shield_texture_value_ptr = graphics_object.if_contains("shield_texture");
            if (shield_texture_value_ptr != nullptr) {
                graphics.shield_texture = boost::json::value_to<std::string>(*shield_texture_value_ptr);
            }

            const boost::json::value * star_body_radius_value_ptr = graphics_object.if_contains("star_body_radius");
            if (star_body_radius_value_ptr != nullptr) {
                graphics.star_body_radius = boost::json::value_to<double>(*star_body_radius_value_ptr);
            }

            const boost::json::value * star_glow_radius_value_ptr = graphics_object.if_contains("star_glow_radius");
            if (star_glow_radius_value_ptr != nullptr) {
                graphics.star_glow_radius = boost::json::value_to<double>(*star_glow_radius_value_ptr);
            }

            const boost::json::value * wormhole_unit_value_ptr = graphics_object.if_contains("wormhole_unit");
            if (wormhole_unit_value_ptr != nullptr) {
                graphics.wormhole_unit = boost::json::value_to<std::string>(*wormhole_unit_value_ptr);
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
            const boost::json::value * master_part_list_value_ptr = data_object.if_contains("master_part_list");
            if (master_part_list_value_ptr != nullptr) {
                data.master_part_list = boost::json::value_to<std::string>(*master_part_list_value_ptr);
            }

            const boost::json::value * using_templates_value_ptr = data_object.if_contains("using_templates");
            if (using_templates_value_ptr != nullptr) {
                data.using_templates = boost::json::value_to<bool>(*using_templates_value_ptr);
            }

        }


        const boost::json::value * game_start_value_ptr = root_object.if_contains("game_start");
        if (game_start_value_ptr != nullptr) {
            boost::json::object game_start_object = game_start_value_ptr->get_object();
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
            const boost::json::value * always_obedient_value_ptr = ai_object.if_contains("always_obedient");
            if (always_obedient_value_ptr != nullptr) {
                ai.always_obedient = boost::json::value_to<bool>(*always_obedient_value_ptr);
            }

            const boost::json::value * assist_friend_in_need_value_ptr = ai_object.if_contains("assist_friend_in_need");
            if (assist_friend_in_need_value_ptr != nullptr) {
                ai.assist_friend_in_need = boost::json::value_to<bool>(*assist_friend_in_need_value_ptr);
            }

            const boost::json::value * ease_to_anger_value_ptr = ai_object.if_contains("ease_to_anger");
            if (ease_to_anger_value_ptr != nullptr) {
                ai.ease_to_anger = boost::json::value_to<double>(*ease_to_anger_value_ptr);
            }

            const boost::json::value * ease_to_appease_value_ptr = ai_object.if_contains("ease_to_appease");
            if (ease_to_appease_value_ptr != nullptr) {
                ai.ease_to_appease = boost::json::value_to<double>(*ease_to_appease_value_ptr);
            }

            const boost::json::value * friend_factor_value_ptr = ai_object.if_contains("friend_factor");
            if (friend_factor_value_ptr != nullptr) {
                ai.friend_factor = boost::json::value_to<double>(*friend_factor_value_ptr);
            }

            const boost::json::value * hull_damage_anger_value_ptr = ai_object.if_contains("hull_damage_anger");
            if (hull_damage_anger_value_ptr != nullptr) {
                ai.hull_damage_anger = boost::json::value_to<int>(*hull_damage_anger_value_ptr);
            }

            const boost::json::value * hull_percent_for_comm_value_ptr = ai_object.if_contains("hull_percent_for_comm");
            if (hull_percent_for_comm_value_ptr != nullptr) {
                ai.hull_percent_for_comm = boost::json::value_to<double>(*hull_percent_for_comm_value_ptr);
            }

            const boost::json::value * kill_factor_value_ptr = ai_object.if_contains("kill_factor");
            if (kill_factor_value_ptr != nullptr) {
                ai.kill_factor = boost::json::value_to<double>(*kill_factor_value_ptr);
            }

            const boost::json::value * lowest_negative_comm_choice_value_ptr = ai_object.if_contains("lowest_negative_comm_choice");
            if (lowest_negative_comm_choice_value_ptr != nullptr) {
                ai.lowest_negative_comm_choice = boost::json::value_to<double>(*lowest_negative_comm_choice_value_ptr);
            }

            const boost::json::value * lowest_positive_comm_choice_value_ptr = ai_object.if_contains("lowest_positive_comm_choice");
            if (lowest_positive_comm_choice_value_ptr != nullptr) {
                ai.lowest_positive_comm_choice = boost::json::value_to<double>(*lowest_positive_comm_choice_value_ptr);
            }

            const boost::json::value * min_relationship_value_ptr = ai_object.if_contains("min_relationship");
            if (min_relationship_value_ptr != nullptr) {
                ai.min_relationship = boost::json::value_to<double>(*min_relationship_value_ptr);
            }

            const boost::json::value * mood_swing_level_value_ptr = ai_object.if_contains("mood_swing_level");
            if (mood_swing_level_value_ptr != nullptr) {
                ai.mood_swing_level = boost::json::value_to<double>(*mood_swing_level_value_ptr);
            }

            const boost::json::value * random_response_range_value_ptr = ai_object.if_contains("random_response_range");
            if (random_response_range_value_ptr != nullptr) {
                ai.random_response_range = boost::json::value_to<double>(*random_response_range_value_ptr);
            }

            const boost::json::value * shield_damage_anger_value_ptr = ai_object.if_contains("shield_damage_anger");
            if (shield_damage_anger_value_ptr != nullptr) {
                ai.shield_damage_anger = boost::json::value_to<int>(*shield_damage_anger_value_ptr);
            }

            const boost::json::value * jump_without_energy_value_ptr = ai_object.if_contains("jump_without_energy");
            if (jump_without_energy_value_ptr != nullptr) {
                ai.jump_without_energy = boost::json::value_to<bool>(*jump_without_energy_value_ptr);
            }

        const boost::json::value * firing_value_ptr = ai_object.if_contains("firing");
        if (firing_value_ptr != nullptr) {
            boost::json::object firing_object = firing_value_ptr->get_object();
            const boost::json::value * missile_probability_value_ptr = firing_object.if_contains("missile_probability");
            if (missile_probability_value_ptr != nullptr) {
                ai.firing.missile_probability = boost::json::value_to<double>(*missile_probability_value_ptr);
            }

            const boost::json::value * aggressivity_value_ptr = firing_object.if_contains("aggressivity");
            if (aggressivity_value_ptr != nullptr) {
                ai.firing.aggressivity = boost::json::value_to<double>(*aggressivity_value_ptr);
            }

        }


        const boost::json::value * targeting_value_ptr = ai_object.if_contains("targeting");
        if (targeting_value_ptr != nullptr) {
            boost::json::object targeting_object = targeting_value_ptr->get_object();
            const boost::json::value * escort_distance_value_ptr = targeting_object.if_contains("escort_distance");
            if (escort_distance_value_ptr != nullptr) {
                ai.targeting.escort_distance = boost::json::value_to<double>(*escort_distance_value_ptr);
            }

            const boost::json::value * turn_leader_distance_value_ptr = targeting_object.if_contains("turn_leader_distance");
            if (turn_leader_distance_value_ptr != nullptr) {
                ai.targeting.turn_leader_distance = boost::json::value_to<double>(*turn_leader_distance_value_ptr);
            }

            const boost::json::value * time_to_recommand_wing_value_ptr = targeting_object.if_contains("time_to_recommand_wing");
            if (time_to_recommand_wing_value_ptr != nullptr) {
                ai.targeting.time_to_recommand_wing = boost::json::value_to<double>(*time_to_recommand_wing_value_ptr);
            }

            const boost::json::value * min_time_to_switch_targets_value_ptr = targeting_object.if_contains("min_time_to_switch_targets");
            if (min_time_to_switch_targets_value_ptr != nullptr) {
                ai.targeting.min_time_to_switch_targets = boost::json::value_to<double>(*min_time_to_switch_targets_value_ptr);
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


        const boost::json::value * physics_value_ptr = root_object.if_contains("physics");
        if (physics_value_ptr != nullptr) {
            boost::json::object physics_object = physics_value_ptr->get_object();
            const boost::json::value * capship_size_value_ptr = physics_object.if_contains("capship_size");
            if (capship_size_value_ptr != nullptr) {
                physics.capship_size = boost::json::value_to<double>(*capship_size_value_ptr);
            }

            const boost::json::value * close_enough_to_autotrack_value_ptr = physics_object.if_contains("close_enough_to_autotrack");
            if (close_enough_to_autotrack_value_ptr != nullptr) {
                physics.close_enough_to_autotrack = boost::json::value_to<double>(*close_enough_to_autotrack_value_ptr);
            }

            const boost::json::value * collision_scale_factor_value_ptr = physics_object.if_contains("collision_scale_factor");
            if (collision_scale_factor_value_ptr != nullptr) {
                physics.collision_scale_factor = boost::json::value_to<double>(*collision_scale_factor_value_ptr);
            }

            const boost::json::value * debris_time_value_ptr = physics_object.if_contains("debris_time");
            if (debris_time_value_ptr != nullptr) {
                physics.debris_time = boost::json::value_to<double>(*debris_time_value_ptr);
            }

            const boost::json::value * default_shield_tightness_value_ptr = physics_object.if_contains("default_shield_tightness");
            if (default_shield_tightness_value_ptr != nullptr) {
                physics.default_shield_tightness = boost::json::value_to<double>(*default_shield_tightness_value_ptr);
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

            const boost::json::value * distance_to_warp_value_ptr = physics_object.if_contains("distance_to_warp");
            if (distance_to_warp_value_ptr != nullptr) {
                physics.distance_to_warp = boost::json::value_to<double>(*distance_to_warp_value_ptr);
            }

            const boost::json::value * does_missile_bounce_value_ptr = physics_object.if_contains("does_missile_bounce");
            if (does_missile_bounce_value_ptr != nullptr) {
                physics.does_missile_bounce = boost::json::value_to<bool>(*does_missile_bounce_value_ptr);
            }

            const boost::json::value * engine_energy_takes_priority_value_ptr = physics_object.if_contains("engine_energy_takes_priority");
            if (engine_energy_takes_priority_value_ptr != nullptr) {
                physics.engine_energy_takes_priority = boost::json::value_to<bool>(*engine_energy_takes_priority_value_ptr);
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

            const boost::json::value * hostile_auto_radius_value_ptr = physics_object.if_contains("hostile_auto_radius");
            if (hostile_auto_radius_value_ptr != nullptr) {
                physics.hostile_auto_radius = boost::json::value_to<double>(*hostile_auto_radius_value_ptr);
            }

            const boost::json::value * indiscriminate_system_destruction_value_ptr = physics_object.if_contains("indiscriminate_system_destruction");
            if (indiscriminate_system_destruction_value_ptr != nullptr) {
                physics.indiscriminate_system_destruction = boost::json::value_to<double>(*indiscriminate_system_destruction_value_ptr);
            }

            const boost::json::value * inelastic_scale_value_ptr = physics_object.if_contains("inelastic_scale");
            if (inelastic_scale_value_ptr != nullptr) {
                physics.inelastic_scale = boost::json::value_to<double>(*inelastic_scale_value_ptr);
            }

            const boost::json::value * jump_mesh_radius_scale_value_ptr = physics_object.if_contains("jump_mesh_radius_scale");
            if (jump_mesh_radius_scale_value_ptr != nullptr) {
                physics.jump_mesh_radius_scale = boost::json::value_to<double>(*jump_mesh_radius_scale_value_ptr);
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

            const boost::json::value * max_lost_target_live_time_value_ptr = physics_object.if_contains("max_lost_target_live_time");
            if (max_lost_target_live_time_value_ptr != nullptr) {
                physics.max_lost_target_live_time = boost::json::value_to<double>(*max_lost_target_live_time_value_ptr);
            }

            const boost::json::value * max_non_player_rotation_rate_value_ptr = physics_object.if_contains("max_non_player_rotation_rate");
            if (max_non_player_rotation_rate_value_ptr != nullptr) {
                physics.max_non_player_rotation_rate = boost::json::value_to<double>(*max_non_player_rotation_rate_value_ptr);
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

            const boost::json::value * min_asteroid_distance_value_ptr = physics_object.if_contains("min_asteroid_distance");
            if (min_asteroid_distance_value_ptr != nullptr) {
                physics.min_asteroid_distance = boost::json::value_to<double>(*min_asteroid_distance_value_ptr);
            }

            const boost::json::value * min_damage_value_ptr = physics_object.if_contains("min_damage");
            if (min_damage_value_ptr != nullptr) {
                physics.min_damage = boost::json::value_to<double>(*min_damage_value_ptr);
            }

            const boost::json::value * min_shield_speeding_discharge_value_ptr = physics_object.if_contains("min_shield_speeding_discharge");
            if (min_shield_speeding_discharge_value_ptr != nullptr) {
                physics.min_shield_speeding_discharge = boost::json::value_to<double>(*min_shield_speeding_discharge_value_ptr);
            }

            const boost::json::value * minimum_mass_value_ptr = physics_object.if_contains("minimum_mass");
            if (minimum_mass_value_ptr != nullptr) {
                physics.minimum_mass = boost::json::value_to<double>(*minimum_mass_value_ptr);
            }

            const boost::json::value * minimum_time_value_ptr = physics_object.if_contains("minimum_time");
            if (minimum_time_value_ptr != nullptr) {
                physics.minimum_time = boost::json::value_to<double>(*minimum_time_value_ptr);
            }

            const boost::json::value * near_autotrack_cone_value_ptr = physics_object.if_contains("near_autotrack_cone");
            if (near_autotrack_cone_value_ptr != nullptr) {
                physics.near_autotrack_cone = boost::json::value_to<double>(*near_autotrack_cone_value_ptr);
            }

            const boost::json::value * nebula_shield_recharge_value_ptr = physics_object.if_contains("nebula_shield_recharge");
            if (nebula_shield_recharge_value_ptr != nullptr) {
                physics.nebula_shield_recharge = boost::json::value_to<double>(*nebula_shield_recharge_value_ptr);
            }

            const boost::json::value * neutral_auto_radius_value_ptr = physics_object.if_contains("neutral_auto_radius");
            if (neutral_auto_radius_value_ptr != nullptr) {
                physics.neutral_auto_radius = boost::json::value_to<double>(*neutral_auto_radius_value_ptr);
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

            const boost::json::value * percent_missile_match_target_velocity_value_ptr = physics_object.if_contains("percent_missile_match_target_velocity");
            if (percent_missile_match_target_velocity_value_ptr != nullptr) {
                physics.percent_missile_match_target_velocity = boost::json::value_to<double>(*percent_missile_match_target_velocity_value_ptr);
            }

            const boost::json::value * planet_dock_min_port_size_value_ptr = physics_object.if_contains("planet_dock_min_port_size");
            if (planet_dock_min_port_size_value_ptr != nullptr) {
                physics.planet_dock_min_port_size = boost::json::value_to<double>(*planet_dock_min_port_size_value_ptr);
            }

            const boost::json::value * planet_dock_port_size_value_ptr = physics_object.if_contains("planet_dock_port_size");
            if (planet_dock_port_size_value_ptr != nullptr) {
                physics.planet_dock_port_size = boost::json::value_to<double>(*planet_dock_port_size_value_ptr);
            }

            const boost::json::value * planets_always_neutral_value_ptr = physics_object.if_contains("planets_always_neutral");
            if (planets_always_neutral_value_ptr != nullptr) {
                physics.planets_always_neutral = boost::json::value_to<bool>(*planets_always_neutral_value_ptr);
            }

            const boost::json::value * planets_can_have_subunits_value_ptr = physics_object.if_contains("planets_can_have_subunits");
            if (planets_can_have_subunits_value_ptr != nullptr) {
                physics.planets_can_have_subunits = boost::json::value_to<bool>(*planets_can_have_subunits_value_ptr);
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

            const boost::json::value * speeding_discharge_value_ptr = physics_object.if_contains("speeding_discharge");
            if (speeding_discharge_value_ptr != nullptr) {
                physics.speeding_discharge = boost::json::value_to<double>(*speeding_discharge_value_ptr);
            }

            const boost::json::value * steady_itts_value_ptr = physics_object.if_contains("steady_itts");
            if (steady_itts_value_ptr != nullptr) {
                physics.steady_itts = boost::json::value_to<bool>(*steady_itts_value_ptr);
            }

            const boost::json::value * system_damage_on_armor_value_ptr = physics_object.if_contains("system_damage_on_armor");
            if (system_damage_on_armor_value_ptr != nullptr) {
                physics.system_damage_on_armor = boost::json::value_to<bool>(*system_damage_on_armor_value_ptr);
            }

            const boost::json::value * target_distance_to_warp_bonus_value_ptr = physics_object.if_contains("target_distance_to_warp_bonus");
            if (target_distance_to_warp_bonus_value_ptr != nullptr) {
                physics.target_distance_to_warp_bonus = boost::json::value_to<double>(*target_distance_to_warp_bonus_value_ptr);
            }

            const boost::json::value * thruster_hit_chance_value_ptr = physics_object.if_contains("thruster_hit_chance");
            if (thruster_hit_chance_value_ptr != nullptr) {
                physics.thruster_hit_chance = boost::json::value_to<double>(*thruster_hit_chance_value_ptr);
            }

            const boost::json::value * use_max_shield_energy_usage_value_ptr = physics_object.if_contains("use_max_shield_energy_usage");
            if (use_max_shield_energy_usage_value_ptr != nullptr) {
                physics.use_max_shield_energy_usage = boost::json::value_to<bool>(*use_max_shield_energy_usage_value_ptr);
            }

            const boost::json::value * velocity_max_value_ptr = physics_object.if_contains("velocity_max");
            if (velocity_max_value_ptr != nullptr) {
                physics.velocity_max = boost::json::value_to<double>(*velocity_max_value_ptr);
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
