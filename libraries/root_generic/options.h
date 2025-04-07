/*
 * Copyright (C) 2001-2023 Daniel Horn, pyramid3d, Stephen G. Tuggy, Benjamen R. Meyer,
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
#ifndef VEGA_STRIKE_ENGINE_OPTIONS_H
#define VEGA_STRIKE_ENGINE_OPTIONS_H

#include <string>
#include <memory>

#ifndef uint
typedef unsigned int uint;
#endif

class vs_options {
public:
    vs_options() {
    }

    ~vs_options() {
    }

    void init();

/* General Options */
    std::string galaxy;
    bool command_interpretor{};
    bool load_last_savegame{};
    bool debug_fs{};
    float simulation_atom{};
    float audio_atom{};

/* Audio Options */
    int threadtime{};
    std::string missionvictorysong;
    bool Music{};
    float sound_gain{};
    float audio_ref_distance{};
    float audio_max_distance{};
    bool Doppler{};
    bool Positional{};
    float Volume{};
    float DopplerScale{};
    int frequency{};
    int MaxSingleSounds{};
    int MaxTotalSounds{};
    bool Sound{};
    bool ai_sound{};
    float explosion_closeness{};
    float loss_relationship{};
    float victory_relationship{};
    float time_between_music{};

/* Cockpit Audio Options */
    std::string comm;
    std::string scanning;
    std::string objective;
    std::string examine;
    std::string view;
    std::string repair;
    std::string manifest;
    int compress_max{};
    std::string compress_loop;
    std::string compress_change;
    std::string compress_stop;
    int compress_interval{};

    bool comm_preload{};

/* Unit Audio Options */
    std::string jumpleave;
    std::string player_armor_hit;
    std::string player_hull_hit;
    std::string player_shield_hit;

/* Graphics Options */
    std::string jumpgate;
    double jump_animation_shrink{};
    double jumpgatesize{};
    double camera_pan_speed{};
    bool background{};
    bool cockpit{};
    bool disabled_cockpit_allowed{};
    std::string splash_screen;
    bool vbo{};
    int num_near_stars{};          //maybe need to be double
    int num_far_stars{};           //maybe need to be double
    double star_spreading{};
    bool use_planet_atmosphere{};
    bool use_planet_fog{};
    double reflectivity{};
    bool hardware_cursor{};
    bool always_make_smooth_cam{};           //Not used Yet
    double precull_dist{};
    bool draw_near_stars_in_front_of_planets{};
    bool star_blend{};
    float sparkle_engine_size_relative_to_ship{};
    float sparkle_absolute_speed{};
    float engine_radii_scale{};
    float engine_length_scale{};
    bool halos_by_velocity{};
    float percent_afterburner_color_change{};
    float percent_halo_fade_in{};
    float afterburner_color_red{};
    float afterburner_color_green{};
    float afterburner_color_blue{};
    float engine_color_red{};
    float engine_color_green{};
    float engine_color_blue{};
    float halo_sparkle_rate{};
    float halo_sparkle_scale{};
    float halo_sparkle_speed{};
    int max_cubemap_size{};
    std::string default_boot_message;
    std::string initial_boot_message;
    std::string splash_audio;
    bool main_menu{};
    std::string startup_cockpit_view;
    bool detail_texture_trilinear{};
    float light_cutoff{};
    float light_optimal_intensity{};
    float light_saturation{};
    int num_lights{};
    bool separate_specular_color{};
    bool lock_vertex_arrays{};
    int fog_detail{};
    float fov{};
    float aspect{};
    float znear{};
    float zfar{};
    float model_detail{};
    bool use_textures{};
    bool use_ship_textures{};
    bool use_planet_textures{};
    bool use_logos{};
    bool use_vs_sprites{};
    bool use_animations{};
    bool use_videos{};
    bool use_wireframe{};
    int max_texture_dimension{};
    int max_movie_dimension{};
    bool rect_textures{};
    bool pot_video_textures{};
    std::string techniquesSubPath;
    bool smooth_shade{};
    int mipmap_detail{};
    int texture_compression{};
    bool reflection{};
    bool displaylists{};
    bool s3tc{};
    bool ext_clamp_to_edge{};
    bool ext_clamp_to_border{};
    bool clear_on_startup{};
    float circle_accuracy{};
    std::string rgb_pixel_format;
    bool gl_accelerated_visual{};
    int z_pixel_format{};
    int x_resolution{};
    int y_resolution{};
    bool full_screen{};
    int color_depth{};
    bool glut_stencil{};
    std::string mac_shader_name;
    std::string shader_name;
    bool framerate_changes_shader{};
    bool draw_weapons{};
    float sparkle_rate{};
    bool only_stretch_in_warp{};
    float warp_stretch_cutoff{};
    float warp_stretch_region0_max{};
    float warp_stretch_max{};
    float warp_stretch_max_speed{};
    float warp_stretch_max_region0_speed{};
    float weapon_gamma{};
    bool split_dead_subunits{};
    float explosion_force{};
    float explosion_torque{};
    std::string explosion_animation;
    bool explosion_face_player{};
    float percent_shockwave{};
    float shockwave_growth{};
    std::string shockwave_animation;
    float bolt_offset{};
    bool blend_guns{};
    float stretch_bolts{};
    float bolt_pixel_size{};

