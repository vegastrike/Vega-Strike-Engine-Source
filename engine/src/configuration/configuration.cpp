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

using vega_config::GetGameConfig;

Configuration::Configuration() {
    //logging.verbose_debug = GetGameConfig().GetBool("data.verbose_debug", false);
}

/* Override the default value(provided by constructor) with the value from the user specified configuration file, if any.
* Behavior is undefined and erroneous if no default is provided in the constructor!
* Descriptions/invariants for each variable are/will be provided in the header file and are not repeated here, except where
* behavior/range bounding is complicated/necessary-for-stability abd additional validation code is required for checking/adjusting user inputs
*/
void Configuration::OverrideDefaultsWithUserConfiguration() {
    general_config_.pitch = GetGameConfig().GetFloat("general.pitch", 0.0F);
    general_config_.yaw = GetGameConfig().GetFloat("general.yaw", 0.0F);
    general_config_.roll = GetGameConfig().GetFloat("general.roll", 0.0F);
    general_config_.force_anonymous_mission_names = GetGameConfig().GetBool("general.force_anonymous_mission_names", true);

    data_config_.master_part_list = GetGameConfig().GetString("data.master_part_list", "master_part_list");
    data_config_.using_templates = GetGameConfig().GetBool("data.usingtemplates", true);

    ai.always_obedient                                  = GetGameConfig().GetBool("AI.always_obedient", true);
    ai.assist_friend_in_need                            = GetGameConfig().GetBool("AI.assist_friend_in_need", true);
    ai.ease_to_anger                                    = GetGameConfig().GetFloat("AI.EaseToAnger", -0.5F);
    ai.ease_to_appease                                  = GetGameConfig().GetFloat("AI.EaseToAppease", 0.5F);
    ai.hull_damage_anger                                = GetGameConfig().GetInt32("AI.HullDamageAnger", 10);
    ai.hull_percent_for_comm                            = GetGameConfig().GetFloat("AI.HullPercentForComm", 0.75F);
    ai.lowest_negative_comm_choice                      = GetGameConfig().GetFloat("AI.LowestNegativeCommChoice", -0.00001F);
    ai.lowest_positive_comm_choice                      = GetGameConfig().GetFloat("AI.LowestPositiveCommChoice", 0.0F);
    ai.mood_swing_level                                 = GetGameConfig().GetFloat("AI.MoodSwingLevel", 0.2F);
    ai.random_response_range                            = GetGameConfig().GetFloat("AI.RandomResponseRange", 0.8F);
    ai.shield_damage_anger                              = GetGameConfig().GetInt32("AI.ShieldDamageAnger", 1);
    ai.jump_without_energy                              = GetGameConfig().GetBool("AI.jump_without_energy", false);
    ai.friend_factor                                    = -GetGameConfig().GetFloat("AI.friend_factor", 0.1F);
    ai.kill_factor                                      = -GetGameConfig().GetFloat("AI.kill_factor", 0.2F);

    audio_config_.every_other_mount                     = GetGameConfig().GetBool("audio.every_other_mount", false);
    audio_config_.shuffle_songs.clear_history_on_list_change = GetGameConfig().GetBool("audio.shuffle_songs.clear_history_on_list_change", false);

    // collision_hacks substruct
    collision_hacks.collision_hack_distance             = GetGameConfig().GetFloat("physics.collision_avoidance_hack_distance", collision_hacks.collision_hack_distance);
    collision_hacks.collision_damage_to_ai              = GetGameConfig().GetBool("physics.collisionDamageToAI", collision_hacks.collision_damage_to_ai);
    collision_hacks.crash_dock_hangar                   = GetGameConfig().GetBool("physics.only_hangar_collision_docks", collision_hacks.crash_dock_hangar);
    collision_hacks.crash_dock_unit                     = GetGameConfig().GetBool("physics.unit_collision_docks", collision_hacks.crash_dock_unit);
    // (STYLE WARNING) front collision_hack_angle appears to expect to equal the cosine of a radian value, but default implies that it reads degrees! Value not specified directly, breaks pattern, should consider normalizing behavior with other config vars
    collision_hacks.front_collision_hack_angle = std::cos(
            M_PI * GetGameConfig().GetDouble("physics.front_collision_avoidance_hack_angle", 40.0)
                    / 180.0); // uses default value - must be changed in tandem with constructor!
    collision_hacks.front_collision_hack_distance = GetGameConfig().GetFloat("physics.front_collision_avoidance_hack_distance",
            collision_hacks.front_collision_hack_distance);
    collision_hacks.cargo_deals_collide_damage = GetGameConfig().GetBool("physics.cargo_deals_collide_damage",
            collision_hacks.cargo_deals_collide_damage);

    // computer substruct
    computer_config_.default_lock_cone = GetGameConfig().GetFloat("physics.lock_cone", computer_config_.default_lock_cone);
    computer_config_.default_max_range = GetGameConfig().GetFloat("graphics.hud.radarRange", computer_config_.default_max_range);
    computer_config_.default_tracking_cone = GetGameConfig().GetFloat("physics.autotracking", computer_config_.default_tracking_cone);

    // fuel substruct
    fuel.afterburner_fuel_usage =
            GetGameConfig().GetFloat("physics.AfterburnerFuelUsage", fuel.afterburner_fuel_usage);
    fuel.fmec_exit_velocity_inverse =
            1.0F / GetGameConfig().GetFloat("physics.FMEC_exit_vel", fuel.fmec_exit_velocity_inverse);
    fuel.fuel_efficiency =
            GetGameConfig().GetFloat("physics.LithiumRelativeEfficiency_Lithium", fuel.fuel_efficiency);
    fuel.fuel_equals_warp = GetGameConfig().GetBool("physics.fuel_equals_warp", fuel.fuel_equals_warp);
    fuel.normal_fuel_usage = GetGameConfig().GetFloat("physics.FuelUsage", fuel.normal_fuel_usage);
    fuel.reactor_uses_fuel = GetGameConfig().GetBool("physics.reactor_uses_fuel", fuel.reactor_uses_fuel);
    fuel.vsd_mj_yield = GetGameConfig().GetFloat("physics.VSD_MJ_yield", 5.4F);
    fuel.no_fuel_thrust = GetGameConfig().GetFloat("physics.NoFuelThrust", 0.4F);
    fuel.no_fuel_afterburn = GetGameConfig().GetFloat("physics.NoFuelAfterburn", 0.1F);
    fuel.variable_fuel_consumption = GetGameConfig().GetBool("physics.VariableFuelConsumption", false);
    // Also known as Lithium6constant
    fuel.deuterium_relative_efficiency_lithium = GetGameConfig().GetFloat("physics.DeuteriumRelativeEfficiency_Lithium", 1.0F);
    //Fuel Mass in metric tons expended per generation of 100MJ
    fuel.fmec_factor = GetGameConfig().GetFloat("physics.FMEC_factor", 0.000000008F);
    fuel.reactor_idle_efficiency = GetGameConfig().GetFloat("physics.reactor_idle_efficiency", 0.98F);
    fuel.min_reactor_efficiency = GetGameConfig().GetFloat("physics.min_reactor_efficiency", 0.00001F);
    fuel.ecm_energy_cost = GetGameConfig().GetFloat("physics.ecm_energy_cost", 0.05F);
    fuel.fuel_conversion = GetGameConfig().GetFloat("physics.FuelConversion", 0.00144F);

    // graphics substruct
    graphics_config_.automatic_landing_zone_warning = GetGameConfig().GetString("graphics.automatic_landing_zone_warning", "comm_docking.ani");
    graphics_config_.automatic_landing_zone_warning1 = GetGameConfig().GetString("graphics.automatic_landing_zone_warning1", "comm_docking.ani");
    graphics_config_.automatic_landing_zone_warning2 = GetGameConfig().GetString("graphics.automatic_landing_zone_warning2", "comm_docking.ani");
    graphics_config_.automatic_landing_zone_warning_text = GetGameConfig().GetEscapedString("graphics.automatic_landing_zone_warning_text", "Now Entering an \"Automatic Landing Zone\".");
    graphics_config_.draw_heading_marker = GetGameConfig().GetBool("graphics.draw_heading_marker", false);
    graphics_config_.missile_explosion_radius_mult = GetGameConfig().GetFloat("graphics.missile_explosion_radius_mult", 1.0F);
    graphics_config_.missile_sparkle = GetGameConfig().GetBool("graphics.missilesparkle", false);
    // Sic - Should be emissive, I think
    graphics_config_.atmosphere_emmissive = GetGameConfig().GetFloat("graphics.atmosphere_emmissive", 1.0F);
    graphics_config_.atmosphere_diffuse = GetGameConfig().GetFloat("graphics.atmosphere_diffuse", 1.0F);
    graphics_config_.default_engine_activation = GetGameConfig().GetFloat("graphics.default_engine_activation", 0.00048828125F);
    graphics_config_.explosion_animation = GetGameConfig().GetString("graphics.explosion_animation", "explosion_orange.ani");
    graphics_config_.shield_detail = GetGameConfig().GetInt32("graphics.shield_detail", 16);
    graphics_config_.shield_texture = GetGameConfig().GetString("graphics.shield_texture", "shield.bmp");
    graphics_config_.shield_technique = GetGameConfig().GetString("graphics.shield_technique", "");
    graphics_config_.fov = GetGameConfig().GetFloat("graphics.fov", 78.0F);
    graphics_config_.reduced_vdus_width = GetGameConfig().GetFloat("graphics.reduced_vdus_width", 0.0F);
    graphics_config_.reduced_vdus_height = GetGameConfig().GetFloat("graphics.reduced_vdus_height", 0.0F);

    graphics_config_.glow_flicker.flicker_time = GetGameConfig().GetFloat("graphics.glowflicker.time", 30.0F);
    graphics_config_.glow_flicker.flicker_off_time = GetGameConfig().GetFloat("graphics.glowflicker.off-time", 2.0F);
    graphics_config_.glow_flicker.hull_for_total_dark = GetGameConfig().GetFloat("graphics.glowflicker.hull-for-total-dark", 0.04F);
    graphics_config_.glow_flicker.num_times_per_second_on = GetGameConfig().GetFloat("graphics.glowflicker.num-times-per-second-on", 0.66F);
    graphics_config_.glow_flicker.min_flicker_cycle = GetGameConfig().GetFloat("graphics.glowflicker.min-cycle", 2.0F);

    // graphics ==> hud subsection
    graphics_config_.hud.draw_rendered_crosshairs = GetGameConfig().GetBool("graphics.hud.draw_rendered_crosshairs", GetGameConfig().GetBool("graphics.draw_rendered_crosshairs", true));
    graphics_config_.hud.already_near_message = GetGameConfig().GetEscapedString("graphics.hud.AlreadyNearMessage", "#ff0000Already Near#000000");
    graphics_config_.hud.armor_hull_size = GetGameConfig().GetFloat("graphics.hud.armor_hull_size", 0.55F);
    graphics_config_.hud.asteroids_near_message = GetGameConfig().GetEscapedString("graphics.hud.AsteroidsNearMessage", "#ff0000Asteroids Near#000000");
    graphics_config_.hud.basename_colon_basename = GetGameConfig().GetBool("graphics.hud.basename:basename", true);
    graphics_config_.hud.box_line_thickness = GetGameConfig().GetFloat("graphics.hud.BoxLineThickness", 1.0F);
    graphics_config_.hud.completed_objectives_last = GetGameConfig().GetBool("graphics.hud.completed_objectives_last", true);
    graphics_config_.hud.crosshairs_on_chase_cam = GetGameConfig().GetBool("graphics.hud.crosshairs_on_chasecam", false);
    graphics_config_.hud.crosshairs_on_padlock = GetGameConfig().GetBool("graphics.hud.crosshairs_on_padlock", false);
    graphics_config_.hud.damage_report_heading = GetGameConfig().GetEscapedString("graphics.hud.damage_report_heading", "#00ff00DAMAGE REPORT\\n\\n");
    graphics_config_.hud.debug_position = GetGameConfig().GetBool("graphics.hud.debug_position", false);
    graphics_config_.hud.diamond_line_thickness = GetGameConfig().GetFloat("graphics.hud.DiamondLineThickness", 1.0F);
    graphics_config_.hud.diamond_rotation_speed = GetGameConfig().GetFloat("graphics.hud.DiamondRotationSpeed", 1.0F);
    graphics_config_.hud.diamond_size = GetGameConfig().GetFloat("graphics.hud.DiamondSize", 2.05F);
    graphics_config_.hud.display_relative_velocity = GetGameConfig().GetBool("graphics.hud.display_relative_velocity", true);
    graphics_config_.hud.display_warp_energy_if_no_jump_drive = GetGameConfig().GetBool("graphics.hud.display_warp_energy_if_no_jump_drive", true);
    graphics_config_.hud.draw_all_target_boxes = GetGameConfig().GetBool("graphics.hud.drawAllTargetBoxes", false);
    graphics_config_.hud.draw_always_itts = GetGameConfig().GetBool("graphics.hud.drawAlwaysITTS", false);
    graphics_config_.hud.draw_line_to_itts = GetGameConfig().GetBool("graphics.hud.drawLineToITTS", false);
    graphics_config_.hud.draw_line_to_target = GetGameConfig().GetBool("graphics.hud.drawLineToTarget", false);
    graphics_config_.hud.draw_line_to_targets_target = GetGameConfig().GetBool("graphics.hud.drawLineToTargetsTarget", false);
    graphics_config_.hud.draw_nav_symbol = GetGameConfig().GetBool("graphics.hud.drawNavSymbol", false);
    graphics_config_.hud.draw_tactical_target = GetGameConfig().GetBool("graphics.hud.DrawTacticalTarget", false);
    graphics_config_.hud.draw_targeting_boxes = GetGameConfig().GetBool("graphics.hud.DrawTargettingBoxes", true);
    graphics_config_.hud.draw_targeting_boxes_inside = GetGameConfig().GetBool("graphics.hud.DrawTargettingBoxesInside", true);
    graphics_config_.hud.draw_arrow_on_chase_cam = GetGameConfig().GetBool("graphics.hud.draw_arrow_on_chasecam", true);
    graphics_config_.hud.draw_arrow_on_pan_cam = GetGameConfig().GetBool("graphics.hud.draw_arrow_on_pancam", false);
    graphics_config_.hud.draw_arrow_on_pan_target = GetGameConfig().GetBool("graphics.hud.draw_arrow_on_pantgt", false);
    graphics_config_.hud.draw_arrow_to_target = GetGameConfig().GetBool("graphics.hud.draw_arrow_to_target", true);
    graphics_config_.hud.draw_blips_on_both_radar = GetGameConfig().GetBool("graphics.hud.draw_blips_on_both_radar", false);
    graphics_config_.hud.draw_damage_sprite = GetGameConfig().GetBool("graphics.hud.draw_damage_sprite", true);
    graphics_config_.hud.draw_gravitational_objects = GetGameConfig().GetBool("graphics.hud.draw_gravitational_objects", true);
    graphics_config_.hud.draw_nav_grid = GetGameConfig().GetBool("graphics.hud.draw_nav_grid", true);
    graphics_config_.hud.draw_significant_blips = GetGameConfig().GetBool("graphics.hud.draw_significant_blips", true);
    graphics_config_.hud.draw_star_direction = GetGameConfig().GetBool("graphics.hud.draw_star_direction", true);
    graphics_config_.hud.draw_unit_on_chase_cam = GetGameConfig().GetBool("graphics.hud.draw_unit_on_chasecam", true);
    graphics_config_.hud.draw_vdu_view_shields = GetGameConfig().GetBool("graphics.hud.draw_vdu_view_shields", true);
    graphics_config_.hud.draw_weapon_sprite = GetGameConfig().GetBool("graphics.hud.draw_weapon_sprite", false);
    graphics_config_.hud.enemy_near_message = GetGameConfig().GetEscapedString("graphics.hud.EnemyNearMessage", "#ff0000Enemy Near#000000");
    graphics_config_.hud.gun_list_columns = GetGameConfig().GetInt32("graphics.hud.gun_list_columns", 1);
    graphics_config_.hud.invert_friendly_shields = GetGameConfig().GetBool("graphics.hud.invert_friendly_shields", false);
    graphics_config_.hud.invert_friendly_sprite = GetGameConfig().GetBool("graphics.hud.invert_friendly_sprite", false);
    graphics_config_.hud.invert_target_shields = GetGameConfig().GetBool("graphics.hud.invert_target_shields", false);
    graphics_config_.hud.invert_target_sprite = GetGameConfig().GetBool("graphics.hud.invert_target_sprite", false);
    graphics_config_.hud.invert_view_shields = GetGameConfig().GetBool("graphics.hud.invert_view_shields", false);
    graphics_config_.hud.itts_for_beams = GetGameConfig().GetBool("graphics.hud.ITTSForBeams", false);
    graphics_config_.hud.itts_for_lockable = GetGameConfig().GetBool("graphics.hud.ITTSForLockable", false);
    graphics_config_.hud.itts_line_to_mark_alpha = GetGameConfig().GetFloat("graphics.hud.ITTSLineToMarkAlpha", 0.1F);
    graphics_config_.hud.itts_use_average_gun_speed = GetGameConfig().GetBool("graphics.hud.ITTSUseAverageGunSpeed", true);
    graphics_config_.hud.lock_center_crosshair = GetGameConfig().GetBool("graphics.hud.LockCenterCrosshair", false);
    graphics_config_.hud.lock_confirm_line_length = GetGameConfig().GetFloat("graphics.hud.LockConfirmLineLength", 1.5F);
    graphics_config_.hud.manifest_heading = GetGameConfig().GetEscapedString("graphics.hud.manifest_heading", "Manifest\\n");
    graphics_config_.hud.max_missile_bracket_size = GetGameConfig().GetFloat("graphics.hud.MaxMissileBracketSize", 2.05F);
    graphics_config_.hud.message_prefix = GetGameConfig().GetEscapedString("graphics.hud.message_prefix", "");
    graphics_config_.hud.min_missile_bracket_size = GetGameConfig().GetFloat("graphics.hud.MinMissileBracketSize", 1.05F);
    graphics_config_.hud.min_lock_box_size = GetGameConfig().GetFloat("graphics.hud.min_lock_box_size", 0.001F);
    graphics_config_.hud.min_radar_blip_size = GetGameConfig().GetFloat("graphics.hud.min_radarblip_size", 0.0F);
    graphics_config_.hud.min_target_box_size = GetGameConfig().GetFloat("graphics.hud.min_target_box_size", 0.01F);
    graphics_config_.hud.mounts_list_empty = GetGameConfig().GetString("graphics.hud.mounts_list_empty", "");
    graphics_config_.hud.nav_cross_line_thickness = GetGameConfig().GetFloat("graphics.hud.NavCrossLineThickness", 1.0F);
    graphics_config_.hud.not_included_in_damage_report = GetGameConfig().GetString("graphics.hud.not_included_in_damage_report", "plasteel_hull tungsten_hull isometal_hull");
    graphics_config_.hud.out_of_cone_distance = GetGameConfig().GetBool("graphics.hud.out_of_cone_distance", false);
    graphics_config_.hud.padlock_view_lag = GetGameConfig().GetFloat("graphics.hud.PadlockViewLag", 1.5F);
    graphics_config_.hud.padlock_view_lag_fix_zone = GetGameConfig().GetFloat("graphics.hud.PadlockViewLagFixZone", 0.0872F);
    graphics_config_.hud.planet_near_message = GetGameConfig().GetEscapedString("graphics.hud.PlanetNearMessage", "#ff0000Planetary Hazard Near#000000");
    graphics_config_.hud.print_damage_percent = GetGameConfig().GetBool("graphics.hud.print_damage_percent", true);
    graphics_config_.hud.print_faction = GetGameConfig().GetBool("graphics.hud.print_faction", true);
    graphics_config_.hud.print_fg_name = GetGameConfig().GetBool("graphics.hud.print_fg_name", true);
    graphics_config_.hud.print_fg_sub_id = GetGameConfig().GetBool("graphics.hud.print_fg_sub_id", false);
    graphics_config_.hud.print_request_docking = GetGameConfig().GetBool("graphics.hud.print_request_docking", true);
    graphics_config_.hud.print_ship_type = GetGameConfig().GetBool("graphics.hud.print_ship_type", true);
    graphics_config_.hud.projectile_means_missile = GetGameConfig().GetBool("graphics.hud.projectile_means_missile", false);
    graphics_config_.hud.radar_type = GetGameConfig().GetString("graphics.hud.radarType", "WC");
    graphics_config_.hud.radar_search_extra_radius = GetGameConfig().GetFloat("graphics.hud.radar_search_extra_radius", 1000.0F);
    graphics_config_.hud.rotating_bracket_inner = GetGameConfig().GetBool("graphics.hud.RotatingBracketInner", true);
    graphics_config_.hud.rotating_bracket_size = GetGameConfig().GetFloat("graphics.hud.RotatingBracketSize", 0.58F);
    graphics_config_.hud.rotating_bracket_width = GetGameConfig().GetFloat("graphics.hud.RotatingBracketWidth", 0.1F);
    graphics_config_.hud.scale_relationship_color = GetGameConfig().GetFloat("graphics.hud.scale_relationship_color", 10.0F);
    graphics_config_.hud.shield_vdu_fade = GetGameConfig().GetBool("graphics.hud.shield_vdu_fade", true);
    bool do_fade = graphics_config_.hud.shield_vdu_fade;
    graphics_config_.hud.shield_vdu_thresh[0] = GetGameConfig().GetFloat("graphics.hud.shield_vdu_thresh0", (do_fade ? 0.0F : 0.25F));
    graphics_config_.hud.shield_vdu_thresh[1] = GetGameConfig().GetFloat("graphics.hud.shield_vdu_thresh1", (do_fade ? 0.33F : 0.5F));
    graphics_config_.hud.shield_vdu_thresh[2] = GetGameConfig().GetFloat("graphics.hud.shield_vdu_thresh2", (do_fade ? 0.66F : 0.75F));
    graphics_config_.hud.show_negative_blips_as_positive = GetGameConfig().GetBool("graphics.hud.show_negative_blips_as_positive", true);
    graphics_config_.hud.simple_manifest = GetGameConfig().GetBool("graphics.hud.simple_manifest", false);
    graphics_config_.hud.starship_near_message = GetGameConfig().GetEscapedString("graphics.StarshipNearMessage", "#ff0000Starship Near#000000");
    graphics_config_.hud.switch_to_target_mode_on_key = GetGameConfig().GetBool("graphics.hud.switchToTargetModeOnKey", true);
    graphics_config_.hud.switch_back_from_comms = GetGameConfig().GetBool("graphics.hud.switch_back_from_comms", true);
    graphics_config_.hud.tac_target_foci = GetGameConfig().GetFloat("graphics.hud.TacTargetFoci", 0.5F);
    graphics_config_.hud.tac_target_length = GetGameConfig().GetFloat("graphics.hud.TacTargetLength", 0.1F);
    graphics_config_.hud.tac_target_thickness = GetGameConfig().GetFloat("graphics.hud.TacTargetThickness", 1.0F);
    graphics_config_.hud.text_background_alpha = GetGameConfig().GetFloat("graphics.hud.text_background_alpha", 0.0625F);
    graphics_config_.hud.top_view = GetGameConfig().GetBool("graphics.hud.top_view", false);
    graphics_config_.hud.untarget_beyond_cone = GetGameConfig().GetBool("graphics.hud.untarget_beyond_cone", false);

    // logging substruct
    logging.verbose_debug = GetGameConfig().GetBool("data.verbose_debug", logging.verbose_debug);

    // physics substruct
    physics_config_.collision_scale_factor =
            GetGameConfig().GetFloat("physics.collision_damage_scale", physics_config_.collision_scale_factor);
    physics_config_.inelastic_scale = GetGameConfig().GetFloat("physics.inelastic_scale", physics_config_.inelastic_scale);
    physics_config_.kilojoules_per_damage =
            GetGameConfig().GetFloat("physics.kilojoules_per_unit_damage", physics_config_.kilojoules_per_damage);
    physics_config_.max_force_multiplier =
            GetGameConfig().GetFloat("physics.maxCollisionForceMultiplier", physics_config_.max_force_multiplier);
    physics_config_.max_shield_lowers_capacitance =
            GetGameConfig().GetBool("physics.max_shield_lowers_capacitance", physics_config_.max_shield_lowers_capacitance);
    physics_config_.max_torque_multiplier =
            GetGameConfig().GetFloat("physics.maxCollisionTorqueMultiplier", physics_config_.max_torque_multiplier);
    physics_config_.minimum_mass = GetGameConfig().GetFloat("physics.minimum_physics_object_mass", physics_config_.minimum_mass);
    physics_config_.minimum_time =
            GetGameConfig().GetFloat("physics.minimum_time_between_recorded_player_collisions", physics_config_.minimum_time);
    physics_config_.default_shield_tightness = GetGameConfig().GetFloat("physics.default_shield_tightness", 0.0F);
    physics_config_.does_missile_bounce = GetGameConfig().GetBool("physics.missile_bounce", false);
    physics_config_.system_damage_on_armor = GetGameConfig().GetBool("physics.system_damage_on_armor", false);
    physics_config_.indiscriminate_system_destruction = GetGameConfig().GetFloat("physics.indiscriminate_system_destruction", 0.25F);
    physics_config_.separate_system_flakiness_component = GetGameConfig().GetBool("physics.separate_system_flakiness_component", false);
    physics_config_.shield_energy_capacitance = GetGameConfig().GetFloat("physics.shield_energy_capacitance", 0.2F);
    physics_config_.use_max_shield_energy_usage = GetGameConfig().GetBool("physics.use_max_shield_energy_usage", false);
    physics_config_.shields_in_spec = GetGameConfig().GetBool("physics.shields_in_spec", false);
    physics_config_.shield_maintenance_charge = GetGameConfig().GetFloat("physics.shield_maintenance_charge", 0.25F);
    physics_config_.max_ecm = GetGameConfig().GetSizeT("physics.max_ecm", 4U);
    physics_config_.max_lost_target_live_time = GetGameConfig().GetFloat("physics.max_lost_target_live_time", 30.0F);
    physics_config_.percent_missile_match_target_velocity = GetGameConfig().GetFloat("physics.percent_missile_match_target_velocity", 1.0F);
    physics_config_.game_speed = GetGameConfig().GetFloat("physics.game_speed", 1.0F);
    physics_config_.game_accel = GetGameConfig().GetFloat("physics.game_accel", 1.0F);
    physics_config_.velocity_max = GetGameConfig().GetFloat("physics.velocity_max", 10000.0F);
    physics_config_.max_player_rotation_rate = GetGameConfig().GetFloat("physics.maxplayerrot", 24.0F);
    physics_config_.max_non_player_rotation_rate = GetGameConfig().GetFloat("physics.maxNPCrot", 360.0F);
    physics_config_.unit_table = GetGameConfig().GetBool("physics.UnitTable", false);
    physics_config_.capship_size = GetGameConfig().GetFloat("physics.capship_size", 500.0F);
    physics_config_.near_autotrack_cone = GetGameConfig().GetFloat("physics.near_autotrack_cone", 0.9F);
    physics_config_.close_enough_to_autotrack = GetGameConfig().GetFloat("physics.close_enough_to_autotrack", 4.0F);
    physics_config_.distance_to_warp = GetGameConfig().GetFloat("physics.distance_to_warp", 1000000000000.0F);
    physics_config_.target_distance_to_warp_bonus = GetGameConfig().GetFloat("physics.target_distance_to_warp_bonus", 1.33F);
    physics_config_.no_spec_jump = GetGameConfig().GetBool("physics.noSPECJUMP", true);
    physics_config_.difficulty_speed_exponent = GetGameConfig().GetFloat("physics.difficulty_speed_exponent", 0.2F);
    physics_config_.min_damage = GetGameConfig().GetFloat("physics.min_damage", 0.001F);
    physics_config_.max_damage = GetGameConfig().GetFloat("physics.max_damage", 0.999F);
    physics_config_.max_radar_cone_damage = GetGameConfig().GetFloat("physics.max_radar_cone_damage", 0.9F);
    physics_config_.max_radar_lock_cone_damage = GetGameConfig().GetFloat("physics.max_radar_lockcone_damage", 0.95F);
    physics_config_.max_radar_track_cone_damage = GetGameConfig().GetFloat("physics.max_radar_trackcone_damage", 0.98F);
    physics_config_.thruster_hit_chance = GetGameConfig().GetFloat("physics.thruster_hit_chance", 0.25F);
    physics_config_.friendly_auto_radius = GetGameConfig().GetFloat("physics.friendly_auto_radius", 0.0F) * physics_config_.game_speed;
    physics_config_.neutral_auto_radius = GetGameConfig().GetFloat("physics.neutral_auto_radius", 0.0F) * physics_config_.game_speed;
    physics_config_.hostile_auto_radius = GetGameConfig().GetFloat("physics.hostile_auto_radius", 1000.0F) * physics_config_.game_speed;
    physics_config_.min_asteroid_distance = GetGameConfig().GetFloat("physics.min_asteroid_distance", -100.0F);
    physics_config_.steady_itts = GetGameConfig().GetBool("physics.steady_itts", false);
    physics_config_.no_unit_collisions = GetGameConfig().GetBool("physics.no_unit_collisions", false);

    // These calculations depend on the physics.game_speed and physics.game_accel values to be set already;
    // that's why they're down here instead of with the other graphics settings
    graphics_config_.in_system_jump_ani_second_ahead = GetGameConfig().GetFloat("graphics.insys_jump_ani_second_ahead", 4.0F)
            / physics_config_.game_speed * physics_config_.game_accel;
    graphics_config_.in_system_jump_ani_second_ahead_end = GetGameConfig().GetFloat("graphics.insys_jump_ani_second_ahead_end", 0.03F)
            / physics_config_.game_speed * physics_config_.game_accel;
    graphics_config_.in_system_jump_animation = GetGameConfig().GetString("graphics.insys_jump_animation", "warp.ani");

    // warp_config_ substruct
    warp_config_.insystem_jump_cost = GetGameConfig().GetFloat("physics.insystem_jump_cost", warp_config_.insystem_jump_cost);
    warp_config_.bleed_factor = GetGameConfig().GetFloat("physics.warpbleed", 2.0F);
    warp_config_.warp_energy_multiplier = GetGameConfig().GetFloat("physics.warp_energy_multiplier", 0.12F);
    warp_config_.player_warp_energy_multiplier = GetGameConfig().GetFloat("physics.player_warp_energy_multiplier", 0.12F);
    warp_config_.warp_ramp_up_time = GetGameConfig().GetFloat("physics.warprampuptime", 5.0F);
    warp_config_.computer_warp_ramp_up_time = GetGameConfig().GetFloat("physics.computerwarprampuptime", 10.0F);
    warp_config_.warp_ramp_down_time = GetGameConfig().GetFloat("physics.warprampdowntime", 0.5F);
    warp_config_.warp_memory_effect = GetGameConfig().GetFloat("physics.WarpMemoryEffect", 0.9F);
    warp_config_.warp_stretch_cutoff = GetGameConfig().GetFloat("physics.warp_stretch_cutoff", 500000.0F) * physics_config_.game_speed;
    warp_config_.warp_stretch_decel_cutoff = GetGameConfig().GetFloat("physics.warp_stretch_decel_cutoff", 500000.0F) * physics_config_.game_speed;
    // Pi^2
    warp_config_.warp_multiplier_min = GetGameConfig().GetFloat("physics.warpMultiplierMin", 9.86968440109F);
    // C
    warp_config_.warp_multiplier_max = GetGameConfig().GetFloat("physics.warpMultiplierMax", 300000000.0F);
    warp_config_.max_effective_velocity = GetGameConfig().GetFloat("physics.warpMaxEfVel", static_cast<float>(M_PI * M_PI * 300000000.0));

    // weapons substruct
    weapons.can_fire_in_cloak = GetGameConfig().GetBool("physics.can_fire_in_cloak", weapons.can_fire_in_cloak);
    weapons.can_fire_in_spec = GetGameConfig().GetBool("physics.can_fire_in_spec", weapons.can_fire_in_spec);
}

