/*
 * main.cpp
 *
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */


#define PY_SSIZE_T_CLEAN
#include <boost/python.hpp>

#include <Python.h>
#include <boost/program_options.hpp>
#include "audio/test.h"
#if defined (HAVE_SDL)
#include <SDL2/SDL.h>
#endif
#include "cmd/role_bitmask.h"
#if defined (WITH_MACOSX_BUNDLE)
#include <sys/param.h>
#endif
#ifdef _WIN32
#include <direct.h>
#include <process.h>
#endif
#include "src/gfxlib.h"
#include "src/in_kb.h"
#include "root_generic/lin_time.h"
#include "src/main_loop.h"
#include "src/config_xml.h"
#include "cmd/script/mission.h"
#include "src/audiolib.h"
#include "src/config_xml.h"
#include "root_generic/vsfilesystem.h"
#include "root_generic/vs_globals.h"
#include "gfx/animation.h"
#include "gfx/cockpit.h"
#include "src/python/init.h"
#include "root_generic/savegame.h"
#include "src/force_feedback.h"
#include "gfx/hud.h"
#include "gldrv/winsys.h"
#include "src/universe_util.h"
#include "src/universe.h"
#include "src/save_util.h"
#include "gfx/masks.h"
#include "cmd/music.h"
#include "src/ship_commands.h"
#include "src/gamemenu.h"
#include "audio/SceneManager.h"
#include "audio/renderers/OpenAL/BorrowedOpenALRenderer.h"
#include "configuration/configuration.h"
#include <time.h>
#if !defined(_WIN32) && !defined (__HAIKU__)
#include <signal.h>
#endif

#if defined (_MSC_VER) && defined (_DEBUG)
#include <crtdbg.h>
#endif

#if defined (CG_SUPPORT)
#include "src/cg_global.h"
#endif

#include "src/vs_logging.h"
#include "root_generic/options.h"
#include "version.h"
#include "src/vs_exit.h"

/*
 * Globals
 */
Universe *_Universe;
TextPlane *bs_tp = nullptr;
char SERVER = 0;

//false if command line option --net is given to start without network
static bool ignore_network = true;
// legacy_data_dir_mode determines whether the application should require the data directory to be specified on the command-line or not
// true - no parameter required, application should start regardless of where it's called from
// false - parameter required, application should only start if specified
// NOTE: This is tied to the executable name. `vegastrike` -> true; `vegastrike-engine` -> false
bool legacy_data_dir_mode;

void enableNetwork(bool usenetwork) {
    ignore_network = !usenetwork;
}

/*
 * Function definitions
 */

void setup_game_data() {
    //pass in config file l8r??
    g_game.audio_frequency_mode = 4;     //22050/16
    g_game.sound_enabled = 1;
    g_game.use_textures = 1;
    g_game.use_ship_textures = 0;
    g_game.use_planet_textures = 0;
    g_game.use_sprites = 1;
    g_game.use_animations = 1;
    g_game.use_videos = 1;
    g_game.use_logos = 1;
    g_game.sound_volume = 1;
    g_game.music_volume = 1;
    g_game.warning_level = 20;
    g_game.capture_mouse = GFXFALSE;
    g_game.y_resolution = 1080;
    g_game.x_resolution = 1920;
    g_game.fov = 78;
    g_game.MouseSensitivityX = 2;
    g_game.MouseSensitivityY = 4;
}

VegaConfig *createVegaConfig(const char *file) {
    return new GameVegaConfig(file);
}

std::pair<std::string, std::string> ParseCommandLine(int argc, char **CmdLine);
/**
 * Returns an exit code >= 0 if the game is supposed to exit rightaway
 * Returns an exit code < 0 if the game can continue loading.
 */
int readCommandLineOptions(int argc, char **argv);

// FIXME: Code should throw exception instead of calling winsys_exit            // Should it really? - stephengtuggy 2020-10-25
void VSExit(int code) {
    Music::CleanupMuzak();
    VegaStrikeLogging::VegaStrikeLogger::instance().FlushLogsProgramExiting();
    STATIC_VARS_DESTROYED = true;
    AUDDestroy();
    winsys_exit(code);
}

