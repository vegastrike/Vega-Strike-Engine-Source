#include "options.h"
#include "configxml.h"

extern VegaConfig *vs_config;

void vs_options::init()
{
	/* General Options */
	write_savegame_on_exit = XMLSupport::parse_bool(vs_config->getVariable("general","write_savegame_on_exit","true"));
	times_to_show_help_screen = XMLSupport::parse_int(vs_config->getVariable("general","times_to_show_help_screen","3"));
	remember_savegame = XMLSupport::parse_bool(vs_config->getVariable("general","remember_savegame","true"));
	new_game_save_name = vs_config->getVariable("general","new_game_save_name","New_Game");
	quick_savegame_summaries_buffer = XMLSupport::parse_int(vs_config->getVariable("general","quick_savegame_summaries_buffer","16384"));
	empty_mission = vs_config->getVariable("general","empty_mission","internal.mission");
	custompython = vs_config->getVariable("general","custompython","import custom;custom.processMessage");
	quick_savegame_summaries = XMLSupport::parse_bool( vs_config->getVariable("general","quick_savegame_summaries","true"));
	garbagecollectfrequency = XMLSupport::parse_int(vs_config->getVariable ("general","garbagecollectfrequency","20"));
	numoldsystems = XMLSupport::parse_int(vs_config->getVariable ("general","numoldsystems","6"));
	deleteoldsystems = XMLSupport::parse_bool (vs_config->getVariable ("general","deleteoldsystems","true"));
	
	/* Audio Options */
	threadtime = XMLSupport::parse_int(vs_config->getVariable("audio","threadtime","1"));
	missionvictorysong = vs_config->getVariable("audio","missionvictorysong","../music/victory.ogg");
	
	/* Cockpit Audio Options */
	comm = vs_config->getVariable("cockpitaudio","comm","vdu_c");
	scanning = vs_config->getVariable("cockpitaudio","scanning","vdu_c");
	objective = vs_config->getVariable("cockpitaudio","objective","vdu_c");
	examine = vs_config->getVariable("cockpitaudio","examine","vdu_b");
	view = vs_config->getVariable("cockpitaudio","view","vdu_b");
	repair = vs_config->getVariable("cockpitaudio","repair","vdu_a");
	manifest = vs_config->getVariable("cockpitaudio","manifest","vdu_a");
	compress_max = XMLSupport::parse_int(vs_config->getVariable("cockpitaudio","compress_max","3"));
	compress_loop = vs_config->getVariable("cockpitaudio","compress_loop","compress_loop");
	compress_change = vs_config->getVariable("cockpitaudio","compress_change","compress_burst");
	compress_stop = vs_config->getVariable("cockpitaudio","compress_stop","compress_end");
	compress_interval = XMLSupport::parse_int(vs_config->getVariable("cockpitaudio","compress_interval","3"));
	
	
	/* Unit Audio Options */
	jumpleave = vs_config->getVariable("unitaudio","jumpleave","sfx43.wav");
	
	/* Graphics Options */
	jumpgate = vs_config->getVariable ("graphics","jumpgate","warp.ani");
	jumpanimationshrink = XMLSupport::parse_float (vs_config->getVariable("graphics","jumpanimationshrink",".95"));
	jumpgatesize = XMLSupport::parse_float (vs_config->getVariable("graphics","jumpgatesize","1.75"));
	switchToTargetModeOnKey = XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","switchToTargetModeOnKey","true"));
	camera_pan_speed = XMLSupport::parse_float(vs_config->getVariable("graphics","camera_pan_speed","0.0001"));
	background = XMLSupport::parse_bool(vs_config->getVariable ("graphics","background","true"));
	cockpit = XMLSupport::parse_bool(vs_config->getVariable("graphics","cockpit","true"));
	disabled_cockpit_allowed = XMLSupport::parse_bool(vs_config->getVariable("graphics","disabled_cockpit_allowed","true"));
	splash_screen = vs_config->getVariable ("graphics","splash_screen","vega_splash.ani");
	vbo = XMLSupport::parse_bool(vs_config->getVariable("graphics","vbo","false"));
	num_near_stars = XMLSupport::parse_int(vs_config->getVariable("graphics","num_near_stars","1000"));
	num_far_stars = XMLSupport::parse_int(vs_config->getVariable("graphics","num_far_stars","2000"));
	star_spreading = XMLSupport::parse_float(vs_config->getVariable("graphics","star_spreading","30000"));
	usePlanetAtmosphere = XMLSupport::parse_bool(vs_config->getVariable("graphics","usePlanetAtmosphere","true"));
	usePlanetFog = XMLSupport::parse_bool(vs_config->getVariable("graphics","usePlanetFog","true"));
	reflectivity = XMLSupport::parse_float(vs_config->getVariable("graphics","reflectivity",".2"));
	hardware_cursor = XMLSupport::parse_bool(vs_config->getVariable("physics","hardware_cursor","false"));
	always_make_smooth_cam = XMLSupport::parse_bool(vs_config->getVariable("graphics","always_make_smooth_cam","false")); // Not used yet
	precull_dist = XMLSupport::parse_float(vs_config->getVariable("graphics","precull_dist","500000000"));
	draw_near_stars_in_front_of_planets = XMLSupport::parse_bool(vs_config->getVariable("graphics","draw_near_stars_in_front_of_planets","false"));
	starblend = XMLSupport::parse_bool(vs_config->getVariable ("graphics","starblend","true"));
	
	
	/* Terrain Options */
	xscale = XMLSupport::parse_float(vs_config->getVariable("terrain","xscale","1.0"));
	yscale = XMLSupport::parse_float(vs_config->getVariable("terrain","yscale","1.0"));
	zscale = XMLSupport::parse_float(vs_config->getVariable("terrain","zscale","1.0"));
	mass = XMLSupport::parse_float(vs_config->getVariable("terrain","mass","100.0"));
	radius = XMLSupport::parse_float(vs_config->getVariable("terrain","radius","10000.0"));
	
	/* Player Options */


	/* Joystick Options */
	joystick_exponent = XMLSupport::parse_floatf(vs_config->getVariable ("joystick","joystick_exponent","1.0"));
	polling_rate = XMLSupport::parse_int(vs_config->getVariable("joystick","polling_rate","0"));
	force_use_of_joystick = XMLSupport::parse_bool(vs_config->getVariable("joystick","force_use_of_joystick","false"));
	debug_digital_hatswitch = XMLSupport::parse_bool(vs_config->getVariable("joystick","debug_digital_hatswitch","false"));
	deadband = XMLSupport::parse_floatf(vs_config->getVariable ("joystick","deadband","0.05"));
	mouse_deadband = XMLSupport::parse_floatf(vs_config->getVariable ("joystick","mouse_deadband","0.025"));
	warp_mouse = XMLSupport::parse_bool(vs_config->getVariable ("joystick","warp_mouse","false"));
	mouse_sensitivity = XMLSupport::parse_floatf(vs_config->getVariable ("joystick","mouse_sensitivity","50.0"));
	mouse_exponent = XMLSupport::parse_floatf(vs_config->getVariable ("joystick","mouse_exponent","3.0"));
	mouse_blur = XMLSupport::parse_floatf(vs_config->getVariable("joystick","mouse_blur",".025"));
	force_feedback = XMLSupport::parse_bool(vs_config->getVariable("joystick","force_feedback","false"));
	ff_device = XMLSupport::parse_int(vs_config->getVariable("joystick","ff_device","0"));
	
	/* AI Options */
	AllowCivilWar = XMLSupport::parse_bool(vs_config->getVariable("AI","AllowCivilWar","false"));
	CappedFactionRating = XMLSupport::parse_bool(vs_config->getVariable("AI","CappedFactionRating","true"));
	AllowNonplayerFactionChange = XMLSupport::parse_bool(vs_config->getVariable("AI","AllowNonplayerFactionChange","false"));
	min_relationship = XMLSupport::parse_float(vs_config->getVariable("AI","min_relationship","-20.0"));
	
	
	/* Physics Options */
	Drone = vs_config->getVariable ("physics","Drone","drone");
	max_missions = XMLSupport::parse_int(vs_config->getVariable ("physics","max_missions","4"));
	game_speed = XMLSupport::parse_float(vs_config->getVariable("physics","game_speed","1.0"));
	runtime_compactness = XMLSupport::parse_float (vs_config->getVariable("physics","runtime_compactness","1.0"));
	autogen_compactness = XMLSupport::parse_float (vs_config->getVariable("physics","autogen_compactness","1.0"));
	AsteroidDifficulty = XMLSupport::parse_float (vs_config->getVariable ("physics","AsteroidDifficulty",".4"));
	YearScale = XMLSupport::parse_float (vs_config->getVariable ("physics","YearScale","10.0"));
	game_speed_affects_autogen_systems = XMLSupport::parse_bool(vs_config->getVariable ("physics","game_speed_affects_autogen_systems","false"));
	star_system_scale = XMLSupport::parse_float(vs_config->getVariable("physics","star_system_scale","1.0"));
	respawn_unit_size = XMLSupport::parse_float (vs_config->getVariable ("physics","respawn_unit_size","400.0"));
	auto_pilot_planet_radius_percent = XMLSupport::parse_floatf (vs_config->getVariable ("physics","auto_pilot_planet_radius_percent",".75"));
	campaigns = vs_config->getVariable("physics","campaigns","privateer_campaign vegastrike_campaign"); // WRONG SECTION   change after 0.5
	NumRunningSystems = XMLSupport::parse_int (vs_config->getVariable ("physics","NumRunningSystems","4"));
	InactiveSystemTime = XMLSupport::parse_floatf (vs_config->getVariable ("physics","InactiveSystemTime","0.3"));
	
	
	
	/* Data Options */
	universe_path = vs_config->getVariable ("data","universe_path", "universe");
	sectors = vs_config->getVariable("data","sectors","sectors");  // Not Used 
	
	/* Galaxy Options */
	PushValuesToMean = XMLSupport::parse_bool(vs_config->getVariable("galaxy","PushValuesToMean","true"));
	
	/* Cargo Options */
	news_from_cargolist = XMLSupport::parse_bool(vs_config->getVariable("cargo","news_from_cargolist","false"));
	
}

string vs_options::getCallsign(int squadnum)
{
	return(vs_config->getVariable(std::string("player")+((squadnum>0)?XMLSupport::tostring(squadnum+1):std::string("")),"callsign","pilot"));
}