// Each of the following constructors MUST specify default values for ALL struct elements - will be subsequently overridden by user specified values, if any

vega_config::CollisionHacks::CollisionHacks() :
        collision_hack_distance(10000.0F),
        collision_damage_to_ai(false),
        crash_dock_hangar(false),
        crash_dock_unit(false),
        front_collision_hack_angle(std::cos(M_PI * 40.0
                / 180.0)), /* Note: Does not follow pattern of directly setting itself to user specified config value, must change default value in override as well if changing it here*/
        front_collision_hack_distance(200000.0F),
        cargo_deals_collide_damage(false) {
}

vega_config::ComputerConfig::ComputerConfig() :
        default_lock_cone(0.8f),
        default_max_range(20000.0f),
        default_tracking_cone(0.93f) {
}

vega_config::Fuel::Fuel() :
        afterburner_fuel_usage(4.0f),
        fmec_exit_velocity_inverse(0.0000002f),
        fuel_efficiency(1.0f),
        fuel_equals_warp(false),
        normal_fuel_usage(1.0f),
        reactor_uses_fuel(false) {
}

vega_config::Logging::Logging() :
        verbose_debug(false) {
}

vega_config::PhysicsConfig::PhysicsConfig() :
        collision_scale_factor(1.0f),
        inelastic_scale(0.8f),
        kilojoules_per_damage(5400.0f),
        max_force_multiplier(5.0f),
        max_shield_lowers_capacitance(false),
        max_torque_multiplier(0.67f),
        minimum_mass(1e-6f), /* this is actually a bit high (1 gram, in the current, non-SI, units that VS uses ), fwiw - we may want to change this some day to, say, a milligram .*/
        minimum_time(0.1f) {
}

vega_config::WarpConfig::WarpConfig() :
        insystem_jump_cost(0.1f) {
}

vega_config::WeaponsConfig::WeaponsConfig() :
        can_fire_in_cloak(false),
        can_fire_in_spec(false) {
}

std::shared_ptr<Configuration> configuration() {
    static const std::shared_ptr<Configuration> kConfiguration = std::make_shared<Configuration>();
    return kConfiguration;
}
