#ifndef _VS_OPTIONS_H
#define _VS_OPTIONS_H

#include <string>

class vs_options
{
	public:
		vs_options() {;}
		~vs_options() {;}
		void init();
		
		/* General Options */
		bool write_savegame_on_exit;
		int times_to_show_help_screen;
		bool remember_savegame;
		std::string new_game_save_name;
		int quick_savegame_summaries_buffer;
		std::string empty_mission;
		std::string custompython;
		bool quick_savegame_summaries;
		int garbagecollectfrequency;
		int numoldsystems;
		bool deleteoldsystems;
		
		/* Audio Options */
		int threadtime;
		std::string missionvictorysong;
		
		/* Cockpit Audio Options */
		std::string comm;
		std::string scanning;
		std::string objective;
		std::string examine;
		std::string view;
		std::string repair;
		std::string manifest;
		int compress_max;
		std::string compress_loop;
		std::string compress_change;
		std::string compress_stop;
		int compress_interval;

		/* Unit Audio Options */
		std::string jumpleave;
		
		/* Graphics Options */
		std::string jumpgate;
		double jumpanimationshrink;
		double jumpgatesize;
		bool switchToTargetModeOnKey;
		double camera_pan_speed;
		bool background;
		bool cockpit;
		bool disabled_cockpit_allowed;
		std::string splash_screen;
		bool vbo;
		int num_near_stars; // maybe need to be double
		int num_far_stars;  // maybe need to be double
		double star_spreading;
		bool usePlanetAtmosphere;
		bool usePlanetFog;
		double reflectivity;
		bool hardware_cursor;
		bool always_make_smooth_cam; // Not used Yet
		double precull_dist;
		bool draw_near_stars_in_front_of_planets;
		bool starblend;

		/* Terrain Options */
		double xscale;
		double yscale;
		double zscale;
		double mass;
		double radius;
		
		/* Player Options */
		std::string callsign;
		
		/* Joystick Options */
		float joystick_exponent; // joystick axes are 32-bit floats.
		int polling_rate;
		bool force_use_of_joystick;
		bool debug_digital_hatswitch;
		float deadband;
		float mouse_deadband;
		bool warp_mouse;
		float mouse_sensitivity;
		float mouse_exponent;
		float mouse_blur;
		bool force_feedback;
		int ff_device;
		
		/* AI Options */
		bool AllowCivilWar;
		bool CappedFactionRating;
		bool AllowNonplayerFactionChange;
		double min_relationship;
		
		/* Physics Options */
		std::string Drone;
		int max_missions;
		double game_speed;
		double runtime_compactness;
		double autogen_compactness;
		double AsteroidDifficulty;
		double YearScale;
		bool game_speed_affects_autogen_systems;
		double star_system_scale;
		double respawn_unit_size;
		float auto_pilot_planet_radius_percent;
		std::string campaigns;  // WRONG SECTION ...change after 0.5
		int NumRunningSystems;
		float InactiveSystemTime;
		
		/* Data Options */
		std::string universe_path;
		std::string sectors;  // Not Used 
		
		/* Galaxy Options */
		bool PushValuesToMean;
		
		/* Cargo Options */
		bool news_from_cargolist;
		
		std::string getCallsign(int);
};


#endif
