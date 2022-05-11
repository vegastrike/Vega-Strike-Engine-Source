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
    //logging.verbose_debug = GameConfig::GetVariable("data", "verbose_debug", false);
}

/* Override the default value(provided by constructor) with the value from the user specified configuration file, if any.
* Behavior is undefined and erroneous if no default is provided in the constructor!
* Descriptions/invariants for each variable are/will be provided in the header file and are not repeated here, except where
* behavior/range bounding is complicated/necessary-for-stability abd additional validation code is required for checking/adjusting user inputs
*/
void Configuration::OverrideDefaultsWithUserConfiguration() {
    ai.assist_friend_in_need = GameConfig::GetVariable("AI", "assist_friend_in_need", true);
    ai.ease_to_anger = GameConfig::GetVariable("AI", "EaseToAnger", -0.5F);
    ai.ease_to_appease = GameConfig::GetVariable("AI", "EaseToAppease", 0.5F);
    ai.hull_damage_anger = GameConfig::GetVariable("AI", "HullDamageAnger", 10);
    ai.hull_percent_for_comm = GameConfig::GetVariable("AI", "HullPercentForComm", 0.75F);
    ai.lowest_negative_comm_choice = GameConfig::GetVariable("AI", "LowestNegativeCommChoice", -0.00001F);
    ai.lowest_positive_comm_choice = GameConfig::GetVariable("AI", "LowestPositiveCommChoice", 0.0F);
    ai.mood_swing_level = GameConfig::GetVariable("AI", "MoodSwingLevel", 0.2F);
    ai.random_response_range = GameConfig::GetVariable("AI", "RandomResponseRange", 0.8F);
    ai.shield_damage_anger = GameConfig::GetVariable("AI", "ShieldDamageAnger", 1);

    // collision_hacks substruct
    collision_hacks.collision_hack_distance = GameConfig::GetVariable("physics",
            "collision_avoidance_hack_distance",
            collision_hacks.collision_hack_distance);
    collision_hacks.collision_damage_to_ai =
            GameConfig::GetVariable("physics", "collisionDamageToAI", collision_hacks.collision_damage_to_ai);
    collision_hacks.crash_dock_hangar =
            GameConfig::GetVariable("physics", "only_hangar_collision_docks", collision_hacks.crash_dock_hangar);
    collision_hacks.crash_dock_unit =
            GameConfig::GetVariable("physics", "unit_collision_docks", collision_hacks.crash_dock_unit);
    // (STYLE WARNING) front collision_hack_angle appears to expect to equal the cosine of a radian value, but default implies that it reads degrees! Value not specified directly, breaks pattern, should consider normalizing behavior with other config vars
    collision_hacks.front_collision_hack_angle = std::cos(
            M_PI * GameConfig::GetVariable("physics", "front_collision_avoidance_hack_angle", 40)
                    / 180.0f); // uses default value - must be changed in tandem with constructor!
    collision_hacks.front_collision_hack_distance = GameConfig::GetVariable("physics",
            "front_collision_avoidance_hack_distance",
            collision_hacks.front_collision_hack_distance);
    collision_hacks.cargo_deals_collide_damage = GameConfig::GetVariable("physics",
            "cargo_deals_collide_damage",
            collision_hacks.cargo_deals_collide_damage);

    // computer substruct
    computer.default_lock_cone = GameConfig::GetVariable("physics", "lock_cone", computer.default_lock_cone);
    computer.default_max_range = GameConfig::GetVariable("graphics", "hud", "radarRange", computer.default_max_range);
    computer.default_tracking_cone = GameConfig::GetVariable("physics", "autotracking", computer.default_tracking_cone);

    // fuel substruct
    fuel.afterburner_fuel_usage =
            GameConfig::GetVariable("physics", "AfterburnerFuelUsage", fuel.afterburner_fuel_usage);
    fuel.fmec_exit_velocity_inverse =
            GameConfig::GetVariable("physics", "FMEC_exit_vel", fuel.fmec_exit_velocity_inverse);
    fuel.fuel_efficiency =
            GameConfig::GetVariable("physics", "LithiumRelativeEfficiency_Lithium", fuel.fuel_efficiency);
    fuel.fuel_equals_warp = GameConfig::GetVariable("physics", "fuel_equals_warp", fuel.fuel_equals_warp);
    fuel.normal_fuel_usage = GameConfig::GetVariable("physics", "FuelUsage", fuel.normal_fuel_usage);
    fuel.reactor_uses_fuel = GameConfig::GetVariable("physics", "reactor_uses_fuel", fuel.reactor_uses_fuel);

    // graphics substruct
    this->graphics.automatic_landing_zone_warning = GameConfig::GetString("graphics", "automatic_landing_zone_warning", "comm_docking.ani");
    this->graphics.automatic_landing_zone_warning1 = GameConfig::GetString("graphics", "automatic_landing_zone_warning1", "comm_docking.ani");
    this->graphics.automatic_landing_zone_warning2 = GameConfig::GetString("graphics", "automatic_landing_zone_warning2", "comm_docking.ani");
    this->graphics.automatic_landing_zone_warning_text = GameConfig::GetEscapedString("graphics", "automatic_landing_zone_warning_text", "Now Entering an \"Automatic Landing Zone\".");
    this->graphics.draw_heading_marker = GameConfig::GetVariable("graphics", "draw_heading_marker", false);

    // graphics ==> hud subsection
    this->graphics.hud.draw_rendered_crosshairs = GameConfig::GetVariable("graphics", "hud", "draw_rendered_crosshairs", GameConfig::GetVariable("graphics", "draw_rendered_crosshairs", true));
    this->graphics.hud.already_near_message = GameConfig::GetEscapedString("graphics", "hud", "AlreadyNearMessage", "#ff0000Already Near#000000");
    this->graphics.hud.armor_hull_size = GameConfig::GetVariable("graphics", "hud", "armor_hull_size", 0.55F);
    this->graphics.hud.asteroids_near_message = GameConfig::GetEscapedString("graphics", "hud", "AsteroidsNearMessage", "#ff0000Asteroids Near#000000");
    this->graphics.hud.basename_colon_basename = GameConfig::GetVariable("graphics", "hud", "basename:basename", true);
    this->graphics.hud.box_line_thickness = GameConfig::GetVariable("graphics", "hud", "BoxLineThickness", 1.0F);
    this->graphics.hud.completed_objectives_last = GameConfig::GetVariable("graphics", "hud", "completed_objectives_last", true);
    this->graphics.hud.crosshairs_on_chase_cam = GameConfig::GetVariable("graphics", "hud", "crosshairs_on_chasecam", false);
    this->graphics.hud.crosshairs_on_padlock = GameConfig::GetVariable("graphics", "hud", "crosshairs_on_padlock", false);
    this->graphics.hud.damage_report_heading = GameConfig::GetEscapedString("graphics", "hud", "damage_report_heading", "#00ff00DAMAGE REPORT\\n\\n");
    this->graphics.hud.debug_position = GameConfig::GetVariable("graphics", "hud", "debug_position", false);
    this->graphics.hud.diamond_line_thickness = GameConfig::GetVariable("graphics", "hud", "DiamondLineThickness", 1.0F);
    this->graphics.hud.diamond_rotation_speed = GameConfig::GetVariable("graphics", "hud", "DiamondRotationSpeed", 1.0F);
    this->graphics.hud.diamond_size = GameConfig::GetVariable("graphics", "hud", "DiamondSize", 2.05F);
    this->graphics.hud.display_relative_velocity = GameConfig::GetVariable("graphics", "hud", "display_relative_velocity", true);
    this->graphics.hud.display_warp_energy_if_no_jump_drive = GameConfig::GetVariable("graphics", "hud", "display_warp_energy_if_no_jump_drive", true);
    this->graphics.hud.draw_all_target_boxes = GameConfig::GetVariable("graphics", "hud", "drawAllTargetBoxes", false);
    this->graphics.hud.draw_always_itts = GameConfig::GetVariable("graphics", "hud", "drawAlwaysITTS", false);
    this->graphics.hud.draw_line_to_itts = GameConfig::GetVariable("graphics", "hud", "drawLineToITTS", false);
    this->graphics.hud.draw_line_to_target = GameConfig::GetVariable("graphics", "hud", "drawLineToTarget", false);
    this->graphics.hud.draw_line_to_targets_target = GameConfig::GetVariable("graphics", "hud", "drawLineToTargetsTarget", false);
    this->graphics.hud.draw_nav_symbol = GameConfig::GetVariable("graphics", "hud", "drawNavSymbol", false);
    this->graphics.hud.draw_tactical_target = GameConfig::GetVariable("graphics", "hud", "DrawTacticalTarget", false);
    this->graphics.hud.draw_targeting_boxes = GameConfig::GetVariable("graphics", "hud", "DrawTargettingBoxes", true);
    this->graphics.hud.draw_targeting_boxes_inside = GameConfig::GetVariable("graphics", "hud", "DrawTargettingBoxesInside", true);
    this->graphics.hud.draw_arrow_on_chase_cam = GameConfig::GetVariable("graphics", "hud", "draw_arrow_on_chasecam", true);
    this->graphics.hud.draw_arrow_on_pan_cam = GameConfig::GetVariable("graphics", "hud", "draw_arrow_on_pancam", false);
    this->graphics.hud.draw_arrow_on_pan_target = GameConfig::GetVariable("graphics", "hud", "draw_arrow_on_pantgt", false);
    this->graphics.hud.draw_arrow_to_target = GameConfig::GetVariable("graphics", "hud", "draw_arrow_to_target", true);
    this->graphics.hud.draw_blips_on_both_radar = GameConfig::GetVariable("graphics", "hud", "draw_blips_on_both_radar", false);
    this->graphics.hud.draw_damage_sprite = GameConfig::GetVariable("graphics", "hud", "draw_damage_sprite", true);
    this->graphics.hud.draw_gravitational_objects = GameConfig::GetVariable("graphics", "hud", "draw_gravitational_objects", true);
    this->graphics.hud.draw_nav_grid = GameConfig::GetVariable("graphics", "hud", "draw_nav_grid", true);
    this->graphics.hud.draw_significant_blips = GameConfig::GetVariable("graphics", "hud", "draw_significant_blips", true);
    this->graphics.hud.draw_star_direction = GameConfig::GetVariable("graphics", "hud", "draw_star_direction", true);
    this->graphics.hud.draw_unit_on_chase_cam = GameConfig::GetVariable("graphics", "hud", "draw_unit_on_chasecam", true);
    this->graphics.hud.draw_vdu_view_shields = GameConfig::GetVariable("graphics", "hud", "draw_vdu_view_shields", true);
    this->graphics.hud.draw_weapon_sprite = GameConfig::GetVariable("graphics", "hud", "draw_weapon_sprite", false);
    this->graphics.hud.enemy_near_message = GameConfig::GetEscapedString("graphics", "hud", "EnemyNearMessage", "#ff0000Enemy Near#000000");
    this->graphics.hud.gun_list_columns = GameConfig::GetVariable("graphics", "hud", "gun_list_columns", 1);
    this->graphics.hud.invert_friendly_shields = GameConfig::GetVariable("graphics", "hud", "invert_friendly_shields", false);
    this->graphics.hud.invert_friendly_sprite = GameConfig::GetVariable("graphics", "hud", "invert_friendly_sprite", false);
    this->graphics.hud.invert_target_shields = GameConfig::GetVariable("graphics", "hud", "invert_target_shields", false);
    this->graphics.hud.invert_target_sprite = GameConfig::GetVariable("graphics", "hud", "invert_target_sprite", false);
    this->graphics.hud.invert_view_shields = GameConfig::GetVariable("graphics", "hud", "invert_view_shields", false);
    this->graphics.hud.itts_for_beams = GameConfig::GetVariable("graphics", "hud", "ITTSForBeams", false);
    this->graphics.hud.itts_for_lockable = GameConfig::GetVariable("graphics", "hud", "ITTSForLockable", false);
    this->graphics.hud.itts_line_to_mark_alpha = GameConfig::GetVariable("graphics", "hud", "ITTSLineToMarkAlpha", 0.1F);
    this->graphics.hud.itts_use_average_gun_speed = GameConfig::GetVariable("graphics", "hud", "ITTSUseAverageGunSpeed", true);
    this->graphics.hud.lock_center_crosshair = GameConfig::GetVariable("graphics", "hud", "LockCenterCrosshair", false);
    this->graphics.hud.lock_confirm_line_length = GameConfig::GetVariable("graphics", "hud", "LockConfirmLineLength", 1.5F);
    this->graphics.hud.manifest_heading = GameConfig::GetEscapedString("graphics", "hud", "manifest_heading", "Manifest\\n");
    this->graphics.hud.max_missile_bracket_size = GameConfig::GetVariable("graphics", "hud", "MaxMissileBracketSize", 2.05F);
    this->graphics.hud.message_prefix = GameConfig::GetEscapedString("graphics", "hud", "message_prefix", "");
    this->graphics.hud.min_missile_bracket_size = GameConfig::GetVariable("graphics", "hud", "MinMissileBracketSize", 1.05F);
    this->graphics.hud.min_lock_box_size = GameConfig::GetVariable("graphics", "hud", "min_lock_box_size", 0.001F);
    this->graphics.hud.min_radar_blip_size = GameConfig::GetVariable("graphics", "hud", "min_radarblip_size", 0.0F);
    this->graphics.hud.min_target_box_size = GameConfig::GetVariable("graphics", "hud", "min_target_box_size", 0.01F);
    this->graphics.hud.mounts_list_empty = GameConfig::GetString("graphics", "hud", "mounts_list_empty", "");
    this->graphics.hud.nav_cross_line_thickness = GameConfig::GetVariable("graphics", "hud", "NavCrossLineThickness", 1.0F);
    this->graphics.hud.not_included_in_damage_report = GameConfig::GetString("graphics", "hud", "not_included_in_damage_report", "plasteel_hull tungsten_hull isometal_hull");
    this->graphics.hud.out_of_cone_distance = GameConfig::GetVariable("graphics", "hud", "out_of_cone_distance", false);
    this->graphics.hud.padlock_view_lag = GameConfig::GetVariable("graphics", "hud", "PadlockViewLag", 1.5F);
    this->graphics.hud.padlock_view_lag_fix_zone = GameConfig::GetVariable("graphics", "hud", "PadlockViewLagFixZone", 0.0872F);
    this->graphics.hud.planet_near_message = GameConfig::GetEscapedString("graphics", "hud", "PlanetNearMessage", "#ff0000Planetary Hazard Near#000000");
    this->graphics.hud.print_damage_percent = GameConfig::GetVariable("graphics", "hud", "print_damage_percent", true);
    this->graphics.hud.print_faction = GameConfig::GetVariable("graphics", "hud", "print_faction", true);
    this->graphics.hud.print_fg_name = GameConfig::GetVariable("graphics", "hud", "print_fg_name", true);
    this->graphics.hud.print_fg_sub_id = GameConfig::GetVariable("graphics", "hud", "print_fg_sub_id", false);
    this->graphics.hud.print_request_docking = GameConfig::GetVariable("graphics", "hud", "print_request_docking", true);
    this->graphics.hud.print_ship_type = GameConfig::GetVariable("graphics", "hud", "print_ship_type", true);
    this->graphics.hud.projectile_means_missile = GameConfig::GetVariable("graphics", "hud", "projectile_means_missile", false);
    this->graphics.hud.radar_range = GameConfig::GetVariable("graphics", "hud", "radarRange", 20000.0F);    // TODO: Deduplicate this with computer.default_max_range
    this->graphics.hud.radar_type = GameConfig::GetString("graphics", "hud", "radarType", "WC");
    this->graphics.hud.radar_search_extra_radius = GameConfig::GetVariable("graphics", "hud", "radar_search_extra_radius", 1000.0F);
    this->graphics.hud.rotating_bracket_inner = GameConfig::GetVariable("graphics", "hud", "RotatingBracketInner", true);
    this->graphics.hud.rotating_bracket_size = GameConfig::GetVariable("graphics", "hud", "RotatingBracketSize", 0.58F);
    this->graphics.hud.rotating_bracket_width = GameConfig::GetVariable("graphics", "hud", "RotatingBracketWidth", 0.1F);
    this->graphics.hud.scale_relationship_color = GameConfig::GetVariable("graphics", "hud", "scale_relationship_color", 10.0F);
    this->graphics.hud.shield_vdu_fade = GameConfig::GetVariable("graphics", "hud", "shield_vdu_fade", true);
    bool do_fade = this->graphics.hud.shield_vdu_fade;
    this->graphics.hud.shield_vdu_thresh[0] = GameConfig::GetVariable("graphics", "hud", "shield_vdu_thresh0", (do_fade ? 0.0F : 0.25F));
    this->graphics.hud.shield_vdu_thresh[1] = GameConfig::GetVariable("graphics", "hud", "shield_vdu_thresh1", (do_fade ? 0.33F : 0.5F));
    this->graphics.hud.shield_vdu_thresh[2] = GameConfig::GetVariable("graphics", "hud", "shield_vdu_thresh2", (do_fade ? 0.66F : 0.75F));
    this->graphics.hud.show_negative_blips_as_positive = GameConfig::GetVariable("graphics", "hud", "show_negative_blips_as_positive", true);
    this->graphics.hud.simple_manifest = GameConfig::GetVariable("graphics", "hud", "simple_manifest", false);
    this->graphics.hud.starship_near_message = GameConfig::GetEscapedString("graphics", "hud", "StarshipNearMessage", "#ff0000Starship Near#000000");
    this->graphics.hud.switch_to_target_mode_on_key = GameConfig::GetVariable("graphics", "hud", "switchToTargetModeOnKey", true);
    this->graphics.hud.switch_back_from_comms = GameConfig::GetVariable("graphics", "hud", "switch_back_from_comms", true);
    this->graphics.hud.tac_target_foci = GameConfig::GetVariable("graphics", "hud", "TacTargetFoci", 0.5F);
    this->graphics.hud.tac_target_length = GameConfig::GetVariable("graphics", "hud", "TacTargetLength", 0.1F);
    this->graphics.hud.tac_target_thickness = GameConfig::GetVariable("graphics", "hud", "TacTargetThickness", 1.0F);
    this->graphics.hud.text_background_alpha = GameConfig::GetVariable("graphics", "hud", "text_background_alpha", 0.0625F);
    this->graphics.hud.top_view = GameConfig::GetVariable("graphics", "hud", "top_view", false);
    this->graphics.hud.untarget_beyond_cone = GameConfig::GetVariable("graphics", "hud", "untarget_beyond_cone", false);

    // logging substruct
    logging.verbose_debug = GameConfig::GetVariable("data", "verbose_debug", logging.verbose_debug);

    // physics substruct
    physics.bleed_factor = GameConfig::GetVariable("physics", "warpbleed", physics.bleed_factor);
    physics.collision_scale_factor =
            GameConfig::GetVariable("physics", "collision_damage_scale", physics.collision_scale_factor);
    physics.inelastic_scale = GameConfig::GetVariable("physics", "inelastic_scale", physics.inelastic_scale);
    physics.kilojoules_per_damage =
            GameConfig::GetVariable("physics", "kilojoules_per_unit_damage", physics.kilojoules_per_damage);
    physics.max_force_multiplier =
            GameConfig::GetVariable("physics", "maxCollisionForceMultiplier", physics.max_force_multiplier);
    physics.max_shield_lowers_capacitance =
            GameConfig::GetVariable("physics", "max_shield_lowers_capacitance", physics.max_shield_lowers_capacitance);
    physics.max_torque_multiplier =
            GameConfig::GetVariable("physics", "maxCollisionTorqueMultiplier", physics.max_torque_multiplier);
    physics.minimum_mass = GameConfig::GetVariable("physics", "minimum_physics_object_mass", physics.minimum_mass);
    physics.minimum_time =
            GameConfig::GetVariable("physics", "minimum_time_between_recorded_player_collisions", physics.minimum_time);
    physics.default_shield_tightness = GameConfig::GetVariable("physics", "default_shield_tightness", 0.0F);
    physics.does_missile_bounce = GameConfig::GetVariable("physics", "missile_bounce", false);

    // warp substruct
    warp.insystem_jump_cost = GameConfig::GetVariable("physics", "insystem_jump_cost", warp.insystem_jump_cost);

    // weapons substruct
    weapons.can_fire_in_cloak = GameConfig::GetVariable("physics", "can_fire_in_cloak", weapons.can_fire_in_cloak);
    weapons.can_fire_in_spec = GameConfig::GetVariable("physics", "can_fire_in_spec", weapons.can_fire_in_spec);
}

// Each of the following constructors MUST specify default values for ALL struct elements - will be subsequently overridden by user specified values, if any

vegastrike_configuration::CollisionHacks::CollisionHacks() :
        collision_hack_distance(10000.0f),
        collision_damage_to_ai(false),
        crash_dock_hangar(false),
        crash_dock_unit(false),
        front_collision_hack_angle(std::cos(M_PI * 40.0
                / 180.0)), /* Note: Does not follow pattern of directly setting itself to user specified config value, must change default value in override as well if changing it here*/
        front_collision_hack_distance(200000.0f),
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
