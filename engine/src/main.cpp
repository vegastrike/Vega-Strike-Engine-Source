/*
 * main.cpp
 *
 * Copyright (C) 2001-2024 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * Benjamen R. Meyer, and other Vega Strike contributors.
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
#include "gfxlib.h"
#include "in_kb.h"
#include "lin_time.h"
#include "main_loop.h"
#include "config_xml.h"
#include "cmd/script/mission.h"
#include "audiolib.h"
#include "config_xml.h"
#include "vsfilesystem.h"
#include "vs_globals.h"
#include "gfx/animation.h"
#include "gfx/cockpit.h"
#include "python/init.h"
#include "savegame.h"
#include "force_feedback.h"
#include "gfx/hud.h"
#include "gldrv/winsys.h"
#include "universe_util.h"
#include "universe.h"
#include "save_util.h"
#include "gfx/masks.h"
#include "cmd/music.h"
#include "ship_commands.h"
#include "gamemenu.h"
#include "audio/SceneManager.h"
#include "audio/renderers/OpenAL/BorrowedOpenALRenderer.h"
#include "configuration/configuration.h"
#include <time.h>
#if !defined(_WIN32) && !defined (__HAIKU__)
#include <sys/signal.h>
#endif

#if defined (_MSC_VER) && defined (_DEBUG)
#include <crtdbg.h>
#endif

#if defined (CG_SUPPORT)
#include "cg_global.h"
#endif

#include "vs_logging.h"
#include "options.h"
#include "version.h"
#include "vs_exit.h"

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
    g_game.y_resolution = 768;
    g_game.x_resolution = 1024;
    g_game.fov = 78;
    g_game.MouseSensitivityX = 2;
    g_game.MouseSensitivityY = 4;
}

VegaConfig *createVegaConfig(const char *file) {
    return new GameVegaConfig(file);
}

std::string ParseCommandLine(int argc, char **CmdLine);
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
    winsys_exit(code);
}

void cleanup(void) {
    // stephengtuggy 2020-10-30: Output message both to the console and to the logs
    printf("Thank you for playing!\n");
    VS_LOG(info, "Thank you for playing!");
    VegaStrikeLogging::VegaStrikeLogger::instance().FlushLogsProgramExiting();
    STATIC_VARS_DESTROYED = true;
    if (_Universe != NULL) {
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

char mission_name[1024];

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

    // This will be set later
    boost::filesystem::path home_subdir_path{};

    // when the program name is `vegastrike-engine` then enforce that the data directory must be specified
    // if the program name is `vegastrike` then enable legacy mode where the current path is assumed.
    legacy_data_dir_mode = (program_name == "vegastrike") || (program_name == "vegastrike.exe");
    VS_LOG(important_info, (boost::format("Legacy Mode: %1%") % legacy_data_dir_mode));

    if (legacy_data_dir_mode) {
        VSFileSystem::datadir = boost::filesystem::current_path().string();
        VS_LOG(important_info, (boost::format("Saving current directory (%1%) as DATA_DIR") % VSFileSystem::datadir));
    }

    if (!program_directory_path.empty())                  // Changing to an empty path does bad things
    {
        boost::filesystem::current_path(program_directory_path);
    }

//    VegaStrikeLogging::VegaStrikeLogger::InitLoggingPart1();

    CONFIGFILE = nullptr;
    mission_name[0] = '\0';
    {
        char pwd[8192] = "";
        if (nullptr != getcwd(pwd, 8191)) {
            pwd[8191] = '\0';
            VS_LOG(important_info, (boost::format(" In path %1%") % pwd));
        } else {
            VS_LOG(error, " In path <<path too long>>");
        }
    }
#ifdef _WIN32
    OSVERSIONINFO osvi;
    ZeroMemory( &osvi, sizeof (OSVERSIONINFO) );
    osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);

    GetVersionEx( &osvi );
    isVista = (osvi.dwMajorVersion == 6);
    VS_LOG(important_info, (boost::format("Windows version %1% %2%") % osvi.dwMajorVersion % osvi.dwMinorVersion));
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
    //this sets up the vegastrike config variable
    setup_game_data();
    //loads the configuration file .vegastrike/vegastrike.config from home dir if such exists
    std::string subdir = ParseCommandLine(argc, argv);
    VS_LOG(important_info, (boost::format("GOT SUBDIR ARG = %1%") % subdir));
    if (CONFIGFILE == 0) {
        CONFIGFILE = new char[42];
        sprintf(CONFIGFILE, "vegastrike.config");
    }
    //Specify the config file and the possible mod subdir to play
    VSFileSystem::InitPaths(CONFIGFILE, subdir);
    // home_subdir_path = boost::filesystem::canonical(boost::filesystem::path(subdir));

    // now that the user config file has been loaded from disk, update the global configuration struct values
    configuration()->OverrideDefaultsWithUserConfiguration();

    // If no debug argument is supplied, set to what the config file has.
    if (g_game.vsdebug == '0' || g_game.vsdebug == '\0') {
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
    if (vs_options::instance().force_client_connect) {
        ignore_network = false;
    }
    if (mission_name[0] == '\0') {
        strncpy(mission_name, vs_options::instance().default_mission.c_str(), 1023);
        mission_name[1023] = '\0';
        VS_LOG(important_info, (boost::format("MISSION_NAME is empty using : %1%") % mission_name));
    }

    int exitcode;
    if ((exitcode = readCommandLineOptions(argc, argv)) >= 0) {
        return exitcode;
    }

    //might overwrite the default mission with the command line
    InitUnitTables();

    // Initialise the master parts list before first use.
    Manifest::MPL();

#ifdef HAVE_PYTHON
    Python::init();

    Python::test();
#endif

    std::vector<std::vector<char> > temp = ROLES::getAllRolePriorities();
#if defined(HAVE_SDL)
#ifndef NO_SDL_JOYSTICK
    if (SDL_InitSubSystem(SDL_INIT_JOYSTICK)) {
        VS_LOG_FLUSH_EXIT(fatal, (boost::format("Couldn't initialize SDL: %1%") % SDL_GetError()), 1);
    }
#endif
#endif
    
    InitTime();

    AUDInit();
    AUDListenerGain(vs_options::instance().sound_gain);
    Music::InitMuzak();

    initSceneManager();
    initALRenderer();
    initScenes();

    //Register commands
    //COmmand Interpretor Seems to break VC8, so I'm leaving disabled for now - Patrick, Dec 24
    if (vs_options::instance().command_interpretor) {
        CommandInterpretor = new commandI;
        InitShipCommands();
    }
    _Universe = new Universe(argc, argv, vs_options::instance().galaxy.c_str());
    TheTopLevelUnit = new Unit(0);
    //InitTime();
    _Universe->Loop(bootstrap_first_loop);

    //Unregister commands - and cleanup memory
    UninitShipCommands();

    closeRenderer();

    cleanup();

    delete _Universe;
    CleanupUnitTables();
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
    bs_tp->Draw(vs_options::instance().default_boot_message.length() > 0 ?
            vs_options::instance().default_boot_message : message.length() > 0 ?
                    message : vs_options::instance().initial_boot_message);

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
        vector<string> s = parse_space_string(vs_options::instance().splash_screen);
        vector<string> sa = parse_space_string(vs_options::instance().splash_audio);
        int snum = time(nullptr) % s.size();
        SplashScreen = new Animation(s[snum].c_str(), false);
        if (!sa.empty() && sa[0].length()) {
            muzak->GotoSong(sa[snum % sa.size()]);
        }
        bs_tp = new TextPlane();
    }
    bootstrap_draw("Vegastrike Loading...", SplashScreen);
    //InitTime();
    if (i++ > 4) {
        if (_Universe) {
            if (vs_options::instance().main_menu) {
                UniverseUtil::startMenuInterface(true);
            } else {
                _Universe->Loop(bootstrap_main_loop);
            }
        }
    }
}

void SetStartupView(Cockpit *cp) {
    cp->SetView(vs_options::instance().startup_cockpit_view
            == "view_target" ? CP_TARGET : (vs_options::instance().startup_cockpit_view
            == "back" ? CP_BACK : (vs_options::instance().startup_cockpit_view
            == "chase" ? CP_CHASE
            : CP_FRONT)));
}

void bootstrap_main_loop() {
    static bool LoadMission = true;
    //InitTime();
    if (LoadMission) {
        LoadMission = false;
        active_missions.push_back(mission = new Mission(mission_name));

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
            pname = vs_options::instance().getPlayer(p);
            ppasswd = vs_options::instance().getPassword(p);

            if (!ignore_network) {
                //In network mode, test if all player sections are present
                if (pname.empty()) {
                    cleanexit = true;
                    VS_LOG_FLUSH_EXIT(fatal, (boost::format("Missing or incomplete section for player %1%") % p), 1);
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

        if (vs_options::instance().load_last_savegame) {
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
            // TODO: Figure out how to refactor this section to use a loop or similar, eliminating code duplication
            if (!vs_options::instance().intro1.empty()) {
                UniverseUtil::IOmessage(0, "game", "all", vs_options::instance().intro1);
                if (!vs_options::instance().intro2.empty()) {
                    UniverseUtil::IOmessage(4, "game", "all", vs_options::instance().intro2);
                    if (!vs_options::instance().intro3.empty()) {
                        UniverseUtil::IOmessage(8, "game", "all", vs_options::instance().intro3);
                        if (!vs_options::instance().intro4.empty()) {
                            UniverseUtil::IOmessage(12, "game", "all", vs_options::instance().intro4);
                            if (!vs_options::instance().intro5.empty()) {
                                UniverseUtil::IOmessage(16, "game", "all", vs_options::instance().intro5);
                            }
                        }
                    }
                }
            }
        }

        if (mission->getVariable("savegame",
                "").length() != 0
                && vs_options::instance().dockOnLoad) {
            for (size_t i = 0; i < _Universe->numPlayers(); i++) {
                QVector vec;
                DockToSavedBases(i, vec);
            }
        }

        if (vs_options::instance().load_last_savegame) {
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
        if (vs_options::instance().auto_hide) {
            UniverseUtil::hideSplashScreen();
        }
    }
    ///Draw Texture
}

// SGT 2020-07-16   This gets called from main() before initLogging,
//                  so it gets a pass on not using the Boost logging stuff
const char helpmessage[] =
        "Command line options for vegastrike\n"
        "\n"
        " -D -d \t Specify data directory\n"
        " -N -n \t Number of players\n"
        " -M -m \t Specify a mod to play\n"
        " -P -p \t Specify player location\n"
        " -J -j \t Start in a specific system\n"
        " -A -a \t Normal resolution (800x600)\n"
        " -H -h \t High resolution (1024x768)\n"
        " -V -v \t Super high resolution (1280x1024)\n"
        " --net \t Networking Enabled (Experimental)\n"
        " --debug[=#] \t Enable debugging output, 1 major warnings, 2 medium, 3 developer notes\n"
        " --test-audio \t Run audio tests\n"
        " --version \t Print the version and exit\n"
        "\n";
const char versionmessage[] =
        // (BenjamenMeyer) this will be `major.minor.patch+githash` once all is said and done
        "Vega Strike Engine Version " VEGASTRIKE_VERSION_STR "\n"
        "\n";

std::string ParseCommandLine(int argc, char **lpCmdLine) {
    std::string st;
    std::string retstr;
    std::string datatmp;
    g_game.vsdebug = '0';
    QVector PlayerLocation;
    for (int i = 1; i < argc; i++) {
        if (lpCmdLine[i][0] == '-') {
            std::cerr << "ARG #" << i << " = " << lpCmdLine[i] << std::endl;
            std::string input(lpCmdLine[i]);
            std::istringstream iStringStream(input);
            switch (lpCmdLine[i][1]) {
                case 'd':
                case 'D':
                    //Specifying data directory
                    if (lpCmdLine[i][2] == 0) {
                        std::cout << "Option -D requires an argument" << std::endl;
                        exit(1);
                    }
                    datatmp = &lpCmdLine[i][2];
                    if (VSFileSystem::DirectoryExists(datatmp)) {
                        VSFileSystem::datadir = datatmp;
                    } else {
                        std::cout << "Specified data directory not found... exiting" << std::endl;
                        exit(1);
                    }
                    std::cout << "Using data dir specified on command line : " << datatmp << std::endl;
                    break;
                case 'r':
                case 'R':
                    break;
                case 'N':
                case 'n':
                    if (!(lpCmdLine[i][2] == '1' && lpCmdLine[i][3] == '\0')) {
                        CONFIGFILE = new char[40 + strlen(lpCmdLine[i]) + 1];
                        sprintf(CONFIGFILE, "vegastrike.config.%splayer", lpCmdLine[i] + 2);
                    }
                    break;
                case 'M':
                case 'm':
                    retstr = string(lpCmdLine[i] + 2);
                    break;
                case 'f':
                case 'F':
                    break;
                case 'U':
                case 'u':
                    break;
                case 'P':
                case 'p':
                    break;
                case 'L':
                case 'l':
                    try {
                        iStringStream.ignore(2);
                        iStringStream >> PlayerLocation.i;
                        iStringStream.ignore(1, ',');
                        iStringStream >> PlayerLocation.j;
                        iStringStream.ignore(1, ',');
                        iStringStream >> PlayerLocation.k;
                    } catch (std::ios_base::failure &inputFailure) {
                        std::cout << "Error reading coordinates for player location: " << inputFailure.what()
                                << std::endl;
                        exit(1);
                    }
                    SetPlayerLoc(PlayerLocation, true);
                    break;
                case 'J':
                case 'j':             //low rez
                    st = string((lpCmdLine[i]) + 2);
                    SetPlayerSystem(st, true);
                    break;
                case 'A':             //average rez
                case 'a':
                    g_game.y_resolution = 600;
                    g_game.x_resolution = 800;
                    break;
                case 'H':
                case 'h':             //high rez
                    g_game.y_resolution = 768;
                    g_game.x_resolution = 1024;
                    break;
                case 'V':
                case 'v':
                    g_game.y_resolution = 1024;
                    g_game.x_resolution = 1280;
                    break;
                case 'G':
                case 'g':
                    break;
                case '-':
                    //long options
                    if (strcmp(lpCmdLine[i], "--benchmark") == 0) {
                        try {
                            iStringStream.ignore(1);
                            iStringStream >> benchmark;
                        } catch (std::ios_base::failure &inputFailure) {
                            std::cout << "Error parsing benchmark value: " << inputFailure.what() << std::endl;
                            exit(1);
                        }
                        i++;
                    } else if (strcmp(lpCmdLine[i], "--net") == 0) {
                        //don't ignore the network section of the config file
                        ignore_network = false;
                    } else if (strcmp(lpCmdLine[i], "--help") == 0) {
                        std::cout << helpmessage;
                        exit(0);
                    } else if (strcmp(lpCmdLine[i], "--version") == 0) {
                        std::cout << versionmessage;
                        exit(0);
                    } else if (strncmp(lpCmdLine[i], "--debug", 7) == 0) {
                        if (lpCmdLine[i][7] == 0) {
                            g_game.vsdebug = 1;
                        } else if (lpCmdLine[i][8] == 0) {
                            std::cout << helpmessage;
                            exit(0);
                        }
                        g_game.vsdebug = lpCmdLine[i][8] - '0';
                        ++i;
                    }
                    break;
            }
        } else {
            //no "-" before it - it's the mission name
            strncpy(mission_name, lpCmdLine[i], 1023);
            mission_name[1023] = '\0';
        }
    }
    if (false == legacy_data_dir_mode) {
        if (true == VSFileSystem::datadir.empty()) {
            VS_LOG_FLUSH_EXIT(fatal, "Data directory not specified.", 1);
        }
    }

    return retstr;
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
