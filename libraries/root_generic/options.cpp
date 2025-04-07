/*
 * Copyright (C) 2001-2025 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike contributors.
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
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

#include "root_generic/options.h"
#include "root_generic/configxml.h"
#include "configuration/configuration.h"

extern VegaConfig *vs_config;

void vs_options::init() {
    /* General Options */
    galaxy = vega_config::config->general.galaxy; /* default: "milky_way.xml" */
    command_interpretor = vega_config::config->general.command_interpreter;
    load_last_savegame = vega_config::config->general.load_last_savegame;
    debug_fs = vega_config::config->general.debug_fs;
    simulation_atom = vega_config::config->general.simulation_atom;
    audio_atom = vega_config::config->general.audio_atom;


    /* Audio Options */
    threadtime = vega_config::config->audio.thread_time;
    missionvictorysong = vega_config::config->audio.mission_victory_song; /* default: "../music/victory.ogg" */
    Music = vega_config::config->audio.music;
    sound_gain = vega_config::config->audio.sound_gain;
    audio_ref_distance = vega_config::config->audio.audio_ref_distance;
    audio_max_distance = vega_config::config->audio.audio_max_distance;
    Doppler = vega_config::config->audio.doppler;
    Positional = vega_config::config->audio.positional;
    Volume = vega_config::config->audio.volume;
    DopplerScale = vega_config::config->audio.doppler_scale;
    frequency = vega_config::config->audio.frequency;
    MaxSingleSounds = vega_config::config->audio.max_single_sounds;
    MaxTotalSounds = vega_config::config->audio.max_total_sounds;
    Sound = vega_config::config->audio.sound;
    ai_sound = vega_config::config->audio.ai_sound;
    explosion_closeness = vega_config::config->audio.explosion_closeness;
    loss_relationship = vega_config::config->audio.loss_relationship;
    victory_relationship = vega_config::config->audio.victory_relationship;
    time_between_music = vega_config::config->audio.time_between_music;
    debris_time = vega_config::config->physics.debris_time;
    /* Cockpit Audio Options */
    comm = vega_config::config->cockpit_audio.comm; /* default: "vdu_c" */
    scanning = vega_config::config->cockpit_audio.scanning; /* default: "vdu_c" */
    objective = vega_config::config->cockpit_audio.objective; /* default: "vdu_c" */
    examine = vega_config::config->cockpit_audio.examine; /* default: "vdu_b" */
    view = vega_config::config->cockpit_audio.view; /* default: "vdu_b" */
    repair = vega_config::config->cockpit_audio.repair; /* default: "vdu_a" */
    manifest = vega_config::config->cockpit_audio.manifest; /* default: "vdu_a" */
    compress_max = vega_config::config->cockpit_audio.compress_max;
    compress_loop = vega_config::config->cockpit_audio.compress_loop; /* default: "compress_loop" */
    compress_change = vega_config::config->cockpit_audio.compress_change; /* default: "compress_burst" */
    compress_stop = vega_config::config->cockpit_audio.compress_stop; /* default: "compress_end" */
    compress_interval = vega_config::config->cockpit_audio.compress_interval;

    comm_preload = vega_config::config->cockpit_audio.comm_preload;

    /* Unit Audio Options */
    jumpleave = vega_config::config->audio.unit_audio.jump_leave; /* default: "sfx43.wav" */
    player_armor_hit = vega_config::config->audio.unit_audio.player_armor_hit; /* default: "bigarmor.wav" */
    player_hull_hit = vega_config::config->audio.unit_audio.player_hull_hit; /* default: "bigarmor.wav" */
    player_shield_hit = vega_config::config->audio.unit_audio.player_shield_hit; /* default: "shieldhit.wav" */


    /* Graphics Options */
    jumpgate = vega_config::config->graphics.jump_gate; /* default: "warp.ani" */
    jump_animation_shrink = vega_config::config->graphics.jump_animation_shrink;
    jumpgatesize = vega_config::config->graphics.jump_gate_size;
    camera_pan_speed = vega_config::config->graphics.camera_pan_speed;
    background = vega_config::config->graphics.background;
    cockpit = vega_config::config->graphics.cockpit;
    disabled_cockpit_allowed =
            vega_config::config->graphics.disabled_cockpit_allowed;
    splash_screen = vega_config::config->graphics.splash_screen; /* default: "vega_splash.ani" */
    vbo = vega_config::config->graphics.vbo;
    num_near_stars = vega_config::config->graphics.num_near_stars;
    num_far_stars = vega_config::config->graphics.num_far_stars;
    star_spreading = vega_config::config->graphics.star_spreading;
    use_planet_atmosphere = vega_config::config->graphics.use_planet_atmosphere;
    use_planet_fog = vega_config::config->graphics.use_planet_fog;
    reflectivity = vega_config::config->graphics.reflectivity;
    hardware_cursor = vega_config::config->physics.hardware_cursor;
    always_make_smooth_cam = vega_config::config->graphics.always_make_smooth_cam; // default: "false"     //Not used yet
    precull_dist = vega_config::config->graphics.precull_dist;
    draw_near_stars_in_front_of_planets =
            vega_config::config->graphics.draw_near_stars_in_front_of_planets;
    star_blend = vega_config::config->graphics.star_blend;
    sparkle_engine_size_relative_to_ship =
            vega_config::config->graphics.sparkle_engine_size_relative_to_ship;
    sparkle_absolute_speed = vega_config::config->graphics.sparkle_absolute_speed;
    engine_radii_scale = vega_config::config->graphics.engine_radii_scale;
    engine_length_scale = vega_config::config->graphics.engine_length_scale; // default: "1.25"
    halos_by_velocity = vega_config::config->graphics.halos_by_velocity; // default: "false"
    percent_afterburner_color_change = vega_config::config->graphics.percent_afterburner_color_change; // default: ".5"
    percent_halo_fade_in = vega_config::config->graphics.percent_halo_fade_in; // default: ".5"
    afterburner_color_red = vega_config::config->graphics.afterburner_color_red; // default: "1.0"
    afterburner_color_green = vega_config::config->graphics.afterburner_color_green; // default: "0.0"
    afterburner_color_blue = vega_config::config->graphics.afterburner_color_blue; // default: "0.0"
    engine_color_red = vega_config::config->graphics.engine_color_red; // default: "1.0"
    engine_color_green = vega_config::config->graphics.engine_color_green; // default: "1.0"
    engine_color_blue = vega_config::config->graphics.engine_color_blue; // default: "1.0"
    halo_sparkle_rate = vega_config::config->graphics.halo_sparkle_rate; // default: "20"
    halo_sparkle_scale = vega_config::config->graphics.halo_sparkle_scale; // default: "6"
    halo_sparkle_speed = vega_config::config->graphics.halo_sparkle_speed; // default: "0.5"
    max_cubemap_size = vega_config::config->graphics.max_cubemap_size; // default: "1024"
    default_boot_message = vega_config::config->graphics.default_boot_message; /* default: "" */
    initial_boot_message = vega_config::config->graphics.initial_boot_message; /* default: "Loading..." */
    splash_audio = vega_config::config->graphics.splash_audio; /* default: "" */
    main_menu = vega_config::config->graphics.main_menu; // default: "false"
    startup_cockpit_view = vega_config::config->graphics.startup_cockpit_view; /* default: "front" */
    detail_texture_trilinear = vega_config::config->graphics.detail_texture_trilinear; // default: "true"
    light_cutoff = vega_config::config->graphics.light_cutoff; // default: ".06"
    light_optimal_intensity = vega_config::config->graphics.light_optimal_intensity; // default: ".06"
    light_saturation = vega_config::config->graphics.light_saturation; // default: ".95"
    num_lights = vega_config::config->graphics.num_lights; // default: "4"
    separate_specular_color = vega_config::config->graphics.separate_specular_color; // default: "false"
    lock_vertex_arrays = vega_config::config->graphics.lock_vertex_arrays; // default: "true"
    fog_detail = vega_config::config->graphics.fog_detail; // default: "0"
    fov = vega_config::config->graphics.fov; // default: "78"
    aspect = vega_config::config->graphics.aspect; // default: "1.33"
    znear = vega_config::config->graphics.znear; // default: "1"
    zfar = vega_config::config->graphics.zfar; // default: "100000"
    model_detail = vega_config::config->graphics.model_detail; // default: "1"
    use_textures = vega_config::config->graphics.use_textures; // default: "true"
    use_ship_textures = vega_config::config->graphics.use_ship_textures; // default: "false"
    use_planet_textures = vega_config::config->graphics.use_planet_textures; // default: "false"
    use_logos = vega_config::config->graphics.use_logos; // default: "true"
    use_vs_sprites = vega_config::config->graphics.use_vs_sprites; // default: "true"
    use_animations = vega_config::config->graphics.use_animations; // default: "true"
    use_videos = vega_config::config->graphics.use_videos; // default: "true"
    use_wireframe = vega_config::config->graphics.use_wireframe; // default: "false"
    max_texture_dimension = vega_config::config->graphics.max_texture_dimension; // default: "65536"
    max_movie_dimension = vega_config::config->graphics.max_movie_dimension; // default: "65536"
    rect_textures = vega_config::config->graphics.rect_textures; // default: "false"
    pot_video_textures = vega_config::config->graphics.pot_video_textures; // default: "false"
    techniquesSubPath = vega_config::config->graphics.technique_set; /* default: "." */
    smooth_shade = vega_config::config->graphics.smooth_shade; // default: "true"
    mipmap_detail = vega_config::config->graphics.mipmap_detail; // default: "2"
    texture_compression = vega_config::config->graphics.texture_compression; // default: "0"
    reflection = vega_config::config->graphics.reflection; // default: "true"
    displaylists = vega_config::config->graphics.displaylists; // default: "false"
    s3tc = vega_config::config->graphics.s3tc; // default: "true"
    ext_clamp_to_edge = vega_config::config->graphics.ext_clamp_to_edge; // default: "true"
    ext_clamp_to_border = vega_config::config->graphics.ext_clamp_to_border; // default: "true"
    clear_on_startup = vega_config::config->graphics.clear_on_startup; // default: "true"
    circle_accuracy = vega_config::config->graphics.circle_accuracy; // default: "0.1"
    rgb_pixel_format = vega_config::config->graphics.rgb_pixel_format; /* default: "undefined" */
    gl_accelerated_visual = vega_config::config->graphics.gl_accelerated_visual; // default: "true"
    z_pixel_format = vega_config::config->graphics.z_pixel_format; // default: "24"
    x_resolution = vega_config::config->graphics.resolution_x;
    y_resolution = vega_config::config->graphics.resolution_y;
    full_screen = vega_config::config->graphics.full_screen; // default: "false"
    color_depth = vega_config::config->graphics.color_depth; // default: "32"
    glut_stencil = vega_config::config->graphics.glut_stencil; // default: "true"
    // The Following makes no sense.   Why differentiate mac and pc shaders if they have unique names anyway?
    mac_shader_name = vega_config::config->graphics.mac_shader_name; /* default: "mac" */
    shader_name = vega_config::config->graphics.shader_name; /* default: "default" */
    framerate_changes_shader = vega_config::config->graphics.framerate_changes_shader; // default: "false"
    draw_weapons = vega_config::config->graphics.draw_weapons; // default: "false"
    sparkle_rate = vega_config::config->graphics.sparkle_rate; // default: "5.0"
    only_stretch_in_warp = vega_config::config->graphics.only_stretch_in_warp; // default: "true"
    warp_stretch_cutoff = vega_config::config->graphics.warp_stretch_cutoff; // default: "50000"
    warp_stretch_region0_max = vega_config::config->graphics.warp_stretch_region0_max; // default: "1"
    warp_stretch_max = vega_config::config->graphics.warp_stretch_max; // default: "4"
    warp_stretch_max_speed =
            vega_config::config->graphics.warp_stretch_max_speed; // default: "1000000"
    warp_stretch_max_region0_speed =
            vega_config::config->graphics.warp_stretch_max_region0_speed; // default: "100000"
    weapon_gamma = vega_config::config->graphics.weapon_gamma; // default: "1.35"
    split_dead_subunits = vega_config::config->graphics.split_dead_subunits; // default: "true"
    explosion_force = vega_config::config->graphics.explosion_force; // default: ".5"
    explosion_torque = vega_config::config->graphics.explosion_torque; // default: ".001"
    explosion_animation = vega_config::config->graphics.explosion_animation; /* default: "explosion_orange.ani" */
    explosion_face_player = vega_config::config->graphics.explosion_face_player; // default: "true"
    percent_shockwave = vega_config::config->graphics.percent_shockwave; // default: ".5"
    shockwave_growth = vega_config::config->graphics.shockwave_growth; // default: "1.05"
    shockwave_animation = vega_config::config->graphics.shockwave_animation; /* default: "explosion_wave.ani" */
    bolt_offset = vega_config::config->graphics.bolt_offset; // default: "-.2"
    blend_guns = vega_config::config->graphics.blend_guns; // default: "true"
    stretch_bolts = vega_config::config->graphics.stretch_bolts; // default: "0"
    bolt_pixel_size = vega_config::config->graphics.bolt_pixel_size; // default: ".5"

    /* Graphics/Mesh Options */
    smooth_lines = vega_config::config->graphics.smooth_lines; //default: true
    smooth_points = vega_config::config->graphics.smooth_points; //default: true


    /* Splash Options */
    auto_hide = vega_config::config->splash.auto_hide; // default: "true"


    /* Terrain Options */
    xscale = vega_config::config->terrain.xscale;
    yscale = vega_config::config->terrain.yscale;
    zscale = vega_config::config->terrain.zscale;
    mass = vega_config::config->terrain.mass;
    radius = vega_config::config->terrain.radius;

    /* Player Options */

    /* Joystick Options */
    joystick_exponent = vega_config::config->joystick.joystick_exponent; // default: "1.0"
    polling_rate = vega_config::config->joystick.polling_rate; // default: "0"
    force_use_of_joystick =
            vega_config::config->joystick.force_use_of_joystick; // default: "false"
    debug_digital_hatswitch =
            vega_config::config->joystick.debug_digital_hatswitch; // default: "false"
    deadband = vega_config::config->joystick.deadband; // default: "0.05"
    mouse_deadband = vega_config::config->joystick.mouse_deadband; // default: "0.025"
    warp_mouse = vega_config::config->joystick.warp_mouse; // default: "false"
    mouse_sensitivity = vega_config::config->joystick.mouse_sensitivity; // default: "50.0"
    mouse_exponent = vega_config::config->joystick.mouse_exponent; // default: "3.0"
    mouse_blur = vega_config::config->joystick.mouse_blur; // default: ".025"
    force_feedback = vega_config::config->joystick.force_feedback; // default: "false"
    ff_device = vega_config::config->joystick.ff_device; // default: "0"
    warp_mouse_zone = vega_config::config->joystick.warp_mouse_zone; // default: "100"

    /* AI Options */
    allow_civil_war = vega_config::config->ai.allow_civil_war;
    capped_faction_rating = vega_config::config->ai.capped_faction_rating;
    allow_nonplayer_faction_change = vega_config::config->ai.allow_nonplayer_faction_change;
    min_relationship = vega_config::config->ai.min_relationship;
    start_docked_to = vega_config::config->ai.start_docked_to; /* default: "MiningBase" */
    dock_on_load = vega_config::config->ai.dock_on_load;

    /* PhysicsConfig Options */
    drone = vega_config::config->physics.drone; /* default: "drone" */
    max_missions = vega_config::config->physics.max_missions; // default: "4"
    game_speed = vega_config::config->physics.game_speed;
    runtime_compactness = vega_config::config->physics.runtime_compactness;
    autogen_compactness = vega_config::config->physics.autogen_compactness;
    asteroid_difficulty = vega_config::config->physics.asteroid_difficulty;
    year_scale = vega_config::config->physics.year_scale;
    game_speed_affects_autogen_systems =
            vega_config::config->physics.game_speed_affects_autogen_systems; // default: "false"
    star_system_scale = vega_config::config->physics.star_system_scale;
    respawn_unit_size = vega_config::config->physics.respawn_unit_size;
    auto_pilot_planet_radius_percent =
            vega_config::config->physics.auto_pilot_planet_radius_percent; // default: ".75"
    campaigns = vega_config::config->game_start.campaigns; /* default: "privateer_campaign vegastrike_campaign" */  // Moved to correct section
    num_running_systems = vega_config::config->physics.num_running_systems; // default: "4"
    inactive_system_time = vega_config::config->physics.inactive_system_time; // default: "0.3"
    jump_radius_scale = vega_config::config->physics.jump_radius_scale; // default: "2"
    jump_disables_shields = vega_config::config->physics.jump_disables_shields; // default: "true"
    display_in_meters = vega_config::config->physics.display_in_meters; // default: "true"
    game_speed_lying = vega_config::config->physics.game_speed_lying; // default: "true"
    num_times_to_simulate_new_star_system =
            vega_config::config->physics.num_times_to_simulate_new_star_system; // default: "20"
    gun_speed_adjusted_game_speed =
            vega_config::config->physics.gun_speed_adjusted_game_speed; // default: "false"
    gun_speed = vega_config::config->physics.gun_speed; // default: "1"
    weapon_damage_efficiency =
            vega_config::config->physics.weapon_damage_efficiency; // default: "1.0"
    refire_difficulty_scaling =
            vega_config::config->physics.refire_difficutly_scaling; // default: "3.0"
    debris_mass = vega_config::config->physics.debris_mass; // default: ".00001"
    explosion_damage_center =
            vega_config::config->physics.explosion_damage_center; // default: "1"
    explosion_damage_edge =
            vega_config::config->graphics.explosion_damage_edge; // default: ".125"
    eject_cargo_on_blowup = vega_config::config->physics.eject_cargo_on_blowup; // default: "0"

    /* Data Options */
    universe_path = vega_config::config->data.universe_path; /* default: "universe" */
    sectors = vega_config::config->data.sectors; /* default: "sectors" */
    techniquesBasePath = vega_config::config->data.techniques; /* default: "techniques" */
    modUnitCSV = vega_config::config->data.ModUnitCSV; /* default: "" */
    cockpits = vega_config::config->data.cockpits; /* default: "cockpits" */
    animations = vega_config::config->data.animations; /* default: "animations" */
    movies = vega_config::config->data.movies; /* default: "movies" */
    sprites = vega_config::config->data.sprites; /* default: "sprites" */
    serialized_xml = vega_config::config->data.serialized_xml; /* default: "serialized_xml" */
    sharedtextures = vega_config::config->data.sharedtextures; /* default: "textures" */
    sharedsounds = vega_config::config->data.sharedsounds; /* default: "sounds" */
    sharedmeshes = vega_config::config->data.sharedmeshes; /* default: "meshes" */
    sharedunits = vega_config::config->data.sharedunits; /* default: "units" */
    ai_directory = vega_config::config->data.ai_directory; /* default: "ai" */
    datadir = vega_config::config->data.datadir; /* default: "" */
    hqtextures = vega_config::config->data.hqtextures; /* default: "" */
    volume_format = vega_config::config->data.volume_format; /* default: "pk3" */

    /* Galaxy Options */
    PushValuesToMean = vega_config::config->galaxy.PushValuesToMean; // default: "true"
    AmbientLightFactor = vega_config::config->galaxy.AmbientLightFactor; // default: "0"
    StarRadiusScale = vega_config::config->galaxy.StarRadiusScale; // default: "1000"
    DefaultAtmosphereTexture = vega_config::config->galaxy.DefaultAtmosphereTexture; /* default: "sol/earthcloudmaptrans.png" */
    AtmosphereProbability = vega_config::config->galaxy.AtmosphereProbability; // default: "0.5"
    RingProbability = vega_config::config->galaxy.RingProbability; // default: ".1"
    DoubleRingProbability = vega_config::config->galaxy.DoubleRingProbability; // default: ".025"
    InnerRingRadius = vega_config::config->galaxy.InnerRingRadius; // default: "1.5"
    OuterRingRadius = vega_config::config->galaxy.OuterRingRadius; // default: "2.5"
    DefaultRingTexture = vega_config::config->galaxy.DefaultRingTexture; /* default: "planets/ring.pngwrapx36wrapy2" */
    SecondRingDifference = vega_config::config->galaxy.SecondRingDifference; // default: ".4"
    MoonRelativeToPlanet = vega_config::config->galaxy.MoonRelativeToPlanet; // default: ".4"
    MoonRelativeToMoon = vega_config::config->galaxy.MoonRelativeToMoon; // default: ".8"
    RockyRelativeToPrimary =
            vega_config::config->galaxy.RockyRelativeToPrimary; // default: ".05"
    CompactnessScale = vega_config::config->galaxy.CompactnessScale; // default: "1.5"
    JumpCompactnessScale = vega_config::config->galaxy.JumpCompactnessScale; // default: "1.5"
    MeanNaturalPhenomena = vega_config::config->galaxy.MeanNaturalPhenomena; // default: "1"
    MeanStarBases = vega_config::config->galaxy.MeanStarBases; // default: "2"