void cleanup(void) {
    // stephengtuggy 2020-10-30: Output message both to the console and to the logs
    printf("Thank you for playing!\n");
    VS_LOG(info, "Thank you for playing!");
    VegaStrikeLogging::VegaStrikeLogger::instance().FlushLogsProgramExiting();
    STATIC_VARS_DESTROYED = true;
    if (_Universe != nullptr) {
        _Universe->WriteSaveGame(true);
    }
#ifdef _WIN32
#if defined (_MSC_VER) && defined (_DEBUG)
    if (!cleanexit) {
        _RPT0( _CRT_ERROR, "WARNING: Vega Strike exit not clean\n" );
    }
    return;
#endif
#else
    while (!cleanexit) {
        usleep(10000);
    }
#endif

#if defined (CG_SUPPORT)
    if (cloak_cg->vertexProgram) cgDestroyProgram( cloak_cg->vertexProgram );
    if (cloak_cg->shaderContext) cgDestroyContext( cloak_cg->shaderContext );
#endif
    Music::CleanupMuzak();
    winsys_shutdown();
    AUDDestroy();
    delete[] CONFIGFILE;
}

LeakVector<Mission *> active_missions;

void bootstrap_main_loop();
void bootstrap_first_loop();
#if defined (WITH_MACOSX_BUNDLE)
//WTF! this causes windowed creation to fail... please justify yourself ;-)  #undef main
#endif

void nothinghappens(unsigned int, unsigned int, bool, int, int) {
}

void initSceneManager() {
    VS_LOG(info, "Creating scene manager...");
    Audio::SceneManager *sm = Audio::SceneManager::getSingleton();

    if (Audio::SceneManager::getSingleton() == 0) {
        throw Audio::Exception("Singleton null after SceneManager instantiation");
    }

    sm->setMaxSources(g_game.max_sound_sources);
}

void initALRenderer() {
    VS_LOG(info, "  Initializing renderer...");
    Audio::SceneManager *sm = Audio::SceneManager::getSingleton();

    if (g_game.sound_enabled) {
        SharedPtr<Audio::Renderer> renderer(new Audio::BorrowedOpenALRenderer);
        renderer->setMeterDistance(1.0);
        renderer->setDopplerFactor(0.0);

        sm->setRenderer(renderer);
    }
}

void initScenes() {
    Audio::SceneManager *sm = Audio::SceneManager::getSingleton();

    sm->createScene("video");
    sm->createScene("music");
    sm->createScene("cockpit");
    sm->createScene("base");
    sm->createScene("space");

    sm->setSceneActive("video", true);
}

void closeRenderer() {
    VS_LOG(info, "Shutting down renderer...");
    Audio::SceneManager::getSingleton()->setRenderer(SharedPtr<Audio::Renderer>());
}

extern void InitUnitTables();
extern void CleanupUnitTables();
bool isVista = false;

Unit *TheTopLevelUnit;

int main(int argc, char *argv[]) {
    // Change to program directory if not already
    // std::string program_as_called();
    const boost::filesystem::path program_path(argv[0]);
    // const boost::filesystem::path canonical_program_path = boost::filesystem::canonical(program_path);

    const boost::filesystem::path program_name{program_path.filename()};  //canonical_program_path.filename();
    const boost::filesystem::path program_directory_path{program_path.parent_path()};

    VSFileSystem::programdir = program_directory_path.string();

    // This will be set later
    boost::filesystem::path home_subdir_path{};

    // when the program name is `vegastrike-engine` then enforce that the data directory must be specified
    // if the program name is `vegastrike` then enable legacy mode where the current path is assumed.
    legacy_data_dir_mode = (program_name == "vegastrike") || (program_name == "vegastrike.exe");
    std::cerr << "Legacy Mode: " << (legacy_data_dir_mode ? "TRUE" : "FALSE") << std::endl;

    if (legacy_data_dir_mode) {
        VSFileSystem::datadir = boost::filesystem::current_path().string();
        std::cerr << "Saving current directory (" << VSFileSystem::datadir << ") as DATA_DIR" << std::endl;
    }

    if (!program_directory_path.empty())                  // Changing to an empty path does bad things
    {
        boost::filesystem::current_path(program_directory_path);
    }

//    VegaStrikeLogging::VegaStrikeLogger::InitLoggingPart1();

    CONFIGFILE = nullptr;
    {
        char pwd[8192] = "";
        if (nullptr != getcwd(pwd, 8191)) {
            pwd[8191] = '\0';
            VS_LOG(info, (boost::format(" In path %1%") % pwd));
        } else {
            VS_LOG(info, " In path <<path too long>>");
        }
    }
#ifdef _WIN32
    OSVERSIONINFO osvi;
    ZeroMemory( &osvi, sizeof (OSVERSIONINFO) );
    osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);

    GetVersionEx( &osvi );
    isVista = (osvi.dwMajorVersion == 6);
    VS_LOG(info, (boost::format("Windows version %1% %2%") % osvi.dwMajorVersion % osvi.dwMinorVersion));
