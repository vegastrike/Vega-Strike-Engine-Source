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
#include <boost/property_tree/json_parser.hpp>

#include "configuration.h"

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <math.h>

vega_config::Config::Config(const std::string json_text) {
    try {
        boost::json::value json_value = boost::json::parse(json_text);
        const boost::json::object & root_object = json_value.get_object();


		boost::json::object general_object = root_object.at("general").get_object();
		general.audio_atom = boost::json::value_to<double>(general_object.at("audio_atom"));
		general.command_interpreter = boost::json::value_to<bool>(general_object.at("command_interpreter"));
		general.custom_python = boost::json::value_to<std::string>(general_object.at("custom_python"));
		general.debug_config = boost::json::value_to<bool>(general_object.at("debug_config"));
		general.debug_fs = boost::json::value_to<int>(general_object.at("debug_fs"));
		general.delete_old_systems = boost::json::value_to<bool>(general_object.at("delete_old_systems"));
		general.docking_time = boost::json::value_to<double>(general_object.at("docking_time"));
		general.docking_fee = boost::json::value_to<double>(general_object.at("docking_fee"));
		general.empty_mission = boost::json::value_to<std::string>(general_object.at("empty_mission"));
		general.force_anonymous_mission_names = boost::json::value_to<bool>(general_object.at("force_anonymous_mission_names"));
		general.fuel_docking_fee = boost::json::value_to<double>(general_object.at("fuel_docking_fee"));
		general.garbage_collect_frequency = boost::json::value_to<int>(general_object.at("garbage_collect_frequency"));
		general.jump_key_delay = boost::json::value_to<double>(general_object.at("jump_key_delay"));
		general.load_last_savegame = boost::json::value_to<bool>(general_object.at("load_last_savegame"));
		general.new_game_save_name = boost::json::value_to<std::string>(general_object.at("new_game_save_name"));
		general.num_old_systems = boost::json::value_to<int>(general_object.at("num_old_systems"));
		general.percentage_speed_change_to_fault_search = boost::json::value_to<double>(general_object.at("percentage_speed_change_to_fault_search"));
		general.persistent_mission_across_ship_switch = boost::json::value_to<bool>(general_object.at("persistent_mission_across_ship_switch"));
		general.pitch = boost::json::value_to<double>(general_object.at("pitch"));
		general.quick_savegame_summaries = boost::json::value_to<bool>(general_object.at("quick_savegame_summaries"));
		general.quick_savegame_summaries_buffer_size = boost::json::value_to<int>(general_object.at("quick_savegame_summaries_buffer_size"));
		general.remember_savegame = boost::json::value_to<bool>(general_object.at("remember_savegame"));
		general.remove_downgrades_less_than_percent = boost::json::value_to<double>(general_object.at("remove_downgrades_less_than_percent"));
		general.remove_impossible_downgrades = boost::json::value_to<bool>(general_object.at("remove_impossible_downgrades"));
		general.roll = boost::json::value_to<double>(general_object.at("roll"));
		general.screen = boost::json::value_to<int>(general_object.at("screen"));
		general.simulation_atom = boost::json::value_to<double>(general_object.at("simulation_atom"));
		general.times_to_show_help_screen = boost::json::value_to<int>(general_object.at("times_to_show_help_screen"));
		general.trade_interface_tracks_prices = boost::json::value_to<bool>(general_object.at("trade_interface_tracks_prices"));
		general.trade_interface_tracks_prices_top_rank = boost::json::value_to<int>(general_object.at("trade_interface_tracks_prices_top_rank"));
		general.verbose_output = boost::json::value_to<int>(general_object.at("verbose_output"));
		general.wheel_increment_lines = boost::json::value_to<int>(general_object.at("wheel_increment_lines"));
		general.while_loading_star_system = boost::json::value_to<bool>(general_object.at("while_loading_star_system"));
		general.write_savegame_on_exit = boost::json::value_to<bool>(general_object.at("write_savegame_on_exit"));
		general.yaw = boost::json::value_to<double>(general_object.at("yaw"));

		boost::json::object graphics_object = root_object.at("graphics").get_object();
		graphics.resolution_x = boost::json::value_to<int>(graphics_object.at("resolution_x"));
		graphics.resolution_y = boost::json::value_to<int>(graphics_object.at("resolution_y"));
		graphics.screen = boost::json::value_to<int>(graphics_object.at("screen"));
		graphics.atmosphere_emissive = boost::json::value_to<double>(graphics_object.at("atmosphere_emissive"));
		graphics.atmosphere_diffuse = boost::json::value_to<double>(graphics_object.at("atmosphere_diffuse"));
		graphics.automatic_landing_zone_warning = boost::json::value_to<std::string>(graphics_object.at("automatic_landing_zone_warning"));
		graphics.automatic_landing_zone_warning1 = boost::json::value_to<std::string>(graphics_object.at("automatic_landing_zone_warning1"));
		graphics.automatic_landing_zone_warning2 = boost::json::value_to<std::string>(graphics_object.at("automatic_landing_zone_warning2"));
		graphics.automatic_landing_zone_warning_text = boost::json::value_to<std::string>(graphics_object.at("automatic_landing_zone_warning_text"));
		graphics.city_light_strength = boost::json::value_to<double>(graphics_object.at("city_light_strength"));
		graphics.day_city_light_strength = boost::json::value_to<double>(graphics_object.at("day_city_light_strength"));
		graphics.default_engine_activation = boost::json::value_to<double>(graphics_object.at("default_engine_activation"));
		graphics.draw_heading_marker = boost::json::value_to<bool>(graphics_object.at("draw_heading_marker"));
		graphics.draw_rendered_crosshairs = boost::json::value_to<bool>(graphics_object.at("draw_rendered_crosshairs"));
		graphics.draw_star_body = boost::json::value_to<bool>(graphics_object.at("draw_star_body"));
		graphics.draw_star_glow = boost::json::value_to<bool>(graphics_object.at("draw_star_glow"));
		graphics.explosion_animation = boost::json::value_to<std::string>(graphics_object.at("explosion_animation"));
		graphics.fov = boost::json::value_to<double>(graphics_object.at("fov"));
		graphics.glow_ambient_star_light = boost::json::value_to<bool>(graphics_object.at("glow_ambient_star_light"));
		graphics.glow_diffuse_star_light = boost::json::value_to<bool>(graphics_object.at("glow_diffuse_star_light"));
		graphics.in_system_jump_ani_second_ahead = boost::json::value_to<double>(graphics_object.at("in_system_jump_ani_second_ahead"));
		graphics.in_system_jump_ani_second_ahead_end = boost::json::value_to<double>(graphics_object.at("in_system_jump_ani_second_ahead_end"));
		graphics.in_system_jump_animation = boost::json::value_to<std::string>(graphics_object.at("in_system_jump_animation"));
		graphics.missile_explosion_radius_mult = boost::json::value_to<double>(graphics_object.at("missile_explosion_radius_mult"));
		graphics.missile_sparkle = boost::json::value_to<bool>(graphics_object.at("missile_sparkle"));
		graphics.num_times_to_draw_shine = boost::json::value_to<int>(graphics_object.at("num_times_to_draw_shine"));
		graphics.reduced_vdus_width = boost::json::value_to<double>(graphics_object.at("reduced_vdus_width"));
		graphics.reduced_vdus_height = boost::json::value_to<double>(graphics_object.at("reduced_vdus_height"));
		graphics.shield_technique = boost::json::value_to<std::string>(graphics_object.at("shield_technique"));
		graphics.shield_texture = boost::json::value_to<std::string>(graphics_object.at("shield_texture"));
		graphics.star_body_radius = boost::json::value_to<double>(graphics_object.at("star_body_radius"));
		graphics.star_glow_radius = boost::json::value_to<double>(graphics_object.at("star_glow_radius"));
		graphics.wormhole_unit = boost::json::value_to<std::string>(graphics_object.at("wormhole_unit"));

		boost::json::object glow_flicker_object = graphics_object.at("glow_flicker").get_object();
		graphics.glow_flicker.flicker_time = boost::json::value_to<double>(glow_flicker_object.at("flicker_time"));
		graphics.glow_flicker.flicker_off_time = boost::json::value_to<double>(glow_flicker_object.at("flicker_off_time"));
		graphics.glow_flicker.min_flicker_cycle = boost::json::value_to<double>(glow_flicker_object.at("min_flicker_cycle"));
		graphics.glow_flicker.num_times_per_second_on = boost::json::value_to<double>(glow_flicker_object.at("num_times_per_second_on"));
		graphics.glow_flicker.hull_for_total_dark = boost::json::value_to<double>(glow_flicker_object.at("hull_for_total_dark"));

		boost::json::object hud_object = graphics_object.at("hud").get_object();
		graphics.hud.already_near_message = boost::json::value_to<std::string>(hud_object.at("already_near_message"));
		graphics.hud.armor_hull_size = boost::json::value_to<double>(hud_object.at("armor_hull_size"));
		graphics.hud.asteroids_near_message = boost::json::value_to<std::string>(hud_object.at("asteroids_near_message"));
		graphics.hud.basename_colon_basename = boost::json::value_to<bool>(hud_object.at("basename_colon_basename"));
		graphics.hud.box_line_thickness = boost::json::value_to<double>(hud_object.at("box_line_thickness"));
		graphics.hud.completed_objectives_last = boost::json::value_to<bool>(hud_object.at("completed_objectives_last"));
		graphics.hud.crosshairs_on_chase_cam = boost::json::value_to<bool>(hud_object.at("crosshairs_on_chase_cam"));
		graphics.hud.crosshairs_on_padlock = boost::json::value_to<bool>(hud_object.at("crosshairs_on_padlock"));
		graphics.hud.damage_report_heading = boost::json::value_to<std::string>(hud_object.at("damage_report_heading"));
		graphics.hud.debug_position = boost::json::value_to<bool>(hud_object.at("debug_position"));
		graphics.hud.diamond_line_thickness = boost::json::value_to<double>(hud_object.at("diamond_line_thickness"));
		graphics.hud.diamond_rotation_speed = boost::json::value_to<double>(hud_object.at("diamond_rotation_speed"));
		graphics.hud.diamond_size = boost::json::value_to<double>(hud_object.at("diamond_size"));
		graphics.hud.display_relative_velocity = boost::json::value_to<bool>(hud_object.at("display_relative_velocity"));
		graphics.hud.display_warp_energy_if_no_jump_drive = boost::json::value_to<bool>(hud_object.at("display_warp_energy_if_no_jump_drive"));
		graphics.hud.draw_all_target_boxes = boost::json::value_to<bool>(hud_object.at("draw_all_target_boxes"));
		graphics.hud.draw_always_itts = boost::json::value_to<bool>(hud_object.at("draw_always_itts"));
		graphics.hud.draw_arrow_on_chase_cam = boost::json::value_to<bool>(hud_object.at("draw_arrow_on_chase_cam"));
		graphics.hud.draw_arrow_on_pan_cam = boost::json::value_to<bool>(hud_object.at("draw_arrow_on_pan_cam"));
		graphics.hud.draw_arrow_on_pan_target = boost::json::value_to<bool>(hud_object.at("draw_arrow_on_pan_target"));
		graphics.hud.draw_arrow_to_target = boost::json::value_to<bool>(hud_object.at("draw_arrow_to_target"));
		graphics.hud.draw_blips_on_both_radar = boost::json::value_to<bool>(hud_object.at("draw_blips_on_both_radar"));
		graphics.hud.draw_damage_sprite = boost::json::value_to<bool>(hud_object.at("draw_damage_sprite"));
		graphics.hud.draw_gravitational_objects = boost::json::value_to<bool>(hud_object.at("draw_gravitational_objects"));
		graphics.hud.draw_line_to_itts = boost::json::value_to<bool>(hud_object.at("draw_line_to_itts"));
		graphics.hud.draw_line_to_target = boost::json::value_to<bool>(hud_object.at("draw_line_to_target"));
		graphics.hud.draw_line_to_targets_target = boost::json::value_to<bool>(hud_object.at("draw_line_to_targets_target"));
		graphics.hud.draw_nav_grid = boost::json::value_to<bool>(hud_object.at("draw_nav_grid"));
		graphics.hud.draw_nav_symbol = boost::json::value_to<bool>(hud_object.at("draw_nav_symbol"));
		graphics.hud.draw_tactical_target = boost::json::value_to<bool>(hud_object.at("draw_tactical_target"));
		graphics.hud.draw_targetting_boxes = boost::json::value_to<bool>(hud_object.at("draw_targetting_boxes"));
		graphics.hud.draw_targetting_boxes_inside = boost::json::value_to<bool>(hud_object.at("draw_targetting_boxes_inside"));
		graphics.hud.draw_significant_blips = boost::json::value_to<bool>(hud_object.at("draw_significant_blips"));
		graphics.hud.draw_star_direction = boost::json::value_to<bool>(hud_object.at("draw_star_direction"));
		graphics.hud.draw_unit_on_chase_cam = boost::json::value_to<bool>(hud_object.at("draw_unit_on_chase_cam"));
		graphics.hud.draw_vdu_view_shields = boost::json::value_to<bool>(hud_object.at("draw_vdu_view_shields"));
		graphics.hud.draw_weapon_sprite = boost::json::value_to<bool>(hud_object.at("draw_weapon_sprite"));
		graphics.hud.enemy_near_message = boost::json::value_to<std::string>(hud_object.at("enemy_near_message"));
		graphics.hud.gun_list_columns = boost::json::value_to<int>(hud_object.at("gun_list_columns"));
		graphics.hud.invert_friendly_shields = boost::json::value_to<bool>(hud_object.at("invert_friendly_shields"));
		graphics.hud.invert_friendly_sprite = boost::json::value_to<bool>(hud_object.at("invert_friendly_sprite"));
		graphics.hud.invert_target_shields = boost::json::value_to<bool>(hud_object.at("invert_target_shields"));
		graphics.hud.invert_target_sprite = boost::json::value_to<bool>(hud_object.at("invert_target_sprite"));
		graphics.hud.invert_view_shields = boost::json::value_to<bool>(hud_object.at("invert_view_shields"));
		graphics.hud.itts_for_beams = boost::json::value_to<bool>(hud_object.at("itts_for_beams"));
		graphics.hud.itts_for_lockable = boost::json::value_to<bool>(hud_object.at("itts_for_lockable"));
		graphics.hud.itts_line_to_mark_alpha = boost::json::value_to<double>(hud_object.at("itts_line_to_mark_alpha"));
		graphics.hud.itts_use_average_gun_speed = boost::json::value_to<bool>(hud_object.at("itts_use_average_gun_speed"));
		graphics.hud.lock_center_crosshair = boost::json::value_to<bool>(hud_object.at("lock_center_crosshair"));
		graphics.hud.lock_confirm_line_length = boost::json::value_to<double>(hud_object.at("lock_confirm_line_length"));
		graphics.hud.manifest_heading = boost::json::value_to<std::string>(hud_object.at("manifest_heading"));
		graphics.hud.max_missile_bracket_size = boost::json::value_to<double>(hud_object.at("max_missile_bracket_size"));
		graphics.hud.message_prefix = boost::json::value_to<std::string>(hud_object.at("message_prefix"));
		graphics.hud.min_missile_bracket_size = boost::json::value_to<double>(hud_object.at("min_missile_bracket_size"));
		graphics.hud.min_lock_box_size = boost::json::value_to<double>(hud_object.at("min_lock_box_size"));
		graphics.hud.min_radar_blip_size = boost::json::value_to<double>(hud_object.at("min_radar_blip_size"));
		graphics.hud.min_target_box_size = boost::json::value_to<double>(hud_object.at("min_target_box_size"));
		graphics.hud.mounts_list_empty = boost::json::value_to<std::string>(hud_object.at("mounts_list_empty"));
		graphics.hud.nav_cross_line_thickness = boost::json::value_to<double>(hud_object.at("nav_cross_line_thickness"));
		graphics.hud.not_included_in_damage_report = boost::json::value_to<std::string>(hud_object.at("not_included_in_damage_report"));
		graphics.hud.out_of_cone_distance = boost::json::value_to<bool>(hud_object.at("out_of_cone_distance"));
		graphics.hud.padlock_view_lag = boost::json::value_to<double>(hud_object.at("padlock_view_lag"));
		graphics.hud.padlock_view_lag_fix_zone = boost::json::value_to<double>(hud_object.at("padlock_view_lag_fix_zone"));
		graphics.hud.planet_near_message = boost::json::value_to<std::string>(hud_object.at("planet_near_message"));
		graphics.hud.print_damage_percent = boost::json::value_to<bool>(hud_object.at("print_damage_percent"));
		graphics.hud.print_faction = boost::json::value_to<bool>(hud_object.at("print_faction"));
		graphics.hud.print_fg_name = boost::json::value_to<bool>(hud_object.at("print_fg_name"));
		graphics.hud.print_fg_sub_id = boost::json::value_to<bool>(hud_object.at("print_fg_sub_id"));
		graphics.hud.print_request_docking = boost::json::value_to<bool>(hud_object.at("print_request_docking"));
		graphics.hud.print_ship_type = boost::json::value_to<bool>(hud_object.at("print_ship_type"));
		graphics.hud.projectile_means_missile = boost::json::value_to<bool>(hud_object.at("projectile_means_missile"));
		graphics.hud.radar_type = boost::json::value_to<std::string>(hud_object.at("radar_type"));
		graphics.hud.radar_search_extra_radius = boost::json::value_to<double>(hud_object.at("radar_search_extra_radius"));
		graphics.hud.rotating_bracket_inner = boost::json::value_to<bool>(hud_object.at("rotating_bracket_inner"));
		graphics.hud.rotating_bracket_size = boost::json::value_to<double>(hud_object.at("rotating_bracket_size"));
		graphics.hud.rotating_bracket_width = boost::json::value_to<double>(hud_object.at("rotating_bracket_width"));
		graphics.hud.scale_relationship_color = boost::json::value_to<double>(hud_object.at("scale_relationship_color"));
		graphics.hud.shield_vdu_fade = boost::json::value_to<bool>(hud_object.at("shield_vdu_fade"));
		graphics.hud.shield_vdu_thresh0 = boost::json::value_to<double>(hud_object.at("shield_vdu_thresh0"));
		graphics.hud.shield_vdu_thresh1 = boost::json::value_to<double>(hud_object.at("shield_vdu_thresh1"));
		graphics.hud.shield_vdu_thresh2 = boost::json::value_to<double>(hud_object.at("shield_vdu_thresh2"));
		graphics.hud.show_negative_blips_as_positive = boost::json::value_to<bool>(hud_object.at("show_negative_blips_as_positive"));
		graphics.hud.simple_manifest = boost::json::value_to<bool>(hud_object.at("simple_manifest"));
		graphics.hud.starship_near_message = boost::json::value_to<std::string>(hud_object.at("starship_near_message"));
		graphics.hud.switch_to_target_mode_on_key = boost::json::value_to<bool>(hud_object.at("switch_to_target_mode_on_key"));
		graphics.hud.switch_back_from_comms = boost::json::value_to<bool>(hud_object.at("switch_back_from_comms"));
		graphics.hud.tac_target_foci = boost::json::value_to<double>(hud_object.at("tac_target_foci"));
		graphics.hud.tac_target_length = boost::json::value_to<double>(hud_object.at("tac_target_length"));
		graphics.hud.tar_target_thickness = boost::json::value_to<double>(hud_object.at("tar_target_thickness"));
		graphics.hud.text_background_alpha = boost::json::value_to<double>(hud_object.at("text_background_alpha"));
		graphics.hud.top_view = boost::json::value_to<bool>(hud_object.at("top_view"));
		graphics.hud.untarget_beyond_cone = boost::json::value_to<bool>(hud_object.at("untarget_beyond_cone"));

		boost::json::object components_object = root_object.at("components").get_object();

		boost::json::object afterburner_object = components_object.at("afterburner").get_object();

		boost::json::object computer_object = components_object.at("computer").get_object();
		components.computer.default_lock_cone = boost::json::value_to<double>(computer_object.at("default_lock_cone"));
		components.computer.default_max_range = boost::json::value_to<double>(computer_object.at("default_max_range"));
		components.computer.default_tracking_cone = boost::json::value_to<double>(computer_object.at("default_tracking_cone"));

		boost::json::object drive_object = components_object.at("drive").get_object();
		components.drive.non_combat_mode_multiplier = boost::json::value_to<int>(drive_object.at("non_combat_mode_multiplier"));
		components.drive.minimum_drive = boost::json::value_to<double>(drive_object.at("minimum_drive"));

		boost::json::object energy_object = components_object.at("energy").get_object();
		components.energy.factor = boost::json::value_to<int>(energy_object.at("factor"));

		boost::json::object fuel_object = components_object.at("fuel").get_object();
		components.fuel.afterburner_fuel_usage = boost::json::value_to<double>(fuel_object.at("afterburner_fuel_usage"));
		components.fuel.deuterium_relative_efficiency_lithium = boost::json::value_to<double>(fuel_object.at("deuterium_relative_efficiency_lithium"));
		components.fuel.ecm_energy_cost = boost::json::value_to<double>(fuel_object.at("ecm_energy_cost"));
		components.fuel.fmec_factor = boost::json::value_to<double>(fuel_object.at("fmec_factor"));
		components.fuel.fuel_efficiency = boost::json::value_to<double>(fuel_object.at("fuel_efficiency"));
		components.fuel.fuel_equals_warp = boost::json::value_to<bool>(fuel_object.at("fuel_equals_warp"));
		components.fuel.factor = boost::json::value_to<int>(fuel_object.at("factor"));
		components.fuel.megajoules_factor = boost::json::value_to<double>(fuel_object.at("megajoules_factor"));
		components.fuel.min_reactor_efficiency = boost::json::value_to<double>(fuel_object.at("min_reactor_efficiency"));
		components.fuel.no_fuel_afterburn = boost::json::value_to<double>(fuel_object.at("no_fuel_afterburn"));
		components.fuel.no_fuel_thrust = boost::json::value_to<double>(fuel_object.at("no_fuel_thrust"));
		components.fuel.normal_fuel_usage = boost::json::value_to<double>(fuel_object.at("normal_fuel_usage"));
		components.fuel.reactor_idle_efficiency = boost::json::value_to<double>(fuel_object.at("reactor_idle_efficiency"));
		components.fuel.reactor_uses_fuel = boost::json::value_to<bool>(fuel_object.at("reactor_uses_fuel"));
		components.fuel.variable_fuel_consumption = boost::json::value_to<bool>(fuel_object.at("variable_fuel_consumption"));
		components.fuel.vsd_mj_yield = boost::json::value_to<double>(fuel_object.at("vsd_mj_yield"));

		boost::json::object ftl_drive_object = components_object.at("ftl_drive").get_object();
		components.ftl_drive.factor = boost::json::value_to<double>(ftl_drive_object.at("factor"));

		boost::json::object ftl_energy_object = components_object.at("ftl_energy").get_object();
		components.ftl_energy.factor = boost::json::value_to<double>(ftl_energy_object.at("factor"));

		boost::json::object jump_drive_object = components_object.at("jump_drive").get_object();
		components.jump_drive.factor = boost::json::value_to<double>(jump_drive_object.at("factor"));

		boost::json::object reactor_object = components_object.at("reactor").get_object();
		components.reactor.factor = boost::json::value_to<double>(reactor_object.at("factor"));

		boost::json::object constants_object = root_object.at("constants").get_object();
		constants.megajoules_multiplier = boost::json::value_to<int>(constants_object.at("megajoules_multiplier"));
		constants.kilo = boost::json::value_to<int>(constants_object.at("kilo"));
		constants.kj_per_unit_damage = boost::json::value_to<int>(constants_object.at("kj_per_unit_damage"));

		boost::json::object data_object = root_object.at("data").get_object();
		data.master_part_list = boost::json::value_to<std::string>(data_object.at("master_part_list"));
		data.using_templates = boost::json::value_to<bool>(data_object.at("using_templates"));

		boost::json::object game_start_object = root_object.at("game_start").get_object();
		game_start.default_mission = boost::json::value_to<std::string>(game_start_object.at("default_mission"));
		game_start.introduction = boost::json::value_to<std::string>(game_start_object.at("introduction"));
		game_start.galaxy = boost::json::value_to<std::string>(game_start_object.at("galaxy"));

		boost::json::object advanced_object = root_object.at("advanced").get_object();

		boost::json::object ai_object = root_object.at("ai").get_object();
		ai.always_obedient = boost::json::value_to<bool>(ai_object.at("always_obedient"));
		ai.assist_friend_in_need = boost::json::value_to<bool>(ai_object.at("assist_friend_in_need"));
		ai.ease_to_anger = boost::json::value_to<double>(ai_object.at("ease_to_anger"));
		ai.ease_to_appease = boost::json::value_to<double>(ai_object.at("ease_to_appease"));
		ai.friend_factor = boost::json::value_to<double>(ai_object.at("friend_factor"));
		ai.hull_damage_anger = boost::json::value_to<int>(ai_object.at("hull_damage_anger"));
		ai.hull_percent_for_comm = boost::json::value_to<double>(ai_object.at("hull_percent_for_comm"));
		ai.kill_factor = boost::json::value_to<double>(ai_object.at("kill_factor"));
		ai.lowest_negative_comm_choice = boost::json::value_to<double>(ai_object.at("lowest_negative_comm_choice"));
		ai.lowest_positive_comm_choice = boost::json::value_to<double>(ai_object.at("lowest_positive_comm_choice"));
		ai.min_relationship = boost::json::value_to<double>(ai_object.at("min_relationship"));
		ai.mood_swing_level = boost::json::value_to<double>(ai_object.at("mood_swing_level"));
		ai.random_response_range = boost::json::value_to<double>(ai_object.at("random_response_range"));
		ai.shield_damage_anger = boost::json::value_to<int>(ai_object.at("shield_damage_anger"));
		ai.jump_without_energy = boost::json::value_to<bool>(ai_object.at("jump_without_energy"));

		boost::json::object firing_object = ai_object.at("firing").get_object();
		ai.firing.missile_probability = boost::json::value_to<double>(firing_object.at("missile_probability"));
		ai.firing.aggressivity = boost::json::value_to<double>(firing_object.at("aggressivity"));

		boost::json::object targeting_object = ai_object.at("targeting").get_object();
		ai.targeting.escort_distance = boost::json::value_to<double>(targeting_object.at("escort_distance"));
		ai.targeting.turn_leader_distance = boost::json::value_to<double>(targeting_object.at("turn_leader_distance"));
		ai.targeting.time_to_recommand_wing = boost::json::value_to<double>(targeting_object.at("time_to_recommand_wing"));
		ai.targeting.min_time_to_switch_targets = boost::json::value_to<double>(targeting_object.at("min_time_to_switch_targets"));

		boost::json::object audio_object = root_object.at("audio").get_object();
		audio.afterburner_gain = boost::json::value_to<double>(audio_object.at("afterburner_gain"));
		audio.ai_high_quality_weapon = boost::json::value_to<bool>(audio_object.at("ai_high_quality_weapon"));
		audio.ai_sound = boost::json::value_to<bool>(audio_object.at("ai_sound"));
		audio.audio_max_distance = boost::json::value_to<double>(audio_object.at("audio_max_distance"));
		audio.audio_ref_distance = boost::json::value_to<double>(audio_object.at("audio_ref_distance"));
		audio.automatic_docking_zone = boost::json::value_to<std::string>(audio_object.at("automatic_docking_zone"));
		audio.battle_playlist = boost::json::value_to<std::string>(audio_object.at("battle_playlist"));
		audio.buzzing_distance = boost::json::value_to<double>(audio_object.at("buzzing_distance"));
		audio.buzzing_needs_afterburner = boost::json::value_to<bool>(audio_object.at("buzzing_needs_afterburner"));
		audio.buzzing_time = boost::json::value_to<double>(audio_object.at("buzzing_time"));
		audio.cache_songs = boost::json::value_to<std::string>(audio_object.at("cache_songs"));
		audio.cross_fade_music = boost::json::value_to<bool>(audio_object.at("cross_fade_music"));
		audio.dj_script = boost::json::value_to<std::string>(audio_object.at("dj_script"));
		audio.doppler = boost::json::value_to<bool>(audio_object.at("doppler"));
		audio.doppler_scale = boost::json::value_to<double>(audio_object.at("doppler_scale"));
		audio.every_other_mount = boost::json::value_to<bool>(audio_object.at("every_other_mount"));
		audio.explosion_closeness = boost::json::value_to<double>(audio_object.at("explosion_closeness"));
		audio.exterior_weapon_gain = boost::json::value_to<double>(audio_object.at("exterior_weapon_gain"));
		audio.frequency = boost::json::value_to<int>(audio_object.at("frequency"));
		audio.high_quality_weapon = boost::json::value_to<bool>(audio_object.at("high_quality_weapon"));
		audio.loading_sound = boost::json::value_to<std::string>(audio_object.at("loading_sound"));
		audio.loss_playlist = boost::json::value_to<std::string>(audio_object.at("loss_playlist"));
		audio.loss_relationship = boost::json::value_to<double>(audio_object.at("loss_relationship"));
		audio.max_single_sounds = boost::json::value_to<int>(audio_object.at("max_single_sounds"));
		audio.max_total_sounds = boost::json::value_to<int>(audio_object.at("max_total_sounds"));
		audio.max_range_to_hear_weapon_fire = boost::json::value_to<double>(audio_object.at("max_range_to_hear_weapon_fire"));
		audio.min_weapon_sound_refire = boost::json::value_to<double>(audio_object.at("min_weapon_sound_refire"));
		audio.mission_victory_song = boost::json::value_to<std::string>(audio_object.at("mission_victory_song"));
		audio.music = boost::json::value_to<bool>(audio_object.at("music"));
		audio.music_layers = boost::json::value_to<int>(audio_object.at("music_layers"));
		audio.music_muting_fade_in = boost::json::value_to<double>(audio_object.at("music_muting_fade_in"));
		audio.music_muting_fade_out = boost::json::value_to<double>(audio_object.at("music_muting_fade_out"));
		audio.music_volume = boost::json::value_to<double>(audio_object.at("music_volume"));
		audio.music_volume_down_latency = boost::json::value_to<double>(audio_object.at("music_volume_down_latency"));
		audio.music_volume_up_latency = boost::json::value_to<double>(audio_object.at("music_volume_up_latency"));
		audio.news_song = boost::json::value_to<std::string>(audio_object.at("news_song"));
		audio.panic_playlist = boost::json::value_to<std::string>(audio_object.at("panic_playlist"));
		audio.peace_playlist = boost::json::value_to<std::string>(audio_object.at("peace_playlist"));
		audio.positional = boost::json::value_to<bool>(audio_object.at("positional"));
		audio.shuffle_songs = boost::json::value_to<bool>(audio_object.at("shuffle_songs"));

		boost::json::object shuffle_songs_section_object = audio_object.at("shuffle_songs_section").get_object();
		audio.shuffle_songs_section.clear_history_on_list_change = boost::json::value_to<bool>(shuffle_songs_section_object.at("clear_history_on_list_change"));
		audio.shuffle_songs_section.history_depth = boost::json::value_to<int>(shuffle_songs_section_object.at("history_depth"));
		audio.sound = boost::json::value_to<bool>(audio_object.at("sound"));
		audio.sound_gain = boost::json::value_to<double>(audio_object.at("sound_gain"));
		audio.thread_time = boost::json::value_to<int>(audio_object.at("thread_time"));
		audio.time_between_music = boost::json::value_to<double>(audio_object.at("time_between_music"));
		audio.victory_playlist = boost::json::value_to<std::string>(audio_object.at("victory_playlist"));
		audio.victory_relationship = boost::json::value_to<double>(audio_object.at("victory_relationship"));
		audio.volume = boost::json::value_to<double>(audio_object.at("volume"));
		audio.weapon_gain = boost::json::value_to<double>(audio_object.at("weapon_gain"));

		boost::json::object cockpit_audio_object = root_object.at("cockpit_audio").get_object();
		cockpit_audio.automatic_landing_zone = boost::json::value_to<std::string>(cockpit_audio_object.at("automatic_landing_zone"));
		cockpit_audio.automatic_landing_zone1 = boost::json::value_to<std::string>(cockpit_audio_object.at("automatic_landing_zone1"));
		cockpit_audio.automatic_landing_zone2 = boost::json::value_to<std::string>(cockpit_audio_object.at("automatic_landing_zone2"));
		cockpit_audio.autopilot_available = boost::json::value_to<std::string>(cockpit_audio_object.at("autopilot_available"));
		cockpit_audio.autopilot_disabled = boost::json::value_to<std::string>(cockpit_audio_object.at("autopilot_disabled"));
		cockpit_audio.autopilot_enabled = boost::json::value_to<std::string>(cockpit_audio_object.at("autopilot_enabled"));
		cockpit_audio.autopilot_unavailable = boost::json::value_to<std::string>(cockpit_audio_object.at("autopilot_unavailable"));
		cockpit_audio.comm = boost::json::value_to<std::string>(cockpit_audio_object.at("comm"));
		cockpit_audio.comm_preload = boost::json::value_to<bool>(cockpit_audio_object.at("comm_preload"));
		cockpit_audio.compress_change = boost::json::value_to<std::string>(cockpit_audio_object.at("compress_change"));
		cockpit_audio.compress_interval = boost::json::value_to<int>(cockpit_audio_object.at("compress_interval"));
		cockpit_audio.compress_loop = boost::json::value_to<std::string>(cockpit_audio_object.at("compress_loop"));
		cockpit_audio.compress_max = boost::json::value_to<int>(cockpit_audio_object.at("compress_max"));
		cockpit_audio.compress_stop = boost::json::value_to<std::string>(cockpit_audio_object.at("compress_stop"));
		cockpit_audio.docking_complete = boost::json::value_to<std::string>(cockpit_audio_object.at("docking_complete"));
		cockpit_audio.docking_denied = boost::json::value_to<std::string>(cockpit_audio_object.at("docking_denied"));
		cockpit_audio.docking_failed = boost::json::value_to<std::string>(cockpit_audio_object.at("docking_failed"));
		cockpit_audio.docking_granted = boost::json::value_to<std::string>(cockpit_audio_object.at("docking_granted"));
		cockpit_audio.examine = boost::json::value_to<std::string>(cockpit_audio_object.at("examine"));
		cockpit_audio.jump_engaged = boost::json::value_to<std::string>(cockpit_audio_object.at("jump_engaged"));
		cockpit_audio.manifest = boost::json::value_to<std::string>(cockpit_audio_object.at("manifest"));
		cockpit_audio.missile_switch = boost::json::value_to<std::string>(cockpit_audio_object.at("missile_switch"));
		cockpit_audio.objective = boost::json::value_to<std::string>(cockpit_audio_object.at("objective"));
		cockpit_audio.overload = boost::json::value_to<std::string>(cockpit_audio_object.at("overload"));
		cockpit_audio.overload_stopped = boost::json::value_to<std::string>(cockpit_audio_object.at("overload_stopped"));
		cockpit_audio.repair = boost::json::value_to<std::string>(cockpit_audio_object.at("repair"));
		cockpit_audio.scanning = boost::json::value_to<std::string>(cockpit_audio_object.at("scanning"));
		cockpit_audio.shield = boost::json::value_to<std::string>(cockpit_audio_object.at("shield"));
		cockpit_audio.sounds_extension_1 = boost::json::value_to<std::string>(cockpit_audio_object.at("sounds_extension_1"));
		cockpit_audio.sounds_extension_2 = boost::json::value_to<std::string>(cockpit_audio_object.at("sounds_extension_2"));
		cockpit_audio.sounds_extension_3 = boost::json::value_to<std::string>(cockpit_audio_object.at("sounds_extension_3"));
		cockpit_audio.sounds_extension_4 = boost::json::value_to<std::string>(cockpit_audio_object.at("sounds_extension_4"));
		cockpit_audio.sounds_extension_5 = boost::json::value_to<std::string>(cockpit_audio_object.at("sounds_extension_5"));
		cockpit_audio.sounds_extension_6 = boost::json::value_to<std::string>(cockpit_audio_object.at("sounds_extension_6"));
		cockpit_audio.sounds_extension_7 = boost::json::value_to<std::string>(cockpit_audio_object.at("sounds_extension_7"));
		cockpit_audio.sounds_extension_8 = boost::json::value_to<std::string>(cockpit_audio_object.at("sounds_extension_8"));
		cockpit_audio.sounds_extension_9 = boost::json::value_to<std::string>(cockpit_audio_object.at("sounds_extension_9"));
		cockpit_audio.target = boost::json::value_to<std::string>(cockpit_audio_object.at("target"));
		cockpit_audio.target_reverse = boost::json::value_to<std::string>(cockpit_audio_object.at("target_reverse"));
		cockpit_audio.undocking_complete = boost::json::value_to<std::string>(cockpit_audio_object.at("undocking_complete"));
		cockpit_audio.undocking_failed = boost::json::value_to<std::string>(cockpit_audio_object.at("undocking_failed"));
		cockpit_audio.vdu_static = boost::json::value_to<std::string>(cockpit_audio_object.at("vdu_static"));
		cockpit_audio.view = boost::json::value_to<std::string>(cockpit_audio_object.at("view"));
		cockpit_audio.weapon_switch = boost::json::value_to<std::string>(cockpit_audio_object.at("weapon_switch"));

		boost::json::object collision_hacks_object = root_object.at("collision_hacks").get_object();
		collision_hacks.collision_hack_distance = boost::json::value_to<double>(collision_hacks_object.at("collision_hack_distance"));
		collision_hacks.collision_damage_to_ai = boost::json::value_to<bool>(collision_hacks_object.at("collision_damage_to_ai"));
		collision_hacks.crash_dock_hangar = boost::json::value_to<bool>(collision_hacks_object.at("crash_dock_hangar"));
		collision_hacks.crash_dock_unit = boost::json::value_to<bool>(collision_hacks_object.at("crash_dock_unit"));
		collision_hacks.front_collision_hack_angle = boost::json::value_to<double>(collision_hacks_object.at("front_collision_hack_angle"));
		collision_hacks.front_collision_hack_distance = boost::json::value_to<double>(collision_hacks_object.at("front_collision_hack_distance"));
		collision_hacks.cargo_deals_collide_damage = boost::json::value_to<bool>(collision_hacks_object.at("cargo_deals_collide_damage"));

		boost::json::object logging_object = root_object.at("logging").get_object();
		logging.vsdebug = boost::json::value_to<int>(logging_object.at("vsdebug"));
		logging.verbose_debug = boost::json::value_to<bool>(logging_object.at("verbose_debug"));

		boost::json::object physics_object = root_object.at("physics").get_object();
		physics.capship_size = boost::json::value_to<double>(physics_object.at("capship_size"));
		physics.close_enough_to_autotrack = boost::json::value_to<double>(physics_object.at("close_enough_to_autotrack"));
		physics.collision_scale_factor = boost::json::value_to<double>(physics_object.at("collision_scale_factor"));
		physics.debris_time = boost::json::value_to<double>(physics_object.at("debris_time"));
		physics.default_shield_tightness = boost::json::value_to<double>(physics_object.at("default_shield_tightness"));
		physics.density_of_jump_point = boost::json::value_to<double>(physics_object.at("density_of_jump_point"));
		physics.density_of_rock = boost::json::value_to<double>(physics_object.at("density_of_rock"));
		physics.difficulty_based_enemy_damage = boost::json::value_to<bool>(physics_object.at("difficulty_based_enemy_damage"));
		physics.difficulty_speed_exponent = boost::json::value_to<double>(physics_object.at("difficulty_speed_exponent"));
		physics.difficulty_based_shield_recharge = boost::json::value_to<bool>(physics_object.at("difficulty_based_shield_recharge"));
		physics.distance_to_warp = boost::json::value_to<double>(physics_object.at("distance_to_warp"));
		physics.does_missile_bounce = boost::json::value_to<bool>(physics_object.at("does_missile_bounce"));
		physics.engine_energy_takes_priority = boost::json::value_to<bool>(physics_object.at("engine_energy_takes_priority"));
		physics.friendly_auto_radius = boost::json::value_to<double>(physics_object.at("friendly_auto_radius"));
		physics.game_accel = boost::json::value_to<double>(physics_object.at("game_accel"));
		physics.game_speed = boost::json::value_to<double>(physics_object.at("game_speed"));
		physics.hostile_auto_radius = boost::json::value_to<double>(physics_object.at("hostile_auto_radius"));
		physics.indiscriminate_system_destruction = boost::json::value_to<double>(physics_object.at("indiscriminate_system_destruction"));
		physics.inelastic_scale = boost::json::value_to<double>(physics_object.at("inelastic_scale"));
		physics.jump_mesh_radius_scale = boost::json::value_to<double>(physics_object.at("jump_mesh_radius_scale"));
		physics.max_damage = boost::json::value_to<double>(physics_object.at("max_damage"));
		physics.max_ecm = boost::json::value_to<int>(physics_object.at("max_ecm"));
		physics.max_force_multiplier = boost::json::value_to<double>(physics_object.at("max_force_multiplier"));
		physics.max_lost_target_live_time = boost::json::value_to<double>(physics_object.at("max_lost_target_live_time"));
		physics.max_non_player_rotation_rate = boost::json::value_to<double>(physics_object.at("max_non_player_rotation_rate"));
		physics.max_player_rotation_rate = boost::json::value_to<double>(physics_object.at("max_player_rotation_rate"));
		physics.max_radar_cone_damage = boost::json::value_to<double>(physics_object.at("max_radar_cone_damage"));
		physics.max_radar_lock_cone_damage = boost::json::value_to<double>(physics_object.at("max_radar_lock_cone_damage"));
		physics.max_radar_track_cone_damage = boost::json::value_to<double>(physics_object.at("max_radar_track_cone_damage"));
		physics.max_shield_lowers_capacitance = boost::json::value_to<bool>(physics_object.at("max_shield_lowers_capacitance"));
		physics.max_torque_multiplier = boost::json::value_to<double>(physics_object.at("max_torque_multiplier"));
		physics.min_asteroid_distance = boost::json::value_to<double>(physics_object.at("min_asteroid_distance"));
		physics.min_damage = boost::json::value_to<double>(physics_object.at("min_damage"));
		physics.min_shield_speeding_discharge = boost::json::value_to<double>(physics_object.at("min_shield_speeding_discharge"));
		physics.minimum_mass = boost::json::value_to<double>(physics_object.at("minimum_mass"));
		physics.minimum_time = boost::json::value_to<double>(physics_object.at("minimum_time"));
		physics.near_autotrack_cone = boost::json::value_to<double>(physics_object.at("near_autotrack_cone"));
		physics.nebula_shield_recharge = boost::json::value_to<double>(physics_object.at("nebula_shield_recharge"));
		physics.neutral_auto_radius = boost::json::value_to<double>(physics_object.at("neutral_auto_radius"));
		physics.no_damage_to_docked_ships = boost::json::value_to<bool>(physics_object.at("no_damage_to_docked_ships"));
		physics.no_spec_jump = boost::json::value_to<bool>(physics_object.at("no_spec_jump"));
		physics.no_unit_collisions = boost::json::value_to<bool>(physics_object.at("no_unit_collisions"));
		physics.percent_missile_match_target_velocity = boost::json::value_to<double>(physics_object.at("percent_missile_match_target_velocity"));
		physics.planet_dock_min_port_size = boost::json::value_to<double>(physics_object.at("planet_dock_min_port_size"));
		physics.planet_dock_port_size = boost::json::value_to<double>(physics_object.at("planet_dock_port_size"));
		physics.planets_always_neutral = boost::json::value_to<bool>(physics_object.at("planets_always_neutral"));
		physics.planets_can_have_subunits = boost::json::value_to<bool>(physics_object.at("planets_can_have_subunits"));
		physics.separate_system_flakiness_component = boost::json::value_to<bool>(physics_object.at("separate_system_flakiness_component"));
		physics.shield_energy_capacitance = boost::json::value_to<double>(physics_object.at("shield_energy_capacitance"));
		physics.shield_maintenance_charge = boost::json::value_to<double>(physics_object.at("shield_maintenance_charge"));
		physics.shields_in_spec = boost::json::value_to<bool>(physics_object.at("shields_in_spec"));
		physics.speeding_discharge = boost::json::value_to<double>(physics_object.at("speeding_discharge"));
		physics.steady_itts = boost::json::value_to<bool>(physics_object.at("steady_itts"));
		physics.system_damage_on_armor = boost::json::value_to<bool>(physics_object.at("system_damage_on_armor"));
		physics.target_distance_to_warp_bonus = boost::json::value_to<double>(physics_object.at("target_distance_to_warp_bonus"));
		physics.thruster_hit_chance = boost::json::value_to<double>(physics_object.at("thruster_hit_chance"));
		physics.use_max_shield_energy_usage = boost::json::value_to<bool>(physics_object.at("use_max_shield_energy_usage"));
		physics.velocity_max = boost::json::value_to<double>(physics_object.at("velocity_max"));

		boost::json::object ejection_object = physics_object.at("ejection").get_object();
		physics.ejection.auto_eject_percent = boost::json::value_to<double>(ejection_object.at("auto_eject_percent"));
		physics.ejection.eject_cargo_percent = boost::json::value_to<double>(ejection_object.at("eject_cargo_percent"));
		physics.ejection.max_dumped_cargo = boost::json::value_to<int>(ejection_object.at("max_dumped_cargo"));
		physics.ejection.hull_damage_to_eject = boost::json::value_to<double>(ejection_object.at("hull_damage_to_eject"));
		physics.ejection.player_auto_eject = boost::json::value_to<bool>(ejection_object.at("player_auto_eject"));

		boost::json::object unit_object = root_object.at("unit").get_object();
		unit.default_aggressivity = boost::json::value_to<double>(unit_object.at("default_aggressivity"));

		boost::json::object warp_object = root_object.at("warp").get_object();
		warp.bleed_factor = boost::json::value_to<double>(warp_object.at("bleed_factor"));
		warp.computer_warp_ramp_up_time = boost::json::value_to<double>(warp_object.at("computer_warp_ramp_up_time"));
		warp.in_system_jump_cost = boost::json::value_to<double>(warp_object.at("in_system_jump_cost"));
		warp.max_effective_velocity = boost::json::value_to<double>(warp_object.at("max_effective_velocity"));
		warp.player_warp_energy_multiplier = boost::json::value_to<double>(warp_object.at("player_warp_energy_multiplier"));
		warp.use_warp_energy_for_cloak = boost::json::value_to<bool>(warp_object.at("use_warp_energy_for_cloak"));
		warp.warp_energy_multiplier = boost::json::value_to<double>(warp_object.at("warp_energy_multiplier"));
		warp.warp_memory_effect = boost::json::value_to<double>(warp_object.at("warp_memory_effect"));
		warp.warp_multiplier_max = boost::json::value_to<double>(warp_object.at("warp_multiplier_max"));
		warp.warp_multiplier_min = boost::json::value_to<double>(warp_object.at("warp_multiplier_min"));
		warp.warp_ramp_down_time = boost::json::value_to<double>(warp_object.at("warp_ramp_down_time"));
		warp.warp_ramp_up_time = boost::json::value_to<double>(warp_object.at("warp_ramp_up_time"));
		warp.warp_stretch_cutoff = boost::json::value_to<double>(warp_object.at("warp_stretch_cutoff"));
		warp.warp_stretch_decel_cutoff = boost::json::value_to<double>(warp_object.at("warp_stretch_decel_cutoff"));

		boost::json::object weapons_object = root_object.at("weapons").get_object();
		weapons.can_fire_in_cloak = boost::json::value_to<bool>(weapons_object.at("can_fire_in_cloak"));
		weapons.can_fire_in_spec = boost::json::value_to<bool>(weapons_object.at("can_fire_in_spec"));

    }
    catch (std::exception const& e)
    {
        std::cerr << e.what() << std::endl;
    }
}
