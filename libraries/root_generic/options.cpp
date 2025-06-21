/*
 * options.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
 *
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
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "root_generic/options.h"
#include "root_generic/configxml.h"
#include "configuration/configuration.h"

extern VegaConfig *vs_config;

void vs_options::init() {
    /* General Options */
    galaxy = configuration()->game_start.galaxy;;
    command_interpretor = configuration()->general.command_interpreter;
    load_last_savegame = configuration()->general.load_last_savegame;
    debug_fs = configuration()->general.debug_fs;


    /* Audio Options */
    threadtime = XMLSupport::parse_int(vs_config->getVariable("audio", "threadtime", "1"));
    missionvictorysong = vs_config->getVariable("audio", "missionvictorysong", "../music/victory.ogg");
    Music = XMLSupport::parse_bool(vs_config->getVariable("audio", "Music", "true"));
    sound_gain = XMLSupport::parse_floatf(vs_config->getVariable("audio", "sound_gain", ".5"));
    audio_ref_distance = XMLSupport::parse_floatf(vs_config->getVariable("audio", "audio_ref_distance", "4000"));
    audio_max_distance = XMLSupport::parse_floatf(vs_config->getVariable("audio", "audio_max_distance", "1000000"));
    Doppler = XMLSupport::parse_bool(vs_config->getVariable("audio", "Doppler", "false"));
    Positional = XMLSupport::parse_bool(vs_config->getVariable("audio", "Positional", "true"));
    Volume = XMLSupport::parse_floatf(vs_config->getVariable("audio", "Volume", "100"));
    DopplerScale = XMLSupport::parse_floatf(vs_config->getVariable("audio", "DopplerScale", "1"));
    frequency = XMLSupport::parse_int(vs_config->getVariable("audio", "frequency", "48000"));
    MaxSingleSounds = XMLSupport::parse_int(vs_config->getVariable("audio", "MaxSingleSounds", "8"));
    MaxTotalSounds = XMLSupport::parse_int(vs_config->getVariable("audio", "MaxTotalSounds", "20"));
    Sound = XMLSupport::parse_bool(vs_config->getVariable("audio", "Sound", "true"));
    ai_sound = XMLSupport::parse_bool(vs_config->getVariable("audio", "ai_sound", "true"));
    explosion_closeness = XMLSupport::parse_floatf(vs_config->getVariable("audio", "explosion_closeness", ".8"));
    loss_relationship = XMLSupport::parse_floatf(vs_config->getVariable("audio", "loss_relationship", "-.1"));
    victory_relationship = XMLSupport::parse_floatf(vs_config->getVariable("audio", "victory_relationship", ".5"));
    time_between_music = XMLSupport::parse_floatf(vs_config->getVariable("audio", "time_between_music", "180"));
    debris_time = XMLSupport::parse_floatf(vs_config->getVariable("physics", "debris_time", "500"));
    /* Cockpit Audio Options */
    comm = vs_config->getVariable("cockpitaudio", "comm", "vdu_c");
    scanning = vs_config->getVariable("cockpitaudio", "scanning", "vdu_c");
    objective = vs_config->getVariable("cockpitaudio", "objective", "vdu_c");
    examine = vs_config->getVariable("cockpitaudio", "examine", "vdu_b");
    view = vs_config->getVariable("cockpitaudio", "view", "vdu_b");
    repair = vs_config->getVariable("cockpitaudio", "repair", "vdu_a");
    manifest = vs_config->getVariable("cockpitaudio", "manifest", "vdu_a");
    compress_max = XMLSupport::parse_int(vs_config->getVariable("cockpitaudio", "compress_max", "3"));
    compress_loop = vs_config->getVariable("cockpitaudio", "compress_loop", "compress_loop");
    compress_change = vs_config->getVariable("cockpitaudio", "compress_change", "compress_burst");
    compress_stop = vs_config->getVariable("cockpitaudio", "compress_stop", "compress_end");
    compress_interval = XMLSupport::parse_int(vs_config->getVariable("cockpitaudio", "compress_interval", "3"));

    comm_preload = XMLSupport::parse_bool(vs_config->getVariable("cockpitaudio", "comm_preload", "false"));

    /* Unit Audio Options */
    jumpleave = vs_config->getVariable("unitaudio", "jumpleave", "sfx43.wav");
    player_armor_hit = vs_config->getVariable("unitaudio", "player_armor_hit", "bigarmor.wav");
    player_hull_hit = vs_config->getVariable("unitaudio", "player_hull_hit", "bigarmor.wav");
    player_shield_hit = vs_config->getVariable("unitaudio", "player_shield_hit", "shieldhit.wav");


    /* Graphics Options */
    jumpgate = configuration()->graphics.jump_gate;
    jumpanimationshrink = configuration()->graphics.jump_animation_shrink;
    jumpgatesize = configuration()->graphics.jump_gate_size;
    camera_pan_speed = configuration()->graphics.camera_pan_speed;
    background = configuration()->graphics.background;
    cockpit = configuration()->graphics.cockpit;
    disabled_cockpit_allowed = configuration()->graphics.disabled_cockpit_allowed;
    splash_screen = configuration()->graphics.splash_screen;
    vbo = configuration()->graphics.vbo;
    num_near_stars = configuration()->graphics.num_near_stars;
    num_far_stars = configuration()->graphics.num_far_stars;
    star_spreading = configuration()->graphics.star_spreading;
    usePlanetAtmosphere = configuration()->graphics.use_planet_atmosphere;
    usePlanetFog = configuration()->graphics.use_planet_fog;
    reflectivity = configuration()->graphics.reflectivity;
    hardware_cursor = configuration()->physics.hardware_cursor;
    always_make_smooth_cam =  configuration()->graphics.always_make_smooth_cam;     //Not used yet
    precull_dist = configuration()->graphics.precull_dist;
    draw_near_stars_in_front_of_planets = configuration()->graphics.draw_near_stars_in_front_of_planets;
    starblend = configuration()->graphics.star_blend;
    sparkleenginesizerelativetoship = configuration()->graphics.sparkle_engine_size_relative_to_ship;
    sparkleabsolutespeed = configuration()->graphics.sparkle_absolute_speed;
    engine_radii_scale = configuration()->graphics.engine_radii_scale;
    engine_length_scale = configuration()->graphics.engine_length_scale;
    halos_by_velocity = configuration()->graphics.halos_by_velocity;
    percent_afterburner_color_change = configuration()->graphics.percent_afterburner_color_change;
    percent_halo_fade_in = configuration()->graphics.percent_halo_fade_in;
    afterburner_color_red = configuration()->graphics.afterburner_color_red;
    afterburner_color_green = configuration()->graphics.afterburner_color_green;
    afterburner_color_blue = configuration()->graphics.afterburner_color_blue;
    engine_color_red = configuration()->graphics.engine_color_red;
    engine_color_green = configuration()->graphics.engine_color_green;
    engine_color_blue = configuration()->graphics.engine_color_blue;
    halosparklerate = configuration()->graphics.halo_sparkle_rate;
    halosparklescale = configuration()->graphics.halo_sparkle_scale;
    halosparklespeed = configuration()->graphics.halo_sparkle_speed;
    max_cubemap_size = configuration()->graphics.max_cubemap_size;
    default_boot_message = configuration()->graphics.default_boot_message;
    initial_boot_message = configuration()->graphics.initial_boot_message;
    splash_audio = configuration()->graphics.splash_audio;
    main_menu = configuration()->graphics.main_menu;
    startup_cockpit_view = configuration()->graphics.startup_cockpit_view;
    detail_texture_trilinear = configuration()->graphics.detail_texture_trilinear;
    lightcutoff = configuration()->graphics.light_cutoff;
    lightoptimalintensity = configuration()->graphics.light_optimal_intensity;
    lightsaturation = configuration()->graphics.light_saturation;
    numlights = configuration()->graphics.num_lights;
    separate_specular_color = configuration()->graphics.separate_specular_color;
    LockVertexArrays = configuration()->graphics.lock_vertex_arrays;
    fogdetail = configuration()->graphics.fog_detail;
    ModelDetail = configuration()->graphics.model_detail;
    UseTextures = configuration()->graphics.use_textures;
    UseShipTextures = configuration()->graphics.use_ship_textures;
    UsePlanetTextures = configuration()->graphics.use_planet_textures;
    UseLogos = configuration()->graphics.use_logos;
    UseVSSprites = configuration()->graphics.use_vs_sprites;
    UseAnimations = configuration()->graphics.use_animations;
    UseVideos = configuration()->graphics.use_videos;
    use_wireframe = configuration()->graphics.use_wireframe;
    max_texture_dimension = configuration()->graphics.max_texture_dimension;
    max_movie_dimension = configuration()->graphics.max_movie_dimension;
    rect_textures = configuration()->graphics.rect_textures;
    pot_video_textures = configuration()->graphics.pot_video_textures;
    techniquesSubPath = configuration()->graphics.technique_set;
    SmoothShade = configuration()->graphics.smooth_shade;
    mipmapdetail = configuration()->graphics.mipmap_detail;
    texture_compression = configuration()->graphics.texture_compression;
    reflection = configuration()->graphics.reflection;
    displaylists = configuration()->graphics.displaylists;
    s3tc = configuration()->graphics.s3tc;
    ext_clamp_to_edge = configuration()->graphics.ext_clamp_to_edge;
    ext_clamp_to_border = configuration()->graphics.ext_clamp_to_border;
    ClearOnStartup = configuration()->graphics.clear_on_startup;
    circle_accuracy = configuration()->graphics.circle_accuracy;
    rgb_pixel_format = configuration()->graphics.rgb_pixel_format;
    gl_accelerated_visual = configuration()->graphics.gl_accelerated_visual;
    z_pixel_format = configuration()->graphics.z_pixel_format;
    x_resolution = configuration()->graphics.resolution_x;
    y_resolution = configuration()->graphics.resolution_y;
    fullscreen = configuration()->graphics.full_screen;
    colordepth = configuration()->graphics.color_depth;
    glut_stencil = configuration()->graphics.glut_stencil;
    // The Following makes no sense.   Why differentiate mac and pc shaders if they have unique names anyway?
    mac_shader_name = configuration()->graphics.mac_shader_name;
    shader_name = configuration()->graphics.shader_name;
    framerate_changes_shader = configuration()->graphics.framerate_changes_shader;
    draw_weapons = configuration()->graphics.draw_weapons;
    sparklerate = configuration()->graphics.sparkle_rate;
    only_stretch_in_warp = configuration()->graphics.only_stretch_in_warp;
    warp_stretch_cutoff = configuration()->graphics.warp_stretch_cutoff;
    warp_stretch_region0_max = configuration()->graphics.warp_stretch_region0_max;
    warp_stretch_max = configuration()->graphics.warp_stretch_max;
    warp_stretch_max_speed = configuration()->graphics.warp_stretch_max_speed;
    warp_stretch_max_region0_speed = configuration()->graphics.warp_stretch_max_region0_speed;
    weapon_gamma = configuration()->graphics.weapon_gamma;
    split_dead_subunits = configuration()->graphics.split_dead_subunits;
    explosionforce = configuration()->graphics.explosion_force;
    explosiontorque = configuration()->graphics.explosion_torque;
    explosion_animation = configuration()->graphics.explosion_animation;
    explosion_face_player = configuration()->graphics.explosion_face_player;
    percent_shockwave = configuration()->graphics.percent_shockwave;
    shockwave_growth = configuration()->graphics.shockwave_growth;
    shockwave_animation = configuration()->graphics.shockwave_animation;
    bolt_offset = configuration()->graphics.bolt_offset;
    StretchBolts = configuration()->graphics.stretch_bolts;
    bolt_pixel_size = configuration()->graphics.bolt_pixel_size;

    /* Graphics/Mesh Options */
    smooth_lines = configuration()->graphics.smooth_lines;
    smooth_points = configuration()->graphics.smooth_points;


    /* Splash Options */
    auto_hide = configuration()->splash.auto_hide;


    /* Terrain Options */
    xscale = configuration()->terrain.xscale;
    yscale = configuration()->terrain.yscale;
    zscale = configuration()->terrain.zscale;
    mass = configuration()->terrain.mass;
    radius = configuration()->terrain.radius;

    /* Player Options */

    /* Joystick Options */
    joystick_exponent = XMLSupport::parse_floatf(vs_config->getVariable("joystick", "joystick_exponent", "1.0"));
    polling_rate = XMLSupport::parse_int(vs_config->getVariable("joystick", "polling_rate", "0"));
    force_use_of_joystick =
            XMLSupport::parse_bool(vs_config->getVariable("joystick", "force_use_of_joystick", "false"));
    debug_digital_hatswitch =
            XMLSupport::parse_bool(vs_config->getVariable("joystick", "debug_digital_hatswitch", "false"));
    deadband = XMLSupport::parse_floatf(vs_config->getVariable("joystick", "deadband", "0.05"));
    mouse_deadband = XMLSupport::parse_floatf(vs_config->getVariable("joystick", "mouse_deadband", "0.025"));
    warp_mouse = XMLSupport::parse_bool(vs_config->getVariable("joystick", "warp_mouse", "false"));
    mouse_sensitivity = XMLSupport::parse_floatf(vs_config->getVariable("joystick", "mouse_sensitivity", "50.0"));
    mouse_exponent = XMLSupport::parse_floatf(vs_config->getVariable("joystick", "mouse_exponent", "3.0"));
    mouse_blur = XMLSupport::parse_floatf(vs_config->getVariable("joystick", "mouse_blur", ".025"));
    force_feedback = XMLSupport::parse_bool(vs_config->getVariable("joystick", "force_feedback", "false"));
    ff_device = XMLSupport::parse_int(vs_config->getVariable("joystick", "ff_device", "0"));
    warp_mouse_zone = XMLSupport::parse_int(vs_config->getVariable("joystick", "warp_mouse_zone", "100"));

    /* AI Options */
    AllowCivilWar = XMLSupport::parse_bool(vs_config->getVariable("AI", "AllowCivilWar", "false"));
    CappedFactionRating = XMLSupport::parse_bool(vs_config->getVariable("AI", "CappedFactionRating", "true"));
    AllowNonplayerFactionChange =
            XMLSupport::parse_bool(vs_config->getVariable("AI", "AllowNonplayerFactionChange", "false"));
    min_relationship = XMLSupport::parse_float(vs_config->getVariable("AI", "min_relationship", "-20.0"));
    startDockedTo = vs_config->getVariable("AI", "startDockedTo", "MiningBase");
    dockOnLoad = XMLSupport::parse_bool(vs_config->getVariable("AI", "dockOnLoad", "true"));

    /* PhysicsConfig Options */
    Drone = vs_config->getVariable("physics", "Drone", "drone");
    max_missions = XMLSupport::parse_int(vs_config->getVariable("physics", "max_missions", "4"));
    game_speed = XMLSupport::parse_float(vs_config->getVariable("physics", "game_speed", "1.0"));
    runtime_compactness = XMLSupport::parse_float(vs_config->getVariable("physics", "runtime_compactness", "1.0"));
    autogen_compactness = XMLSupport::parse_float(vs_config->getVariable("physics", "autogen_compactness", "1.0"));
    AsteroidDifficulty = XMLSupport::parse_float(vs_config->getVariable("physics", "AsteroidDifficulty", ".4"));
    YearScale = XMLSupport::parse_float(vs_config->getVariable("physics", "YearScale", "10.0"));
    game_speed_affects_autogen_systems =
            XMLSupport::parse_bool(vs_config->getVariable("physics", "game_speed_affects_autogen_systems", "false"));
    star_system_scale = XMLSupport::parse_float(vs_config->getVariable("physics", "star_system_scale", "1.0"));
    respawn_unit_size = XMLSupport::parse_float(vs_config->getVariable("physics", "respawn_unit_size", "400.0"));
    campaigns = vs_config->getVariable("physics",
                                       "campaigns",
                                       "privateer_campaign vegastrike_campaign");     //WRONG SECTION   change after 0.5
    NumRunningSystems = XMLSupport::parse_int(vs_config->getVariable("physics", "NumRunningSystems", "4"));
    InactiveSystemTime = XMLSupport::parse_floatf(vs_config->getVariable("physics", "InactiveSystemTime", "0.3"));
    jump_radius_scale = XMLSupport::parse_floatf(vs_config->getVariable("physics", "jump_radius_scale", "2"));
    jump_disables_shields = XMLSupport::parse_bool(vs_config->getVariable("physics", "jump_disables_shields", "true"));
    display_in_meters = XMLSupport::parse_bool(vs_config->getVariable("physics", "display_in_meters", "true"));
    game_speed_lying = XMLSupport::parse_bool(vs_config->getVariable("physics", "game_speed_lying", "true"));
    num_times_to_simulate_new_star_system =
            XMLSupport::parse_int(vs_config->getVariable("physics", "num_times_to_simulate_new_star_system", "20"));
    gun_speed_adjusted_game_speed =
            XMLSupport::parse_bool(vs_config->getVariable("physics", "gun_speed_adjusted_game_speed", "false"));
    gun_speed = XMLSupport::parse_floatf(vs_config->getVariable("physics", "gun_speed", "1"));
    weapon_damage_efficiency =
            XMLSupport::parse_floatf(vs_config->getVariable("physics", "weapon_damage_efficiency", "1.0"));
    refire_difficulty_scaling =
            XMLSupport::parse_floatf(vs_config->getVariable("physics", "refire_difficutly_scaling", "3.0"));
    debris_mass = XMLSupport::parse_floatf(vs_config->getVariable("physics", "debris_mass", ".00001"));
    explosion_damage_center =
            XMLSupport::parse_floatf(vs_config->getVariable("physics", "explosion_damage_center", "1"));
    explosion_damage_edge = configuration()->physics.explosion_damage_edge;
    eject_cargo_on_blowup = XMLSupport::parse_int(vs_config->getVariable("physics", "eject_cargo_on_blowup", "0"));

    /* Data Options */
    universe_path = vs_config->getVariable("data", "universe_path", "universe");
    sectors = vs_config->getVariable("data", "sectors", "sectors");
    techniquesBasePath = vs_config->getVariable("data", "techniques", "techniques");
    modUnitCSV = vs_config->getVariable("data", "ModUnitCSV", "");
    cockpits = vs_config->getVariable("data", "cockpits", "cockpits");
    animations = vs_config->getVariable("data", "animations", "animations");
    movies = vs_config->getVariable("data", "movies", "movies");
    sprites = vs_config->getVariable("data", "sprites", "sprites");
    serialized_xml = vs_config->getVariable("data", "serialized_xml", "serialized_xml");
    sharedtextures = vs_config->getVariable("data", "sharedtextures", "textures");
    sharedsounds = vs_config->getVariable("data", "sharedsounds", "sounds");
    sharedmeshes = vs_config->getVariable("data", "sharedmeshes", "meshes");
    sharedunits = vs_config->getVariable("data", "sharedunits", "units");
    ai_directory = vs_config->getVariable("data", "ai_directory", "ai");
    datadir = vs_config->getVariable("data", "datadir", "");
    hqtextures = vs_config->getVariable("data", "hqtextures", "");
    volume_format = vs_config->getVariable("data", "volume_format", "pk3");

    /* Galaxy Options */
    PushValuesToMean = XMLSupport::parse_bool(vs_config->getVariable("galaxy", "PushValuesToMean", "true"));
    AmbientLightFactor = XMLSupport::parse_floatf(vs_config->getVariable("galaxy", "AmbientLightFactor", "0"));
    StarRadiusScale = XMLSupport::parse_floatf(vs_config->getVariable("galaxy", "StarRadiusScale", "1000"));
    DefaultAtmosphereTexture =
            vs_config->getVariable("galaxy", "DefaultAtmosphereTexture", "sol/earthcloudmaptrans.png");
    AtmosphereProbability = XMLSupport::parse_floatf(vs_config->getVariable("galaxy", "AtmosphereProbability", "0.5"));
    RingProbability = XMLSupport::parse_floatf(vs_config->getVariable("galaxy", "RingProbability", ".1"));
    DoubleRingProbability = XMLSupport::parse_floatf(vs_config->getVariable("galaxy", "DoubleRingProbability", ".025"));
    InnerRingRadius = XMLSupport::parse_floatf(vs_config->getVariable("galaxy", "InnerRingRadius", "1.5"));
    OuterRingRadius = XMLSupport::parse_floatf(vs_config->getVariable("galaxy", "OuterRingRadius", "2.5"));
    DefaultRingTexture = vs_config->getVariable("galaxy", "DefaultRingTexture", "planets/ring.pngwrapx36wrapy2");
    SecondRingDifference = XMLSupport::parse_floatf(vs_config->getVariable("galaxy", "SecondRingDifference", ".4"));
    MoonRelativeToPlanet = XMLSupport::parse_floatf(vs_config->getVariable("galaxy", "MoonRelativeToPlanet", ".4"));
    MoonRelativeToMoon = XMLSupport::parse_floatf(vs_config->getVariable("galaxy", "MoonRelativeToMoon", ".8"));
    RockyRelativeToPrimary =
            XMLSupport::parse_floatf(vs_config->getVariable("galaxy", "RockyRelativeToPrimary", ".05"));
    CompactnessScale = XMLSupport::parse_floatf(vs_config->getVariable("galaxy", "CompactnessScale", "1.5"));
    JumpCompactnessScale = XMLSupport::parse_floatf(vs_config->getVariable("galaxy", "JumpCompactnessScale", "1.5"));
    MeanNaturalPhenomena = XMLSupport::parse_int(vs_config->getVariable("galaxy", "MeanNaturalPhenomena", "1"));
    MeanStarBases = XMLSupport::parse_int(vs_config->getVariable("galaxy", "MeanStarBases", "2"));