#endif
    /* Print copyright notice */
    printf("Vega Strike "  " \n"
           "See http://www.gnu.org/copyleft/gpl.html for license details.\n\n");
    /* Seed the random number generator */
    if (benchmark < 0.0) {
        srand(time(nullptr));
    } else {
        //in benchmark mode, always use the same seed
        srand(171070);
    }

    // Initial mission name. Can be loaded from command line arguments.
    // Usually loaded from config.json
    std::string mission_name;

    //this sets up the vegastrike config variable
    setup_game_data();
    //loads the configuration file .vegastrike/vegastrike.config from home dir if such exists
    {
        std::pair<std::string, std::string> pair = ParseCommandLine(argc, argv);
        std::string subdir = pair.first;
        mission_name = pair.second;

        VS_LOG(info, (boost::format("GOT SUBDIR ARG = %1%") % subdir));
        if (CONFIGFILE == 0) {
            CONFIGFILE = new char[42];
            sprintf(CONFIGFILE, "vegastrike.config");
        }
        //Specify the config file and the possible mod subdir to play
        VSFileSystem::InitPaths(CONFIGFILE, subdir);
        // home_subdir_path = boost::filesystem::canonical(boost::filesystem::path(subdir));
    }

    // now that the user config file has been loaded from disk, update the global configuration struct values
    configuration()->OverrideDefaultsWithUserConfiguration();

    // If no debug argument is supplied, set to what the config file has.
    if (g_game.vsdebug == '0') {
        g_game.vsdebug = configuration()->logging.vsdebug;
    }

    // Ugly hack until we can find a way to redo all the directory initialization stuff properly.
    // Use the subdirectory "logs" under the Vega Strike home directory. Make sure we don't duplicate the ".vegastrike/" or ".pu/", etc. part.
    const boost::filesystem::path home_path{boost::filesystem::absolute(VSFileSystem::homedir)};
    if (home_path.string().find(VSFileSystem::HOMESUBDIR) == std::string::npos) {
        const boost::filesystem::path home_subdir(VSFileSystem::HOMESUBDIR);
        home_subdir_path = boost::filesystem::absolute(home_subdir, home_path);
    } else {
        home_subdir_path = home_path;
    }

    VegaStrikeLogging::VegaStrikeLogger::instance().InitLoggingPart2(g_game.vsdebug, home_subdir_path);

    // can use the vegastrike config variable to read in the default mission
    if (game_options()->force_client_connect) {
        ignore_network = false;
    }

    // Override config with command line argument
    if (!mission_name.empty()) {
        configuration()->game_start.default_mission = mission_name;
        VS_LOG(info, (boost::format("MISSION_NAME is empty using : %1%") % mission_name));
    }

    int exitcode;
    if ((exitcode = readCommandLineOptions(argc, argv)) >= 0) {
        return exitcode;
    }

    //might overwrite the default mission with the command line
    InitUnitTables();

    // Initialise the master parts list before first use.
    Manifest::MPL();

    Python::init();

    Python::test();

    std::vector<std::vector<char> > temp = ROLES::getAllRolePriorities();

    InitTime();
    UpdateTime();

    AUDInit();
    AUDListenerGain(game_options()->sound_gain);
    Music::InitMuzak();

    initSceneManager();
    initALRenderer();
    initScenes();

    //Register commands
    //COmmand Interpretor Seems to break VC8, so I'm leaving disabled for now - Patrick, Dec 24
    if (game_options()->command_interpretor) {
        CommandInterpretor = new commandI;
        InitShipCommands();
    }
    _Universe = new Universe(argc, argv, game_options()->galaxy.c_str());
    TheTopLevelUnit = new Unit(0);
    _Universe->Loop(bootstrap_first_loop);

    //Unregister commands - and cleanup memory
    UninitShipCommands();

    closeRenderer();

    cleanup();

    delete _Universe;
    CleanupUnitTables();
    // Just to be sure -- stephengtuggy 2020-07-27
    VegaStrikeLogging::VegaStrikeLogger::instance().FlushLogsProgramExiting();
    return 0;
}