//    SmallUnitsMultiplier   = vega_config::config->galaxy.SmallUnitsMultiplier /* default: 0 */ );

    /* Network Options */
    force_client_connect = vega_config::config->network.force_client_connect; // default: "false"
    use_account_server = vega_config::config->network.use_account_server; // default: "true"
    server_ip = vega_config::config->network.server_ip; /* default: "" */
    server_port = vega_config::config->network.server_port; /* default: "6777" */
    account_server_url = vega_config::config->network.account_server_url; /* default: "http://vegastrike.sourceforge.net/cgi-bin/accountserver.py?" */
    chat_only_in_network = vega_config::config->network.chat_only_in_network; // default: "false"

    /* Cargo Options */
    news_from_cargolist = vega_config::config->cargo.news_from_cargolist; // default: "false"

    /* Keyboard Options */
    enable_unicode = vega_config::config->keyboard.enable_unicode; // default: "true"

    /* Player Options */
    password = vega_config::config->player.password; /* default: "" */
    callsign = vega_config::config->player.callsign; /* default: "" */

}

string vs_options::getCallsign(int squadnum) {
    return vs_config->getVariable(std::string("player")
            + ((squadnum > 0) ? XMLSupport::tostring(squadnum + 1) : std::string(
                    "")), "callsign", "pilot");
}

string vs_options::getPlayer(int playernum) {
    return (vs_config->getVariable("player" + ((playernum > 0) ? XMLSupport::tostring(playernum + 1) : string("")),
            "callsign",
            ""));
}

string vs_options::getPassword(int playernum) {
    return (vs_config->getVariable("player" + ((playernum > 0) ? XMLSupport::tostring(playernum + 1) : string("")),
            "password",
            ""));
}

std::shared_ptr<vs_options> game_options() {
    static const std::shared_ptr<vs_options> GAME_OPTIONS = std::make_shared<vs_options>();
    return GAME_OPTIONS;
}