//    SmallUnitsMultiplier   = XMLSupport::parse_floatf( vs_config->getVariable( "galaxy", "SmallUnitsMultiplier", "0" ) );

    /* Network Options */
    force_client_connect = XMLSupport::parse_bool(vs_config->getVariable("network", "force_client_connect", "false"));
    use_account_server = XMLSupport::parse_bool(vs_config->getVariable("network", "use_account_server", "true"));
    server_ip = vs_config->getVariable("network", "server_ip", "");
    server_port = vs_config->getVariable("network", "server_port", "6777");
    account_server_url = vs_config->getVariable("network",
            "account_server_url",
            "http://vegastrike.sourceforge.net/cgi-bin/accountserver.py?");
    chat_only_in_network = XMLSupport::parse_bool(vs_config->getVariable("network", "chat_only_in_network", "false"));

    /* Cargo Options */
    news_from_cargolist = XMLSupport::parse_bool(vs_config->getVariable("cargo", "news_from_cargolist", "false"));

    /* Keyboard Options */
    enable_unicode = XMLSupport::parse_bool(vs_config->getVariable("keyboard", "enable_unicode", "true"));

    /* Player Options */
    password = vs_config->getVariable("player", "password", "");
    callsign = vs_config->getVariable("player", "callsign", "");

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