static Animation *SplashScreen = NULL;
static bool BootstrapMyStarSystemLoading = true;

void SetStarSystemLoading(bool value) {
    BootstrapMyStarSystemLoading = value;
}

bool GetStarSystemLoading() {
    return BootstrapMyStarSystemLoading;
}

void SetSplashScreen(Animation *ss) {
    SplashScreen = ss;
}

Animation *GetSplashScreen() {
    return SplashScreen;
}

void bootstrap_draw(const std::string &message, Animation *newSplashScreen) {
    static Animation *ani = NULL;
    static bool reentryWatchdog = false;
    if (!BootstrapMyStarSystemLoading || reentryWatchdog) {
        return;
    }
    Music::MuzakCycle();     //Allow for loading music...
    if (SplashScreen == NULL && newSplashScreen == NULL) {
        //if there's no splashscreen, we don't draw on it
        //this happens, when the splash screens texture is loaded
        return;
    }

    reentryWatchdog = true;
    if (newSplashScreen != NULL) {
        ani = newSplashScreen;
    }
    UpdateTime();

    Matrix tmp;
    Identity(tmp);
    BootstrapMyStarSystemLoading = false;
    static Texture dummy("white.bmp", 0, MIPMAP, TEXTURE2D, TEXTURE_2D, 1);
    BootstrapMyStarSystemLoading = true;
    dummy.MakeActive();
    GFXDisable(LIGHTING);
    GFXDisable(DEPTHTEST);
    GFXBlendMode(ONE, ZERO);
    GFXEnable(TEXTURE0);
    GFXDisable(TEXTURE1);
    GFXColor4f(1, 1, 1, 1);
    GFXClear(GFXTRUE);
    GFXLoadIdentity(PROJECTION);
    GFXLoadIdentity(VIEW);
    GFXLoadMatrixModel(tmp);
    GFXBeginScene();

    bs_tp->SetPos(-.99, -.97);     //Leave a little bit of room for the bottoms of characters.
    bs_tp->SetCharSize(.4, .8);
    ScaleMatrix(tmp, Vector(6500, 6500, 0));
    GFXLoadMatrixModel(tmp);
    GFXHudMode(GFXTRUE);
    if (ani) {
        if (GetElapsedTime() < 10) {
            ani->UpdateAllFrame();
        }
        {
            ani->DrawNow(tmp);
        }
    }
    bs_tp->Draw(game_options()->default_boot_message.length() > 0 ?
            game_options()->default_boot_message : message.length() > 0 ?
                    message : game_options()->initial_boot_message);

    GFXHudMode(GFXFALSE);
    GFXEndScene();

    reentryWatchdog = false;
}

extern Unit **fighters;

bool SetPlayerLoc(QVector &sys, bool set) {
    static QVector mysys;
    static bool isset = false;
    if (set) {
        isset = true;
        mysys = sys;
        return true;
    } else {
        if (isset) {
            sys = mysys;
        }
        return isset;
    }
}

bool SetPlayerSystem(std::string &sys, bool set) {
    static std::string mysys;
    static bool isset = false;
    if (set) {
        isset = true;
        mysys = sys;
        return true;
    } else {
        if (isset) {
            sys = mysys;
        }
        return isset;
    }
}

vector<string> parse_space_string(std::string s) {
    vector<string> ret;
    string::size_type index = 0;
    while ((index = s.find(" ")) != string::npos) {
        ret.push_back(s.substr(0, index));
        s = s.substr(index + 1);
    }
    ret.push_back(s);
    return ret;
}

