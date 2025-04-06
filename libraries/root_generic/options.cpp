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
    galaxy = vs_config->getVariable("general", "galaxy", "milky_way.xml");
    command_interpretor = vega_config::config->general.command_interpreter;
    load_last_savegame = vega_config::config->general.load_last_savegame;
    debug_fs = vega_config::config->general.debug_fs;
    simulation_atom = vega_config::config->general.simulation_atom;
    audio_atom = vega_config::config->general.audio_atom;


    /* Audio Options */
    threadtime = vega_config::config->audio.threadtime;
    missionvictorysong = vs_config->getVariable("audio", "missionvictorysong", "../music/victory.ogg");
    Music = vega_config::config->audio.Music;
    sound_gain = vega_config::config->audio.sound_gain;
    audio_ref_distance = vega_config::config->audio.audio_ref_distance;
    audio_max_distance = vega_config::config->audio.audio_max_distance;
    Doppler = vega_config::config->audio.Doppler;
    Positional = vega_config::config->audio.Positional;
    Volume = vega_config::config->audio.Volume;
    DopplerScale = vega_config::config->audio.DopplerScale;
    frequency = vega_config::config->audio.frequency;
    MaxSingleSounds = vega_config::config->audio.MaxSingleSounds;
    MaxTotalSounds = vega_config::config->audio.MaxTotalSounds;
    Sound = vega_config::config->audio.Sound;
    ai_sound = vega_config::config->audio.ai_sound;
    explosion_closeness = vega_config::config->audio.explosion_closeness;
    loss_relationship = vega_config::config->audio.loss_relationship;
    victory_relationship = vega_config::config->audio.victory_relationship;
    time_between_music = vega_config::config->audio.time_between_music;
    debris_time = vega_config::config->physics.debris_time;
    /* Cockpit Audio Options */
    comm = vs_config->getVariable("cockpitaudio", "comm", "vdu_c");
    scanning = vs_config->getVariable("cockpitaudio", "scanning", "vdu_c");
    objective = vs_config->getVariable("cockpitaudio", "objective", "vdu_c");
    examine = vs_config->getVariable("cockpitaudio", "examine", "vdu_b");
    view = vs_config->getVariable("cockpitaudio", "view", "vdu_b");
    repair = vs_config->getVariable("cockpitaudio", "repair", "vdu_a");
    manifest = vs_config->getVariable("cockpitaudio", "manifest", "vdu_a");
    compress_max = vega_config::config->cockpitaudio.compress_max;
    compress_loop = vs_config->getVariable("cockpitaudio", "compress_loop", "compress_loop");
    compress_change = vs_config->getVariable("cockpitaudio", "compress_change", "compress_burst");
    compress_stop = vs_config->getVariable("cockpitaudio", "compress_stop", "compress_end");
    compress_interval = vega_config::config->cockpitaudio.compress_interval;

    comm_preload = vega_config::config->cockpitaudio.comm_preload;

    /* Unit Audio Options */
    jumpleave = vs_config->getVariable("unitaudio", "jumpleave", "sfx43.wav");
    player_armor_hit = vs_config->getVariable("unitaudio", "player_armor_hit", "bigarmor.wav");
    player_hull_hit = vs_config->getVariable("unitaudio", "player_hull_hit", "bigarmor.wav");
    player_shield_hit = vs_config->getVariable("unitaudio", "player_shield_hit", "shieldhit.wav");


    /* Graphics Options */
    jumpgate = vs_config->getVariable("graphics", "jumpgate", "warp.ani");
    jumpanimationshrink = vega_config::config->graphics.jumpanimationshrink
    jumpgatesize = vega_config::config->graphics.jumpgatesize
    camera_pan_speed = vega_config::config->graphics.camera_pan_speed
    background = vega_config::config->graphics.background;
    cockpit = vega_config::config->graphics.cockpit;
    disabled_cockpit_allowed =
            vega_config::config->graphics.disabled_cockpit_allowed;
    splash_screen = vs_config->getVariable("graphics", "splash_screen", "vega_splash.ani");
    vbo = vega_config::config->graphics.vbo;
    num_near_stars = vega_config::config->graphics.num_near_stars;
    num_far_stars = vega_config::config->graphics.num_far_stars;
    star_spreading = vega_config::config->graphics.star_spreading
    usePlanetAtmosphere = vega_config::config->graphics.usePlanetAtmosphere;
    usePlanetFog = vega_config::config->graphics.usePlanetFog;
    reflectivity = vega_config::config->graphics.reflectivity
    hardware_cursor = vega_config::config->physics.hardware_cursor;
    always_make_smooth_cam = XMLSupport::parse_bool(vs_config->getVariable("graphics",
            "always_make_smooth_cam",
            "false"));     //Not used yet
    precull_dist = vega_config::config->graphics.precull_dist;
    draw_near_stars_in_front_of_planets =
            vega_config::config->graphics.draw_near_stars_in_front_of_planets;
    starblend = vega_config::config->graphics.starblend;
    sparkleenginesizerelativetoship =
            vega_config::config->graphics.sparkleenginesizerelativetoship;
    sparkleabsolutespeed = vega_config::config->graphics.sparkleabsolutespeed;
    engine_radii_scale = vega_config::config->graphics.engine_radii_scale;
    engine_length_scale = XMLSupport::parse_floatf(vs_config->getVariable("graphics", "engine_length_scale", "1.25"));
    halos_by_velocity = XMLSupport::parse_bool(vs_config->getVariable("graphics", "halos_by_velocity", "false"));
    percent_afterburner_color_change =
            XMLSupport::parse_floatf(vs_config->getVariable("graphics", "percent_afterburner_color_change", ".5"));
    percent_halo_fade_in = XMLSupport::parse_floatf(vs_config->getVariable("graphics", "percent_halo_fade_in", ".5"));
    afterburner_color_red =
            XMLSupport::parse_floatf(vs_config->getVariable("graphics", "afterburner_color_red", "1.0"));
    afterburner_color_green =
            XMLSupport::parse_floatf(vs_config->getVariable("graphics", "afterburner_color_green", "0.0"));
    afterburner_color_blue =
            XMLSupport::parse_floatf(vs_config->getVariable("graphics", "afterburner_color_blue", "0.0"));
    engine_color_red = XMLSupport::parse_floatf(vs_config->getVariable("graphics", "engine_color_red", "1.0"));
    engine_color_green = XMLSupport::parse_floatf(vs_config->getVariable("graphics", "engine_color_green", "1.0"));
    engine_color_blue = XMLSupport::parse_floatf(vs_config->getVariable("graphics", "engine_color_blue", "1.0"));
    halosparklerate = XMLSupport::parse_floatf(vs_config->getVariable("graphics", "halosparklerate", "20"));
    halosparklescale = XMLSupport::parse_floatf(vs_config->getVariable("graphics", "halosparklescale", "6"));
    halosparklespeed = XMLSupport::parse_floatf(vs_config->getVariable("graphics", "halosparklespeed", "0.5"));
    max_cubemap_size = XMLSupport::parse_int(vs_config->getVariable("graphics", "max_cubemap_size", "1024"));
    default_boot_message = vs_config->getVariable("graphics", "default_boot_message", "");
    initial_boot_message = vs_config->getVariable("graphics", "initial_boot_message", "Loading...");
    splash_audio = vs_config->getVariable("graphics", "splash_audio", "");
    main_menu = XMLSupport::parse_bool(vs_config->getVariable("graphics", "main_menu", "false"));
    startup_cockpit_view = vs_config->getVariable("graphics", "startup_cockpit_view", "front");
    detail_texture_trilinear =
            XMLSupport::parse_bool(vs_config->getVariable("graphics", "detail_texture_trilinear", "true"));
    lightcutoff = XMLSupport::parse_floatf(vs_config->getVariable("graphics", "lightcutoff", ".06"));
    lightoptimalintensity =
            XMLSupport::parse_floatf(vs_config->getVariable("graphics", "lightoptimalintensity", ".06"));
    lightsaturation = XMLSupport::parse_floatf(vs_config->getVariable("graphics", "lightsaturation", ".95"));
    numlights = XMLSupport::parse_int(vs_config->getVariable("graphics", "numlights", "4"));
    separatespecularcolor =
            XMLSupport::parse_bool(vs_config->getVariable("graphics", "separatespecularcolor", "false"));
    LockVertexArrays = XMLSupport::parse_bool(vs_config->getVariable("graphics", "LockVertexArrays", "true"));
    fogdetail = XMLSupport::parse_int(vs_config->getVariable("graphics", "fogdetail", "0"));
    fov = XMLSupport::parse_floatf(vs_config->getVariable("graphics", "fov", "78"));
    aspect = XMLSupport::parse_floatf(vs_config->getVariable("graphics", "aspect", "1.33"));
    znear = XMLSupport::parse_floatf(vs_config->getVariable("graphics", "znear", "1"));
    zfar = XMLSupport::parse_floatf(vs_config->getVariable("graphics", "zfar", "100000"));
    ModelDetail = XMLSupport::parse_floatf(vs_config->getVariable("graphics", "ModelDetail", "1"));
    UseTextures = XMLSupport::parse_bool(vs_config->getVariable("graphics", "UseTextures", "true"));
    UseShipTextures = XMLSupport::parse_bool(vs_config->getVariable("graphics", "UseShipTextures", "false"));
    UsePlanetTextures = XMLSupport::parse_bool(vs_config->getVariable("graphics", "UsePlanetTextures", "false"));
    UseLogos = XMLSupport::parse_bool(vs_config->getVariable("graphics", "UseLogos", "true"));
    UseVSSprites = XMLSupport::parse_bool(vs_config->getVariable("graphics", "UseVSSprites", "true"));
    UseAnimations = XMLSupport::parse_bool(vs_config->getVariable("graphics", "UseAnimations", "true"));
    UseVideos = XMLSupport::parse_bool(vs_config->getVariable("graphics", "UseVideos", "true"));
    use_wireframe = XMLSupport::parse_bool(vs_config->getVariable("graphics", "use_wireframe", "false"));
    max_texture_dimension = XMLSupport::parse_int(vs_config->getVariable("graphics", "max_texture_dimension", "65536"));
    max_movie_dimension = XMLSupport::parse_int(vs_config->getVariable("graphics", "max_movie_dimension", "65536"));
    rect_textures = XMLSupport::parse_bool(vs_config->getVariable("graphics", "rect_textures", "false"));
    pot_video_textures = XMLSupport::parse_bool(vs_config->getVariable("graphics", "pot_video_textures", "false"));
    techniquesSubPath = vs_config->getVariable("graphics", "technique_set", ".");
    SmoothShade = XMLSupport::parse_bool(vs_config->getVariable("graphics", "SmoothShade", "true"));
    mipmapdetail = XMLSupport::parse_int(vs_config->getVariable("graphics", "mipmapdetail", "2"));
    texture_compression = XMLSupport::parse_int(vs_config->getVariable("graphics", "texture_compression", "0"));
    reflection = XMLSupport::parse_bool(vs_config->getVariable("graphics", "reflection", "true"));
    displaylists = XMLSupport::parse_bool(vs_config->getVariable("graphics", "displaylists", "false"));
    s3tc = XMLSupport::parse_bool(vs_config->getVariable("graphics", "s3tc", "true"));
    ext_clamp_to_edge = XMLSupport::parse_bool(vs_config->getVariable("graphics", "ext_clamp_to_edge", "true"));
    ext_clamp_to_border = XMLSupport::parse_bool(vs_config->getVariable("graphics", "ext_clamp_to_border", "true"));
    ClearOnStartup = XMLSupport::parse_bool(vs_config->getVariable("graphics", "ClearOnStartup", "true"));
    circle_accuracy = XMLSupport::parse_floatf(vs_config->getVariable("graphics", "circle_accuracy", "0.1"));
    rgb_pixel_format = vs_config->getVariable("graphics", "rgb_pixel_format", "undefined");
    gl_accelerated_visual = XMLSupport::parse_bool(vs_config->getVariable("graphics", "gl_accelerated_visual", "true"));
    z_pixel_format = XMLSupport::parse_int(vs_config->getVariable("graphics", "z_pixel_format", "24"));
    x_resolution = vega_config::config->graphics.resolution_x;
    y_resolution = vega_config::config->graphics.resolution_y;
    fullscreen = XMLSupport::parse_bool(vs_config->getVariable("graphics", "fullscreen", "false"));
    colordepth = XMLSupport::parse_int(vs_config->getVariable("graphics", "colordepth", "32"));
    glut_stencil = XMLSupport::parse_bool(vs_config->getVariable("graphics", "glut_stencil", "true"));
    // The Following makes no sense.   Why differentiate mac and pc shaders if they have unique names anyway?
    mac_shader_name = vs_config->getVariable("graphics", "mac_shader_name", "mac");
    shader_name = vs_config->getVariable("graphics", "shader_name", "default");
    framerate_changes_shader =
            XMLSupport::parse_bool(vs_config->getVariable("graphics", "framerate_changes_shader", "false"));
    draw_weapons = XMLSupport::parse_bool(vs_config->getVariable("graphics", "draw_weapons", "false"));
    sparklerate = XMLSupport::parse_floatf(vs_config->getVariable("graphics", "sparklerate", "5.0"));
    only_stretch_in_warp = XMLSupport::parse_bool(vs_config->getVariable("graphics", "only_stretch_in_warp", "true"));
    warp_stretch_cutoff = XMLSupport::parse_floatf(vs_config->getVariable("graphics", "warp_stretch_cutoff", "50000"));
    warp_stretch_region0_max =
            XMLSupport::parse_floatf(vs_config->getVariable("graphics", "warp_stretch_region0_max", "1"));
    warp_stretch_max = XMLSupport::parse_floatf(vs_config->getVariable("graphics", "warp_stretch_max", "4"));
    warp_stretch_max_speed =
            XMLSupport::parse_floatf(vs_config->getVariable("graphics", "warp_stretch_max_speed", "1000000"));
    warp_stretch_max_region0_speed =
            XMLSupport::parse_floatf(vs_config->getVariable("graphics", "warp_stretch_max_region0_speed", "100000"));
    weapon_gamma = XMLSupport::parse_floatf(vs_config->getVariable("graphics", "weapon_gamma", "1.35"));
    split_dead_subunits = XMLSupport::parse_bool(vs_config->getVariable("graphics", "split_dead_subunits", "true"));
    explosionforce = XMLSupport::parse_floatf(vs_config->getVariable("graphics", "explosionforce", ".5"));
    explosiontorque = XMLSupport::parse_floatf(vs_config->getVariable("graphics", "explosiontorque", ".001"));
    explosion_animation = vs_config->getVariable("graphics", "explosion_animation", "explosion_orange.ani");
    explosion_face_player = XMLSupport::parse_bool(vs_config->getVariable("graphics", "explosion_face_player", "true"));
    percent_shockwave = XMLSupport::parse_floatf(vs_config->getVariable("graphics", "percent_shockwave", ".5"));
    shockwave_growth = XMLSupport::parse_floatf(vs_config->getVariable("graphics", "shockwave_growth", "1.05"));
    shockwave_animation = vs_config->getVariable("graphics", "shockwave_animation", "explosion_wave.ani");
    bolt_offset = XMLSupport::parse_floatf(vs_config->getVariable("graphics", "bolt_offset", "-.2"));
    BlendGuns = XMLSupport::parse_bool(vs_config->getVariable("graphics", "BlendGuns", "true"));
    StretchBolts = XMLSupport::parse_floatf(vs_config->getVariable("graphics", "StretchBolts", "0"));
    bolt_pixel_size = XMLSupport::parse_floatf(vs_config->getVariable("graphics", "bolt_pixel_size", ".5"));

    /* Graphics/Mesh Options */
    smooth_lines = XMLSupport::parse_bool(vs_config->getVariable("graphics/mesh", "smooth_lines", "true"));
    smooth_points = XMLSupport::parse_bool(vs_config->getVariable("graphics/mesh", "smooth_points", "true"));


    /* Splash Options */
    auto_hide = XMLSupport::parse_bool(vs_config->getVariable("splash", "auto_hide", "true"));


    /* Terrain Options */
    xscale = vega_config::config->terrain.xscale;
    yscale = vega_config::config->terrain.yscale;
    zscale = vega_config::config->terrain.zscale;
    mass = vega_config::config->terrain.mass;
    radius = vega_config::config->terrain.radius;

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
    AllowCivilWar = vega_config::config->ai.AllowCivilWar;
    CappedFactionRating = vega_config::config->ai.CappedFactionRating;
    AllowNonplayerFactionChange =
            vega_config::config->ai.AllowNonplayerFactionChange;
    min_relationship = vega_config::config->ai.min_relationship;
    startDockedTo = vs_config->getVariable("AI", "startDockedTo", "MiningBase");
    dockOnLoad = vega_config::config->ai.dockOnLoad;

    /* PhysicsConfig Options */
    Drone = vs_config->getVariable("physics", "Drone", "drone");
    max_missions = XMLSupport::parse_int(vs_config->getVariable("physics", "max_missions", "4"));
    game_speed = vega_config::config->physics.game_speed;
    runtime_compactness = vega_config::config->physics.runtime_compactness;
    autogen_compactness = vega_config::config->physics.autogen_compactness;
    AsteroidDifficulty = vega_config::config->physics.AsteroidDifficulty;
    YearScale = vega_config::config->physics.YearScale;
    game_speed_affects_autogen_systems =
            XMLSupport::parse_bool(vs_config->getVariable("physics", "game_speed_affects_autogen_systems", "false"));
    star_system_scale = vega_config::config->physics.star_system_scale;
    respawn_unit_size = vega_config::config->physics.respawn_unit_size;
    auto_pilot_planet_radius_percent =
            XMLSupport::parse_floatf(vs_config->getVariable("physics", "auto_pilot_planet_radius_percent", ".75"));
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
    explosion_damage_edge =
            XMLSupport::parse_floatf(vs_config->getVariable("graphics", "explosion_damage_edge", ".125"));
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
