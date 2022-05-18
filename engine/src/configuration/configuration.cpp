/*
 * configuration.cpp
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


#include "configuration.h"
#include "game_config.h"
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <math.h>

Configuration::Configuration() {
    //logging.verbose_debug = GameConfig::GetVariable("data.verbose_debug", false);
}

/* Override the default value(provided by constructor) with the value from the user specified configuration file, if any.
* Behavior is undefined and erroneous if no default is provided in the constructor!
* Descriptions/invariants for each variable are/will be provided in the header file and are not repeated here, except where
* behavior/range bounding is complicated/necessary-for-stability abd additional validation code is required for checking/adjusting user inputs
*/
void Configuration::OverrideDefaultsWithUserConfiguration() {
    general_config_.pitch = GameConfig::GetVariable("general.pitch", 0.0F);
    general_config_.yaw = GameConfig::GetVariable("general.yaw", 0.0F);
    general_config_.roll = GameConfig::GetVariable("general.roll", 0.0F);

    data_config_.master_part_list = GameConfig::GetString("data.master_part_list", "master_part_list");
    data_config_.using_templates = GameConfig::GetVariable("data.usingtemplates", true);

    ai.assist_friend_in_need                            = GameConfig::GetVariable("AI.assist_friend_in_need", true);
    ai.ease_to_anger                                    = GameConfig::GetVariable("AI.EaseToAnger", -0.5F);
    ai.ease_to_appease                                  = GameConfig::GetVariable("AI.EaseToAppease", 0.5F);
    ai.hull_damage_anger                                = GameConfig::GetVariable("AI.HullDamageAnger", 10);
    ai.hull_percent_for_comm                            = GameConfig::GetVariable("AI.HullPercentForComm", 0.75F);
    ai.lowest_negative_comm_choice                      = GameConfig::GetVariable("AI.LowestNegativeCommChoice", -0.00001F);
    ai.lowest_positive_comm_choice                      = GameConfig::GetVariable("AI.LowestPositiveCommChoice", 0.0F);
    ai.mood_swing_level                                 = GameConfig::GetVariable("AI.MoodSwingLevel", 0.2F);
    ai.random_response_range                            = GameConfig::GetVariable("AI.RandomResponseRange", 0.8F);
    ai.shield_damage_anger                              = GameConfig::GetVariable("AI.ShieldDamageAnger", 1);
    ai.jump_without_energy = GameConfig::GetVariable("AI.jump_without_energy", false);
    ai.friend_factor = -GameConfig::GetVariable("AI.friend_factor", 0.1F);
    ai.kill_factor = -GameConfig::GetVariable("AI.kill_factor", 0.2F);

    audio_config_.every_other_mount = GameConfig::GetVariable("audio.every_other_mount", false);
    audio_config_.shuffle_songs.clear_history_on_list_change = GameConfig::GetVariable("audio.shuffle_songs.clear_history_on_list_change", false);

    // collision_hacks substruct
    collision_hacks.collision_hack_distance             = GameConfig::GetVariable("physics.collision_avoidance_hack_distance", collision_hacks.collision_hack_distance);
    collision_hacks.collision_damage_to_ai              = GameConfig::GetVariable("physics.collisionDamageToAI", collision_hacks.collision_damage_to_ai);
    collision_hacks.crash_dock_hangar                   = GameConfig::GetVariable("physics.only_hangar_collision_docks", collision_hacks.crash_dock_hangar);
    collision_hacks.crash_dock_unit                     = GameConfig::GetVariable("physics.unit_collision_docks", collision_hacks.crash_dock_unit);
    // (STYLE WARNING) front collision_hack_angle appears to expect to equal the cosine of a radian value, but default implies that it reads degrees! Value not specified directly, breaks pattern, should consider normalizing behavior with other config vars
    collision_hacks.front_collision_hack_angle = std::cos(
            M_PI * GameConfig::GetVariable("physics.front_collision_avoidance_hack_angle", 40.0)
                    / 180.0); // uses default value - must be changed in tandem with constructor!
    collision_hacks.front_collision_hack_distance = GameConfig::GetVariable("physics.front_collision_avoidance_hack_distance",
            collision_hacks.front_collision_hack_distance);
    collision_hacks.cargo_deals_collide_damage = GameConfig::GetVariable("physics.cargo_deals_collide_damage",
            collision_hacks.cargo_deals_collide_damage);

    // computer substruct
    computer.default_lock_cone = GameConfig::GetVariable("physics.lock_cone", computer.default_lock_cone);
    computer.default_max_range = GameConfig::GetVariable("graphics.hud.radarRange", computer.default_max_range);
    computer.default_tracking_cone = GameConfig::GetVariable("physics.autotracking", computer.default_tracking_cone);

    // fuel substruct
    fuel.afterburner_fuel_usage =
            GameConfig::GetVariable("physics.AfterburnerFuelUsage", fuel.afterburner_fuel_usage);
    fuel.fmec_exit_velocity_inverse =
            GameConfig::GetVariable("physics.FMEC_exit_vel", fuel.fmec_exit_velocity_inverse);
    fuel.fuel_efficiency =
            GameConfig::GetVariable("physics.LithiumRelativeEfficiency_Lithium", fuel.fuel_efficiency);
    fuel.fuel_equals_warp = GameConfig::GetVariable("physics.fuel_equals_warp", fuel.fuel_equals_warp);
    fuel.normal_fuel_usage = GameConfig::GetVariable("physics.FuelUsage", fuel.normal_fuel_usage);
    fuel.reactor_uses_fuel = GameConfig::GetVariable("physics.reactor_uses_fuel", fuel.reactor_uses_fuel);

    // graphics substruct
    graphics_config_.automatic_landing_zone_warning = GameConfig::GetString("graphics.automatic_landing_zone_warning", "comm_docking.ani");
    graphics_config_.automatic_landing_zone_warning1 = GameConfig::GetString("graphics.automatic_landing_zone_warning1", "comm_docking.ani");
    graphics_config_.automatic_landing_zone_warning2 = GameConfig::GetString("graphics.automatic_landing_zone_warning2", "comm_docking.ani");
    graphics_config_.automatic_landing_zone_warning_text = GameConfig::GetEscapedString("graphics.automatic_landing_zone_warning_text", "Now Entering an \"Automatic Landing Zone\".");
    graphics_config_.draw_heading_marker = GameConfig::GetVariable("graphics.draw_heading_marker", false);
    graphics_config_.missile_explosion_radius_mult = GameConfig::GetVariable("graphics.missile_explosion_radius_mult", 1.0F);
    graphics_config_.missile_sparkle = GameConfig::GetVariable("graphics.missilesparkle", false);
    // Sic - Should be emissive, I think
    graphics_config_.atmosphere_emmissive = GameConfig::GetVariable("graphics.atmosphere_emmissive", 1.0F);
    graphics_config_.atmosphere_diffuse = GameConfig::GetVariable("graphics.atmosphere_diffuse", 1.0F);
    graphics_config_.default_engine_activation = GameConfig::GetVariable("graphics.default_engine_activation", 0.00048828125F);
    graphics_config_.explosion_animation = GameConfig::GetString("graphics.explosion_animation", "explosion_orange.ani");
    graphics_config_.shield_detail = GameConfig::GetVariable("graphics.shield_detail", 16);
    graphics_config_.shield_texture = GameConfig::GetString("graphics.shield_texture", "shield.bmp");
    graphics_config_.shield_technique = GameConfig::GetString("graphics.shield_technique", "");

    graphics_config_.glow_flicker.flicker_time = GameConfig::GetVariable("graphics.glowflicker.time", 30.0F);
    graphics_config_.glow_flicker.flicker_off_time = GameConfig::GetVariable("graphics.glowflicker.off-time", 2.0F);
    graphics_config_.glow_flicker.hull_for_total_dark = GameConfig::GetVariable("graphics.glowflicker.hull-for-total-dark", 0.04F);
    graphics_config_.glow_flicker.num_times_per_second_on = GameConfig::GetVariable("graphics.glowflicker.num-times-per-second-on", 0.66F);
    graphics_config_.glow_flicker.min_flicker_cycle = GameConfig::GetVariable("graphics.glowflicker.min-cycle", 2.0F);

    // graphics ==> hud subsection
    graphics_config_.hud.draw_rendered_crosshairs = GameConfig::GetVariable("graphics.hud.draw_rendered_crosshairs", GameConfig::GetVariable("graphics.draw_rendered_crosshairs", true));
    graphics_config_.hud.already_near_message = GameConfig::GetEscapedString("graphics.hud.AlreadyNearMessage", "#ff0000Already Near#000000");
    graphics_config_.hud.armor_hull_size = GameConfig::GetVariable("graphics.hud.armor_hull_size", 0.55F);
    graphics_config_.hud.asteroids_near_message = GameConfig::GetEscapedString("graphics.hud.AsteroidsNearMessage", "#ff0000Asteroids Near#000000");
    graphics_config_.hud.basename_colon_basename = GameConfig::GetVariable("graphics.hud.basename:basename", true);
    graphics_config_.hud.box_line_thickness = GameConfig::GetVariable("graphics.hud.BoxLineThickness", 1.0F);
    graphics_config_.hud.completed_objectives_last = GameConfig::GetVariable("graphics.hud.completed_objectives_last", true);
    graphics_config_.hud.crosshairs_on_chase_cam = GameConfig::GetVariable("graphics.hud.crosshairs_on_chasecam", false);
    graphics_config_.hud.crosshairs_on_padlock = GameConfig::GetVariable("graphics.hud.crosshairs_on_padlock", false);
    graphics_config_.hud.damage_report_heading = GameConfig::GetEscapedString("graphics.hud.damage_report_heading", "#00ff00DAMAGE REPORT\\n\\n");
    graphics_config_.hud.debug_position = GameConfig::GetVariable("graphics.hud.debug_position", false);
    graphics_config_.hud.diamond_line_thickness = GameConfig::GetVariable("graphics.hud.DiamondLineThickness", 1.0F);
    graphics_config_.hud.diamond_rotation_speed = GameConfig::GetVariable("graphics.hud.DiamondRotationSpeed", 1.0F);
    graphics_config_.hud.diamond_size = GameConfig::GetVariable("graphics.hud.DiamondSize", 2.05F);
    graphics_config_.hud.display_relative_velocity = GameConfig::GetVariable("graphics.hud.display_relative_velocity", true);
    graphics_config_.hud.display_warp_energy_if_no_jump_drive = GameConfig::GetVariable("graphics.hud.display_warp_energy_if_no_jump_drive", true);
    graphics_config_.hud.draw_all_target_boxes = GameConfig::GetVariable("graphics.hud.drawAllTargetBoxes", false);
    graphics_config_.hud.draw_always_itts = GameConfig::GetVariable("graphics.hud.drawAlwaysITTS", false);
    graphics_config_.hud.draw_line_to_itts = GameConfig::GetVariable("graphics.hud.drawLineToITTS", false);
    graphics_config_.hud.draw_line_to_target = GameConfig::GetVariable("graphics.hud.drawLineToTarget", false);
    graphics_config_.hud.draw_line_to_targets_target = GameConfig::GetVariable("graphics.hud.drawLineToTargetsTarget", false);
    graphics_config_.hud.draw_nav_symbol = GameConfig::GetVariable("graphics.hud.drawNavSymbol", false);
    graphics_config_.hud.draw_tactical_target = GameConfig::GetVariable("graphics.hud.DrawTacticalTarget", false);
    graphics_config_.hud.draw_targeting_boxes = GameConfig::GetVariable("graphics.hud.DrawTargettingBoxes", true);
    graphics_config_.hud.draw_targeting_boxes_inside = GameConfig::GetVariable("graphics.hud.DrawTargettingBoxesInside", true);
    graphics_config_.hud.draw_arrow_on_chase_cam = GameConfig::GetVariable("graphics.hud.draw_arrow_on_chasecam", true);
    graphics_config_.hud.draw_arrow_on_pan_cam = GameConfig::GetVariable("graphics.hud.draw_arrow_on_pancam", false);
    graphics_config_.hud.draw_arrow_on_pan_target = GameConfig::GetVariable("graphics.hud.draw_arrow_on_pantgt", false);
    graphics_config_.hud.draw_arrow_to_target = GameConfig::GetVariable("graphics.hud.draw_arrow_to_target", true);
    graphics_config_.hud.draw_blips_on_both_radar = GameConfig::GetVariable("graphics.hud.draw_blips_on_both_radar", false);
    graphics_config_.hud.draw_damage_sprite = GameConfig::GetVariable("graphics.hud.draw_damage_sprite", true);
    graphics_config_.hud.draw_gravitational_objects = GameConfig::GetVariable("graphics.hud.draw_gravitational_objects", true);
    graphics_config_.hud.draw_nav_grid = GameConfig::GetVariable("graphics.hud.draw_nav_grid", true);
    graphics_config_.hud.draw_significant_blips = GameConfig::GetVariable("graphics.hud.draw_significant_blips", true);
    graphics_config_.hud.draw_star_direction = GameConfig::GetVariable("graphics.hud.draw_star_direction", true);
    graphics_config_.hud.draw_unit_on_chase_cam = GameConfig::GetVariable("graphics.hud.draw_unit_on_chasecam", true);
    graphics_config_.hud.draw_vdu_view_shields = GameConfig::GetVariable("graphics.hud.draw_vdu_view_shields", true);
    graphics_config_.hud.draw_weapon_sprite = GameConfig::GetVariable("graphics.hud.draw_weapon_sprite", false);
    graphics_config_.hud.enemy_near_message = GameConfig::GetEscapedString("graphics.hud.EnemyNearMessage", "#ff0000Enemy Near#000000");
    graphics_config_.hud.gun_list_columns = GameConfig::GetVariable("graphics.hud.gun_list_columns", 1);
    graphics_config_.hud.invert_friendly_shields = GameConfig::GetVariable("graphics.hud.invert_friendly_shields", false);
    graphics_config_.hud.invert_friendly_sprite = GameConfig::GetVariable("graphics.hud.invert_friendly_sprite", false);
    graphics_config_.hud.invert_target_shields = GameConfig::GetVariable("graphics.hud.invert_target_shields", false);
    graphics_config_.hud.invert_target_sprite = GameConfig::GetVariable("graphics.hud.invert_target_sprite", false);
    graphics_config_.hud.invert_view_shields = GameConfig::GetVariable("graphics.hud.invert_view_shields", false);
    graphics_config_.hud.itts_for_beams = GameConfig::GetVariable("graphics.hud.ITTSForBeams", false);
    graphics_config_.hud.itts_for_lockable = GameConfig::GetVariable("graphics.hud.ITTSForLockable", false);
    graphics_config_.hud.itts_line_to_mark_alpha = GameConfig::GetVariable("graphics.hud.ITTSLineToMarkAlpha", 0.1F);
    graphics_config_.hud.itts_use_average_gun_speed = GameConfig::GetVariable("graphics.hud.ITTSUseAverageGunSpeed", true);
    graphics_config_.hud.lock_center_crosshair = GameConfig::GetVariable("graphics.hud.LockCenterCrosshair", false);
    graphics_config_.hud.lock_confirm_line_length = GameConfig::GetVariable("graphics.hud.LockConfirmLineLength", 1.5F);
    graphics_config_.hud.manifest_heading = GameConfig::GetEscapedString("graphics.hud.manifest_heading", "Manifest\\n");
    graphics_config_.hud.max_missile_bracket_size = GameConfig::GetVariable("graphics.hud.MaxMissileBracketSize", 2.05F);
    graphics_config_.hud.message_prefix = GameConfig::GetEscapedString("graphics.hud.message_prefix", "");
    graphics_config_.hud.min_missile_bracket_size = GameConfig::GetVariable("graphics.hud.MinMissileBracketSize", 1.05F);
    graphics_config_.hud.min_lock_box_size = GameConfig::GetVariable("graphics.hud.min_lock_box_size", 0.001F);
    graphics_config_.hud.min_radar_blip_size = GameConfig::GetVariable("graphics.hud.min_radarblip_size", 0.0F);
    graphics_config_.hud.min_target_box_size = GameConfig::GetVariable("graphics.hud.min_target_box_size", 0.01F);
    graphics_config_.hud.mounts_list_empty = GameConfig::GetString("graphics.hud.mounts_list_empty", "");
    graphics_config_.hud.nav_cross_line_thickness = GameConfig::GetVariable("graphics.hud.NavCrossLineThickness", 1.0F);
    graphics_config_.hud.not_included_in_damage_report = GameConfig::GetString("graphics.hud.not_included_in_damage_report", "plasteel_hull tungsten_hull isometal_hull");
    graphics_config_.hud.out_of_cone_distance = GameConfig::GetVariable("graphics.hud.out_of_cone_distance", false);
    graphics_config_.hud.padlock_view_lag = GameConfig::GetVariable("graphics.hud.PadlockViewLag", 1.5F);
    graphics_config_.hud.padlock_view_lag_fix_zone = GameConfig::GetVariable("graphics.hud.PadlockViewLagFixZone", 0.0872F);
    graphics_config_.hud.planet_near_message = GameConfig::GetEscapedString("graphics.hud.PlanetNearMessage", "#ff0000Planetary Hazard Near#000000");
    graphics_config_.hud.print_damage_percent = GameConfig::GetVariable("graphics.hud.print_damage_percent", true);
    graphics_config_.hud.print_faction = GameConfig::GetVariable("graphics.hud.print_faction", true);
    graphics_config_.hud.print_fg_name = GameConfig::GetVariable("graphics.hud.print_fg_name", true);
    graphics_config_.hud.print_fg_sub_id = GameConfig::GetVariable("graphics.hud.print_fg_sub_id", false);
    graphics_config_.hud.print_request_docking = GameConfig::GetVariable("graphics.hud.print_request_docking", true);
    graphics_config_.hud.print_ship_type = GameConfig::GetVariable("graphics.hud.print_ship_type", true);
    graphics_config_.hud.projectile_means_missile = GameConfig::GetVariable("graphics.hud.projectile_means_missile", false);
    graphics_config_.hud.radar_range = GameConfig::GetVariable("graphics.hud.radarRange", 20000.0F);    // TODO: Deduplicate this with computer.default_max_range
    graphics_config_.hud.radar_type = GameConfig::GetString("graphics.hud.radarType", "WC");
    graphics_config_.hud.radar_search_extra_radius = GameConfig::GetVariable("graphics.hud.radar_search_extra_radius", 1000.0F);
    graphics_config_.hud.rotating_bracket_inner = GameConfig::GetVariable("graphics.hud.RotatingBracketInner", true);
    graphics_config_.hud.rotating_bracket_size = GameConfig::GetVariable("graphics.hud.RotatingBracketSize", 0.58F);
    graphics_config_.hud.rotating_bracket_width = GameConfig::GetVariable("graphics.hud.RotatingBracketWidth", 0.1F);
    graphics_config_.hud.scale_relationship_color = GameConfig::GetVariable("graphics.hud.scale_relationship_color", 10.0F);
    graphics_config_.hud.shield_vdu_fade = GameConfig::GetVariable("graphics.hud.shield_vdu_fade", true);
    bool do_fade = graphics_config_.hud.shield_vdu_fade;
    graphics_config_.hud.shield_vdu_thresh[0] = GameConfig::GetVariable("graphics.hud.shield_vdu_thresh0", (do_fade ? 0.0F : 0.25F));
    graphics_config_.hud.shield_vdu_thresh[1] = GameConfig::GetVariable("graphics.hud.shield_vdu_thresh1", (do_fade ? 0.33F : 0.5F));
    graphics_config_.hud.shield_vdu_thresh[2] = GameConfig::GetVariable("graphics.hud.shield_vdu_thresh2", (do_fade ? 0.66F : 0.75F));
    graphics_config_.hud.show_negative_blips_as_positive = GameConfig::GetVariable("graphics.hud.show_negative_blips_as_positive", true);
    graphics_config_.hud.simple_manifest = GameConfig::GetVariable("graphics.hud.simple_manifest", false);
    graphics_config_.hud.starship_near_message = GameConfig::GetEscapedString("graphics.StarshipNearMessage", "#ff0000Starship Near#000000");
    graphics_config_.hud.switch_to_target_mode_on_key = GameConfig::GetVariable("graphics.hud.switchToTargetModeOnKey", true);
    graphics_config_.hud.switch_back_from_comms = GameConfig::GetVariable("graphics.hud.switch_back_from_comms", true);
    graphics_config_.hud.tac_target_foci = GameConfig::GetVariable("graphics.hud.TacTargetFoci", 0.5F);
    graphics_config_.hud.tac_target_length = GameConfig::GetVariable("graphics.hud.TacTargetLength", 0.1F);
    graphics_config_.hud.tac_target_thickness = GameConfig::GetVariable("graphics.hud.TacTargetThickness", 1.0F);
    graphics_config_.hud.text_background_alpha = GameConfig::GetVariable("graphics.hud.text_background_alpha", 0.0625F);
    graphics_config_.hud.top_view = GameConfig::GetVariable("graphics.hud.top_view", false);
    graphics_config_.hud.untarget_beyond_cone = GameConfig::GetVariable("graphics.hud.untarget_beyond_cone", false);

    // logging substruct
    logging.verbose_debug = GameConfig::GetVariable("data.verbose_debug", logging.verbose_debug);

    // physics substruct
    physics.bleed_factor = GameConfig::GetVariable("physics.warpbleed", physics.bleed_factor);
    physics.collision_scale_factor =
            GameConfig::GetVariable("physics.collision_damage_scale", physics.collision_scale_factor);
    physics.inelastic_scale = GameConfig::GetVariable("physics.inelastic_scale", physics.inelastic_scale);
    physics.kilojoules_per_damage =
            GameConfig::GetVariable("physics.kilojoules_per_unit_damage", physics.kilojoules_per_damage);
    physics.max_force_multiplier =
            GameConfig::GetVariable("physics.maxCollisionForceMultiplier", physics.max_force_multiplier);
    physics.max_shield_lowers_capacitance =
            GameConfig::GetVariable("physics.max_shield_lowers_capacitance", physics.max_shield_lowers_capacitance);
    physics.max_torque_multiplier =
            GameConfig::GetVariable("physics.maxCollisionTorqueMultiplier", physics.max_torque_multiplier);
    physics.minimum_mass = GameConfig::GetVariable("physics.minimum_physics_object_mass", physics.minimum_mass);
    physics.minimum_time =
            GameConfig::GetVariable("physics.minimum_time_between_recorded_player_collisions", physics.minimum_time);
    physics.default_shield_tightness = GameConfig::GetVariable("physics.default_shield_tightness", 0.0F);
    physics.does_missile_bounce = GameConfig::GetVariable("physics.missile_bounce", false);
    physics.system_damage_on_armor = GameConfig::GetVariable("physics.system_damage_on_armor", false);
    physics.indiscriminate_system_destruction = GameConfig::GetVariable("physics.indiscriminate_system_destruction", 0.25F);
    physics.separate_system_flakiness_component = GameConfig::GetVariable("physics.separate_system_flakiness_component", false);
    physics.shield_energy_capacitance = GameConfig::GetVariable("physics.shield_energy_capacitance", 0.2F);
    physics.use_max_shield_energy_usage = GameConfig::GetVariable("physics.use_max_shield_energy_usage", false);
    //Fuel Mass in metric tons expended per generation of 100MJ
    physics.fmec_factor = GameConfig::GetVariable("physics.FMEC_factor", 0.000000008F);
    physics.reactor_idle_efficiency = GameConfig::GetVariable("physics.reactor_idle_efficiency", 0.98F);
    physics.min_reactor_efficiency = GameConfig::GetVariable("physics.min_reactor_efficiency", 0.00001F);
    physics.ecm_energy_cost = GameConfig::GetVariable("physics.ecm_energy_cost", 0.05F);
    physics.shields_in_spec = GameConfig::GetVariable("physics.shields_in_spec", false);
    physics.shield_maintenance_charge = GameConfig::GetVariable("physics.shield_maintenance_charge", 0.25F);
    physics.warp_energy_multiplier = GameConfig::GetVariable("physics.warp_energy_multiplier", 0.12F);
    physics.player_warp_energy_multiplier = GameConfig::GetVariable("physics.player_warp_energy_multiplier", 0.12F);
    physics.vsd_mj_yield = GameConfig::GetVariable("physics.VSD_MJ_yield", 5.4F);
    physics.max_ecm = GameConfig::GetVariable("physics.max_ecm", 4ULL);
    physics.max_lost_target_live_time = GameConfig::GetVariable("physics.max_lost_target_live_time", 30.0F);
    physics.percent_missile_match_target_velocity = GameConfig::GetVariable("physics.percent_missile_match_target_velocity", 1.0F);
    physics.game_speed = GameConfig::GetVariable("physics.game_speed", 1.0F);
    physics.game_accel = GameConfig::GetVariable("physics.game_accel", 1.0F);
    physics.velocity_max = GameConfig::GetVariable("physics.velocity_max", 10000.0F);
    physics.warp_ramp_up_time = GameConfig::GetVariable("physics.warprampuptime", 5.0F);
    physics.computer_warp_ramp_up_time = GameConfig::GetVariable("physics.computerwarprampuptime", 10.0F);
    physics.warp_ramp_down_time = GameConfig::GetVariable("physics.warprampdowntime", 0.5F);
    physics.warp_memory_effect = GameConfig::GetVariable("physics.WarpMemoryEffect", 0.9F);
    physics.max_player_rotation_rate = GameConfig::GetVariable("physics.maxplayerrot", 24.0F);
    physics.max_non_player_rotation_rate = GameConfig::GetVariable("physics.maxNPCrot", 360.0F);
    physics.warp_stretch_cutoff = GameConfig::GetVariable("physics.warp_stretch_cutoff", 500000.0F) * physics.game_speed;
    physics.warp_stretch_decel_cutoff = GameConfig::GetVariable("physics.warp_stretch_decel_cutoff", 500000.0F) * physics.game_speed;
    // Pi^2
    physics.warp_multiplier_min = GameConfig::GetVariable("physics.warpMultiplierMin", 9.86968440109F);
    // C
    physics.warp_multiplier_max = GameConfig::GetVariable("physics.warpMultiplierMax", 300000000.0F);
    // Pi^2 * C
    physics.warp_max_effective_velocity = GameConfig::GetVariable("physics.warpMaxEfVal", 2.96088e+09F);
    VS_LOG(debug, (boost::format("%1%: physics.warp_max_effective_velocity: %2%") % __func__ % physics.warp_max_effective_velocity));
    VS_LOG(debug, (boost::format("%1%: GameConfig::GetVariable(\"physics.warpMaxEfVal\", %2%): %3%") % __func__ % 54.0F % GameConfig::GetVariable("physics.warpMaxEfVel", 54.0F)));
    physics.fuel_conversion = GameConfig::GetVariable("physics.FuelConversion", 0.00144F);
    physics.unit_table = GameConfig::GetVariable("physics.UnitTable", false);
    physics.capship_size = GameConfig::GetVariable("physics.capship_size", 500.0F);
    physics.near_autotrack_cone = GameConfig::GetVariable("physics.near_autotrack_cone", 0.9F);
    physics.close_enough_to_autotrack = GameConfig::GetVariable("physics.close_enough_to_autotrack", 4.0F);
    physics.distance_to_warp = GameConfig::GetVariable("physics.distance_to_warp", 1000000000000.0F);
    physics.target_distance_to_warp_bonus = GameConfig::GetVariable("physics.target_distance_to_warp_bonus", 1.33F);
    physics.no_spec_jump = GameConfig::GetVariable("physics.noSPECJUMP", true);
    physics.difficulty_speed_exponent = GameConfig::GetVariable("physics.difficulty_speed_exponent", 0.2F);
    physics.no_fuel_thrust = GameConfig::GetVariable("physics.NoFuelThrust", 0.4F);
    physics.no_fuel_afterburn = GameConfig::GetVariable("physics.NoFuelAfterburn", 0.1F);
    physics.variable_fuel_consumption = GameConfig::GetVariable("physics.VariableFuelConsumption", false);
    // Also known as Lithium6constant
    physics.deuterium_relative_efficiency_lithium = GameConfig::GetVariable("physics.DeuteriumRelativeEfficiency_Lithium", 1.0F);
    physics.min_damage = GameConfig::GetVariable("physics.min_damage", 0.001F);
    physics.max_damage = GameConfig::GetVariable("physics.max_damage", 0.999F);
    physics.max_radar_cone_damage = GameConfig::GetVariable("physics.max_radar_cone_damage", 0.9F);
    physics.max_radar_lock_cone_damage = GameConfig::GetVariable("physics.max_radar_lockcone_damage", 0.95F);
    physics.max_radar_track_cone_damage = GameConfig::GetVariable("physics.max_radar_trackcone_damage", 0.98F);
    physics.thruster_hit_chance = GameConfig::GetVariable("physics.thruster_hit_chance", 0.25F);

    // These calculations depend on the physics.game_speed and physics.game_accel values to be set already;
    // that's why they're down here instead of with the other graphics settings
    graphics_config_.in_system_jump_ani_second_ahead = GameConfig::GetVariable("graphics.insys_jump_ani_second_ahead", 4.0F)
            / physics.game_speed * physics.game_accel;
    graphics_config_.in_system_jump_ani_second_ahead_end = GameConfig::GetVariable("graphics.insys_jump_ani_second_ahead_end", 0.03F)
            / physics.game_speed * physics.game_accel;
    graphics_config_.in_system_jump_animation = GameConfig::GetString("graphics.insys_jump_animation", "warp.ani");

    // warp substruct
    warp.insystem_jump_cost = GameConfig::GetVariable("physics.insystem_jump_cost", warp.insystem_jump_cost);

    // weapons substruct
    weapons.can_fire_in_cloak = GameConfig::GetVariable("physics.can_fire_in_cloak", weapons.can_fire_in_cloak);
    weapons.can_fire_in_spec = GameConfig::GetVariable("physics.can_fire_in_spec", weapons.can_fire_in_spec);
}