void bootstrap_first_loop() {
    static int i = 0;
    if (i == 0) {
        vector<string> s = parse_space_string(game_options()->splash_screen);
        vector<string> sa = parse_space_string(game_options()->splash_audio);
        int snum = time(nullptr) % s.size();
        SplashScreen = new Animation(s[snum].c_str(), false);
        if (!sa.empty() && sa[0].length()) {
            muzak->GotoSong(sa[snum % sa.size()]);
        }
        bs_tp = new TextPlane();
    }
    bootstrap_draw("Vegastrike Loading...", SplashScreen);
    if (i++ > 4) {
        if (_Universe) {
            if (game_options()->main_menu) {
                UniverseUtil::startMenuInterface(true);
            } else {
                _Universe->Loop(bootstrap_main_loop);
            }
        }
    }
}

void SetStartupView(Cockpit *cp) {
    cp->SetView(game_options()->startup_cockpit_view
            == "view_target" ? CP_TARGET : (game_options()->startup_cockpit_view
            == "back" ? CP_BACK : (game_options()->startup_cockpit_view
            == "chase" ? CP_CHASE
            : CP_FRONT)));
}

void bootstrap_main_loop() {
    static bool LoadMission = true;
    // InitTime();
    if (LoadMission) {
        LoadMission = false;
        active_missions.push_back(mission = new Mission(configuration()->game_start.default_mission.c_str()));

        mission->initMission();

        UniverseUtil::showSplashScreen("");         //Twice for double or triple-buffering
        UniverseUtil::showSplashScreen("");

        QVector pos;
        string planetname;

        mission->GetOrigin(pos, planetname);
        string mysystem = mission->getVariable("system", "sol.system");

        int numplayers;
        numplayers = XMLSupport::parse_int(mission->getVariable("num_players", "1"));
        vector<std::string> playername;
        vector<std::string> playerpasswd;
        string pname, ppasswd;
        for (int p = 0; p < numplayers; p++) {
            pname = game_options()->getPlayer(p);
            ppasswd = game_options()->getPassword(p);

            if (!ignore_network) {
                //In network mode, test if all player sections are present
                if (pname.empty()) {
                    VS_LOG_AND_FLUSH(fatal, (boost::format("Missing or incomplete section for player %1%") % p));
                    cleanexit = true;
                    VSExit(1);
                }
            }
            playername.push_back(pname);
            playerpasswd.push_back(ppasswd);
        }
        float credits = XMLSupport::parse_float(mission->getVariable("credits", "0"));
        g_game.difficulty = XMLSupport::parse_float(mission->getVariable("difficulty", "1"));
        string savegamefile = mission->getVariable("savegame", "");
        vector<SavedUnits> savedun;
        vector<string> playersaveunit;
        vector<StarSystem *> ss;
        vector<string> starsysname;
        vector<QVector> playerNloc;

        vector<vector<std::string> > savefiles;

        _Universe->SetupCockpits(playername);

        vector<std::string>::iterator it, jt;
        unsigned int k = 0;

        Cockpit *cp = _Universe->AccessCockpit(k);
        SetStartupView(cp);
        bool setplayerXloc = false;
        std::string psu;
        if (k == 0) {
            QVector myVec;
            if (SetPlayerLoc(myVec, false)) {
                _Universe->AccessCockpit(0)->savegame->SetPlayerLocation(myVec);
            }
            std::string st;
            if (SetPlayerSystem(st, false)) {
                _Universe->AccessCockpit(0)->savegame->SetStarSystem(st);
            }
        }
        vector<SavedUnits> saved;
        vector<string> packedInfo;

        if (game_options()->load_last_savegame) {
            _Universe->AccessCockpit(k)->savegame->ParseSaveGame(savegamefile,
                    mysystem,
                    mysystem,
                    pos,
                    setplayerXloc,
                    credits,
                    packedInfo,
                    k);
        } else {
            _Universe->AccessCockpit(k)->savegame->SetOutputFileName(savegamefile);
        }

        _Universe->AccessCockpit(k)->UnpackUnitInfo(packedInfo);
        CopySavedShips(playername[k], k, packedInfo, true);
        playersaveunit.push_back(_Universe->AccessCockpit(k)->GetUnitFileName());
        _Universe->AccessCockpit(k)->credits = credits;
        ss.push_back(_Universe->Init(mysystem, Vector(0, 0, 0), planetname));
        if (setplayerXloc) {
            playerNloc.push_back(pos);
        } else {
            playerNloc.push_back(QVector(FLT_MAX, FLT_MAX, FLT_MAX));
        }
        for (unsigned int j = 0; j < saved.size(); j++) {
            savedun.push_back(saved[j]);
        }

        SetStarSystemLoading(true);
        InitializeInput();

        vs_config->bindKeys();         //gotta do this before we do ai

        createObjects(playersaveunit, ss, playerNloc, savefiles);
        while (!savedun.empty()) {
            AddUnitToSystem(&savedun.back());
            savedun.pop_back();
        }
        forcefeedback = new ForceFeedback();

        UpdateTime();
        FactionUtil::LoadContrabandLists();
        {
            std::vector<std::string> intro_lines;
            boost::split(intro_lines, configuration()->game_start.introduction, boost::is_any_of("\n"));

            for(const std::string& line : intro_lines) {
                UniverseUtil::IOmessage(0, "game", "all", line);
            }
        }

        if (mission->getVariable("savegame",
                "").length() != 0
                && game_options()->dockOnLoad) {
            for (size_t i = 0; i < _Universe->numPlayers(); i++) {
                QVector vec;
                DockToSavedBases(i, vec);
            }
        }

        if (game_options()->load_last_savegame) {
            //Don't write if we didn't load...
            for (unsigned int i = 0; i < _Universe->numPlayers(); ++i) {
                WriteSaveGame(_Universe->AccessCockpit(i), false);
            }
        }
        cur_check = getNewTime();
        for (unsigned int i = 0; i < _Universe->numPlayers(); ++i) {
            _Universe->AccessCockpit(i)->savegame->LoadSavedMissions();
        }
        _Universe->Loop(main_loop);
        ///return to idle func which now should call main_loop mohahahah
        if (game_options()->auto_hide) {
            UniverseUtil::hideSplashScreen();
        }
    }
    ///Draw Texture
}

