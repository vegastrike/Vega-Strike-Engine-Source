/**
* options.cpp
*
* Copyright (c) 2001-2002 Daniel Horn
* Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
* Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
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

#include "options.h"
#include "configxml.h"

extern VegaConfig *vs_config;

void vs_options::init()
{
    /* General Options */
    write_savegame_on_exit   = XMLSupport::parse_bool( vs_config->getVariable( "general", "write_savegame_on_exit", "true" ) );
    times_to_show_help_screen = XMLSupport::parse_int( vs_config->getVariable( "general", "times_to_show_help_screen", "3" ) );
    remember_savegame        = XMLSupport::parse_bool( vs_config->getVariable( "general", "remember_savegame", "true" ) );
    new_game_save_name       = vs_config->getVariable( "general", "new_game_save_name", "New_Game" );
    quick_savegame_summaries_buffer = XMLSupport::parse_int( vs_config->getVariable( "general", "quick_savegame_summaries_buffer", "16384" ) );
    empty_mission            = vs_config->getVariable( "general", "empty_mission", "internal.mission" );
    custompython             = vs_config->getVariable( "general", "custompython", "import custom;custom.processMessage" );
    quick_savegame_summaries = XMLSupport::parse_bool( vs_config->getVariable( "general", "quick_savegame_summaries", "true" ) );
    garbagecollectfrequency  = XMLSupport::parse_int( vs_config->getVariable( "general", "garbagecollectfrequency", "20" ) );
    numoldsystems            = XMLSupport::parse_int( vs_config->getVariable( "general", "numoldsystems", "6" ) );
    deleteoldsystems         = XMLSupport::parse_bool( vs_config->getVariable( "general", "deleteoldsystems", "true" ) );
    vsdebug                  = XMLSupport::parse_int(vs_config->getVariable("general", "verbose_output", "0" ));
    while_loading_starsystem = XMLSupport::parse_bool( vs_config->getVariable( "general", "while_loading_starsystem", "false" ) );
    default_mission          = vs_config->getVariable( "general", "default_mission", "test/test1.mission" );
    galaxy                   = vs_config->getVariable( "general", "galaxy", "milky_way.xml" );
    command_interpretor      = XMLSupport::parse_bool( vs_config->getVariable( "general", "command_interpretor", "false" ) );
    load_last_savegame       = XMLSupport::parse_bool( vs_config->getVariable( "general", "load_last_savegame", "false" ) );
    intro1                   = vs_config->getVariable("general","intro1","Welcome to Vega Strike! Use #8080FFTab#000000 to afterburn (#8080FF+,-#000000 cruise control), #8080FFarrows#000000 to steer." );
    intro2                   = vs_config->getVariable("general","intro2","The #8080FFt#000000 key targets objects; #8080FFspace#000000 fires at them & #8080FFa#000000 activates the SPEC drive. To" );
    intro3                   = vs_config->getVariable("general","intro3","go to another star system, buy a jump drive for about 10000 credits, fly to a" );
    intro4                   = vs_config->getVariable("general","intro4","wireframe jump-point and press #8080FFj#000000 to warp to a near star. Target a base or planet;" );
    intro5                   = vs_config->getVariable("general","intro5","When you get close a green box will appear. Inside the box, #8080FFd#000000 will land." );
    debug_fs                 = XMLSupport::parse_int( vs_config->getVariable( "general", "debug_fs", "0" ) );
    simulation_atom          = XMLSupport::parse_floatf( vs_config->getVariable( "general", "simulation_atom", "0.1" ) );
    audio_atom               = XMLSupport::parse_floatf( vs_config->getVariable( "general", "audio_atom", "0.05555555556" ) );


    /* Audio Options */
    threadtime               = XMLSupport::parse_int( vs_config->getVariable( "audio", "threadtime", "1" ) );
    missionvictorysong       = vs_config->getVariable( "audio", "missionvictorysong", "../music/victory.ogg" );
    Music                    = XMLSupport::parse_bool( vs_config->getVariable( "audio", "Music", "true" ) );
    sound_gain               = XMLSupport::parse_floatf( vs_config->getVariable( "audio", "sound_gain", ".5" ) );
    audio_ref_distance       = XMLSupport::parse_floatf( vs_config->getVariable( "audio", "audio_ref_distance", "4000" ) );
    audio_max_distance       = XMLSupport::parse_floatf( vs_config->getVariable( "audio", "audio_max_distance", "1000000" ) );
    Doppler                  = XMLSupport::parse_bool( vs_config->getVariable( "audio", "Doppler", "false" ) );
    Positional               = XMLSupport::parse_bool( vs_config->getVariable( "audio", "Positional", "true" ) );
    Volume                   = XMLSupport::parse_floatf( vs_config->getVariable( "audio", "Volume", "100" ) );
    DopplerScale             = XMLSupport::parse_floatf( vs_config->getVariable( "audio", "DopplerScale", "1" ) );
    frequency                = XMLSupport::parse_int( vs_config->getVariable( "audio", "frequency", "48000" ) );
    MaxSingleSounds          = XMLSupport::parse_int( vs_config->getVariable( "audio", "MaxSingleSounds", "8" ) );
    MaxTotalSounds           = XMLSupport::parse_int( vs_config->getVariable( "audio", "MaxTotalSounds", "20" ) );
    Sound                    = XMLSupport::parse_bool( vs_config->getVariable( "audio", "Sound", "true" ) );
    ai_sound                 = XMLSupport::parse_bool( vs_config->getVariable( "audio", "ai_sound", "true" ) );
    explosion_closeness      = XMLSupport::parse_floatf( vs_config->getVariable( "audio", "explosion_closeness", ".8" ) );
    loss_relationship        = XMLSupport::parse_floatf( vs_config->getVariable( "audio", "loss_relationship", "-.1" ) );
    victory_relationship     = XMLSupport::parse_floatf( vs_config->getVariable( "audio", "victory_relationship", ".5" ) );
    time_between_music       = XMLSupport::parse_floatf( vs_config->getVariable( "audio", "time_between_music", "180" ) );
    debris_time		     = XMLSupport::parse_floatf( vs_config->getVariable( "physics", "debris_time", "500" ) );
    /* Cockpit Audio Options */
    comm                     = vs_config->getVariable( "cockpitaudio", "comm", "vdu_c" );
    scanning                 = vs_config->getVariable( "cockpitaudio", "scanning", "vdu_c" );
    objective                = vs_config->getVariable( "cockpitaudio", "objective", "vdu_c" );
    examine                  = vs_config->getVariable( "cockpitaudio", "examine", "vdu_b" );
    view                     = vs_config->getVariable( "cockpitaudio", "view", "vdu_b" );
    repair                   = vs_config->getVariable( "cockpitaudio", "repair", "vdu_a" );
    manifest                 = vs_config->getVariable( "cockpitaudio", "manifest", "vdu_a" );
    compress_max             = XMLSupport::parse_int( vs_config->getVariable( "cockpitaudio", "compress_max", "3" ) );
    compress_loop            = vs_config->getVariable( "cockpitaudio", "compress_loop", "compress_loop" );
    compress_change          = vs_config->getVariable( "cockpitaudio", "compress_change", "compress_burst" );
    compress_stop            = vs_config->getVariable( "cockpitaudio", "compress_stop", "compress_end" );
    compress_interval        = XMLSupport::parse_int( vs_config->getVariable( "cockpitaudio", "compress_interval", "3" ) );

    comm_preload             = XMLSupport::parse_bool( vs_config->getVariable( "cockpitaudio", "comm_preload", "false" ) );

    /* Unit Audio Options */
    jumpleave                = vs_config->getVariable( "unitaudio", "jumpleave", "sfx43.wav" );
    player_armor_hit         = vs_config->getVariable( "unitaudio", "player_armor_hit", "bigarmor.wav" );
    player_hull_hit          = vs_config->getVariable( "unitaudio", "player_hull_hit", "bigarmor.wav" );
    player_shield_hit        = vs_config->getVariable( "unitaudio", "player_shield_hit", "shieldhit.wav" );


    /* Graphics Options */
    jumpgate                 = vs_config->getVariable( "graphics", "jumpgate", "warp.ani" );
    jumpanimationshrink      = XMLSupport::parse_float( vs_config->getVariable( "graphics", "jumpanimationshrink", ".95" ) );
    jumpgatesize             = XMLSupport::parse_float( vs_config->getVariable( "graphics", "jumpgatesize", "1.75" ) );
    switchToTargetModeOnKey  = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "hud", "switchToTargetModeOnKey", "true" ) );
    camera_pan_speed         = XMLSupport::parse_float( vs_config->getVariable( "graphics", "camera_pan_speed", "0.0001" ) );
    background               = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "background", "true" ) );
    cockpit                  = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "cockpit", "true" ) );
    disabled_cockpit_allowed = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "disabled_cockpit_allowed", "true" ) );
    splash_screen            = vs_config->getVariable( "graphics", "splash_screen", "vega_splash.ani" );
    vbo = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "vbo", "false" ) );
    num_near_stars           = XMLSupport::parse_int( vs_config->getVariable( "graphics", "num_near_stars", "1000" ) );
    num_far_stars            = XMLSupport::parse_int( vs_config->getVariable( "graphics", "num_far_stars", "2000" ) );
    star_spreading           = XMLSupport::parse_float( vs_config->getVariable( "graphics", "star_spreading", "30000" ) );
    usePlanetAtmosphere      = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "usePlanetAtmosphere", "true" ) );
    usePlanetFog             = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "usePlanetFog", "true" ) );
    reflectivity             = XMLSupport::parse_float( vs_config->getVariable( "graphics", "reflectivity", ".2" ) );
    hardware_cursor          = XMLSupport::parse_bool( vs_config->getVariable( "physics", "hardware_cursor", "false" ) );
    always_make_smooth_cam   = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "always_make_smooth_cam", "false" ) );     //Not used yet
    precull_dist             = XMLSupport::parse_float( vs_config->getVariable( "graphics", "precull_dist", "500000000" ) );
    draw_near_stars_in_front_of_planets = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "draw_near_stars_in_front_of_planets", "false" ) );
    starblend                = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "starblend", "true" ) );
    sparkleenginesizerelativetoship = XMLSupport::parse_floatf( vs_config->getVariable( "graphics", "sparkleenginesizerelativetoship", "0.1875" ) );
    sparkleabsolutespeed     = XMLSupport::parse_floatf( vs_config->getVariable(  "graphics", "sparkleabsolutespeed", ".02" ) );
    engine_radii_scale       = XMLSupport::parse_floatf( vs_config->getVariable( "graphics", "engine_radii_scale", ".4" ) );
    engine_length_scale      = XMLSupport::parse_floatf( vs_config->getVariable( "graphics", "engine_length_scale", "1.25" ) );
    halos_by_velocity        = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "halos_by_velocity","false" ) );
    percent_afterburner_color_change = XMLSupport::parse_floatf(vs_config->getVariable("graphics","percent_afterburner_color_change",".5"));
    percent_halo_fade_in     = XMLSupport::parse_floatf(vs_config->getVariable("graphics","percent_halo_fade_in",".5"));
    afterburner_color_red    = XMLSupport::parse_floatf(vs_config->getVariable("graphics","afterburner_color_red","1.0"));
    afterburner_color_green  = XMLSupport::parse_floatf(vs_config->getVariable("graphics","afterburner_color_green","0.0"));
    afterburner_color_blue   = XMLSupport::parse_floatf(vs_config->getVariable("graphics","afterburner_color_blue","0.0"));
    engine_color_red         = XMLSupport::parse_floatf(vs_config->getVariable("graphics","engine_color_red","1.0"));
    engine_color_green       = XMLSupport::parse_floatf(vs_config->getVariable("graphics","engine_color_green","1.0"));
    engine_color_blue        = XMLSupport::parse_floatf(vs_config->getVariable("graphics","engine_color_blue","1.0"));
    halosparklerate          = XMLSupport::parse_floatf( vs_config->getVariable( "graphics", "halosparklerate", "20" ) );
    halosparklescale         = XMLSupport::parse_floatf( vs_config->getVariable( "graphics", "halosparklescale", "6" ) );
    halosparklespeed         = XMLSupport::parse_floatf( vs_config->getVariable( "graphics", "halosparklespeed", "0.5" ) );
    max_cubemap_size         = XMLSupport::parse_int( vs_config->getVariable( "graphics", "max_cubemap_size", "1024" ) );
    default_boot_message     = vs_config->getVariable( "graphics", "default_boot_message", "" );
    initial_boot_message     = vs_config->getVariable( "graphics", "initial_boot_message", "Loading..." );
    splash_audio             = vs_config->getVariable( "graphics", "splash_audio", "" );
    main_menu                = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "main_menu", "false" ) );
    startup_cockpit_view     = vs_config->getVariable( "graphics", "startup_cockpit_view", "front" );
    detail_texture_trilinear = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "detail_texture_trilinear","true" ) );
    lightcutoff              = XMLSupport::parse_floatf( vs_config->getVariable( "graphics", "lightcutoff", ".06" ) ); 
    lightoptimalintensity    = XMLSupport::parse_floatf( vs_config->getVariable( "graphics", "lightoptimalintensity", ".06" ) );
    lightsaturation          = XMLSupport::parse_floatf( vs_config->getVariable( "graphics", "lightsaturation", ".95" ) );
    numlights                = XMLSupport::parse_int( vs_config->getVariable( "graphics", "numlights", "4" ) );
    separatespecularcolor    = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "separatespecularcolor", "false" ) );
    LockVertexArrays         = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "LockVertexArrays", "true" ) );
    fogdetail                = XMLSupport::parse_int( vs_config->getVariable( "graphics", "fogdetail", "0" ) );
    fov                      = XMLSupport::parse_floatf( vs_config->getVariable( "graphics", "fov", "78" ) );
    aspect                   = XMLSupport::parse_floatf( vs_config->getVariable( "graphics", "aspect", "1.33" ) );
    znear                    = XMLSupport::parse_floatf( vs_config->getVariable( "graphics", "znear", "1" ) );    
    zfar                     = XMLSupport::parse_floatf( vs_config->getVariable( "graphics", "zfar", "100000" ) );    
    ModelDetail              = XMLSupport::parse_floatf( vs_config->getVariable( "graphics", "ModelDetail", "1" ) );
    UseTextures              = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "UseTextures", "true" ) );
    UseShipTextures          = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "UseShipTextures", "false" ) );
    UsePlanetTextures        = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "UsePlanetTextures", "false" ) );
    UseLogos                 = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "UseLogos", "true" ) );
    UseVSSprites             = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "UseVSSprites", "true" ) );
    UseAnimations            = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "UseAnimations", "true" ) );
    UseVideos                = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "UseVideos", "true" ) );
    use_wireframe            = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "use_wireframe", "false" ) );
    max_texture_dimension    = XMLSupport::parse_int( vs_config->getVariable( "graphics", "max_texture_dimension", "65536" ) );
    max_movie_dimension      = XMLSupport::parse_int( vs_config->getVariable( "graphics", "max_movie_dimension", "65536" ) );
    rect_textures            = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "rect_textures", "false") );
    pot_video_textures       = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "pot_video_textures", "false") );
    techniquesSubPath        = vs_config->getVariable( "graphics", "technique_set", "." );
    SmoothShade              = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "SmoothShade", "true" ) );
    mipmapdetail             = XMLSupport::parse_int( vs_config->getVariable( "graphics", "mipmapdetail", "2" ) );
    texture_compression      = XMLSupport::parse_int( vs_config->getVariable( "graphics", "texture_compression", "0" ) );
    reflection               = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "reflection", "true" ) );
    displaylists             = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "displaylists", "false" ) );
    s3tc                     = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "s3tc", "true" ) );
    ext_clamp_to_edge        = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "ext_clamp_to_edge", "true" ) );
    ext_clamp_to_border      = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "ext_clamp_to_border", "true" ) );
    ClearOnStartup           = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "ClearOnStartup", "true" ) );
    circle_accuracy          = XMLSupport::parse_floatf( vs_config->getVariable( "graphics", "circle_accuracy", "0.1" ) );
    rgb_pixel_format         = vs_config->getVariable( "graphics", "rgb_pixel_format", "undefined" );
    gl_accelerated_visual    = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "gl_accelerated_visual", "true" ) );
    z_pixel_format           = XMLSupport::parse_int( vs_config->getVariable( "graphics", "z_pixel_format", "24" ) );
    x_resolution             = XMLSupport::parse_int( vs_config->getVariable( "graphics", "x_resolution", "1024" ) );
    y_resolution             = XMLSupport::parse_int( vs_config->getVariable( "graphics", "y_resolution", "768" ) );
    fullscreen               = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "fullscreen", "false" ) );
    colordepth               = XMLSupport::parse_int( vs_config->getVariable( "graphics", "colordepth", "32" ) );
    glut_stencil             = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "glut_stencil", "true" ) );
    // The Following makes no sense.   Why differentiate mac and pc shaders if they have unique names anyway?
    mac_shader_name          = vs_config->getVariable( "graphics", "mac_shader_name", "mac" );
    shader_name              = vs_config->getVariable( "graphics", "shader_name", "default" );
    framerate_changes_shader = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "framerate_changes_shader", "false" ) );
    draw_weapons             = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "draw_weapons", "false" ) );
    sparklerate              = XMLSupport::parse_floatf( vs_config->getVariable( "graphics", "sparklerate", "5.0" ) );
    only_stretch_in_warp     = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "only_stretch_in_warp", "true" ) );
    warp_stretch_cutoff      = XMLSupport::parse_floatf( vs_config->getVariable( "graphics", "warp_stretch_cutoff", "50000") );
    warp_stretch_region0_max = XMLSupport::parse_floatf( vs_config->getVariable( "graphics", "warp_stretch_region0_max", "1" ) );
    warp_stretch_max         = XMLSupport::parse_floatf( vs_config->getVariable( "graphics", "warp_stretch_max", "4" ) );
    warp_stretch_max_speed   = XMLSupport::parse_floatf( vs_config->getVariable( "graphics", "warp_stretch_max_speed", "1000000" ) );
    warp_stretch_max_region0_speed = XMLSupport::parse_floatf( vs_config->getVariable( "graphics", "warp_stretch_max_region0_speed","100000" ) );
    weapon_gamma             = XMLSupport::parse_floatf( vs_config->getVariable( "graphics", "weapon_gamma", "1.35" ) );
    projectile_means_missile = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "hud", "projectile_means_missile", "false" ) );
    split_dead_subunits      = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "split_dead_subunits", "true" ) );
    explosionforce           = XMLSupport::parse_floatf( vs_config->getVariable( "graphics", "explosionforce", ".5" ) );
    explosiontorque	     = XMLSupport::parse_floatf( vs_config->getVariable( "graphics", "explosiontorque", ".001" ) );
    explosion_animation      = vs_config->getVariable( "graphics", "explosion_animation", "explosion_orange.ani" );
    explosion_face_player    = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "explosion_face_player", "true" ) );
    percent_shockwave        = XMLSupport::parse_floatf( vs_config->getVariable( "graphics", "percent_shockwave", ".5" ) );
    shockwave_growth	     = XMLSupport::parse_floatf( vs_config->getVariable( "graphics", "shockwave_growth", "1.05" ) );
    shockwave_animation      = vs_config->getVariable( "graphics", "shockwave_animation", "explosion_wave.ani" );
    bolt_offset              = XMLSupport::parse_floatf( vs_config->getVariable( "graphics", "bolt_offset", "-.2" ) );
    BlendGuns                = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "BlendGuns", "true" ) );
    StretchBolts             = XMLSupport::parse_floatf( vs_config->getVariable( "graphics", "StretchBolts", "0" ) );
    bolt_pixel_size          = XMLSupport::parse_floatf( vs_config->getVariable( "graphics", "bolt_pixel_size", ".5" ) );

    /* Graphics/Mesh Options */
    smooth_lines             = XMLSupport::parse_bool( vs_config->getVariable( "graphics/mesh", "smooth_lines", "true" ) );
    smooth_points            = XMLSupport::parse_bool( vs_config->getVariable( "graphics/mesh", "smooth_points", "true" ) );


    /* Splash Options */
    auto_hide                = XMLSupport::parse_bool(vs_config->getVariable( "splash", "auto_hide", "true" ) );


    /* Terrain Options */
    xscale                   = XMLSupport::parse_float( vs_config->getVariable( "terrain", "xscale", "1.0" ) );
    yscale                   = XMLSupport::parse_float( vs_config->getVariable( "terrain", "yscale", "1.0" ) );
    zscale                   = XMLSupport::parse_float( vs_config->getVariable( "terrain", "zscale", "1.0" ) );
    mass                     = XMLSupport::parse_float( vs_config->getVariable( "terrain", "mass", "100.0" ) );
    radius                   = XMLSupport::parse_float( vs_config->getVariable( "terrain", "radius", "10000.0" ) );

    /* Player Options */

    /* Joystick Options */
    joystick_exponent        = XMLSupport::parse_floatf( vs_config->getVariable( "joystick", "joystick_exponent", "1.0" ) );
    polling_rate             = XMLSupport::parse_int( vs_config->getVariable( "joystick", "polling_rate", "0" ) );
    force_use_of_joystick    = XMLSupport::parse_bool( vs_config->getVariable( "joystick", "force_use_of_joystick", "false" ) );
    debug_digital_hatswitch  = XMLSupport::parse_bool( vs_config->getVariable( "joystick", "debug_digital_hatswitch", "false" ) );
    deadband                 = XMLSupport::parse_floatf( vs_config->getVariable( "joystick", "deadband", "0.05" ) );
    mouse_deadband           = XMLSupport::parse_floatf( vs_config->getVariable( "joystick", "mouse_deadband", "0.025" ) );
    warp_mouse               = XMLSupport::parse_bool( vs_config->getVariable( "joystick", "warp_mouse", "false" ) );
    mouse_sensitivity        = XMLSupport::parse_floatf( vs_config->getVariable( "joystick", "mouse_sensitivity", "50.0" ) );
    mouse_exponent           = XMLSupport::parse_floatf( vs_config->getVariable( "joystick", "mouse_exponent", "3.0" ) );
    mouse_blur               = XMLSupport::parse_floatf( vs_config->getVariable( "joystick", "mouse_blur", ".025" ) );
    force_feedback           = XMLSupport::parse_bool( vs_config->getVariable( "joystick", "force_feedback", "false" ) );
    ff_device                = XMLSupport::parse_int( vs_config->getVariable( "joystick", "ff_device", "0" ) );
    warp_mouse_zone          = XMLSupport::parse_int( vs_config->getVariable( "joystick", "warp_mouse_zone", "100") );

    /* AI Options */
    AllowCivilWar            = XMLSupport::parse_bool( vs_config->getVariable( "AI", "AllowCivilWar", "false" ) );
    CappedFactionRating      = XMLSupport::parse_bool( vs_config->getVariable( "AI", "CappedFactionRating", "true" ) );
    AllowNonplayerFactionChange = XMLSupport::parse_bool( vs_config->getVariable( "AI", "AllowNonplayerFactionChange", "false" ) );
    min_relationship         = XMLSupport::parse_float( vs_config->getVariable( "AI", "min_relationship", "-20.0" ) );
    startDockedTo            = vs_config->getVariable( "AI", "startDockedTo", "MiningBase" );
    dockOnLoad               = XMLSupport::parse_bool( vs_config->getVariable( "AI", "dockOnLoad", "true" ) );

    /* Physics Options */
    Drone                    = vs_config->getVariable( "physics", "Drone", "drone" );
    max_missions             = XMLSupport::parse_int( vs_config->getVariable( "physics", "max_missions", "4" ) );
    game_speed               = XMLSupport::parse_float( vs_config->getVariable( "physics", "game_speed", "1.0" ) );
    runtime_compactness      = XMLSupport::parse_float( vs_config->getVariable( "physics", "runtime_compactness", "1.0" ) );
    autogen_compactness      = XMLSupport::parse_float( vs_config->getVariable( "physics", "autogen_compactness", "1.0" ) );
    AsteroidDifficulty       = XMLSupport::parse_float( vs_config->getVariable( "physics", "AsteroidDifficulty", ".4" ) );
    YearScale                = XMLSupport::parse_float( vs_config->getVariable( "physics", "YearScale", "10.0" ) );
    game_speed_affects_autogen_systems = XMLSupport::parse_bool( vs_config->getVariable( "physics", "game_speed_affects_autogen_systems", "false" ) );
    star_system_scale        = XMLSupport::parse_float( vs_config->getVariable( "physics", "star_system_scale", "1.0" ) );
    respawn_unit_size        = XMLSupport::parse_float( vs_config->getVariable( "physics", "respawn_unit_size", "400.0" ) );
    auto_pilot_planet_radius_percent = XMLSupport::parse_floatf( vs_config->getVariable( "physics", "auto_pilot_planet_radius_percent", ".75" ) );
    campaigns                = vs_config->getVariable( "physics", "campaigns", "privateer_campaign vegastrike_campaign" );     //WRONG SECTION   change after 0.5
    NumRunningSystems        = XMLSupport::parse_int( vs_config->getVariable( "physics", "NumRunningSystems", "4" ) );
    InactiveSystemTime       = XMLSupport::parse_floatf( vs_config->getVariable( "physics", "InactiveSystemTime", "0.3" ) );
    jump_radius_scale        = XMLSupport::parse_floatf( vs_config->getVariable( "physics", "jump_radius_scale", "2" ) );
    jump_disables_shields    = XMLSupport::parse_bool( vs_config->getVariable( "physics", "jump_disables_shields", "true" ) );
    display_in_meters        = XMLSupport::parse_bool( vs_config->getVariable( "physics", "display_in_meters", "true" ) );
    game_speed_lying         = XMLSupport::parse_bool( vs_config->getVariable( "physics", "game_speed_lying", "true" ) );
    num_times_to_simulate_new_star_system = XMLSupport::parse_int( vs_config->getVariable( "physics", "num_times_to_simulate_new_star_system", "20" ) );
    gun_speed_adjusted_game_speed = XMLSupport::parse_bool( vs_config->getVariable( "physics", "gun_speed_adjusted_game_speed", "false" ) );
    gun_speed                = XMLSupport::parse_floatf( vs_config->getVariable( "physics", "gun_speed", "1" ) );
    weapon_damage_efficiency = XMLSupport::parse_floatf( vs_config->getVariable( "physics", "weapon_damage_efficiency", "1.0" ) );
    refire_difficulty_scaling = XMLSupport::parse_floatf( vs_config->getVariable( "physics", "refire_difficutly_scaling", "3.0" ) );
    debris_mass              = XMLSupport::parse_floatf( vs_config->getVariable( "physics", "debris_mass", ".00001" ) );
    explosion_damage_center  = XMLSupport::parse_floatf( vs_config->getVariable( "physics", "explosion_damage_center", "1" ) );
    explosion_damage_edge    = XMLSupport::parse_floatf( vs_config->getVariable( "graphics", "explosion_damage_edge", ".125" ) );
    eject_cargo_on_blowup    = XMLSupport::parse_int( vs_config->getVariable( "physics", "eject_cargo_on_blowup", "0" ) );

    /* Data Options */
    universe_path            = vs_config->getVariable( "data", "universe_path", "universe" );
    sectors                  = vs_config->getVariable( "data", "sectors", "sectors" );     
    techniquesBasePath       = vs_config->getVariable( "data", "techniques", "techniques" );
    unitCSV                  = vs_config->getVariable( "data", "UnitCSV", "units.csv" );
    modUnitCSV               = vs_config->getVariable( "data", "ModUnitCSV", "" );
    cockpits                 = vs_config->getVariable( "data", "cockpits", "cockpits" );
    animations               = vs_config->getVariable( "data", "animations", "animations" );
    movies                   = vs_config->getVariable( "data", "movies", "movies" );
    sprites                  = vs_config->getVariable( "data", "sprites", "sprites" );
    serialized_xml           = vs_config->getVariable( "data", "serialized_xml", "serialized_xml" );
    sharedtextures           = vs_config->getVariable( "data", "sharedtextures", "textures" );
    sharedsounds             = vs_config->getVariable( "data", "sharedsounds", "sounds" );
    sharedmeshes             = vs_config->getVariable( "data", "sharedmeshes", "meshes" );
    sharedunits              = vs_config->getVariable( "data", "sharedunits", "units" );
    ai_directory             = vs_config->getVariable( "data", "ai_directory", "ai" );
    datadir                  = vs_config->getVariable( "data", "datadir", "" );
    hqtextures               = vs_config->getVariable( "data", "hqtextures", "" );
    volume_format            = vs_config->getVariable( "data", "volume_format", "pk3" );
    
    /* Galaxy Options */
    PushValuesToMean         = XMLSupport::parse_bool( vs_config->getVariable( "galaxy", "PushValuesToMean", "true" ) );
    AmbientLightFactor       = XMLSupport::parse_floatf( vs_config->getVariable( "galaxy", "AmbientLightFactor", "0" ) );
    StarRadiusScale          = XMLSupport::parse_floatf( vs_config->getVariable( "galaxy", "StarRadiusScale", "1000" ) );
    DefaultAtmosphereTexture = vs_config->getVariable( "galaxy", "DefaultAtmosphereTexture", "sol/earthcloudmaptrans.png" );
    AtmosphereProbability    = XMLSupport::parse_floatf( vs_config->getVariable( "galaxy", "AtmosphereProbability", "0.5" ) );
    RingProbability          = XMLSupport::parse_floatf( vs_config->getVariable( "galaxy", "RingProbability", ".1" ) );
    DoubleRingProbability    = XMLSupport::parse_floatf( vs_config->getVariable( "galaxy", "DoubleRingProbability", ".025" ) );
    InnerRingRadius          = XMLSupport::parse_floatf( vs_config->getVariable( "galaxy", "InnerRingRadius", "1.5" ) );
    OuterRingRadius          = XMLSupport::parse_floatf( vs_config->getVariable( "galaxy", "OuterRingRadius", "2.5" ) );
    DefaultRingTexture       = vs_config->getVariable( "galaxy","DefaultRingTexture","planets/ring.pngwrapx36wrapy2" );
    SecondRingDifference     = XMLSupport::parse_floatf( vs_config->getVariable( "galaxy", "SecondRingDifference", ".4" ) );
    MoonRelativeToPlanet     = XMLSupport::parse_floatf( vs_config->getVariable( "galaxy", "MoonRelativeToPlanet", ".4" ) );
    MoonRelativeToMoon       = XMLSupport::parse_floatf( vs_config->getVariable( "galaxy", "MoonRelativeToMoon", ".8" ) );
    RockyRelativeToPrimary   = XMLSupport::parse_floatf( vs_config->getVariable( "galaxy", "RockyRelativeToPrimary", ".05" ) );
    CompactnessScale         = XMLSupport::parse_floatf( vs_config->getVariable( "galaxy", "CompactnessScale", "1.5" ) );
    JumpCompactnessScale     = XMLSupport::parse_floatf( vs_config->getVariable( "galaxy", "JumpCompactnessScale", "1.5" ) );
    MeanNaturalPhenomena     = XMLSupport::parse_int( vs_config->getVariable( "galaxy", "MeanNaturalPhenomena", "1" ) );
    MeanStarBases            = XMLSupport::parse_int( vs_config->getVariable( "galaxy", "MeanStarBases", "2" ) );