// Each of the following constructors MUST specify default values for ALL struct elements - will be subsequently overridden by user specified values, if any

vegastrike_configuration::CollisionHacks::CollisionHacks() :
        collision_hack_distance(10000.0F),
        collision_damage_to_ai(false),
        crash_dock_hangar(false),
        crash_dock_unit(false),
        front_collision_hack_angle(std::cos(M_PI * 40.0
                / 180.0)), /* Note: Does not follow pattern of directly setting itself to user specified config value, must change default value in override as well if changing it here*/
        front_collision_hack_distance(200000.0F),
        cargo_deals_collide_damage(false) {
}

vegastrike_configuration::Computer::Computer() :
        default_lock_cone(0.8f),
        default_max_range(20000.0f),
        default_tracking_cone(0.93f) {
}

vegastrike_configuration::Fuel::Fuel() :
        afterburner_fuel_usage(4.0f),
        fmec_exit_velocity_inverse(0.0000002f),
        fuel_efficiency(1.0f),
        fuel_equals_warp(false),
        normal_fuel_usage(1.0f),
        reactor_uses_fuel(false) {
}

vegastrike_configuration::Logging::Logging() :
        verbose_debug(false) {
}

vegastrike_configuration::Physics::Physics() :
        bleed_factor(2.0f),
        collision_scale_factor(1.0f),
        inelastic_scale(0.8f),
        kilojoules_per_damage(5400.0f),
        max_force_multiplier(5.0f),
        max_shield_lowers_capacitance(false),
        max_torque_multiplier(0.67f),
        minimum_mass(1e-6f), /* this is actually a bit high (1 gram, in the current, non-SI, units that VS uses ), fwiw - we may want to change this some day to, say, a milligram .*/
        minimum_time(0.1f) {
}

vegastrike_configuration::Warp::Warp() :
        insystem_jump_cost(0.1f) {
}

vegastrike_configuration::Weapons::Weapons() :
        can_fire_in_cloak(false),
        can_fire_in_spec(false) {
}

std::shared_ptr<Configuration> configuration() {
    static const std::shared_ptr<Configuration> CONFIGURATION = std::make_shared<Configuration>();
    return CONFIGURATION;
}