const char versionmessage[] =
        // (BenjamenMeyer) this will be `major.minor.patch+githash` once all is said and done
        "Vega Strike Engine Version " VEGASTRIKE_VERSION_STR "\n"
        "\n";

std::pair<std::string, std::string> ParseCommandLine(int argc, char **lpCmdLine) {
    std::string retstr;
    std::string mission_name;
    g_game.vsdebug = '0';

    boost::program_options::options_description vs_switches("Command line options for vegastrike; short options are case insensitive");
    vs_switches.add_options()
        ("target,D", boost::program_options::value<std::string>(), "Specify data directory, full path expected")
        ("num-players,n", boost::program_options::value<int>(), "Number of players")
        ("mod,m", boost::program_options::value<std::string>(), "Specify a mod to play")
        ("start-in-system,j", boost::program_options::value<std::string>(), "Start in a specific system")
        ("location,l", boost::program_options::value<std::string>(), "Specify player location")
        ("h", "Medium-low resolution (1024x768)")
        ("v", "Medium resolution (1280x1024)")
        ("help,h", "Show this help")
        ("version", "Print the version and exit")
        ("debug", boost::program_options::value<char>()->default_value('0'), "Enable debugging output, 1 major warnings, 2 medium, 3 developer notes")
        ("benchmark", boost::program_options::value<double>(), "Benchmark")
        ("test-audio", "Run audio tests")  // is handled in readCommandLineOptions, here is serves only for help message
        ("r", "No-op")
        ("f", "No-op")
        ("p", "No-op")
        ("g", "No-op")
        ("u", "No-op")
        ;

    boost::program_options::options_description vs_modules;
    vs_modules.add_options()
        ("mission_name", boost::program_options::value<std::string>(), "Mission name")
        ;

    boost::program_options::options_description vs_options;
    vs_options.add(vs_switches).add(vs_modules);

    boost::program_options::positional_options_description vs_positionals;
    vs_positionals.add("mission_name", 1);

    auto style = static_cast<boost::program_options::command_line_style::style_t>(boost::program_options::command_line_style::unix_style
                                                                                | boost::program_options::command_line_style::case_insensitive);
    boost::program_options::variables_map cmd_args;
    try {
        boost::program_options::store(
            boost::program_options::command_line_parser(argc, lpCmdLine).options(vs_options).positional(vs_positionals).style(style).run(),
            cmd_args
        );
        boost::program_options::notify(cmd_args);
    } catch (const boost::program_options::error& e) {
        VS_LOG(fatal, (boost::format("Failed to parse arguments: %1%") % e.what()));
        VS_LOG_FLUSH_EXIT(fatal, (boost::format("%1%") % vs_switches), EXIT_FAILURE);
    }

    if (cmd_args.count("help")) {
        std::cout << vs_switches << std::endl;
        exit(0);
    }

    if (cmd_args.count("version")) {
        std::cout << versionmessage << std::endl;
        exit(0);
    }

    if (cmd_args.count("target")) {
        std::string datatmp;
        datatmp = cmd_args["target"].as<std::string>();
        if (datatmp.empty()) {
            VS_LOG_FLUSH_EXIT(fatal, "Target-data-directory option requires an argument", EXIT_FAILURE);
        } else if (VSFileSystem::DirectoryExists(datatmp)) {
            VSFileSystem::datadir = datatmp;
            VS_LOG(important_info, (boost::format("Using target data directory specified on command line: '%1%'") % datatmp));
        } else {
            VS_LOG_FLUSH_EXIT(fatal, (boost::format("Specified target data directory '%1%' not found... exiting") % datatmp), EXIT_FAILURE);
        }
    }

    if (cmd_args.count("num-players")) {
        int num_players = cmd_args["num-players"].as<int>();
        if (num_players > 1 && num_players <= 9) {
            CONFIGFILE = new char[42];
            snprintf(CONFIGFILE, 42, "vegastrike.config.%dplayer", num_players);
        } else if (num_players != 1) {
            VS_LOG(warning, "Specified number of players out of range (1-9)");
        }
    }

    if (cmd_args.count("mod")) {
        retstr = cmd_args["mod"].as<std::string>();
    }

    if (cmd_args.count("start-in-system")) {
        std::string st;
        st = cmd_args["start-in-system"].as<std::string>();
        SetPlayerSystem(st, true);
    }

    if (cmd_args.count("location")) {
        QVector PlayerLocation;
        try {
            std::string input = cmd_args["location"].as<std::string>();
            std::istringstream iStringStream(input);
            iStringStream.ignore(2);
            iStringStream >> PlayerLocation.i;
            iStringStream.ignore(1, ',');
            iStringStream >> PlayerLocation.j;
            iStringStream.ignore(1, ',');
            iStringStream >> PlayerLocation.k;
        } catch (std::ios_base::failure &inputFailure) {
            std::cout << "Error reading coordinates for player location: " << inputFailure.what()
                    << std::endl;
            exit(EXIT_FAILURE);
        }
        SetPlayerLoc(PlayerLocation, true);
    }

    if (cmd_args.count("debug")) {
        char vs_debug_level_temp = cmd_args["debug"].as<char>();
        if (vs_debug_level_temp >= '0' && vs_debug_level_temp <= '3') {
            g_game.vsdebug = vs_debug_level_temp - '0';
        } else {
            VS_LOG_FLUSH_EXIT(fatal, "Invalid debug level specified", EXIT_FAILURE);
        }
    }

    if (cmd_args.count("net")) {
        //don't ignore the network section of the config file
        ignore_network = false;
    }

    if (cmd_args.count("benchmark")) {
        benchmark = cmd_args["benchmark"].as<double>();
    }

    if (cmd_args.count("test-audio")) {
        // This is handled separately, since we have to undefine main first
        // return Audio::Test::main(argc, argv);
    }

    if (cmd_args.count("h")) {
        g_game.x_resolution = 1024;
        g_game.y_resolution = 768;
    }
    if (cmd_args.count("v")) {
        g_game.x_resolution = 1280;
        g_game.y_resolution = 1024;
    }

    if (cmd_args.count("mission_name")) {
        mission_name = cmd_args["mission_name"].as<std::string>();
    }

    if (false == legacy_data_dir_mode) {
        if (true == VSFileSystem::datadir.empty()) {
            VS_LOG_FLUSH_EXIT(fatal, "Data directory not specified.", EXIT_FAILURE);
        }
    }

    return std::pair<std::string, std::string>(retstr, mission_name);
}

#undef main

int readCommandLineOptions(int argc, char **argv) {
    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            if (strcmp("--test-audio", argv[i]) == 0) {
                return Audio::Test::main(argc, argv);
            }
        }
    }
    return -1;
}