//    SmallUnitsMultiplier   = XMLSupport::parse_floatf( vs_config->getVariable( "galaxy", "SmallUnitsMultiplier", "0" ) );

    /* Network Options */
    force_client_connect     = XMLSupport::parse_bool( vs_config->getVariable( "network", "force_client_connect", "false" ) );
    use_account_server       = XMLSupport::parse_bool( vs_config->getVariable( "network", "use_account_server", "true" ) );
    server_ip                = vs_config->getVariable( "network", "server_ip", "" );
    server_port              = vs_config->getVariable( "network", "server_port", "6777" );
    account_server_url       = vs_config->getVariable( "network", "account_server_url","http://vegastrike.sourceforge.net/cgi-bin/accountserver.py?" );
    chat_only_in_network     = XMLSupport::parse_bool( vs_config->getVariable( "network", "chat_only_in_network", "false" ) );

    /* Cargo Options */
    news_from_cargolist      = XMLSupport::parse_bool( vs_config->getVariable( "cargo", "news_from_cargolist", "false" ) );

    /* Keyboard Options */
    enable_unicode           = XMLSupport::parse_bool( vs_config->getVariable( "keyboard", "enable_unicode", "true" ) );

    /* Player Options */
    password                 = vs_config->getVariable( "player", "password", "" );
    callsign                 = vs_config->getVariable( "player", "callsign", "" );

}

string vs_options::getCallsign( int squadnum )
{
    return vs_config->getVariable( std::string( "player" )
                                   +( (squadnum > 0) ? XMLSupport::tostring( squadnum+1 ) : std::string(
                                         "" ) ), "callsign", "pilot" );
}

string vs_options::getPlayer( int playernum)
{
    return( vs_config->getVariable( "player"+( (playernum > 0) ? XMLSupport::tostring( playernum+1 ) : string( "" ) ), "callsign", "" ));
}

string vs_options::getPassword( int playernum)
{
    return( vs_config->getVariable( "player"+( (playernum > 0) ? XMLSupport::tostring( playernum+1 ) : string( "" ) ), "password", "" ));
}