/* Graphics/Mesh Options */
    bool smooth_lines{};
    bool smooth_points{};

/* Splash Options */
    bool auto_hide{};

/* Terrain Options */
    double xscale{};
    double yscale{};
    double zscale{};
    double mass{};
    double radius{};

/* Player Options */
    std::string password;
    std::string callsign;

/* Joystick Options */
    float joystick_exponent{};            //joystick axes are 32-bit floats.
    int polling_rate{};
    bool force_use_of_joystick{};
    bool debug_digital_hatswitch{};
    float deadband{};
    float mouse_deadband{};
    bool warp_mouse{};
    float mouse_sensitivity{};
    float mouse_exponent{};
    float mouse_blur{};
    bool force_feedback{};
    int ff_device{};
    int warp_mouse_zone{};

/* AI Options */
    bool allow_civil_war{};
    bool capped_faction_rating{};
    bool allow_nonplayer_faction_change{};
    double min_relationship{};
    std::string start_docked_to;
    bool dock_on_load{};

/* PhysicsConfig Options */
    std::string drone;
    int max_missions{};
    double game_speed{};
    double runtime_compactness{};
    double autogen_compactness{};
    double asteroid_difficulty{};
    double year_scale{};
    bool game_speed_affects_autogen_systems{};
    double star_system_scale{};
    double respawn_unit_size{};
    float auto_pilot_planet_radius_percent{};
    std::string campaigns;              //WRONG SECTION ...change after 0.5
    uint num_running_systems{};
    float inactive_system_time{};
    float jump_radius_scale{};
    bool jump_disables_shields{};
    bool display_in_meters{};
    bool game_speed_lying{};
    int num_times_to_simulate_new_star_system{};
    bool gun_speed_adjusted_game_speed{};
    float gun_speed{};
    float weapon_damage_efficiency{};
    float refire_difficulty_scaling{};
    float debris_mass{};
    float explosion_damage_center{};
    float explosion_damage_edge{};
    float debris_time{};
    int eject_cargo_on_blowup{};

/* Data Options */
    std::string universe_path;
    std::string sectors;
    std::string techniques_base_path;
    std::string unitCSV;
    std::string mod_unit_csv;
    std::string cockpits;
    std::string animations;
    std::string movies;
    std::string sprites;
    std::string serialized_xml;
    std::string shared_textures;
    std::string shared_sounds;
    std::string shared_meshes;
    std::string shared_units;
    std::string ai_directory;
    std::string data_dir;
    std::string hqtextures;
    std::string volume_format;

/* Galaxy Options */
    bool push_values_to_mean{};
    float ambient_light_factor{};
    float star_radius_scale{};
    std::string DefaultAtmosphereTexture;
    float atmosphere_probability{};
    float ring_probability{};
    float double_ring_probability{};
    float inner_ring_radius{};
    float outer_ring_radius{};
    std::string default_ring_texture;
    float second_ring_difference{};
    float moon_relative_to_planet{};
    float moon_relative_to_moon{};
    float rocky_relative_to_primary{};
    float compactness_scale{};
    float jump_compactness_scale{};
    int mean_natural_phenomena{};
    int mean_star_bases{};
//    float    SmallUnitsMultiplier;  not used


/* Network Options */
    bool force_client_connect{};
    bool use_account_server{};
    std::string server_ip;
    std::string server_port;
    std::string account_server_url;
    bool chat_only_in_network{};

/* Cargo Options */
    bool news_from_cargo_list{};

/* Keyboard Options */
    bool enable_unicode{};

    std::string getCallsign(int);
    std::string getPlayer(int);
    std::string getPassword(int);
};

extern std::shared_ptr<vs_options> game_options();

#endif //VEGA_STRIKE_ENGINE_OPTIONS_H
