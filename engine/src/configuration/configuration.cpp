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



        const boost::system::result<const boost::json::value &> general_value = root_object.try_at("general");
        if (general_value.has_value()) {
            boost::json::object general_object = general_value.value().get_object();
            const boost::system::result<const boost::json::value &> audio_atom_result = general_object.try_at("audio_atom");
            if (audio_atom_result.has_value()) {
                general.audio_atom = boost::json::value_to<double>(audio_atom_result.value());
            }

            const boost::system::result<const boost::json::value &> command_interpreter_result = general_object.try_at("command_interpreter");
            if (command_interpreter_result.has_value()) {
                general.command_interpreter = boost::json::value_to<bool>(command_interpreter_result.value());
            }

            const boost::system::result<const boost::json::value &> custom_python_result = general_object.try_at("custom_python");
            if (custom_python_result.has_value()) {
                general.custom_python = boost::json::value_to<std::string>(custom_python_result.value());
            }

            const boost::system::result<const boost::json::value &> debug_config_result = general_object.try_at("debug_config");
            if (debug_config_result.has_value()) {
                general.debug_config = boost::json::value_to<bool>(debug_config_result.value());
            }

            const boost::system::result<const boost::json::value &> debug_fs_result = general_object.try_at("debug_fs");
            if (debug_fs_result.has_value()) {
                general.debug_fs = boost::json::value_to<int>(debug_fs_result.value());
            }

            const boost::system::result<const boost::json::value &> delete_old_systems_result = general_object.try_at("delete_old_systems");
            if (delete_old_systems_result.has_value()) {
                general.delete_old_systems = boost::json::value_to<bool>(delete_old_systems_result.value());
            }

            const boost::system::result<const boost::json::value &> docking_time_result = general_object.try_at("docking_time");
            if (docking_time_result.has_value()) {
                general.docking_time = boost::json::value_to<double>(docking_time_result.value());
            }

            const boost::system::result<const boost::json::value &> docking_fee_result = general_object.try_at("docking_fee");
            if (docking_fee_result.has_value()) {
                general.docking_fee = boost::json::value_to<double>(docking_fee_result.value());
            }

            const boost::system::result<const boost::json::value &> empty_mission_result = general_object.try_at("empty_mission");
            if (empty_mission_result.has_value()) {
                general.empty_mission = boost::json::value_to<std::string>(empty_mission_result.value());
            }

            const boost::system::result<const boost::json::value &> force_anonymous_mission_names_result = general_object.try_at("force_anonymous_mission_names");
            if (force_anonymous_mission_names_result.has_value()) {
                general.force_anonymous_mission_names = boost::json::value_to<bool>(force_anonymous_mission_names_result.value());
            }

            const boost::system::result<const boost::json::value &> fuel_docking_fee_result = general_object.try_at("fuel_docking_fee");
            if (fuel_docking_fee_result.has_value()) {
                general.fuel_docking_fee = boost::json::value_to<double>(fuel_docking_fee_result.value());
            }

            const boost::system::result<const boost::json::value &> garbage_collect_frequency_result = general_object.try_at("garbage_collect_frequency");
            if (garbage_collect_frequency_result.has_value()) {
                general.garbage_collect_frequency = boost::json::value_to<int>(garbage_collect_frequency_result.value());
            }

            const boost::system::result<const boost::json::value &> jump_key_delay_result = general_object.try_at("jump_key_delay");
            if (jump_key_delay_result.has_value()) {
                general.jump_key_delay = boost::json::value_to<double>(jump_key_delay_result.value());
            }

            const boost::system::result<const boost::json::value &> load_last_savegame_result = general_object.try_at("load_last_savegame");
            if (load_last_savegame_result.has_value()) {
                general.load_last_savegame = boost::json::value_to<bool>(load_last_savegame_result.value());
            }

            const boost::system::result<const boost::json::value &> new_game_save_name_result = general_object.try_at("new_game_save_name");
            if (new_game_save_name_result.has_value()) {
                general.new_game_save_name = boost::json::value_to<std::string>(new_game_save_name_result.value());
            }

            const boost::system::result<const boost::json::value &> num_old_systems_result = general_object.try_at("num_old_systems");
            if (num_old_systems_result.has_value()) {
                general.num_old_systems = boost::json::value_to<int>(num_old_systems_result.value());
            }

            const boost::system::result<const boost::json::value &> percentage_speed_change_to_fault_search_result = general_object.try_at("percentage_speed_change_to_fault_search");
            if (percentage_speed_change_to_fault_search_result.has_value()) {
                general.percentage_speed_change_to_fault_search = boost::json::value_to<double>(percentage_speed_change_to_fault_search_result.value());
            }

            const boost::system::result<const boost::json::value &> persistent_mission_across_ship_switch_result = general_object.try_at("persistent_mission_across_ship_switch");
            if (persistent_mission_across_ship_switch_result.has_value()) {
                general.persistent_mission_across_ship_switch = boost::json::value_to<bool>(persistent_mission_across_ship_switch_result.value());
            }

            const boost::system::result<const boost::json::value &> pitch_result = general_object.try_at("pitch");
            if (pitch_result.has_value()) {
                general.pitch = boost::json::value_to<double>(pitch_result.value());
            }

            const boost::system::result<const boost::json::value &> quick_savegame_summaries_result = general_object.try_at("quick_savegame_summaries");
            if (quick_savegame_summaries_result.has_value()) {
                general.quick_savegame_summaries = boost::json::value_to<bool>(quick_savegame_summaries_result.value());
            }

            const boost::system::result<const boost::json::value &> quick_savegame_summaries_buffer_size_result = general_object.try_at("quick_savegame_summaries_buffer_size");
            if (quick_savegame_summaries_buffer_size_result.has_value()) {
                general.quick_savegame_summaries_buffer_size = boost::json::value_to<int>(quick_savegame_summaries_buffer_size_result.value());
            }

            const boost::system::result<const boost::json::value &> remember_savegame_result = general_object.try_at("remember_savegame");
            if (remember_savegame_result.has_value()) {
                general.remember_savegame = boost::json::value_to<bool>(remember_savegame_result.value());
            }

            const boost::system::result<const boost::json::value &> remove_downgrades_less_than_percent_result = general_object.try_at("remove_downgrades_less_than_percent");
            if (remove_downgrades_less_than_percent_result.has_value()) {
                general.remove_downgrades_less_than_percent = boost::json::value_to<double>(remove_downgrades_less_than_percent_result.value());
            }

            const boost::system::result<const boost::json::value &> remove_impossible_downgrades_result = general_object.try_at("remove_impossible_downgrades");
            if (remove_impossible_downgrades_result.has_value()) {
                general.remove_impossible_downgrades = boost::json::value_to<bool>(remove_impossible_downgrades_result.value());
            }

            const boost::system::result<const boost::json::value &> roll_result = general_object.try_at("roll");
            if (roll_result.has_value()) {
                general.roll = boost::json::value_to<double>(roll_result.value());
            }

            const boost::system::result<const boost::json::value &> screen_result = general_object.try_at("screen");
            if (screen_result.has_value()) {
                general.screen = boost::json::value_to<int>(screen_result.value());
            }

            const boost::system::result<const boost::json::value &> simulation_atom_result = general_object.try_at("simulation_atom");
            if (simulation_atom_result.has_value()) {
                general.simulation_atom = boost::json::value_to<double>(simulation_atom_result.value());
            }

            const boost::system::result<const boost::json::value &> times_to_show_help_screen_result = general_object.try_at("times_to_show_help_screen");
            if (times_to_show_help_screen_result.has_value()) {
                general.times_to_show_help_screen = boost::json::value_to<int>(times_to_show_help_screen_result.value());
            }

            const boost::system::result<const boost::json::value &> trade_interface_tracks_prices_result = general_object.try_at("trade_interface_tracks_prices");
            if (trade_interface_tracks_prices_result.has_value()) {
                general.trade_interface_tracks_prices = boost::json::value_to<bool>(trade_interface_tracks_prices_result.value());
            }

            const boost::system::result<const boost::json::value &> trade_interface_tracks_prices_top_rank_result = general_object.try_at("trade_interface_tracks_prices_top_rank");
            if (trade_interface_tracks_prices_top_rank_result.has_value()) {
                general.trade_interface_tracks_prices_top_rank = boost::json::value_to<int>(trade_interface_tracks_prices_top_rank_result.value());
            }

            const boost::system::result<const boost::json::value &> verbose_output_result = general_object.try_at("verbose_output");
            if (verbose_output_result.has_value()) {
                general.verbose_output = boost::json::value_to<int>(verbose_output_result.value());
            }

            const boost::system::result<const boost::json::value &> wheel_increment_lines_result = general_object.try_at("wheel_increment_lines");
            if (wheel_increment_lines_result.has_value()) {
                general.wheel_increment_lines = boost::json::value_to<int>(wheel_increment_lines_result.value());
            }

            const boost::system::result<const boost::json::value &> while_loading_star_system_result = general_object.try_at("while_loading_star_system");
            if (while_loading_star_system_result.has_value()) {
                general.while_loading_star_system = boost::json::value_to<bool>(while_loading_star_system_result.value());
            }

            const boost::system::result<const boost::json::value &> write_savegame_on_exit_result = general_object.try_at("write_savegame_on_exit");
            if (write_savegame_on_exit_result.has_value()) {
                general.write_savegame_on_exit = boost::json::value_to<bool>(write_savegame_on_exit_result.value());
            }

            const boost::system::result<const boost::json::value &> yaw_result = general_object.try_at("yaw");
            if (yaw_result.has_value()) {
                general.yaw = boost::json::value_to<double>(yaw_result.value());
            }

        }


        const boost::system::result<const boost::json::value &> graphics_value = root_object.try_at("graphics");
        if (graphics_value.has_value()) {
            boost::json::object graphics_object = graphics_value.value().get_object();
            const boost::system::result<const boost::json::value &> resolution_x_result = graphics_object.try_at("resolution_x");
            if (resolution_x_result.has_value()) {
                graphics.resolution_x = boost::json::value_to<int>(resolution_x_result.value());
            }

            const boost::system::result<const boost::json::value &> resolution_y_result = graphics_object.try_at("resolution_y");
            if (resolution_y_result.has_value()) {
                graphics.resolution_y = boost::json::value_to<int>(resolution_y_result.value());
            }

            const boost::system::result<const boost::json::value &> screen_result = graphics_object.try_at("screen");
            if (screen_result.has_value()) {
                graphics.screen = boost::json::value_to<int>(screen_result.value());
            }

            const boost::system::result<const boost::json::value &> atmosphere_emissive_result = graphics_object.try_at("atmosphere_emissive");
            if (atmosphere_emissive_result.has_value()) {
                graphics.atmosphere_emissive = boost::json::value_to<double>(atmosphere_emissive_result.value());
            }

            const boost::system::result<const boost::json::value &> atmosphere_diffuse_result = graphics_object.try_at("atmosphere_diffuse");
            if (atmosphere_diffuse_result.has_value()) {
                graphics.atmosphere_diffuse = boost::json::value_to<double>(atmosphere_diffuse_result.value());
            }

            const boost::system::result<const boost::json::value &> automatic_landing_zone_warning_result = graphics_object.try_at("automatic_landing_zone_warning");
            if (automatic_landing_zone_warning_result.has_value()) {
                graphics.automatic_landing_zone_warning = boost::json::value_to<std::string>(automatic_landing_zone_warning_result.value());
            }

            const boost::system::result<const boost::json::value &> automatic_landing_zone_warning1_result = graphics_object.try_at("automatic_landing_zone_warning1");
            if (automatic_landing_zone_warning1_result.has_value()) {
                graphics.automatic_landing_zone_warning1 = boost::json::value_to<std::string>(automatic_landing_zone_warning1_result.value());
            }

            const boost::system::result<const boost::json::value &> automatic_landing_zone_warning2_result = graphics_object.try_at("automatic_landing_zone_warning2");
            if (automatic_landing_zone_warning2_result.has_value()) {
                graphics.automatic_landing_zone_warning2 = boost::json::value_to<std::string>(automatic_landing_zone_warning2_result.value());
            }

            const boost::system::result<const boost::json::value &> automatic_landing_zone_warning_text_result = graphics_object.try_at("automatic_landing_zone_warning_text");
            if (automatic_landing_zone_warning_text_result.has_value()) {
                graphics.automatic_landing_zone_warning_text = boost::json::value_to<std::string>(automatic_landing_zone_warning_text_result.value());
            }

            const boost::system::result<const boost::json::value &> city_light_strength_result = graphics_object.try_at("city_light_strength");
            if (city_light_strength_result.has_value()) {
                graphics.city_light_strength = boost::json::value_to<double>(city_light_strength_result.value());
            }

            const boost::system::result<const boost::json::value &> day_city_light_strength_result = graphics_object.try_at("day_city_light_strength");
            if (day_city_light_strength_result.has_value()) {
                graphics.day_city_light_strength = boost::json::value_to<double>(day_city_light_strength_result.value());
            }

            const boost::system::result<const boost::json::value &> default_engine_activation_result = graphics_object.try_at("default_engine_activation");
            if (default_engine_activation_result.has_value()) {
                graphics.default_engine_activation = boost::json::value_to<double>(default_engine_activation_result.value());
            }

            const boost::system::result<const boost::json::value &> draw_heading_marker_result = graphics_object.try_at("draw_heading_marker");
            if (draw_heading_marker_result.has_value()) {
                graphics.draw_heading_marker = boost::json::value_to<bool>(draw_heading_marker_result.value());
            }

            const boost::system::result<const boost::json::value &> draw_rendered_crosshairs_result = graphics_object.try_at("draw_rendered_crosshairs");
            if (draw_rendered_crosshairs_result.has_value()) {
                graphics.draw_rendered_crosshairs = boost::json::value_to<bool>(draw_rendered_crosshairs_result.value());
            }

            const boost::system::result<const boost::json::value &> draw_star_body_result = graphics_object.try_at("draw_star_body");
            if (draw_star_body_result.has_value()) {
                graphics.draw_star_body = boost::json::value_to<bool>(draw_star_body_result.value());
            }

            const boost::system::result<const boost::json::value &> draw_star_glow_result = graphics_object.try_at("draw_star_glow");
            if (draw_star_glow_result.has_value()) {
                graphics.draw_star_glow = boost::json::value_to<bool>(draw_star_glow_result.value());
            }

            const boost::system::result<const boost::json::value &> explosion_animation_result = graphics_object.try_at("explosion_animation");
            if (explosion_animation_result.has_value()) {
                graphics.explosion_animation = boost::json::value_to<std::string>(explosion_animation_result.value());
            }

            const boost::system::result<const boost::json::value &> fov_result = graphics_object.try_at("fov");
            if (fov_result.has_value()) {
                graphics.fov = boost::json::value_to<double>(fov_result.value());
            }

            const boost::system::result<const boost::json::value &> glow_ambient_star_light_result = graphics_object.try_at("glow_ambient_star_light");
            if (glow_ambient_star_light_result.has_value()) {
                graphics.glow_ambient_star_light = boost::json::value_to<bool>(glow_ambient_star_light_result.value());
            }

            const boost::system::result<const boost::json::value &> glow_diffuse_star_light_result = graphics_object.try_at("glow_diffuse_star_light");
            if (glow_diffuse_star_light_result.has_value()) {
                graphics.glow_diffuse_star_light = boost::json::value_to<bool>(glow_diffuse_star_light_result.value());
            }

            const boost::system::result<const boost::json::value &> in_system_jump_ani_second_ahead_result = graphics_object.try_at("in_system_jump_ani_second_ahead");
            if (in_system_jump_ani_second_ahead_result.has_value()) {
                graphics.in_system_jump_ani_second_ahead = boost::json::value_to<double>(in_system_jump_ani_second_ahead_result.value());
            }

            const boost::system::result<const boost::json::value &> in_system_jump_ani_second_ahead_end_result = graphics_object.try_at("in_system_jump_ani_second_ahead_end");
            if (in_system_jump_ani_second_ahead_end_result.has_value()) {
                graphics.in_system_jump_ani_second_ahead_end = boost::json::value_to<double>(in_system_jump_ani_second_ahead_end_result.value());
            }

            const boost::system::result<const boost::json::value &> in_system_jump_animation_result = graphics_object.try_at("in_system_jump_animation");
            if (in_system_jump_animation_result.has_value()) {
                graphics.in_system_jump_animation = boost::json::value_to<std::string>(in_system_jump_animation_result.value());
            }

            const boost::system::result<const boost::json::value &> missile_explosion_radius_mult_result = graphics_object.try_at("missile_explosion_radius_mult");
            if (missile_explosion_radius_mult_result.has_value()) {
                graphics.missile_explosion_radius_mult = boost::json::value_to<double>(missile_explosion_radius_mult_result.value());
            }

            const boost::system::result<const boost::json::value &> missile_sparkle_result = graphics_object.try_at("missile_sparkle");
            if (missile_sparkle_result.has_value()) {
                graphics.missile_sparkle = boost::json::value_to<bool>(missile_sparkle_result.value());
            }

            const boost::system::result<const boost::json::value &> num_times_to_draw_shine_result = graphics_object.try_at("num_times_to_draw_shine");
            if (num_times_to_draw_shine_result.has_value()) {
                graphics.num_times_to_draw_shine = boost::json::value_to<int>(num_times_to_draw_shine_result.value());
            }

            const boost::system::result<const boost::json::value &> planet_detail_stack_count_result = graphics_object.try_at("planet_detail_stack_count");
            if (planet_detail_stack_count_result.has_value()) {
                graphics.planet_detail_stack_count = boost::json::value_to<int>(planet_detail_stack_count_result.value());
            }

            const boost::system::result<const boost::json::value &> reduced_vdus_width_result = graphics_object.try_at("reduced_vdus_width");
            if (reduced_vdus_width_result.has_value()) {
                graphics.reduced_vdus_width = boost::json::value_to<double>(reduced_vdus_width_result.value());
            }

            const boost::system::result<const boost::json::value &> reduced_vdus_height_result = graphics_object.try_at("reduced_vdus_height");
            if (reduced_vdus_height_result.has_value()) {
                graphics.reduced_vdus_height = boost::json::value_to<double>(reduced_vdus_height_result.value());
            }

            const boost::system::result<const boost::json::value &> shield_detail_stack_count_result = graphics_object.try_at("shield_detail_stack_count");
            if (shield_detail_stack_count_result.has_value()) {
                graphics.shield_detail_stack_count = boost::json::value_to<int>(shield_detail_stack_count_result.value());
            }

            const boost::system::result<const boost::json::value &> shield_technique_result = graphics_object.try_at("shield_technique");
            if (shield_technique_result.has_value()) {
                graphics.shield_technique = boost::json::value_to<std::string>(shield_technique_result.value());
            }

            const boost::system::result<const boost::json::value &> shield_texture_result = graphics_object.try_at("shield_texture");
            if (shield_texture_result.has_value()) {
                graphics.shield_texture = boost::json::value_to<std::string>(shield_texture_result.value());
            }

            const boost::system::result<const boost::json::value &> star_body_radius_result = graphics_object.try_at("star_body_radius");
            if (star_body_radius_result.has_value()) {
                graphics.star_body_radius = boost::json::value_to<double>(star_body_radius_result.value());
            }

            const boost::system::result<const boost::json::value &> star_glow_radius_result = graphics_object.try_at("star_glow_radius");
            if (star_glow_radius_result.has_value()) {
                graphics.star_glow_radius = boost::json::value_to<double>(star_glow_radius_result.value());
            }

            const boost::system::result<const boost::json::value &> wormhole_unit_result = graphics_object.try_at("wormhole_unit");
            if (wormhole_unit_result.has_value()) {
                graphics.wormhole_unit = boost::json::value_to<std::string>(wormhole_unit_result.value());
            }

        const boost::system::result<const boost::json::value &> glow_flicker_value = graphics_object.try_at("glow_flicker");
        if (glow_flicker_value.has_value()) {
            boost::json::object glow_flicker_object = glow_flicker_value.value().get_object();
            const boost::system::result<const boost::json::value &> flicker_time_result = glow_flicker_object.try_at("flicker_time");
            if (flicker_time_result.has_value()) {
                graphics.glow_flicker.flicker_time = boost::json::value_to<double>(flicker_time_result.value());
            }

            const boost::system::result<const boost::json::value &> flicker_off_time_result = glow_flicker_object.try_at("flicker_off_time");
            if (flicker_off_time_result.has_value()) {
                graphics.glow_flicker.flicker_off_time = boost::json::value_to<double>(flicker_off_time_result.value());
            }

            const boost::system::result<const boost::json::value &> min_flicker_cycle_result = glow_flicker_object.try_at("min_flicker_cycle");
            if (min_flicker_cycle_result.has_value()) {
                graphics.glow_flicker.min_flicker_cycle = boost::json::value_to<double>(min_flicker_cycle_result.value());
            }

            const boost::system::result<const boost::json::value &> num_times_per_second_on_result = glow_flicker_object.try_at("num_times_per_second_on");
            if (num_times_per_second_on_result.has_value()) {
                graphics.glow_flicker.num_times_per_second_on = boost::json::value_to<double>(num_times_per_second_on_result.value());
            }

            const boost::system::result<const boost::json::value &> hull_for_total_dark_result = glow_flicker_object.try_at("hull_for_total_dark");
            if (hull_for_total_dark_result.has_value()) {
                graphics.glow_flicker.hull_for_total_dark = boost::json::value_to<double>(hull_for_total_dark_result.value());
            }

        }


        const boost::system::result<const boost::json::value &> hud_value = graphics_object.try_at("hud");
        if (hud_value.has_value()) {
            boost::json::object hud_object = hud_value.value().get_object();
            const boost::system::result<const boost::json::value &> already_near_message_result = hud_object.try_at("already_near_message");
            if (already_near_message_result.has_value()) {
                graphics.hud.already_near_message = boost::json::value_to<std::string>(already_near_message_result.value());
            }

            const boost::system::result<const boost::json::value &> armor_hull_size_result = hud_object.try_at("armor_hull_size");
            if (armor_hull_size_result.has_value()) {
                graphics.hud.armor_hull_size = boost::json::value_to<double>(armor_hull_size_result.value());
            }

            const boost::system::result<const boost::json::value &> asteroids_near_message_result = hud_object.try_at("asteroids_near_message");
            if (asteroids_near_message_result.has_value()) {
                graphics.hud.asteroids_near_message = boost::json::value_to<std::string>(asteroids_near_message_result.value());
            }

            const boost::system::result<const boost::json::value &> basename_colon_basename_result = hud_object.try_at("basename_colon_basename");
            if (basename_colon_basename_result.has_value()) {
                graphics.hud.basename_colon_basename = boost::json::value_to<bool>(basename_colon_basename_result.value());
            }

            const boost::system::result<const boost::json::value &> box_line_thickness_result = hud_object.try_at("box_line_thickness");
            if (box_line_thickness_result.has_value()) {
                graphics.hud.box_line_thickness = boost::json::value_to<double>(box_line_thickness_result.value());
            }

            const boost::system::result<const boost::json::value &> completed_objectives_last_result = hud_object.try_at("completed_objectives_last");
            if (completed_objectives_last_result.has_value()) {
                graphics.hud.completed_objectives_last = boost::json::value_to<bool>(completed_objectives_last_result.value());
            }

            const boost::system::result<const boost::json::value &> crosshairs_on_chase_cam_result = hud_object.try_at("crosshairs_on_chase_cam");
            if (crosshairs_on_chase_cam_result.has_value()) {
                graphics.hud.crosshairs_on_chase_cam = boost::json::value_to<bool>(crosshairs_on_chase_cam_result.value());
            }

            const boost::system::result<const boost::json::value &> crosshairs_on_padlock_result = hud_object.try_at("crosshairs_on_padlock");
            if (crosshairs_on_padlock_result.has_value()) {
                graphics.hud.crosshairs_on_padlock = boost::json::value_to<bool>(crosshairs_on_padlock_result.value());
            }

            const boost::system::result<const boost::json::value &> damage_report_heading_result = hud_object.try_at("damage_report_heading");
            if (damage_report_heading_result.has_value()) {
                graphics.hud.damage_report_heading = boost::json::value_to<std::string>(damage_report_heading_result.value());
            }

            const boost::system::result<const boost::json::value &> debug_position_result = hud_object.try_at("debug_position");
            if (debug_position_result.has_value()) {
                graphics.hud.debug_position = boost::json::value_to<bool>(debug_position_result.value());
            }

            const boost::system::result<const boost::json::value &> diamond_line_thickness_result = hud_object.try_at("diamond_line_thickness");
            if (diamond_line_thickness_result.has_value()) {
                graphics.hud.diamond_line_thickness = boost::json::value_to<double>(diamond_line_thickness_result.value());
            }

            const boost::system::result<const boost::json::value &> diamond_rotation_speed_result = hud_object.try_at("diamond_rotation_speed");
            if (diamond_rotation_speed_result.has_value()) {
                graphics.hud.diamond_rotation_speed = boost::json::value_to<double>(diamond_rotation_speed_result.value());
            }

            const boost::system::result<const boost::json::value &> diamond_size_result = hud_object.try_at("diamond_size");
            if (diamond_size_result.has_value()) {
                graphics.hud.diamond_size = boost::json::value_to<double>(diamond_size_result.value());
            }

            const boost::system::result<const boost::json::value &> display_relative_velocity_result = hud_object.try_at("display_relative_velocity");
            if (display_relative_velocity_result.has_value()) {
                graphics.hud.display_relative_velocity = boost::json::value_to<bool>(display_relative_velocity_result.value());
            }

            const boost::system::result<const boost::json::value &> display_warp_energy_if_no_jump_drive_result = hud_object.try_at("display_warp_energy_if_no_jump_drive");
            if (display_warp_energy_if_no_jump_drive_result.has_value()) {
                graphics.hud.display_warp_energy_if_no_jump_drive = boost::json::value_to<bool>(display_warp_energy_if_no_jump_drive_result.value());
            }

            const boost::system::result<const boost::json::value &> draw_all_target_boxes_result = hud_object.try_at("draw_all_target_boxes");
            if (draw_all_target_boxes_result.has_value()) {
                graphics.hud.draw_all_target_boxes = boost::json::value_to<bool>(draw_all_target_boxes_result.value());
            }

            const boost::system::result<const boost::json::value &> draw_always_itts_result = hud_object.try_at("draw_always_itts");
            if (draw_always_itts_result.has_value()) {
                graphics.hud.draw_always_itts = boost::json::value_to<bool>(draw_always_itts_result.value());
            }

            const boost::system::result<const boost::json::value &> draw_arrow_on_chase_cam_result = hud_object.try_at("draw_arrow_on_chase_cam");
            if (draw_arrow_on_chase_cam_result.has_value()) {
                graphics.hud.draw_arrow_on_chase_cam = boost::json::value_to<bool>(draw_arrow_on_chase_cam_result.value());
            }

            const boost::system::result<const boost::json::value &> draw_arrow_on_pan_cam_result = hud_object.try_at("draw_arrow_on_pan_cam");
            if (draw_arrow_on_pan_cam_result.has_value()) {
                graphics.hud.draw_arrow_on_pan_cam = boost::json::value_to<bool>(draw_arrow_on_pan_cam_result.value());
            }

            const boost::system::result<const boost::json::value &> draw_arrow_on_pan_target_result = hud_object.try_at("draw_arrow_on_pan_target");
            if (draw_arrow_on_pan_target_result.has_value()) {
                graphics.hud.draw_arrow_on_pan_target = boost::json::value_to<bool>(draw_arrow_on_pan_target_result.value());
            }

            const boost::system::result<const boost::json::value &> draw_arrow_to_target_result = hud_object.try_at("draw_arrow_to_target");
            if (draw_arrow_to_target_result.has_value()) {
                graphics.hud.draw_arrow_to_target = boost::json::value_to<bool>(draw_arrow_to_target_result.value());
            }

            const boost::system::result<const boost::json::value &> draw_blips_on_both_radar_result = hud_object.try_at("draw_blips_on_both_radar");
            if (draw_blips_on_both_radar_result.has_value()) {
                graphics.hud.draw_blips_on_both_radar = boost::json::value_to<bool>(draw_blips_on_both_radar_result.value());
            }

            const boost::system::result<const boost::json::value &> draw_rendered_crosshairs_result = hud_object.try_at("draw_rendered_crosshairs");
            if (draw_rendered_crosshairs_result.has_value()) {
                graphics.hud.draw_rendered_crosshairs = boost::json::value_to<bool>(draw_rendered_crosshairs_result.value());
            }

            const boost::system::result<const boost::json::value &> draw_damage_sprite_result = hud_object.try_at("draw_damage_sprite");
            if (draw_damage_sprite_result.has_value()) {
                graphics.hud.draw_damage_sprite = boost::json::value_to<bool>(draw_damage_sprite_result.value());
            }

            const boost::system::result<const boost::json::value &> draw_gravitational_objects_result = hud_object.try_at("draw_gravitational_objects");
            if (draw_gravitational_objects_result.has_value()) {
                graphics.hud.draw_gravitational_objects = boost::json::value_to<bool>(draw_gravitational_objects_result.value());
            }

            const boost::system::result<const boost::json::value &> draw_line_to_itts_result = hud_object.try_at("draw_line_to_itts");
            if (draw_line_to_itts_result.has_value()) {
                graphics.hud.draw_line_to_itts = boost::json::value_to<bool>(draw_line_to_itts_result.value());
            }

            const boost::system::result<const boost::json::value &> draw_line_to_target_result = hud_object.try_at("draw_line_to_target");
            if (draw_line_to_target_result.has_value()) {
                graphics.hud.draw_line_to_target = boost::json::value_to<bool>(draw_line_to_target_result.value());
            }

            const boost::system::result<const boost::json::value &> draw_line_to_targets_target_result = hud_object.try_at("draw_line_to_targets_target");
            if (draw_line_to_targets_target_result.has_value()) {
                graphics.hud.draw_line_to_targets_target = boost::json::value_to<bool>(draw_line_to_targets_target_result.value());
            }

            const boost::system::result<const boost::json::value &> draw_nav_grid_result = hud_object.try_at("draw_nav_grid");
            if (draw_nav_grid_result.has_value()) {
                graphics.hud.draw_nav_grid = boost::json::value_to<bool>(draw_nav_grid_result.value());
            }

            const boost::system::result<const boost::json::value &> draw_nav_symbol_result = hud_object.try_at("draw_nav_symbol");
            if (draw_nav_symbol_result.has_value()) {
                graphics.hud.draw_nav_symbol = boost::json::value_to<bool>(draw_nav_symbol_result.value());
            }

            const boost::system::result<const boost::json::value &> draw_tactical_target_result = hud_object.try_at("draw_tactical_target");
            if (draw_tactical_target_result.has_value()) {
                graphics.hud.draw_tactical_target = boost::json::value_to<bool>(draw_tactical_target_result.value());
            }

            const boost::system::result<const boost::json::value &> draw_targetting_boxes_result = hud_object.try_at("draw_targetting_boxes");
            if (draw_targetting_boxes_result.has_value()) {
                graphics.hud.draw_targetting_boxes = boost::json::value_to<bool>(draw_targetting_boxes_result.value());
            }

            const boost::system::result<const boost::json::value &> draw_targetting_boxes_inside_result = hud_object.try_at("draw_targetting_boxes_inside");
            if (draw_targetting_boxes_inside_result.has_value()) {
                graphics.hud.draw_targetting_boxes_inside = boost::json::value_to<bool>(draw_targetting_boxes_inside_result.value());
            }

            const boost::system::result<const boost::json::value &> draw_significant_blips_result = hud_object.try_at("draw_significant_blips");
            if (draw_significant_blips_result.has_value()) {
                graphics.hud.draw_significant_blips = boost::json::value_to<bool>(draw_significant_blips_result.value());
            }

            const boost::system::result<const boost::json::value &> draw_star_direction_result = hud_object.try_at("draw_star_direction");
            if (draw_star_direction_result.has_value()) {
                graphics.hud.draw_star_direction = boost::json::value_to<bool>(draw_star_direction_result.value());
            }

            const boost::system::result<const boost::json::value &> draw_unit_on_chase_cam_result = hud_object.try_at("draw_unit_on_chase_cam");
            if (draw_unit_on_chase_cam_result.has_value()) {
                graphics.hud.draw_unit_on_chase_cam = boost::json::value_to<bool>(draw_unit_on_chase_cam_result.value());
            }

            const boost::system::result<const boost::json::value &> draw_vdu_view_shields_result = hud_object.try_at("draw_vdu_view_shields");
            if (draw_vdu_view_shields_result.has_value()) {
                graphics.hud.draw_vdu_view_shields = boost::json::value_to<bool>(draw_vdu_view_shields_result.value());
            }

            const boost::system::result<const boost::json::value &> draw_weapon_sprite_result = hud_object.try_at("draw_weapon_sprite");
            if (draw_weapon_sprite_result.has_value()) {
                graphics.hud.draw_weapon_sprite = boost::json::value_to<bool>(draw_weapon_sprite_result.value());
            }

            const boost::system::result<const boost::json::value &> enemy_near_message_result = hud_object.try_at("enemy_near_message");
            if (enemy_near_message_result.has_value()) {
                graphics.hud.enemy_near_message = boost::json::value_to<std::string>(enemy_near_message_result.value());
            }

            const boost::system::result<const boost::json::value &> gun_list_columns_result = hud_object.try_at("gun_list_columns");
            if (gun_list_columns_result.has_value()) {
                graphics.hud.gun_list_columns = boost::json::value_to<int>(gun_list_columns_result.value());
            }

            const boost::system::result<const boost::json::value &> invert_friendly_shields_result = hud_object.try_at("invert_friendly_shields");
            if (invert_friendly_shields_result.has_value()) {
                graphics.hud.invert_friendly_shields = boost::json::value_to<bool>(invert_friendly_shields_result.value());
            }

            const boost::system::result<const boost::json::value &> invert_friendly_sprite_result = hud_object.try_at("invert_friendly_sprite");
            if (invert_friendly_sprite_result.has_value()) {
                graphics.hud.invert_friendly_sprite = boost::json::value_to<bool>(invert_friendly_sprite_result.value());
            }

            const boost::system::result<const boost::json::value &> invert_target_shields_result = hud_object.try_at("invert_target_shields");
            if (invert_target_shields_result.has_value()) {
                graphics.hud.invert_target_shields = boost::json::value_to<bool>(invert_target_shields_result.value());
            }

            const boost::system::result<const boost::json::value &> invert_target_sprite_result = hud_object.try_at("invert_target_sprite");
            if (invert_target_sprite_result.has_value()) {
                graphics.hud.invert_target_sprite = boost::json::value_to<bool>(invert_target_sprite_result.value());
            }

            const boost::system::result<const boost::json::value &> invert_view_shields_result = hud_object.try_at("invert_view_shields");
            if (invert_view_shields_result.has_value()) {
                graphics.hud.invert_view_shields = boost::json::value_to<bool>(invert_view_shields_result.value());
            }

            const boost::system::result<const boost::json::value &> itts_for_beams_result = hud_object.try_at("itts_for_beams");
            if (itts_for_beams_result.has_value()) {
                graphics.hud.itts_for_beams = boost::json::value_to<bool>(itts_for_beams_result.value());
            }

            const boost::system::result<const boost::json::value &> itts_for_lockable_result = hud_object.try_at("itts_for_lockable");
            if (itts_for_lockable_result.has_value()) {
                graphics.hud.itts_for_lockable = boost::json::value_to<bool>(itts_for_lockable_result.value());
            }

            const boost::system::result<const boost::json::value &> itts_line_to_mark_alpha_result = hud_object.try_at("itts_line_to_mark_alpha");
            if (itts_line_to_mark_alpha_result.has_value()) {
                graphics.hud.itts_line_to_mark_alpha = boost::json::value_to<double>(itts_line_to_mark_alpha_result.value());
            }

            const boost::system::result<const boost::json::value &> itts_use_average_gun_speed_result = hud_object.try_at("itts_use_average_gun_speed");
            if (itts_use_average_gun_speed_result.has_value()) {
                graphics.hud.itts_use_average_gun_speed = boost::json::value_to<bool>(itts_use_average_gun_speed_result.value());
            }

            const boost::system::result<const boost::json::value &> lock_center_crosshair_result = hud_object.try_at("lock_center_crosshair");
            if (lock_center_crosshair_result.has_value()) {
                graphics.hud.lock_center_crosshair = boost::json::value_to<bool>(lock_center_crosshair_result.value());
            }

            const boost::system::result<const boost::json::value &> lock_confirm_line_length_result = hud_object.try_at("lock_confirm_line_length");
            if (lock_confirm_line_length_result.has_value()) {
                graphics.hud.lock_confirm_line_length = boost::json::value_to<double>(lock_confirm_line_length_result.value());
            }

            const boost::system::result<const boost::json::value &> manifest_heading_result = hud_object.try_at("manifest_heading");
            if (manifest_heading_result.has_value()) {
                graphics.hud.manifest_heading = boost::json::value_to<std::string>(manifest_heading_result.value());
            }

            const boost::system::result<const boost::json::value &> max_missile_bracket_size_result = hud_object.try_at("max_missile_bracket_size");
            if (max_missile_bracket_size_result.has_value()) {
                graphics.hud.max_missile_bracket_size = boost::json::value_to<double>(max_missile_bracket_size_result.value());
            }

            const boost::system::result<const boost::json::value &> message_prefix_result = hud_object.try_at("message_prefix");
            if (message_prefix_result.has_value()) {
                graphics.hud.message_prefix = boost::json::value_to<std::string>(message_prefix_result.value());
            }

            const boost::system::result<const boost::json::value &> min_missile_bracket_size_result = hud_object.try_at("min_missile_bracket_size");
            if (min_missile_bracket_size_result.has_value()) {
                graphics.hud.min_missile_bracket_size = boost::json::value_to<double>(min_missile_bracket_size_result.value());
            }

            const boost::system::result<const boost::json::value &> min_lock_box_size_result = hud_object.try_at("min_lock_box_size");
            if (min_lock_box_size_result.has_value()) {
                graphics.hud.min_lock_box_size = boost::json::value_to<double>(min_lock_box_size_result.value());
            }

            const boost::system::result<const boost::json::value &> min_radar_blip_size_result = hud_object.try_at("min_radar_blip_size");
            if (min_radar_blip_size_result.has_value()) {
                graphics.hud.min_radar_blip_size = boost::json::value_to<double>(min_radar_blip_size_result.value());
            }

            const boost::system::result<const boost::json::value &> min_target_box_size_result = hud_object.try_at("min_target_box_size");
            if (min_target_box_size_result.has_value()) {
                graphics.hud.min_target_box_size = boost::json::value_to<double>(min_target_box_size_result.value());
            }

            const boost::system::result<const boost::json::value &> mounts_list_empty_result = hud_object.try_at("mounts_list_empty");
            if (mounts_list_empty_result.has_value()) {
                graphics.hud.mounts_list_empty = boost::json::value_to<std::string>(mounts_list_empty_result.value());
            }

            const boost::system::result<const boost::json::value &> nav_cross_line_thickness_result = hud_object.try_at("nav_cross_line_thickness");
            if (nav_cross_line_thickness_result.has_value()) {
                graphics.hud.nav_cross_line_thickness = boost::json::value_to<double>(nav_cross_line_thickness_result.value());
            }

            const boost::system::result<const boost::json::value &> not_included_in_damage_report_result = hud_object.try_at("not_included_in_damage_report");
            if (not_included_in_damage_report_result.has_value()) {
                graphics.hud.not_included_in_damage_report = boost::json::value_to<std::string>(not_included_in_damage_report_result.value());
            }

            const boost::system::result<const boost::json::value &> out_of_cone_distance_result = hud_object.try_at("out_of_cone_distance");
            if (out_of_cone_distance_result.has_value()) {
                graphics.hud.out_of_cone_distance = boost::json::value_to<bool>(out_of_cone_distance_result.value());
            }

            const boost::system::result<const boost::json::value &> padlock_view_lag_result = hud_object.try_at("padlock_view_lag");
            if (padlock_view_lag_result.has_value()) {
                graphics.hud.padlock_view_lag = boost::json::value_to<double>(padlock_view_lag_result.value());
            }

            const boost::system::result<const boost::json::value &> padlock_view_lag_fix_zone_result = hud_object.try_at("padlock_view_lag_fix_zone");
            if (padlock_view_lag_fix_zone_result.has_value()) {
                graphics.hud.padlock_view_lag_fix_zone = boost::json::value_to<double>(padlock_view_lag_fix_zone_result.value());
            }

            const boost::system::result<const boost::json::value &> planet_near_message_result = hud_object.try_at("planet_near_message");
            if (planet_near_message_result.has_value()) {
                graphics.hud.planet_near_message = boost::json::value_to<std::string>(planet_near_message_result.value());
            }

            const boost::system::result<const boost::json::value &> print_damage_percent_result = hud_object.try_at("print_damage_percent");
            if (print_damage_percent_result.has_value()) {
                graphics.hud.print_damage_percent = boost::json::value_to<bool>(print_damage_percent_result.value());
            }

            const boost::system::result<const boost::json::value &> print_faction_result = hud_object.try_at("print_faction");
            if (print_faction_result.has_value()) {
                graphics.hud.print_faction = boost::json::value_to<bool>(print_faction_result.value());
            }

            const boost::system::result<const boost::json::value &> print_fg_name_result = hud_object.try_at("print_fg_name");
            if (print_fg_name_result.has_value()) {
                graphics.hud.print_fg_name = boost::json::value_to<bool>(print_fg_name_result.value());
            }

            const boost::system::result<const boost::json::value &> print_fg_sub_id_result = hud_object.try_at("print_fg_sub_id");
            if (print_fg_sub_id_result.has_value()) {
                graphics.hud.print_fg_sub_id = boost::json::value_to<bool>(print_fg_sub_id_result.value());
            }

            const boost::system::result<const boost::json::value &> print_request_docking_result = hud_object.try_at("print_request_docking");
            if (print_request_docking_result.has_value()) {
                graphics.hud.print_request_docking = boost::json::value_to<bool>(print_request_docking_result.value());
            }

            const boost::system::result<const boost::json::value &> print_ship_type_result = hud_object.try_at("print_ship_type");
            if (print_ship_type_result.has_value()) {
                graphics.hud.print_ship_type = boost::json::value_to<bool>(print_ship_type_result.value());
            }

            const boost::system::result<const boost::json::value &> projectile_means_missile_result = hud_object.try_at("projectile_means_missile");
            if (projectile_means_missile_result.has_value()) {
                graphics.hud.projectile_means_missile = boost::json::value_to<bool>(projectile_means_missile_result.value());
            }

            const boost::system::result<const boost::json::value &> radar_type_result = hud_object.try_at("radar_type");
            if (radar_type_result.has_value()) {
                graphics.hud.radar_type = boost::json::value_to<std::string>(radar_type_result.value());
            }

            const boost::system::result<const boost::json::value &> radar_search_extra_radius_result = hud_object.try_at("radar_search_extra_radius");
            if (radar_search_extra_radius_result.has_value()) {
                graphics.hud.radar_search_extra_radius = boost::json::value_to<double>(radar_search_extra_radius_result.value());
            }

            const boost::system::result<const boost::json::value &> rotating_bracket_inner_result = hud_object.try_at("rotating_bracket_inner");
            if (rotating_bracket_inner_result.has_value()) {
                graphics.hud.rotating_bracket_inner = boost::json::value_to<bool>(rotating_bracket_inner_result.value());
            }

            const boost::system::result<const boost::json::value &> rotating_bracket_size_result = hud_object.try_at("rotating_bracket_size");
            if (rotating_bracket_size_result.has_value()) {
                graphics.hud.rotating_bracket_size = boost::json::value_to<double>(rotating_bracket_size_result.value());
            }

            const boost::system::result<const boost::json::value &> rotating_bracket_width_result = hud_object.try_at("rotating_bracket_width");
            if (rotating_bracket_width_result.has_value()) {
                graphics.hud.rotating_bracket_width = boost::json::value_to<double>(rotating_bracket_width_result.value());
            }

            const boost::system::result<const boost::json::value &> scale_relationship_color_result = hud_object.try_at("scale_relationship_color");
            if (scale_relationship_color_result.has_value()) {
                graphics.hud.scale_relationship_color = boost::json::value_to<double>(scale_relationship_color_result.value());
            }

            const boost::system::result<const boost::json::value &> shield_vdu_fade_result = hud_object.try_at("shield_vdu_fade");
            if (shield_vdu_fade_result.has_value()) {
                graphics.hud.shield_vdu_fade = boost::json::value_to<bool>(shield_vdu_fade_result.value());
            }

            const boost::system::result<const boost::json::value &> shield_vdu_thresh0_result = hud_object.try_at("shield_vdu_thresh0");
            if (shield_vdu_thresh0_result.has_value()) {
                graphics.hud.shield_vdu_thresh0 = boost::json::value_to<double>(shield_vdu_thresh0_result.value());
            }

            const boost::system::result<const boost::json::value &> shield_vdu_thresh1_result = hud_object.try_at("shield_vdu_thresh1");
            if (shield_vdu_thresh1_result.has_value()) {
                graphics.hud.shield_vdu_thresh1 = boost::json::value_to<double>(shield_vdu_thresh1_result.value());
            }

            const boost::system::result<const boost::json::value &> shield_vdu_thresh2_result = hud_object.try_at("shield_vdu_thresh2");
            if (shield_vdu_thresh2_result.has_value()) {
                graphics.hud.shield_vdu_thresh2 = boost::json::value_to<double>(shield_vdu_thresh2_result.value());
            }

            const boost::system::result<const boost::json::value &> show_negative_blips_as_positive_result = hud_object.try_at("show_negative_blips_as_positive");
            if (show_negative_blips_as_positive_result.has_value()) {
                graphics.hud.show_negative_blips_as_positive = boost::json::value_to<bool>(show_negative_blips_as_positive_result.value());
            }

            const boost::system::result<const boost::json::value &> simple_manifest_result = hud_object.try_at("simple_manifest");
            if (simple_manifest_result.has_value()) {
                graphics.hud.simple_manifest = boost::json::value_to<bool>(simple_manifest_result.value());
            }

            const boost::system::result<const boost::json::value &> starship_near_message_result = hud_object.try_at("starship_near_message");
            if (starship_near_message_result.has_value()) {
                graphics.hud.starship_near_message = boost::json::value_to<std::string>(starship_near_message_result.value());
            }

            const boost::system::result<const boost::json::value &> switch_to_target_mode_on_key_result = hud_object.try_at("switch_to_target_mode_on_key");
            if (switch_to_target_mode_on_key_result.has_value()) {
                graphics.hud.switch_to_target_mode_on_key = boost::json::value_to<bool>(switch_to_target_mode_on_key_result.value());
            }

            const boost::system::result<const boost::json::value &> switch_back_from_comms_result = hud_object.try_at("switch_back_from_comms");
            if (switch_back_from_comms_result.has_value()) {
                graphics.hud.switch_back_from_comms = boost::json::value_to<bool>(switch_back_from_comms_result.value());
            }

            const boost::system::result<const boost::json::value &> tac_target_foci_result = hud_object.try_at("tac_target_foci");
            if (tac_target_foci_result.has_value()) {
                graphics.hud.tac_target_foci = boost::json::value_to<double>(tac_target_foci_result.value());
            }

            const boost::system::result<const boost::json::value &> tac_target_length_result = hud_object.try_at("tac_target_length");
            if (tac_target_length_result.has_value()) {
                graphics.hud.tac_target_length = boost::json::value_to<double>(tac_target_length_result.value());
            }

            const boost::system::result<const boost::json::value &> tac_target_thickness_result = hud_object.try_at("tac_target_thickness");
            if (tac_target_thickness_result.has_value()) {
                graphics.hud.tac_target_thickness = boost::json::value_to<double>(tac_target_thickness_result.value());
            }

            const boost::system::result<const boost::json::value &> text_background_alpha_result = hud_object.try_at("text_background_alpha");
            if (text_background_alpha_result.has_value()) {
                graphics.hud.text_background_alpha = boost::json::value_to<double>(text_background_alpha_result.value());
            }

            const boost::system::result<const boost::json::value &> top_view_result = hud_object.try_at("top_view");
            if (top_view_result.has_value()) {
                graphics.hud.top_view = boost::json::value_to<bool>(top_view_result.value());
            }

            const boost::system::result<const boost::json::value &> untarget_beyond_cone_result = hud_object.try_at("untarget_beyond_cone");
            if (untarget_beyond_cone_result.has_value()) {
                graphics.hud.untarget_beyond_cone = boost::json::value_to<bool>(untarget_beyond_cone_result.value());
            }

        }


        }


        const boost::system::result<const boost::json::value &> components_value = root_object.try_at("components");
        if (components_value.has_value()) {
            boost::json::object components_object = components_value.value().get_object();
        const boost::system::result<const boost::json::value &> afterburner_value = components_object.try_at("afterburner");
        if (afterburner_value.has_value()) {
            boost::json::object afterburner_object = afterburner_value.value().get_object();
            const boost::system::result<const boost::json::value &> energy_source_result = afterburner_object.try_at("energy_source");
            if (energy_source_result.has_value()) {
                components.afterburner.energy_source = boost::json::value_to<std::string>(energy_source_result.value());
            }

        }


        const boost::system::result<const boost::json::value &> cloak_value = components_object.try_at("cloak");
        if (cloak_value.has_value()) {
            boost::json::object cloak_object = cloak_value.value().get_object();
            const boost::system::result<const boost::json::value &> energy_source_result = cloak_object.try_at("energy_source");
            if (energy_source_result.has_value()) {
                components.cloak.energy_source = boost::json::value_to<std::string>(energy_source_result.value());
            }

        }


        const boost::system::result<const boost::json::value &> computer_value = components_object.try_at("computer");
        if (computer_value.has_value()) {
            boost::json::object computer_object = computer_value.value().get_object();
            const boost::system::result<const boost::json::value &> default_lock_cone_result = computer_object.try_at("default_lock_cone");
            if (default_lock_cone_result.has_value()) {
                components.computer.default_lock_cone = boost::json::value_to<double>(default_lock_cone_result.value());
            }

            const boost::system::result<const boost::json::value &> default_max_range_result = computer_object.try_at("default_max_range");
            if (default_max_range_result.has_value()) {
                components.computer.default_max_range = boost::json::value_to<double>(default_max_range_result.value());
            }

            const boost::system::result<const boost::json::value &> default_tracking_cone_result = computer_object.try_at("default_tracking_cone");
            if (default_tracking_cone_result.has_value()) {
                components.computer.default_tracking_cone = boost::json::value_to<double>(default_tracking_cone_result.value());
            }

        }


        const boost::system::result<const boost::json::value &> drive_value = components_object.try_at("drive");
        if (drive_value.has_value()) {
            boost::json::object drive_object = drive_value.value().get_object();
            const boost::system::result<const boost::json::value &> energy_source_result = drive_object.try_at("energy_source");
            if (energy_source_result.has_value()) {
                components.drive.energy_source = boost::json::value_to<std::string>(energy_source_result.value());
            }

            const boost::system::result<const boost::json::value &> non_combat_mode_multiplier_result = drive_object.try_at("non_combat_mode_multiplier");
            if (non_combat_mode_multiplier_result.has_value()) {
                components.drive.non_combat_mode_multiplier = boost::json::value_to<int>(non_combat_mode_multiplier_result.value());
            }

            const boost::system::result<const boost::json::value &> minimum_drive_result = drive_object.try_at("minimum_drive");
            if (minimum_drive_result.has_value()) {
                components.drive.minimum_drive = boost::json::value_to<double>(minimum_drive_result.value());
            }

        }


        const boost::system::result<const boost::json::value &> energy_value = components_object.try_at("energy");
        if (energy_value.has_value()) {
            boost::json::object energy_object = energy_value.value().get_object();
            const boost::system::result<const boost::json::value &> factor_result = energy_object.try_at("factor");
            if (factor_result.has_value()) {
                components.energy.factor = boost::json::value_to<int>(factor_result.value());
            }

        }


        const boost::system::result<const boost::json::value &> fuel_value = components_object.try_at("fuel");
        if (fuel_value.has_value()) {
            boost::json::object fuel_object = fuel_value.value().get_object();
            const boost::system::result<const boost::json::value &> afterburner_fuel_usage_result = fuel_object.try_at("afterburner_fuel_usage");
            if (afterburner_fuel_usage_result.has_value()) {
                components.fuel.afterburner_fuel_usage = boost::json::value_to<double>(afterburner_fuel_usage_result.value());
            }

            const boost::system::result<const boost::json::value &> deuterium_relative_efficiency_lithium_result = fuel_object.try_at("deuterium_relative_efficiency_lithium");
            if (deuterium_relative_efficiency_lithium_result.has_value()) {
                components.fuel.deuterium_relative_efficiency_lithium = boost::json::value_to<double>(deuterium_relative_efficiency_lithium_result.value());
            }

            const boost::system::result<const boost::json::value &> ecm_energy_cost_result = fuel_object.try_at("ecm_energy_cost");
            if (ecm_energy_cost_result.has_value()) {
                components.fuel.ecm_energy_cost = boost::json::value_to<double>(ecm_energy_cost_result.value());
            }

            const boost::system::result<const boost::json::value &> fmec_factor_result = fuel_object.try_at("fmec_factor");
            if (fmec_factor_result.has_value()) {
                components.fuel.fmec_factor = boost::json::value_to<double>(fmec_factor_result.value());
            }

            const boost::system::result<const boost::json::value &> fuel_efficiency_result = fuel_object.try_at("fuel_efficiency");
            if (fuel_efficiency_result.has_value()) {
                components.fuel.fuel_efficiency = boost::json::value_to<double>(fuel_efficiency_result.value());
            }

            const boost::system::result<const boost::json::value &> fuel_equals_warp_result = fuel_object.try_at("fuel_equals_warp");
            if (fuel_equals_warp_result.has_value()) {
                components.fuel.fuel_equals_warp = boost::json::value_to<bool>(fuel_equals_warp_result.value());
            }

            const boost::system::result<const boost::json::value &> factor_result = fuel_object.try_at("factor");
            if (factor_result.has_value()) {
                components.fuel.factor = boost::json::value_to<int>(factor_result.value());
            }

            const boost::system::result<const boost::json::value &> megajoules_factor_result = fuel_object.try_at("megajoules_factor");
            if (megajoules_factor_result.has_value()) {
                components.fuel.megajoules_factor = boost::json::value_to<double>(megajoules_factor_result.value());
            }

            const boost::system::result<const boost::json::value &> min_reactor_efficiency_result = fuel_object.try_at("min_reactor_efficiency");
            if (min_reactor_efficiency_result.has_value()) {
                components.fuel.min_reactor_efficiency = boost::json::value_to<double>(min_reactor_efficiency_result.value());
            }

            const boost::system::result<const boost::json::value &> no_fuel_afterburn_result = fuel_object.try_at("no_fuel_afterburn");
            if (no_fuel_afterburn_result.has_value()) {
                components.fuel.no_fuel_afterburn = boost::json::value_to<double>(no_fuel_afterburn_result.value());
            }

            const boost::system::result<const boost::json::value &> no_fuel_thrust_result = fuel_object.try_at("no_fuel_thrust");
            if (no_fuel_thrust_result.has_value()) {
                components.fuel.no_fuel_thrust = boost::json::value_to<double>(no_fuel_thrust_result.value());
            }

            const boost::system::result<const boost::json::value &> normal_fuel_usage_result = fuel_object.try_at("normal_fuel_usage");
            if (normal_fuel_usage_result.has_value()) {
                components.fuel.normal_fuel_usage = boost::json::value_to<double>(normal_fuel_usage_result.value());
            }

            const boost::system::result<const boost::json::value &> reactor_idle_efficiency_result = fuel_object.try_at("reactor_idle_efficiency");
            if (reactor_idle_efficiency_result.has_value()) {
                components.fuel.reactor_idle_efficiency = boost::json::value_to<double>(reactor_idle_efficiency_result.value());
            }

            const boost::system::result<const boost::json::value &> reactor_uses_fuel_result = fuel_object.try_at("reactor_uses_fuel");
            if (reactor_uses_fuel_result.has_value()) {
                components.fuel.reactor_uses_fuel = boost::json::value_to<bool>(reactor_uses_fuel_result.value());
            }

            const boost::system::result<const boost::json::value &> variable_fuel_consumption_result = fuel_object.try_at("variable_fuel_consumption");
            if (variable_fuel_consumption_result.has_value()) {
                components.fuel.variable_fuel_consumption = boost::json::value_to<bool>(variable_fuel_consumption_result.value());
            }

            const boost::system::result<const boost::json::value &> vsd_mj_yield_result = fuel_object.try_at("vsd_mj_yield");
            if (vsd_mj_yield_result.has_value()) {
                components.fuel.vsd_mj_yield = boost::json::value_to<double>(vsd_mj_yield_result.value());
            }

        }


        const boost::system::result<const boost::json::value &> ftl_drive_value = components_object.try_at("ftl_drive");
        if (ftl_drive_value.has_value()) {
            boost::json::object ftl_drive_object = ftl_drive_value.value().get_object();
            const boost::system::result<const boost::json::value &> energy_source_result = ftl_drive_object.try_at("energy_source");
            if (energy_source_result.has_value()) {
                components.ftl_drive.energy_source = boost::json::value_to<std::string>(energy_source_result.value());
            }

            const boost::system::result<const boost::json::value &> factor_result = ftl_drive_object.try_at("factor");
            if (factor_result.has_value()) {
                components.ftl_drive.factor = boost::json::value_to<double>(factor_result.value());
            }

        }


        const boost::system::result<const boost::json::value &> ftl_energy_value = components_object.try_at("ftl_energy");
        if (ftl_energy_value.has_value()) {
            boost::json::object ftl_energy_object = ftl_energy_value.value().get_object();
            const boost::system::result<const boost::json::value &> factor_result = ftl_energy_object.try_at("factor");
            if (factor_result.has_value()) {
                components.ftl_energy.factor = boost::json::value_to<double>(factor_result.value());
            }

        }


        const boost::system::result<const boost::json::value &> jump_drive_value = components_object.try_at("jump_drive");
        if (jump_drive_value.has_value()) {
            boost::json::object jump_drive_object = jump_drive_value.value().get_object();
            const boost::system::result<const boost::json::value &> energy_source_result = jump_drive_object.try_at("energy_source");
            if (energy_source_result.has_value()) {
                components.jump_drive.energy_source = boost::json::value_to<std::string>(energy_source_result.value());
            }

            const boost::system::result<const boost::json::value &> factor_result = jump_drive_object.try_at("factor");
            if (factor_result.has_value()) {
                components.jump_drive.factor = boost::json::value_to<double>(factor_result.value());
            }

        }


        const boost::system::result<const boost::json::value &> reactor_value = components_object.try_at("reactor");
        if (reactor_value.has_value()) {
            boost::json::object reactor_object = reactor_value.value().get_object();
            const boost::system::result<const boost::json::value &> energy_source_result = reactor_object.try_at("energy_source");
            if (energy_source_result.has_value()) {
                components.reactor.energy_source = boost::json::value_to<std::string>(energy_source_result.value());
            }

            const boost::system::result<const boost::json::value &> factor_result = reactor_object.try_at("factor");
            if (factor_result.has_value()) {
                components.reactor.factor = boost::json::value_to<double>(factor_result.value());
            }

        }


        const boost::system::result<const boost::json::value &> shield_value = components_object.try_at("shield");
        if (shield_value.has_value()) {
            boost::json::object shield_object = shield_value.value().get_object();
            const boost::system::result<const boost::json::value &> energy_source_result = shield_object.try_at("energy_source");
            if (energy_source_result.has_value()) {
                components.shield.energy_source = boost::json::value_to<std::string>(energy_source_result.value());
            }

            const boost::system::result<const boost::json::value &> maintenance_factor_result = shield_object.try_at("maintenance_factor");
            if (maintenance_factor_result.has_value()) {
                components.shield.maintenance_factor = boost::json::value_to<double>(maintenance_factor_result.value());
            }

            const boost::system::result<const boost::json::value &> regeneration_factor_result = shield_object.try_at("regeneration_factor");
            if (regeneration_factor_result.has_value()) {
                components.shield.regeneration_factor = boost::json::value_to<double>(regeneration_factor_result.value());
            }

        }


        }


        const boost::system::result<const boost::json::value &> constants_value = root_object.try_at("constants");
        if (constants_value.has_value()) {
            boost::json::object constants_object = constants_value.value().get_object();
            const boost::system::result<const boost::json::value &> megajoules_multiplier_result = constants_object.try_at("megajoules_multiplier");
            if (megajoules_multiplier_result.has_value()) {
                constants.megajoules_multiplier = boost::json::value_to<int>(megajoules_multiplier_result.value());
            }

            const boost::system::result<const boost::json::value &> kilo_result = constants_object.try_at("kilo");
            if (kilo_result.has_value()) {
                constants.kilo = boost::json::value_to<int>(kilo_result.value());
            }

            const boost::system::result<const boost::json::value &> kj_per_unit_damage_result = constants_object.try_at("kj_per_unit_damage");
            if (kj_per_unit_damage_result.has_value()) {
                constants.kj_per_unit_damage = boost::json::value_to<int>(kj_per_unit_damage_result.value());
            }

        }


        const boost::system::result<const boost::json::value &> data_value = root_object.try_at("data");
        if (data_value.has_value()) {
            boost::json::object data_object = data_value.value().get_object();
            const boost::system::result<const boost::json::value &> master_part_list_result = data_object.try_at("master_part_list");
            if (master_part_list_result.has_value()) {
                data.master_part_list = boost::json::value_to<std::string>(master_part_list_result.value());
            }

            const boost::system::result<const boost::json::value &> using_templates_result = data_object.try_at("using_templates");
            if (using_templates_result.has_value()) {
                data.using_templates = boost::json::value_to<bool>(using_templates_result.value());
            }

        }


        const boost::system::result<const boost::json::value &> game_start_value = root_object.try_at("game_start");
        if (game_start_value.has_value()) {
            boost::json::object game_start_object = game_start_value.value().get_object();
            const boost::system::result<const boost::json::value &> default_mission_result = game_start_object.try_at("default_mission");
            if (default_mission_result.has_value()) {
                game_start.default_mission = boost::json::value_to<std::string>(default_mission_result.value());
            }

            const boost::system::result<const boost::json::value &> introduction_result = game_start_object.try_at("introduction");
            if (introduction_result.has_value()) {
                game_start.introduction = boost::json::value_to<std::string>(introduction_result.value());
            }

            const boost::system::result<const boost::json::value &> galaxy_result = game_start_object.try_at("galaxy");
            if (galaxy_result.has_value()) {
                game_start.galaxy = boost::json::value_to<std::string>(galaxy_result.value());
            }

        }


        const boost::system::result<const boost::json::value &> advanced_value = root_object.try_at("advanced");
        if (advanced_value.has_value()) {
            boost::json::object advanced_object = advanced_value.value().get_object();
        }


        const boost::system::result<const boost::json::value &> ai_value = root_object.try_at("ai");
        if (ai_value.has_value()) {
            boost::json::object ai_object = ai_value.value().get_object();
            const boost::system::result<const boost::json::value &> always_obedient_result = ai_object.try_at("always_obedient");
            if (always_obedient_result.has_value()) {
                ai.always_obedient = boost::json::value_to<bool>(always_obedient_result.value());
            }

            const boost::system::result<const boost::json::value &> assist_friend_in_need_result = ai_object.try_at("assist_friend_in_need");
            if (assist_friend_in_need_result.has_value()) {
                ai.assist_friend_in_need = boost::json::value_to<bool>(assist_friend_in_need_result.value());
            }

            const boost::system::result<const boost::json::value &> ease_to_anger_result = ai_object.try_at("ease_to_anger");
            if (ease_to_anger_result.has_value()) {
                ai.ease_to_anger = boost::json::value_to<double>(ease_to_anger_result.value());
            }

            const boost::system::result<const boost::json::value &> ease_to_appease_result = ai_object.try_at("ease_to_appease");
            if (ease_to_appease_result.has_value()) {
                ai.ease_to_appease = boost::json::value_to<double>(ease_to_appease_result.value());
            }

            const boost::system::result<const boost::json::value &> friend_factor_result = ai_object.try_at("friend_factor");
            if (friend_factor_result.has_value()) {
                ai.friend_factor = boost::json::value_to<double>(friend_factor_result.value());
            }

            const boost::system::result<const boost::json::value &> hull_damage_anger_result = ai_object.try_at("hull_damage_anger");
            if (hull_damage_anger_result.has_value()) {
                ai.hull_damage_anger = boost::json::value_to<int>(hull_damage_anger_result.value());
            }

            const boost::system::result<const boost::json::value &> hull_percent_for_comm_result = ai_object.try_at("hull_percent_for_comm");
            if (hull_percent_for_comm_result.has_value()) {
                ai.hull_percent_for_comm = boost::json::value_to<double>(hull_percent_for_comm_result.value());
            }

            const boost::system::result<const boost::json::value &> kill_factor_result = ai_object.try_at("kill_factor");
            if (kill_factor_result.has_value()) {
                ai.kill_factor = boost::json::value_to<double>(kill_factor_result.value());
            }

            const boost::system::result<const boost::json::value &> lowest_negative_comm_choice_result = ai_object.try_at("lowest_negative_comm_choice");
            if (lowest_negative_comm_choice_result.has_value()) {
                ai.lowest_negative_comm_choice = boost::json::value_to<double>(lowest_negative_comm_choice_result.value());
            }

            const boost::system::result<const boost::json::value &> lowest_positive_comm_choice_result = ai_object.try_at("lowest_positive_comm_choice");
            if (lowest_positive_comm_choice_result.has_value()) {
                ai.lowest_positive_comm_choice = boost::json::value_to<double>(lowest_positive_comm_choice_result.value());
            }

            const boost::system::result<const boost::json::value &> min_relationship_result = ai_object.try_at("min_relationship");
            if (min_relationship_result.has_value()) {
                ai.min_relationship = boost::json::value_to<double>(min_relationship_result.value());
            }

            const boost::system::result<const boost::json::value &> mood_swing_level_result = ai_object.try_at("mood_swing_level");
            if (mood_swing_level_result.has_value()) {
                ai.mood_swing_level = boost::json::value_to<double>(mood_swing_level_result.value());
            }

            const boost::system::result<const boost::json::value &> random_response_range_result = ai_object.try_at("random_response_range");
            if (random_response_range_result.has_value()) {
                ai.random_response_range = boost::json::value_to<double>(random_response_range_result.value());
            }

            const boost::system::result<const boost::json::value &> shield_damage_anger_result = ai_object.try_at("shield_damage_anger");
            if (shield_damage_anger_result.has_value()) {
                ai.shield_damage_anger = boost::json::value_to<int>(shield_damage_anger_result.value());
            }

            const boost::system::result<const boost::json::value &> jump_without_energy_result = ai_object.try_at("jump_without_energy");
            if (jump_without_energy_result.has_value()) {
                ai.jump_without_energy = boost::json::value_to<bool>(jump_without_energy_result.value());
            }

        const boost::system::result<const boost::json::value &> firing_value = ai_object.try_at("firing");
        if (firing_value.has_value()) {
            boost::json::object firing_object = firing_value.value().get_object();
            const boost::system::result<const boost::json::value &> missile_probability_result = firing_object.try_at("missile_probability");
            if (missile_probability_result.has_value()) {
                ai.firing.missile_probability = boost::json::value_to<double>(missile_probability_result.value());
            }

            const boost::system::result<const boost::json::value &> aggressivity_result = firing_object.try_at("aggressivity");
            if (aggressivity_result.has_value()) {
                ai.firing.aggressivity = boost::json::value_to<double>(aggressivity_result.value());
            }

        }


        const boost::system::result<const boost::json::value &> targeting_value = ai_object.try_at("targeting");
        if (targeting_value.has_value()) {
            boost::json::object targeting_object = targeting_value.value().get_object();
            const boost::system::result<const boost::json::value &> escort_distance_result = targeting_object.try_at("escort_distance");
            if (escort_distance_result.has_value()) {
                ai.targeting.escort_distance = boost::json::value_to<double>(escort_distance_result.value());
            }

            const boost::system::result<const boost::json::value &> turn_leader_distance_result = targeting_object.try_at("turn_leader_distance");
            if (turn_leader_distance_result.has_value()) {
                ai.targeting.turn_leader_distance = boost::json::value_to<double>(turn_leader_distance_result.value());
            }

            const boost::system::result<const boost::json::value &> time_to_recommand_wing_result = targeting_object.try_at("time_to_recommand_wing");
            if (time_to_recommand_wing_result.has_value()) {
                ai.targeting.time_to_recommand_wing = boost::json::value_to<double>(time_to_recommand_wing_result.value());
            }

            const boost::system::result<const boost::json::value &> min_time_to_switch_targets_result = targeting_object.try_at("min_time_to_switch_targets");
            if (min_time_to_switch_targets_result.has_value()) {
                ai.targeting.min_time_to_switch_targets = boost::json::value_to<double>(min_time_to_switch_targets_result.value());
            }

        }


        }


        const boost::system::result<const boost::json::value &> audio_value = root_object.try_at("audio");
        if (audio_value.has_value()) {
            boost::json::object audio_object = audio_value.value().get_object();
            const boost::system::result<const boost::json::value &> afterburner_gain_result = audio_object.try_at("afterburner_gain");
            if (afterburner_gain_result.has_value()) {
                audio.afterburner_gain = boost::json::value_to<double>(afterburner_gain_result.value());
            }

            const boost::system::result<const boost::json::value &> ai_high_quality_weapon_result = audio_object.try_at("ai_high_quality_weapon");
            if (ai_high_quality_weapon_result.has_value()) {
                audio.ai_high_quality_weapon = boost::json::value_to<bool>(ai_high_quality_weapon_result.value());
            }

            const boost::system::result<const boost::json::value &> ai_sound_result = audio_object.try_at("ai_sound");
            if (ai_sound_result.has_value()) {
                audio.ai_sound = boost::json::value_to<bool>(ai_sound_result.value());
            }

            const boost::system::result<const boost::json::value &> audio_max_distance_result = audio_object.try_at("audio_max_distance");
            if (audio_max_distance_result.has_value()) {
                audio.audio_max_distance = boost::json::value_to<double>(audio_max_distance_result.value());
            }

            const boost::system::result<const boost::json::value &> audio_ref_distance_result = audio_object.try_at("audio_ref_distance");
            if (audio_ref_distance_result.has_value()) {
                audio.audio_ref_distance = boost::json::value_to<double>(audio_ref_distance_result.value());
            }

            const boost::system::result<const boost::json::value &> automatic_docking_zone_result = audio_object.try_at("automatic_docking_zone");
            if (automatic_docking_zone_result.has_value()) {
                audio.automatic_docking_zone = boost::json::value_to<std::string>(automatic_docking_zone_result.value());
            }

            const boost::system::result<const boost::json::value &> battle_playlist_result = audio_object.try_at("battle_playlist");
            if (battle_playlist_result.has_value()) {
                audio.battle_playlist = boost::json::value_to<std::string>(battle_playlist_result.value());
            }

            const boost::system::result<const boost::json::value &> buzzing_distance_result = audio_object.try_at("buzzing_distance");
            if (buzzing_distance_result.has_value()) {
                audio.buzzing_distance = boost::json::value_to<double>(buzzing_distance_result.value());
            }

            const boost::system::result<const boost::json::value &> buzzing_needs_afterburner_result = audio_object.try_at("buzzing_needs_afterburner");
            if (buzzing_needs_afterburner_result.has_value()) {
                audio.buzzing_needs_afterburner = boost::json::value_to<bool>(buzzing_needs_afterburner_result.value());
            }

            const boost::system::result<const boost::json::value &> buzzing_time_result = audio_object.try_at("buzzing_time");
            if (buzzing_time_result.has_value()) {
                audio.buzzing_time = boost::json::value_to<double>(buzzing_time_result.value());
            }

            const boost::system::result<const boost::json::value &> cache_songs_result = audio_object.try_at("cache_songs");
            if (cache_songs_result.has_value()) {
                audio.cache_songs = boost::json::value_to<std::string>(cache_songs_result.value());
            }

            const boost::system::result<const boost::json::value &> cross_fade_music_result = audio_object.try_at("cross_fade_music");
            if (cross_fade_music_result.has_value()) {
                audio.cross_fade_music = boost::json::value_to<bool>(cross_fade_music_result.value());
            }

            const boost::system::result<const boost::json::value &> dj_script_result = audio_object.try_at("dj_script");
            if (dj_script_result.has_value()) {
                audio.dj_script = boost::json::value_to<std::string>(dj_script_result.value());
            }

            const boost::system::result<const boost::json::value &> doppler_result = audio_object.try_at("doppler");
            if (doppler_result.has_value()) {
                audio.doppler = boost::json::value_to<bool>(doppler_result.value());
            }

            const boost::system::result<const boost::json::value &> doppler_scale_result = audio_object.try_at("doppler_scale");
            if (doppler_scale_result.has_value()) {
                audio.doppler_scale = boost::json::value_to<double>(doppler_scale_result.value());
            }

            const boost::system::result<const boost::json::value &> every_other_mount_result = audio_object.try_at("every_other_mount");
            if (every_other_mount_result.has_value()) {
                audio.every_other_mount = boost::json::value_to<bool>(every_other_mount_result.value());
            }

            const boost::system::result<const boost::json::value &> explosion_closeness_result = audio_object.try_at("explosion_closeness");
            if (explosion_closeness_result.has_value()) {
                audio.explosion_closeness = boost::json::value_to<double>(explosion_closeness_result.value());
            }

            const boost::system::result<const boost::json::value &> exterior_weapon_gain_result = audio_object.try_at("exterior_weapon_gain");
            if (exterior_weapon_gain_result.has_value()) {
                audio.exterior_weapon_gain = boost::json::value_to<double>(exterior_weapon_gain_result.value());
            }

            const boost::system::result<const boost::json::value &> frequency_result = audio_object.try_at("frequency");
            if (frequency_result.has_value()) {
                audio.frequency = boost::json::value_to<int>(frequency_result.value());
            }

            const boost::system::result<const boost::json::value &> high_quality_weapon_result = audio_object.try_at("high_quality_weapon");
            if (high_quality_weapon_result.has_value()) {
                audio.high_quality_weapon = boost::json::value_to<bool>(high_quality_weapon_result.value());
            }

            const boost::system::result<const boost::json::value &> loading_sound_result = audio_object.try_at("loading_sound");
            if (loading_sound_result.has_value()) {
                audio.loading_sound = boost::json::value_to<std::string>(loading_sound_result.value());
            }

            const boost::system::result<const boost::json::value &> loss_playlist_result = audio_object.try_at("loss_playlist");
            if (loss_playlist_result.has_value()) {
                audio.loss_playlist = boost::json::value_to<std::string>(loss_playlist_result.value());
            }

            const boost::system::result<const boost::json::value &> loss_relationship_result = audio_object.try_at("loss_relationship");
            if (loss_relationship_result.has_value()) {
                audio.loss_relationship = boost::json::value_to<double>(loss_relationship_result.value());
            }

            const boost::system::result<const boost::json::value &> max_single_sounds_result = audio_object.try_at("max_single_sounds");
            if (max_single_sounds_result.has_value()) {
                audio.max_single_sounds = boost::json::value_to<int>(max_single_sounds_result.value());
            }

            const boost::system::result<const boost::json::value &> max_total_sounds_result = audio_object.try_at("max_total_sounds");
            if (max_total_sounds_result.has_value()) {
                audio.max_total_sounds = boost::json::value_to<int>(max_total_sounds_result.value());
            }

            const boost::system::result<const boost::json::value &> max_range_to_hear_weapon_fire_result = audio_object.try_at("max_range_to_hear_weapon_fire");
            if (max_range_to_hear_weapon_fire_result.has_value()) {
                audio.max_range_to_hear_weapon_fire = boost::json::value_to<double>(max_range_to_hear_weapon_fire_result.value());
            }

            const boost::system::result<const boost::json::value &> min_weapon_sound_refire_result = audio_object.try_at("min_weapon_sound_refire");
            if (min_weapon_sound_refire_result.has_value()) {
                audio.min_weapon_sound_refire = boost::json::value_to<double>(min_weapon_sound_refire_result.value());
            }

            const boost::system::result<const boost::json::value &> mission_victory_song_result = audio_object.try_at("mission_victory_song");
            if (mission_victory_song_result.has_value()) {
                audio.mission_victory_song = boost::json::value_to<std::string>(mission_victory_song_result.value());
            }

            const boost::system::result<const boost::json::value &> music_result = audio_object.try_at("music");
            if (music_result.has_value()) {
                audio.music = boost::json::value_to<bool>(music_result.value());
            }

            const boost::system::result<const boost::json::value &> music_layers_result = audio_object.try_at("music_layers");
            if (music_layers_result.has_value()) {
                audio.music_layers = boost::json::value_to<int>(music_layers_result.value());
            }

            const boost::system::result<const boost::json::value &> music_muting_fade_in_result = audio_object.try_at("music_muting_fade_in");
            if (music_muting_fade_in_result.has_value()) {
                audio.music_muting_fade_in = boost::json::value_to<double>(music_muting_fade_in_result.value());
            }

            const boost::system::result<const boost::json::value &> music_muting_fade_out_result = audio_object.try_at("music_muting_fade_out");
            if (music_muting_fade_out_result.has_value()) {
                audio.music_muting_fade_out = boost::json::value_to<double>(music_muting_fade_out_result.value());
            }

            const boost::system::result<const boost::json::value &> music_volume_result = audio_object.try_at("music_volume");
            if (music_volume_result.has_value()) {
                audio.music_volume = boost::json::value_to<double>(music_volume_result.value());
            }

            const boost::system::result<const boost::json::value &> music_volume_down_latency_result = audio_object.try_at("music_volume_down_latency");
            if (music_volume_down_latency_result.has_value()) {
                audio.music_volume_down_latency = boost::json::value_to<double>(music_volume_down_latency_result.value());
            }

            const boost::system::result<const boost::json::value &> music_volume_up_latency_result = audio_object.try_at("music_volume_up_latency");
            if (music_volume_up_latency_result.has_value()) {
                audio.music_volume_up_latency = boost::json::value_to<double>(music_volume_up_latency_result.value());
            }

            const boost::system::result<const boost::json::value &> news_song_result = audio_object.try_at("news_song");
            if (news_song_result.has_value()) {
                audio.news_song = boost::json::value_to<std::string>(news_song_result.value());
            }

            const boost::system::result<const boost::json::value &> panic_playlist_result = audio_object.try_at("panic_playlist");
            if (panic_playlist_result.has_value()) {
                audio.panic_playlist = boost::json::value_to<std::string>(panic_playlist_result.value());
            }

            const boost::system::result<const boost::json::value &> peace_playlist_result = audio_object.try_at("peace_playlist");
            if (peace_playlist_result.has_value()) {
                audio.peace_playlist = boost::json::value_to<std::string>(peace_playlist_result.value());
            }

            const boost::system::result<const boost::json::value &> positional_result = audio_object.try_at("positional");
            if (positional_result.has_value()) {
                audio.positional = boost::json::value_to<bool>(positional_result.value());
            }

            const boost::system::result<const boost::json::value &> shuffle_songs_result = audio_object.try_at("shuffle_songs");
            if (shuffle_songs_result.has_value()) {
                audio.shuffle_songs = boost::json::value_to<bool>(shuffle_songs_result.value());
            }

        const boost::system::result<const boost::json::value &> shuffle_songs_section_value = audio_object.try_at("shuffle_songs_section");
        if (shuffle_songs_section_value.has_value()) {
            boost::json::object shuffle_songs_section_object = shuffle_songs_section_value.value().get_object();
            const boost::system::result<const boost::json::value &> clear_history_on_list_change_result = shuffle_songs_section_object.try_at("clear_history_on_list_change");
            if (clear_history_on_list_change_result.has_value()) {
                audio.shuffle_songs_section.clear_history_on_list_change = boost::json::value_to<bool>(clear_history_on_list_change_result.value());
            }

            const boost::system::result<const boost::json::value &> history_depth_result = shuffle_songs_section_object.try_at("history_depth");
            if (history_depth_result.has_value()) {
                audio.shuffle_songs_section.history_depth = boost::json::value_to<int>(history_depth_result.value());
            }

        }


            const boost::system::result<const boost::json::value &> sound_result = audio_object.try_at("sound");
            if (sound_result.has_value()) {
                audio.sound = boost::json::value_to<bool>(sound_result.value());
            }

            const boost::system::result<const boost::json::value &> sound_gain_result = audio_object.try_at("sound_gain");
            if (sound_gain_result.has_value()) {
                audio.sound_gain = boost::json::value_to<double>(sound_gain_result.value());
            }

            const boost::system::result<const boost::json::value &> thread_time_result = audio_object.try_at("thread_time");
            if (thread_time_result.has_value()) {
                audio.thread_time = boost::json::value_to<int>(thread_time_result.value());
            }

            const boost::system::result<const boost::json::value &> time_between_music_result = audio_object.try_at("time_between_music");
            if (time_between_music_result.has_value()) {
                audio.time_between_music = boost::json::value_to<double>(time_between_music_result.value());
            }

            const boost::system::result<const boost::json::value &> victory_playlist_result = audio_object.try_at("victory_playlist");
            if (victory_playlist_result.has_value()) {
                audio.victory_playlist = boost::json::value_to<std::string>(victory_playlist_result.value());
            }

            const boost::system::result<const boost::json::value &> victory_relationship_result = audio_object.try_at("victory_relationship");
            if (victory_relationship_result.has_value()) {
                audio.victory_relationship = boost::json::value_to<double>(victory_relationship_result.value());
            }

            const boost::system::result<const boost::json::value &> volume_result = audio_object.try_at("volume");
            if (volume_result.has_value()) {
                audio.volume = boost::json::value_to<double>(volume_result.value());
            }

            const boost::system::result<const boost::json::value &> weapon_gain_result = audio_object.try_at("weapon_gain");
            if (weapon_gain_result.has_value()) {
                audio.weapon_gain = boost::json::value_to<double>(weapon_gain_result.value());
            }

        }


        const boost::system::result<const boost::json::value &> cockpit_audio_value = root_object.try_at("cockpit_audio");
        if (cockpit_audio_value.has_value()) {
            boost::json::object cockpit_audio_object = cockpit_audio_value.value().get_object();
            const boost::system::result<const boost::json::value &> automatic_landing_zone_result = cockpit_audio_object.try_at("automatic_landing_zone");
            if (automatic_landing_zone_result.has_value()) {
                cockpit_audio.automatic_landing_zone = boost::json::value_to<std::string>(automatic_landing_zone_result.value());
            }

            const boost::system::result<const boost::json::value &> automatic_landing_zone1_result = cockpit_audio_object.try_at("automatic_landing_zone1");
            if (automatic_landing_zone1_result.has_value()) {
                cockpit_audio.automatic_landing_zone1 = boost::json::value_to<std::string>(automatic_landing_zone1_result.value());
            }

            const boost::system::result<const boost::json::value &> automatic_landing_zone2_result = cockpit_audio_object.try_at("automatic_landing_zone2");
            if (automatic_landing_zone2_result.has_value()) {
                cockpit_audio.automatic_landing_zone2 = boost::json::value_to<std::string>(automatic_landing_zone2_result.value());
            }

            const boost::system::result<const boost::json::value &> autopilot_available_result = cockpit_audio_object.try_at("autopilot_available");
            if (autopilot_available_result.has_value()) {
                cockpit_audio.autopilot_available = boost::json::value_to<std::string>(autopilot_available_result.value());
            }

            const boost::system::result<const boost::json::value &> autopilot_disabled_result = cockpit_audio_object.try_at("autopilot_disabled");
            if (autopilot_disabled_result.has_value()) {
                cockpit_audio.autopilot_disabled = boost::json::value_to<std::string>(autopilot_disabled_result.value());
            }

            const boost::system::result<const boost::json::value &> autopilot_enabled_result = cockpit_audio_object.try_at("autopilot_enabled");
            if (autopilot_enabled_result.has_value()) {
                cockpit_audio.autopilot_enabled = boost::json::value_to<std::string>(autopilot_enabled_result.value());
            }

            const boost::system::result<const boost::json::value &> autopilot_unavailable_result = cockpit_audio_object.try_at("autopilot_unavailable");
            if (autopilot_unavailable_result.has_value()) {
                cockpit_audio.autopilot_unavailable = boost::json::value_to<std::string>(autopilot_unavailable_result.value());
            }

            const boost::system::result<const boost::json::value &> comm_result = cockpit_audio_object.try_at("comm");
            if (comm_result.has_value()) {
                cockpit_audio.comm = boost::json::value_to<std::string>(comm_result.value());
            }

            const boost::system::result<const boost::json::value &> comm_preload_result = cockpit_audio_object.try_at("comm_preload");
            if (comm_preload_result.has_value()) {
                cockpit_audio.comm_preload = boost::json::value_to<bool>(comm_preload_result.value());
            }

            const boost::system::result<const boost::json::value &> compress_change_result = cockpit_audio_object.try_at("compress_change");
            if (compress_change_result.has_value()) {
                cockpit_audio.compress_change = boost::json::value_to<std::string>(compress_change_result.value());
            }

            const boost::system::result<const boost::json::value &> compress_interval_result = cockpit_audio_object.try_at("compress_interval");
            if (compress_interval_result.has_value()) {
                cockpit_audio.compress_interval = boost::json::value_to<int>(compress_interval_result.value());
            }

            const boost::system::result<const boost::json::value &> compress_loop_result = cockpit_audio_object.try_at("compress_loop");
            if (compress_loop_result.has_value()) {
                cockpit_audio.compress_loop = boost::json::value_to<std::string>(compress_loop_result.value());
            }

            const boost::system::result<const boost::json::value &> compress_max_result = cockpit_audio_object.try_at("compress_max");
            if (compress_max_result.has_value()) {
                cockpit_audio.compress_max = boost::json::value_to<int>(compress_max_result.value());
            }

            const boost::system::result<const boost::json::value &> compress_stop_result = cockpit_audio_object.try_at("compress_stop");
            if (compress_stop_result.has_value()) {
                cockpit_audio.compress_stop = boost::json::value_to<std::string>(compress_stop_result.value());
            }

            const boost::system::result<const boost::json::value &> docking_complete_result = cockpit_audio_object.try_at("docking_complete");
            if (docking_complete_result.has_value()) {
                cockpit_audio.docking_complete = boost::json::value_to<std::string>(docking_complete_result.value());
            }

            const boost::system::result<const boost::json::value &> docking_denied_result = cockpit_audio_object.try_at("docking_denied");
            if (docking_denied_result.has_value()) {
                cockpit_audio.docking_denied = boost::json::value_to<std::string>(docking_denied_result.value());
            }

            const boost::system::result<const boost::json::value &> docking_failed_result = cockpit_audio_object.try_at("docking_failed");
            if (docking_failed_result.has_value()) {
                cockpit_audio.docking_failed = boost::json::value_to<std::string>(docking_failed_result.value());
            }

            const boost::system::result<const boost::json::value &> docking_granted_result = cockpit_audio_object.try_at("docking_granted");
            if (docking_granted_result.has_value()) {
                cockpit_audio.docking_granted = boost::json::value_to<std::string>(docking_granted_result.value());
            }

            const boost::system::result<const boost::json::value &> examine_result = cockpit_audio_object.try_at("examine");
            if (examine_result.has_value()) {
                cockpit_audio.examine = boost::json::value_to<std::string>(examine_result.value());
            }

            const boost::system::result<const boost::json::value &> jump_engaged_result = cockpit_audio_object.try_at("jump_engaged");
            if (jump_engaged_result.has_value()) {
                cockpit_audio.jump_engaged = boost::json::value_to<std::string>(jump_engaged_result.value());
            }

            const boost::system::result<const boost::json::value &> manifest_result = cockpit_audio_object.try_at("manifest");
            if (manifest_result.has_value()) {
                cockpit_audio.manifest = boost::json::value_to<std::string>(manifest_result.value());
            }

            const boost::system::result<const boost::json::value &> missile_switch_result = cockpit_audio_object.try_at("missile_switch");
            if (missile_switch_result.has_value()) {
                cockpit_audio.missile_switch = boost::json::value_to<std::string>(missile_switch_result.value());
            }

            const boost::system::result<const boost::json::value &> objective_result = cockpit_audio_object.try_at("objective");
            if (objective_result.has_value()) {
                cockpit_audio.objective = boost::json::value_to<std::string>(objective_result.value());
            }

            const boost::system::result<const boost::json::value &> overload_result = cockpit_audio_object.try_at("overload");
            if (overload_result.has_value()) {
                cockpit_audio.overload = boost::json::value_to<std::string>(overload_result.value());
            }

            const boost::system::result<const boost::json::value &> overload_stopped_result = cockpit_audio_object.try_at("overload_stopped");
            if (overload_stopped_result.has_value()) {
                cockpit_audio.overload_stopped = boost::json::value_to<std::string>(overload_stopped_result.value());
            }

            const boost::system::result<const boost::json::value &> repair_result = cockpit_audio_object.try_at("repair");
            if (repair_result.has_value()) {
                cockpit_audio.repair = boost::json::value_to<std::string>(repair_result.value());
            }

            const boost::system::result<const boost::json::value &> scanning_result = cockpit_audio_object.try_at("scanning");
            if (scanning_result.has_value()) {
                cockpit_audio.scanning = boost::json::value_to<std::string>(scanning_result.value());
            }

            const boost::system::result<const boost::json::value &> shield_result = cockpit_audio_object.try_at("shield");
            if (shield_result.has_value()) {
                cockpit_audio.shield = boost::json::value_to<std::string>(shield_result.value());
            }

            const boost::system::result<const boost::json::value &> sounds_extension_1_result = cockpit_audio_object.try_at("sounds_extension_1");
            if (sounds_extension_1_result.has_value()) {
                cockpit_audio.sounds_extension_1 = boost::json::value_to<std::string>(sounds_extension_1_result.value());
            }

            const boost::system::result<const boost::json::value &> sounds_extension_2_result = cockpit_audio_object.try_at("sounds_extension_2");
            if (sounds_extension_2_result.has_value()) {
                cockpit_audio.sounds_extension_2 = boost::json::value_to<std::string>(sounds_extension_2_result.value());
            }

            const boost::system::result<const boost::json::value &> sounds_extension_3_result = cockpit_audio_object.try_at("sounds_extension_3");
            if (sounds_extension_3_result.has_value()) {
                cockpit_audio.sounds_extension_3 = boost::json::value_to<std::string>(sounds_extension_3_result.value());
            }

            const boost::system::result<const boost::json::value &> sounds_extension_4_result = cockpit_audio_object.try_at("sounds_extension_4");
            if (sounds_extension_4_result.has_value()) {
                cockpit_audio.sounds_extension_4 = boost::json::value_to<std::string>(sounds_extension_4_result.value());
            }

            const boost::system::result<const boost::json::value &> sounds_extension_5_result = cockpit_audio_object.try_at("sounds_extension_5");
            if (sounds_extension_5_result.has_value()) {
                cockpit_audio.sounds_extension_5 = boost::json::value_to<std::string>(sounds_extension_5_result.value());
            }

            const boost::system::result<const boost::json::value &> sounds_extension_6_result = cockpit_audio_object.try_at("sounds_extension_6");
            if (sounds_extension_6_result.has_value()) {
                cockpit_audio.sounds_extension_6 = boost::json::value_to<std::string>(sounds_extension_6_result.value());
            }

            const boost::system::result<const boost::json::value &> sounds_extension_7_result = cockpit_audio_object.try_at("sounds_extension_7");
            if (sounds_extension_7_result.has_value()) {
                cockpit_audio.sounds_extension_7 = boost::json::value_to<std::string>(sounds_extension_7_result.value());
            }

            const boost::system::result<const boost::json::value &> sounds_extension_8_result = cockpit_audio_object.try_at("sounds_extension_8");
            if (sounds_extension_8_result.has_value()) {
                cockpit_audio.sounds_extension_8 = boost::json::value_to<std::string>(sounds_extension_8_result.value());
            }

            const boost::system::result<const boost::json::value &> sounds_extension_9_result = cockpit_audio_object.try_at("sounds_extension_9");
            if (sounds_extension_9_result.has_value()) {
                cockpit_audio.sounds_extension_9 = boost::json::value_to<std::string>(sounds_extension_9_result.value());
            }

            const boost::system::result<const boost::json::value &> target_result = cockpit_audio_object.try_at("target");
            if (target_result.has_value()) {
                cockpit_audio.target = boost::json::value_to<std::string>(target_result.value());
            }

            const boost::system::result<const boost::json::value &> target_reverse_result = cockpit_audio_object.try_at("target_reverse");
            if (target_reverse_result.has_value()) {
                cockpit_audio.target_reverse = boost::json::value_to<std::string>(target_reverse_result.value());
            }

            const boost::system::result<const boost::json::value &> undocking_complete_result = cockpit_audio_object.try_at("undocking_complete");
            if (undocking_complete_result.has_value()) {
                cockpit_audio.undocking_complete = boost::json::value_to<std::string>(undocking_complete_result.value());
            }

            const boost::system::result<const boost::json::value &> undocking_failed_result = cockpit_audio_object.try_at("undocking_failed");
            if (undocking_failed_result.has_value()) {
                cockpit_audio.undocking_failed = boost::json::value_to<std::string>(undocking_failed_result.value());
            }

            const boost::system::result<const boost::json::value &> vdu_static_result = cockpit_audio_object.try_at("vdu_static");
            if (vdu_static_result.has_value()) {
                cockpit_audio.vdu_static = boost::json::value_to<std::string>(vdu_static_result.value());
            }

            const boost::system::result<const boost::json::value &> view_result = cockpit_audio_object.try_at("view");
            if (view_result.has_value()) {
                cockpit_audio.view = boost::json::value_to<std::string>(view_result.value());
            }

            const boost::system::result<const boost::json::value &> weapon_switch_result = cockpit_audio_object.try_at("weapon_switch");
            if (weapon_switch_result.has_value()) {
                cockpit_audio.weapon_switch = boost::json::value_to<std::string>(weapon_switch_result.value());
            }

        }


        const boost::system::result<const boost::json::value &> collision_hacks_value = root_object.try_at("collision_hacks");
        if (collision_hacks_value.has_value()) {
            boost::json::object collision_hacks_object = collision_hacks_value.value().get_object();
            const boost::system::result<const boost::json::value &> collision_hack_distance_result = collision_hacks_object.try_at("collision_hack_distance");
            if (collision_hack_distance_result.has_value()) {
                collision_hacks.collision_hack_distance = boost::json::value_to<double>(collision_hack_distance_result.value());
            }

            const boost::system::result<const boost::json::value &> collision_damage_to_ai_result = collision_hacks_object.try_at("collision_damage_to_ai");
            if (collision_damage_to_ai_result.has_value()) {
                collision_hacks.collision_damage_to_ai = boost::json::value_to<bool>(collision_damage_to_ai_result.value());
            }

            const boost::system::result<const boost::json::value &> crash_dock_hangar_result = collision_hacks_object.try_at("crash_dock_hangar");
            if (crash_dock_hangar_result.has_value()) {
                collision_hacks.crash_dock_hangar = boost::json::value_to<bool>(crash_dock_hangar_result.value());
            }

            const boost::system::result<const boost::json::value &> crash_dock_unit_result = collision_hacks_object.try_at("crash_dock_unit");
            if (crash_dock_unit_result.has_value()) {
                collision_hacks.crash_dock_unit = boost::json::value_to<bool>(crash_dock_unit_result.value());
            }

            const boost::system::result<const boost::json::value &> front_collision_hack_angle_result = collision_hacks_object.try_at("front_collision_hack_angle");
            if (front_collision_hack_angle_result.has_value()) {
                collision_hacks.front_collision_hack_angle = boost::json::value_to<double>(front_collision_hack_angle_result.value());
            }

            const boost::system::result<const boost::json::value &> front_collision_hack_distance_result = collision_hacks_object.try_at("front_collision_hack_distance");
            if (front_collision_hack_distance_result.has_value()) {
                collision_hacks.front_collision_hack_distance = boost::json::value_to<double>(front_collision_hack_distance_result.value());
            }

            const boost::system::result<const boost::json::value &> cargo_deals_collide_damage_result = collision_hacks_object.try_at("cargo_deals_collide_damage");
            if (cargo_deals_collide_damage_result.has_value()) {
                collision_hacks.cargo_deals_collide_damage = boost::json::value_to<bool>(cargo_deals_collide_damage_result.value());
            }

        }


        const boost::system::result<const boost::json::value &> logging_value = root_object.try_at("logging");
        if (logging_value.has_value()) {
            boost::json::object logging_object = logging_value.value().get_object();
            const boost::system::result<const boost::json::value &> vsdebug_result = logging_object.try_at("vsdebug");
            if (vsdebug_result.has_value()) {
                logging.vsdebug = boost::json::value_to<int>(vsdebug_result.value());
            }

            const boost::system::result<const boost::json::value &> verbose_debug_result = logging_object.try_at("verbose_debug");
            if (verbose_debug_result.has_value()) {
                logging.verbose_debug = boost::json::value_to<bool>(verbose_debug_result.value());
            }

        }


        const boost::system::result<const boost::json::value &> physics_value = root_object.try_at("physics");
        if (physics_value.has_value()) {
            boost::json::object physics_object = physics_value.value().get_object();
            const boost::system::result<const boost::json::value &> capship_size_result = physics_object.try_at("capship_size");
            if (capship_size_result.has_value()) {
                physics.capship_size = boost::json::value_to<double>(capship_size_result.value());
            }

            const boost::system::result<const boost::json::value &> close_enough_to_autotrack_result = physics_object.try_at("close_enough_to_autotrack");
            if (close_enough_to_autotrack_result.has_value()) {
                physics.close_enough_to_autotrack = boost::json::value_to<double>(close_enough_to_autotrack_result.value());
            }

            const boost::system::result<const boost::json::value &> collision_scale_factor_result = physics_object.try_at("collision_scale_factor");
            if (collision_scale_factor_result.has_value()) {
                physics.collision_scale_factor = boost::json::value_to<double>(collision_scale_factor_result.value());
            }

            const boost::system::result<const boost::json::value &> debris_time_result = physics_object.try_at("debris_time");
            if (debris_time_result.has_value()) {
                physics.debris_time = boost::json::value_to<double>(debris_time_result.value());
            }

            const boost::system::result<const boost::json::value &> default_shield_tightness_result = physics_object.try_at("default_shield_tightness");
            if (default_shield_tightness_result.has_value()) {
                physics.default_shield_tightness = boost::json::value_to<double>(default_shield_tightness_result.value());
            }

            const boost::system::result<const boost::json::value &> density_of_jump_point_result = physics_object.try_at("density_of_jump_point");
            if (density_of_jump_point_result.has_value()) {
                physics.density_of_jump_point = boost::json::value_to<double>(density_of_jump_point_result.value());
            }

            const boost::system::result<const boost::json::value &> density_of_rock_result = physics_object.try_at("density_of_rock");
            if (density_of_rock_result.has_value()) {
                physics.density_of_rock = boost::json::value_to<double>(density_of_rock_result.value());
            }

            const boost::system::result<const boost::json::value &> difficulty_based_enemy_damage_result = physics_object.try_at("difficulty_based_enemy_damage");
            if (difficulty_based_enemy_damage_result.has_value()) {
                physics.difficulty_based_enemy_damage = boost::json::value_to<bool>(difficulty_based_enemy_damage_result.value());
            }

            const boost::system::result<const boost::json::value &> difficulty_speed_exponent_result = physics_object.try_at("difficulty_speed_exponent");
            if (difficulty_speed_exponent_result.has_value()) {
                physics.difficulty_speed_exponent = boost::json::value_to<double>(difficulty_speed_exponent_result.value());
            }

            const boost::system::result<const boost::json::value &> difficulty_based_shield_recharge_result = physics_object.try_at("difficulty_based_shield_recharge");
            if (difficulty_based_shield_recharge_result.has_value()) {
                physics.difficulty_based_shield_recharge = boost::json::value_to<bool>(difficulty_based_shield_recharge_result.value());
            }

            const boost::system::result<const boost::json::value &> distance_to_warp_result = physics_object.try_at("distance_to_warp");
            if (distance_to_warp_result.has_value()) {
                physics.distance_to_warp = boost::json::value_to<double>(distance_to_warp_result.value());
            }

            const boost::system::result<const boost::json::value &> does_missile_bounce_result = physics_object.try_at("does_missile_bounce");
            if (does_missile_bounce_result.has_value()) {
                physics.does_missile_bounce = boost::json::value_to<bool>(does_missile_bounce_result.value());
            }

            const boost::system::result<const boost::json::value &> engine_energy_takes_priority_result = physics_object.try_at("engine_energy_takes_priority");
            if (engine_energy_takes_priority_result.has_value()) {
                physics.engine_energy_takes_priority = boost::json::value_to<bool>(engine_energy_takes_priority_result.value());
            }

            const boost::system::result<const boost::json::value &> friendly_auto_radius_result = physics_object.try_at("friendly_auto_radius");
            if (friendly_auto_radius_result.has_value()) {
                physics.friendly_auto_radius = boost::json::value_to<double>(friendly_auto_radius_result.value());
            }

            const boost::system::result<const boost::json::value &> game_accel_result = physics_object.try_at("game_accel");
            if (game_accel_result.has_value()) {
                physics.game_accel = boost::json::value_to<double>(game_accel_result.value());
            }

            const boost::system::result<const boost::json::value &> game_speed_result = physics_object.try_at("game_speed");
            if (game_speed_result.has_value()) {
                physics.game_speed = boost::json::value_to<double>(game_speed_result.value());
            }

            const boost::system::result<const boost::json::value &> hostile_auto_radius_result = physics_object.try_at("hostile_auto_radius");
            if (hostile_auto_radius_result.has_value()) {
                physics.hostile_auto_radius = boost::json::value_to<double>(hostile_auto_radius_result.value());
            }

            const boost::system::result<const boost::json::value &> indiscriminate_system_destruction_result = physics_object.try_at("indiscriminate_system_destruction");
            if (indiscriminate_system_destruction_result.has_value()) {
                physics.indiscriminate_system_destruction = boost::json::value_to<double>(indiscriminate_system_destruction_result.value());
            }

            const boost::system::result<const boost::json::value &> inelastic_scale_result = physics_object.try_at("inelastic_scale");
            if (inelastic_scale_result.has_value()) {
                physics.inelastic_scale = boost::json::value_to<double>(inelastic_scale_result.value());
            }

            const boost::system::result<const boost::json::value &> jump_mesh_radius_scale_result = physics_object.try_at("jump_mesh_radius_scale");
            if (jump_mesh_radius_scale_result.has_value()) {
                physics.jump_mesh_radius_scale = boost::json::value_to<double>(jump_mesh_radius_scale_result.value());
            }

            const boost::system::result<const boost::json::value &> max_damage_result = physics_object.try_at("max_damage");
            if (max_damage_result.has_value()) {
                physics.max_damage = boost::json::value_to<double>(max_damage_result.value());
            }

            const boost::system::result<const boost::json::value &> max_ecm_result = physics_object.try_at("max_ecm");
            if (max_ecm_result.has_value()) {
                physics.max_ecm = boost::json::value_to<int>(max_ecm_result.value());
            }

            const boost::system::result<const boost::json::value &> max_force_multiplier_result = physics_object.try_at("max_force_multiplier");
            if (max_force_multiplier_result.has_value()) {
                physics.max_force_multiplier = boost::json::value_to<double>(max_force_multiplier_result.value());
            }

            const boost::system::result<const boost::json::value &> max_lost_target_live_time_result = physics_object.try_at("max_lost_target_live_time");
            if (max_lost_target_live_time_result.has_value()) {
                physics.max_lost_target_live_time = boost::json::value_to<double>(max_lost_target_live_time_result.value());
            }

            const boost::system::result<const boost::json::value &> max_non_player_rotation_rate_result = physics_object.try_at("max_non_player_rotation_rate");
            if (max_non_player_rotation_rate_result.has_value()) {
                physics.max_non_player_rotation_rate = boost::json::value_to<double>(max_non_player_rotation_rate_result.value());
            }

            const boost::system::result<const boost::json::value &> max_player_rotation_rate_result = physics_object.try_at("max_player_rotation_rate");
            if (max_player_rotation_rate_result.has_value()) {
                physics.max_player_rotation_rate = boost::json::value_to<double>(max_player_rotation_rate_result.value());
            }

            const boost::system::result<const boost::json::value &> max_radar_cone_damage_result = physics_object.try_at("max_radar_cone_damage");
            if (max_radar_cone_damage_result.has_value()) {
                physics.max_radar_cone_damage = boost::json::value_to<double>(max_radar_cone_damage_result.value());
            }

            const boost::system::result<const boost::json::value &> max_radar_lock_cone_damage_result = physics_object.try_at("max_radar_lock_cone_damage");
            if (max_radar_lock_cone_damage_result.has_value()) {
                physics.max_radar_lock_cone_damage = boost::json::value_to<double>(max_radar_lock_cone_damage_result.value());
            }

            const boost::system::result<const boost::json::value &> max_radar_track_cone_damage_result = physics_object.try_at("max_radar_track_cone_damage");
            if (max_radar_track_cone_damage_result.has_value()) {
                physics.max_radar_track_cone_damage = boost::json::value_to<double>(max_radar_track_cone_damage_result.value());
            }

            const boost::system::result<const boost::json::value &> max_shield_lowers_capacitance_result = physics_object.try_at("max_shield_lowers_capacitance");
            if (max_shield_lowers_capacitance_result.has_value()) {
                physics.max_shield_lowers_capacitance = boost::json::value_to<bool>(max_shield_lowers_capacitance_result.value());
            }

            const boost::system::result<const boost::json::value &> max_torque_multiplier_result = physics_object.try_at("max_torque_multiplier");
            if (max_torque_multiplier_result.has_value()) {
                physics.max_torque_multiplier = boost::json::value_to<double>(max_torque_multiplier_result.value());
            }

            const boost::system::result<const boost::json::value &> min_asteroid_distance_result = physics_object.try_at("min_asteroid_distance");
            if (min_asteroid_distance_result.has_value()) {
                physics.min_asteroid_distance = boost::json::value_to<double>(min_asteroid_distance_result.value());
            }

            const boost::system::result<const boost::json::value &> min_damage_result = physics_object.try_at("min_damage");
            if (min_damage_result.has_value()) {
                physics.min_damage = boost::json::value_to<double>(min_damage_result.value());
            }

            const boost::system::result<const boost::json::value &> min_shield_speeding_discharge_result = physics_object.try_at("min_shield_speeding_discharge");
            if (min_shield_speeding_discharge_result.has_value()) {
                physics.min_shield_speeding_discharge = boost::json::value_to<double>(min_shield_speeding_discharge_result.value());
            }

            const boost::system::result<const boost::json::value &> minimum_mass_result = physics_object.try_at("minimum_mass");
            if (minimum_mass_result.has_value()) {
                physics.minimum_mass = boost::json::value_to<double>(minimum_mass_result.value());
            }

            const boost::system::result<const boost::json::value &> minimum_time_result = physics_object.try_at("minimum_time");
            if (minimum_time_result.has_value()) {
                physics.minimum_time = boost::json::value_to<double>(minimum_time_result.value());
            }

            const boost::system::result<const boost::json::value &> near_autotrack_cone_result = physics_object.try_at("near_autotrack_cone");
            if (near_autotrack_cone_result.has_value()) {
                physics.near_autotrack_cone = boost::json::value_to<double>(near_autotrack_cone_result.value());
            }

            const boost::system::result<const boost::json::value &> nebula_shield_recharge_result = physics_object.try_at("nebula_shield_recharge");
            if (nebula_shield_recharge_result.has_value()) {
                physics.nebula_shield_recharge = boost::json::value_to<double>(nebula_shield_recharge_result.value());
            }

            const boost::system::result<const boost::json::value &> neutral_auto_radius_result = physics_object.try_at("neutral_auto_radius");
            if (neutral_auto_radius_result.has_value()) {
                physics.neutral_auto_radius = boost::json::value_to<double>(neutral_auto_radius_result.value());
            }

            const boost::system::result<const boost::json::value &> no_damage_to_docked_ships_result = physics_object.try_at("no_damage_to_docked_ships");
            if (no_damage_to_docked_ships_result.has_value()) {
                physics.no_damage_to_docked_ships = boost::json::value_to<bool>(no_damage_to_docked_ships_result.value());
            }

            const boost::system::result<const boost::json::value &> no_spec_jump_result = physics_object.try_at("no_spec_jump");
            if (no_spec_jump_result.has_value()) {
                physics.no_spec_jump = boost::json::value_to<bool>(no_spec_jump_result.value());
            }

            const boost::system::result<const boost::json::value &> no_unit_collisions_result = physics_object.try_at("no_unit_collisions");
            if (no_unit_collisions_result.has_value()) {
                physics.no_unit_collisions = boost::json::value_to<bool>(no_unit_collisions_result.value());
            }

            const boost::system::result<const boost::json::value &> percent_missile_match_target_velocity_result = physics_object.try_at("percent_missile_match_target_velocity");
            if (percent_missile_match_target_velocity_result.has_value()) {
                physics.percent_missile_match_target_velocity = boost::json::value_to<double>(percent_missile_match_target_velocity_result.value());
            }

            const boost::system::result<const boost::json::value &> planet_dock_min_port_size_result = physics_object.try_at("planet_dock_min_port_size");
            if (planet_dock_min_port_size_result.has_value()) {
                physics.planet_dock_min_port_size = boost::json::value_to<double>(planet_dock_min_port_size_result.value());
            }

            const boost::system::result<const boost::json::value &> planet_dock_port_size_result = physics_object.try_at("planet_dock_port_size");
            if (planet_dock_port_size_result.has_value()) {
                physics.planet_dock_port_size = boost::json::value_to<double>(planet_dock_port_size_result.value());
            }

            const boost::system::result<const boost::json::value &> planets_always_neutral_result = physics_object.try_at("planets_always_neutral");
            if (planets_always_neutral_result.has_value()) {
                physics.planets_always_neutral = boost::json::value_to<bool>(planets_always_neutral_result.value());
            }

            const boost::system::result<const boost::json::value &> planets_can_have_subunits_result = physics_object.try_at("planets_can_have_subunits");
            if (planets_can_have_subunits_result.has_value()) {
                physics.planets_can_have_subunits = boost::json::value_to<bool>(planets_can_have_subunits_result.value());
            }

            const boost::system::result<const boost::json::value &> separate_system_flakiness_component_result = physics_object.try_at("separate_system_flakiness_component");
            if (separate_system_flakiness_component_result.has_value()) {
                physics.separate_system_flakiness_component = boost::json::value_to<bool>(separate_system_flakiness_component_result.value());
            }

            const boost::system::result<const boost::json::value &> shield_energy_capacitance_result = physics_object.try_at("shield_energy_capacitance");
            if (shield_energy_capacitance_result.has_value()) {
                physics.shield_energy_capacitance = boost::json::value_to<double>(shield_energy_capacitance_result.value());
            }

            const boost::system::result<const boost::json::value &> shield_maintenance_charge_result = physics_object.try_at("shield_maintenance_charge");
            if (shield_maintenance_charge_result.has_value()) {
                physics.shield_maintenance_charge = boost::json::value_to<double>(shield_maintenance_charge_result.value());
            }

            const boost::system::result<const boost::json::value &> shields_in_spec_result = physics_object.try_at("shields_in_spec");
            if (shields_in_spec_result.has_value()) {
                physics.shields_in_spec = boost::json::value_to<bool>(shields_in_spec_result.value());
            }

            const boost::system::result<const boost::json::value &> speeding_discharge_result = physics_object.try_at("speeding_discharge");
            if (speeding_discharge_result.has_value()) {
                physics.speeding_discharge = boost::json::value_to<double>(speeding_discharge_result.value());
            }

            const boost::system::result<const boost::json::value &> steady_itts_result = physics_object.try_at("steady_itts");
            if (steady_itts_result.has_value()) {
                physics.steady_itts = boost::json::value_to<bool>(steady_itts_result.value());
            }

            const boost::system::result<const boost::json::value &> system_damage_on_armor_result = physics_object.try_at("system_damage_on_armor");
            if (system_damage_on_armor_result.has_value()) {
                physics.system_damage_on_armor = boost::json::value_to<bool>(system_damage_on_armor_result.value());
            }

            const boost::system::result<const boost::json::value &> target_distance_to_warp_bonus_result = physics_object.try_at("target_distance_to_warp_bonus");
            if (target_distance_to_warp_bonus_result.has_value()) {
                physics.target_distance_to_warp_bonus = boost::json::value_to<double>(target_distance_to_warp_bonus_result.value());
            }

            const boost::system::result<const boost::json::value &> thruster_hit_chance_result = physics_object.try_at("thruster_hit_chance");
            if (thruster_hit_chance_result.has_value()) {
                physics.thruster_hit_chance = boost::json::value_to<double>(thruster_hit_chance_result.value());
            }

            const boost::system::result<const boost::json::value &> use_max_shield_energy_usage_result = physics_object.try_at("use_max_shield_energy_usage");
            if (use_max_shield_energy_usage_result.has_value()) {
                physics.use_max_shield_energy_usage = boost::json::value_to<bool>(use_max_shield_energy_usage_result.value());
            }

            const boost::system::result<const boost::json::value &> velocity_max_result = physics_object.try_at("velocity_max");
            if (velocity_max_result.has_value()) {
                physics.velocity_max = boost::json::value_to<double>(velocity_max_result.value());
            }

        const boost::system::result<const boost::json::value &> ejection_value = physics_object.try_at("ejection");
        if (ejection_value.has_value()) {
            boost::json::object ejection_object = ejection_value.value().get_object();
            const boost::system::result<const boost::json::value &> auto_eject_percent_result = ejection_object.try_at("auto_eject_percent");
            if (auto_eject_percent_result.has_value()) {
                physics.ejection.auto_eject_percent = boost::json::value_to<double>(auto_eject_percent_result.value());
            }

            const boost::system::result<const boost::json::value &> eject_cargo_percent_result = ejection_object.try_at("eject_cargo_percent");
            if (eject_cargo_percent_result.has_value()) {
                physics.ejection.eject_cargo_percent = boost::json::value_to<double>(eject_cargo_percent_result.value());
            }

            const boost::system::result<const boost::json::value &> max_dumped_cargo_result = ejection_object.try_at("max_dumped_cargo");
            if (max_dumped_cargo_result.has_value()) {
                physics.ejection.max_dumped_cargo = boost::json::value_to<int>(max_dumped_cargo_result.value());
            }

            const boost::system::result<const boost::json::value &> hull_damage_to_eject_result = ejection_object.try_at("hull_damage_to_eject");
            if (hull_damage_to_eject_result.has_value()) {
                physics.ejection.hull_damage_to_eject = boost::json::value_to<double>(hull_damage_to_eject_result.value());
            }

            const boost::system::result<const boost::json::value &> player_auto_eject_result = ejection_object.try_at("player_auto_eject");
            if (player_auto_eject_result.has_value()) {
                physics.ejection.player_auto_eject = boost::json::value_to<bool>(player_auto_eject_result.value());
            }

        }


        }


        const boost::system::result<const boost::json::value &> unit_value = root_object.try_at("unit");
        if (unit_value.has_value()) {
            boost::json::object unit_object = unit_value.value().get_object();
            const boost::system::result<const boost::json::value &> default_aggressivity_result = unit_object.try_at("default_aggressivity");
            if (default_aggressivity_result.has_value()) {
                unit.default_aggressivity = boost::json::value_to<double>(default_aggressivity_result.value());
            }

        }


        const boost::system::result<const boost::json::value &> warp_value = root_object.try_at("warp");
        if (warp_value.has_value()) {
            boost::json::object warp_object = warp_value.value().get_object();
            const boost::system::result<const boost::json::value &> bleed_factor_result = warp_object.try_at("bleed_factor");
            if (bleed_factor_result.has_value()) {
                warp.bleed_factor = boost::json::value_to<double>(bleed_factor_result.value());
            }

            const boost::system::result<const boost::json::value &> computer_warp_ramp_up_time_result = warp_object.try_at("computer_warp_ramp_up_time");
            if (computer_warp_ramp_up_time_result.has_value()) {
                warp.computer_warp_ramp_up_time = boost::json::value_to<double>(computer_warp_ramp_up_time_result.value());
            }

            const boost::system::result<const boost::json::value &> in_system_jump_cost_result = warp_object.try_at("in_system_jump_cost");
            if (in_system_jump_cost_result.has_value()) {
                warp.in_system_jump_cost = boost::json::value_to<double>(in_system_jump_cost_result.value());
            }

            const boost::system::result<const boost::json::value &> max_effective_velocity_result = warp_object.try_at("max_effective_velocity");
            if (max_effective_velocity_result.has_value()) {
                warp.max_effective_velocity = boost::json::value_to<double>(max_effective_velocity_result.value());
            }

            const boost::system::result<const boost::json::value &> player_warp_energy_multiplier_result = warp_object.try_at("player_warp_energy_multiplier");
            if (player_warp_energy_multiplier_result.has_value()) {
                warp.player_warp_energy_multiplier = boost::json::value_to<double>(player_warp_energy_multiplier_result.value());
            }

            const boost::system::result<const boost::json::value &> use_warp_energy_for_cloak_result = warp_object.try_at("use_warp_energy_for_cloak");
            if (use_warp_energy_for_cloak_result.has_value()) {
                warp.use_warp_energy_for_cloak = boost::json::value_to<bool>(use_warp_energy_for_cloak_result.value());
            }

            const boost::system::result<const boost::json::value &> warp_energy_multiplier_result = warp_object.try_at("warp_energy_multiplier");
            if (warp_energy_multiplier_result.has_value()) {
                warp.warp_energy_multiplier = boost::json::value_to<double>(warp_energy_multiplier_result.value());
            }

            const boost::system::result<const boost::json::value &> warp_memory_effect_result = warp_object.try_at("warp_memory_effect");
            if (warp_memory_effect_result.has_value()) {
                warp.warp_memory_effect = boost::json::value_to<double>(warp_memory_effect_result.value());
            }

            const boost::system::result<const boost::json::value &> warp_multiplier_max_result = warp_object.try_at("warp_multiplier_max");
            if (warp_multiplier_max_result.has_value()) {
                warp.warp_multiplier_max = boost::json::value_to<double>(warp_multiplier_max_result.value());
            }

            const boost::system::result<const boost::json::value &> warp_multiplier_min_result = warp_object.try_at("warp_multiplier_min");
            if (warp_multiplier_min_result.has_value()) {
                warp.warp_multiplier_min = boost::json::value_to<double>(warp_multiplier_min_result.value());
            }

            const boost::system::result<const boost::json::value &> warp_ramp_down_time_result = warp_object.try_at("warp_ramp_down_time");
            if (warp_ramp_down_time_result.has_value()) {
                warp.warp_ramp_down_time = boost::json::value_to<double>(warp_ramp_down_time_result.value());
            }

            const boost::system::result<const boost::json::value &> warp_ramp_up_time_result = warp_object.try_at("warp_ramp_up_time");
            if (warp_ramp_up_time_result.has_value()) {
                warp.warp_ramp_up_time = boost::json::value_to<double>(warp_ramp_up_time_result.value());
            }

            const boost::system::result<const boost::json::value &> warp_stretch_cutoff_result = warp_object.try_at("warp_stretch_cutoff");
            if (warp_stretch_cutoff_result.has_value()) {
                warp.warp_stretch_cutoff = boost::json::value_to<double>(warp_stretch_cutoff_result.value());
            }

            const boost::system::result<const boost::json::value &> warp_stretch_decel_cutoff_result = warp_object.try_at("warp_stretch_decel_cutoff");
            if (warp_stretch_decel_cutoff_result.has_value()) {
                warp.warp_stretch_decel_cutoff = boost::json::value_to<double>(warp_stretch_decel_cutoff_result.value());
            }

        }


        const boost::system::result<const boost::json::value &> weapons_value = root_object.try_at("weapons");
        if (weapons_value.has_value()) {
            boost::json::object weapons_object = weapons_value.value().get_object();
            const boost::system::result<const boost::json::value &> can_fire_in_cloak_result = weapons_object.try_at("can_fire_in_cloak");
            if (can_fire_in_cloak_result.has_value()) {
                weapons.can_fire_in_cloak = boost::json::value_to<bool>(can_fire_in_cloak_result.value());
            }

            const boost::system::result<const boost::json::value &> can_fire_in_spec_result = weapons_object.try_at("can_fire_in_spec");
            if (can_fire_in_spec_result.has_value()) {
                weapons.can_fire_in_spec = boost::json::value_to<bool>(can_fire_in_spec_result.value());
            }

        }


    }
    catch (std::exception const& e)
    {
        VS_LOG(error, (boost::format("Error in %1%: '%2%'") % __FUNCTION__ % e.what()));
    }
}
