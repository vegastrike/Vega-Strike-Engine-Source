/*
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include "cs_python.h"
#include "audio/test.h"
#if defined (HAVE_SDL)
#include <SDL/SDL.h>
#endif
#include "cmd/role_bitmask.h"
#if defined (WITH_MACOSX_BUNDLE)
#import <sys/param.h>
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
#include "cmd/unit.h"
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
#include "cmd/unit_factory.h"

#include "audio/SceneManager.h"
#include "audio/TemplateManager.h"
#include "audio/renderers/OpenAL/BorrowedOpenALRenderer.h"

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

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

#include "options.h"

using std::cout;
using std::cerr;
using std::endl;

vs_options game_options;

namespace logging = boost::log;

/*
 * Globals
 */
Universe  *_Universe;
TextPlane *bs_tp = NULL;
char SERVER = 0;

//false if command line option --net is given to start without network
static bool ignore_network = true;

void enableNetwork( bool usenetwork )
{
    ignore_network = !usenetwork;
}

/*
 * Function definitions
 */

void setup_game_data()
{
    //pass in config file l8r??
    g_game.audio_frequency_mode = 4;     //22050/16
    g_game.sound_enabled = 1;
    g_game.use_textures = 1;
    g_game.use_ship_textures    = 0;
    g_game.use_planet_textures  = 0;
    g_game.use_sprites       = 1;
    g_game.use_animations    = 1;
    g_game.use_videos        = 1;
    g_game.use_logos = 1;
    g_game.sound_volume      = 1;
    g_game.music_volume      = 1;
    g_game.warning_level     = 20;
    g_game.capture_mouse     = GFXFALSE;
    g_game.y_resolution      = 768;
    g_game.x_resolution      = 1024;
    g_game.fov = 78;
    g_game.MouseSensitivityX = 2;
    g_game.MouseSensitivityY = 4;
}
VegaConfig * createVegaConfig( const char *file )
{
    return new GameVegaConfig( file );
}

std::string ParseCommandLine(int argc, char ** CmdLine);
/**
 * Returns an exit code >= 0 if the game is supposed to exit rightaway
 * Returns an exit code < 0 if the game can continue loading.
 */
int readCommandLineOptions(int argc, char ** argv);

// FIXME: Code should throw exception instead of calling exit
void VSExit( int code)
{
    Music::CleanupMuzak();
    winsys_exit( code );
}

void cleanup( void )
{
    STATIC_VARS_DESTROYED = true;
    printf( "Thank you for playing!\n" );
    if (_Universe != NULL )
        _Universe->WriteSaveGame( true );
#ifdef _WIN32
#if defined (_MSC_VER) && defined (_DEBUG)
    if (!cleanexit)
        _RPT0( _CRT_ERROR, "WARNING: Vega Strike exit not clean\n" );
    return;
#endif
#else
    while (!cleanexit)
        usleep(10000);
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

LeakVector< Mission* >active_missions;

char mission_name[1024];

void bootstrap_main_loop();
void bootstrap_first_loop();
#if defined (WITH_MACOSX_BUNDLE)
//WTF! this causes windowed creation to fail... please justify yourself ;-)  #undef main
#endif
void nothinghappens( unsigned int, unsigned int, bool, int, int ) {}

void initSceneManager()
{
    cout << "Creating scene manager..." << endl;
    Audio::SceneManager *sm = Audio::SceneManager::getSingleton();
    
    cout << "Creating template manager..." << endl;
    Audio::TemplateManager::getSingleton();
    
    if (Audio::SceneManager::getSingleton() == 0)
        throw Audio::Exception("Singleton null after SceneManager instantiation");

    sm->setMaxSources( g_game.max_sound_sources );
}

void initALRenderer()
{
    cerr << "  Initializing renderer..." << endl;
    Audio::SceneManager *sm = Audio::SceneManager::getSingleton();
    
    if (g_game.sound_enabled) {
        SharedPtr<Audio::Renderer> renderer(new Audio::BorrowedOpenALRenderer);
        renderer->setMeterDistance(1.0);
        renderer->setDopplerFactor(0.0);
        
        sm->setRenderer( renderer );
    }
}

void initScenes()
{
    Audio::SceneManager *sm = Audio::SceneManager::getSingleton();
    
    sm->createScene("video");
    sm->createScene("music");
    sm->createScene("cockpit");
    sm->createScene("base");
    sm->createScene("space");
    
    sm->setSceneActive("video", true);
}

void closeRenderer()
{
    cerr << "Shutting down renderer..." << endl;
    Audio::SceneManager::getSingleton()->setRenderer( SharedPtr<Audio::Renderer>() );
}

extern void InitUnitTables();
extern void CleanupUnitTables();
bool isVista = false;

Unit *TheTopLevelUnit;

void initLogging(char debugLevel){
    auto loggingCore = logging::core::get();

    switch (debugLevel) {
    case 1:
        loggingCore->set_filter(logging::trivial::severity >= logging::trivial::info);
        break;
    case 2:
        loggingCore->set_filter(logging::trivial::severity >= logging::trivial::debug);
        break;
    case 3:
        loggingCore->set_filter(logging::trivial::severity >= logging::trivial::trace);
        break;
    default:
        loggingCore->set_filter(logging::trivial::severity >= logging::trivial::warning);
        break;
    }
}

int main( int argc, char *argv[] )
{

    VSFileSystem::ChangeToProgramDirectory( argv[0] );

#ifdef WIN32
	VSFileSystem::InitHomeDirectory();
#endif

    CONFIGFILE = 0;
    mission_name[0] = '\0';
    {
        char pwd[8192] = "";
        if (NULL != getcwd( pwd, 8191 )) {
            pwd[8191] = '\0';
            printf( " In path %s\n", pwd );
        } else {
            printf( " In path <<path too long>>\n" );
        }
    }
#ifdef _WIN32
    OSVERSIONINFO osvi;
    ZeroMemory( &osvi, sizeof (OSVERSIONINFO) );
    osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);

    GetVersionEx( &osvi );
    isVista = (osvi.dwMajorVersion == 6);
    printf( "Windows version %d %d\n", osvi.dwMajorVersion, osvi.dwMinorVersion );
#endif
    /* Print copyright notice */
    printf( "Vega Strike "  " \n"
            "See http://www.gnu.org/copyleft/gpl.html for license details.\n\n" );
    /* Seed the random number generator */
    if (benchmark < 0.0)
        srand( time( NULL ) );
    else
        //in benchmark mode, always use the same seed
        srand( 171070 );
    //this sets up the vegastrike config variable
    setup_game_data();
    //loads the configuration file .vegastrike/vegastrike.config from home dir if such exists
    {
        string subdir = ParseCommandLine( argc, argv );
        cerr<<"GOT SUBDIR ARG = "<<subdir<<endl;
        if (CONFIGFILE == 0) {
            CONFIGFILE = new char[42];
            sprintf( CONFIGFILE, "vegastrike.config" );
        }
        //Specify the config file and the possible mod subdir to play
        VSFileSystem::InitPaths( CONFIGFILE, subdir );
    }

    // If no debug argument is supplied, set to what the config file has.
    if (g_game.vsdebug == '0')
        g_game.vsdebug = game_options.vsdebug;

    initLogging(g_game.vsdebug);

    // can use the vegastrike config variable to read in the default mission
    if ( game_options.force_client_connect )
        ignore_network = false;
    if (mission_name[0] == '\0') {
        strncpy( mission_name, game_options.default_mission.c_str(), 1023 );
        mission_name[1023] = '\0';
        cerr<<"MISSION_NAME is empty using : "<<mission_name<<endl;
    }
    

    int exitcode;
    if ((exitcode = readCommandLineOptions(argc,argv)) >= 0)
        return exitcode;

    //might overwrite the default mission with the command line
    InitUnitTables();
#ifdef HAVE_PYTHON
      Python::init();

      Python::test();
#endif

    std::vector<std::vector <char > > temp = ROLES::getAllRolePriorities();
#if defined(HAVE_SDL)
#ifndef NO_SDL_JOYSTICK
    if ( SDL_InitSubSystem( SDL_INIT_JOYSTICK ) ) {
        VSFileSystem::vs_fprintf( stderr, "Couldn't initialize SDL: %s\n", SDL_GetError() );
        winsys_exit( 1 );
    }
#endif
#endif
#if 0
    InitTime();
    UpdateTime();
#endif

    AUDInit();
    AUDListenerGain( game_options.sound_gain );
    Music::InitMuzak();
    
    initSceneManager();
    initALRenderer();
    initScenes();
    
    //Register commands
    //COmmand Interpretor Seems to break VC8, so I'm leaving disabled for now - Patrick, Dec 24
    if ( game_options.command_interpretor ) {
        CommandInterpretor = new commandI;
        InitShipCommands();
    }
    _Universe = new GameUniverse( argc, argv, game_options.galaxy.c_str() );
    TheTopLevelUnit = UnitFactory::createUnit();
    _Universe->Loop( bootstrap_first_loop );

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
void SetStarSystemLoading( bool value )
{
    BootstrapMyStarSystemLoading = value;
}
bool GetStarSystemLoading()
{
    return BootstrapMyStarSystemLoading;
}
void SetSplashScreen( Animation *ss )
{
    SplashScreen = ss;
}
Animation * GetSplashScreen()
{
    return SplashScreen;
}
void bootstrap_draw( const std::string &message, Animation *newSplashScreen )
{
    static Animation *ani = NULL;
    static bool reentryWatchdog = false;
    if (!BootstrapMyStarSystemLoading || reentryWatchdog)
        return;
    Music::MuzakCycle();     //Allow for loading music...
    if (SplashScreen == NULL && newSplashScreen == NULL) {
        //if there's no splashscreen, we don't draw on it
        //this happens, when the splash screens texture is loaded
        return;
    }
    
    reentryWatchdog = true;
    if (newSplashScreen != NULL)
        ani = newSplashScreen;
    UpdateTime();

    Matrix tmp;
    Identity( tmp );
    BootstrapMyStarSystemLoading = false;
    static Texture dummy( "white.bmp", 0, MIPMAP, TEXTURE2D, TEXTURE_2D, 1 );
    BootstrapMyStarSystemLoading = true;
    dummy.MakeActive();
    GFXDisable( LIGHTING );
    GFXDisable( DEPTHTEST );
    GFXBlendMode( ONE, ZERO );
    GFXEnable( TEXTURE0 );
    GFXDisable( TEXTURE1 );
    GFXColor4f( 1, 1, 1, 1 );
    GFXClear( GFXTRUE );
    GFXLoadIdentity( PROJECTION );
    GFXLoadIdentity( VIEW );
    GFXLoadMatrixModel( tmp );
    GFXBeginScene();

    bs_tp->SetPos( -.99, -.97 );     //Leave a little bit of room for the bottoms of characters.
    bs_tp->SetCharSize( .4, .8 );
    ScaleMatrix( tmp, Vector( 6500, 6500, 0 ) );
    GFXLoadMatrixModel( tmp );
    GFXHudMode( GFXTRUE );
    if (ani) {
        if (GetElapsedTime() < 10) ani->UpdateAllFrame();
        {
        ani->DrawNow( tmp ); //VSFileSystem::vs_fprintf( stderr, "(new?) splash screen ('animation'?) %d.  ", (long long)ani ); //temporary, by chuck
        }
    }
    bs_tp->Draw( game_options.default_boot_message.length() > 0 ? 
		 game_options.default_boot_message : message.length() > 0 ? 
		 message : game_options.initial_boot_message );

    GFXHudMode( GFXFALSE );
    GFXEndScene();

    reentryWatchdog = false;
}
extern Unit **fighters;

bool SetPlayerLoc( QVector &sys, bool set )
{
    static QVector mysys;
    static bool    isset = false;
    if (set) {
        isset = true;
        mysys = sys;
        return true;
    } else {
        if (isset)
            sys = mysys;
        return isset;
    }
}
bool SetPlayerSystem( std::string &sys, bool set )
{
    static std::string mysys;
    static bool isset = false;
    if (set) {
        isset = true;
        mysys = sys;
        return true;
    } else {
        if (isset)
            sys = mysys;
        return isset;
    }
}
vector< string >parse_space_string( std::string s )
{
    vector< string >  ret;
    string::size_type index = 0;
    while ( ( index = s.find( " " ) ) != string::npos ) {
        ret.push_back( s.substr( 0, index ) );
        s = s.substr( index+1 );
    }
    ret.push_back( s );
    return ret;
}
void bootstrap_first_loop()
{
    static int  i = 0;
    if (i == 0) {
        vector< string >s  = parse_space_string( game_options.splash_screen );
        vector< string >sa = parse_space_string( game_options.splash_audio );
        int snum = time( NULL )%s.size();
        SplashScreen = new Animation( s[snum].c_str(), 0 );
        if ( sa.size() && sa[0].length() ) muzak->GotoSong( sa[snum%sa.size()] );
        bs_tp = new TextPlane();
    }
    bootstrap_draw( "Vegastrike Loading...", SplashScreen );
    if (i++ > 4) {
        if (_Universe) {
            if (game_options.main_menu)
                UniverseUtil::startMenuInterface( true );
            else
                _Universe->Loop( bootstrap_main_loop );
        }
    }
}

void SetStartupView( Cockpit *cp )
{
    cp->SetView( game_options.startup_cockpit_view
                == "view_target" ? CP_TARGET : ( game_options.startup_cockpit_view
                                                == "back" ? CP_BACK : (game_options.startup_cockpit_view == "chase" ? CP_CHASE : CP_FRONT) ) );
}
void bootstrap_main_loop()
{
    static bool LoadMission  = true;
    InitTime();
    if (LoadMission) {
        LoadMission = false;
        active_missions.push_back( mission = new Mission( mission_name ) );

        mission->initMission();

        UniverseUtil::showSplashScreen( "" );         //Twice for double or triple-buffering
        UniverseUtil::showSplashScreen( "" );

        QVector pos;
        string  planetname;

        mission->GetOrigin( pos, planetname );
        string  mysystem     = mission->getVariable( "system", "sol.system" );

        int     numplayers;
        numplayers = XMLSupport::parse_int( mission->getVariable( "num_players", "1" ) );
        vector< std::string >playername;
        vector< std::string >playerpasswd;
        string pname, ppasswd;
        for (int p = 0; p < numplayers; p++) {
            pname   = game_options.getPlayer(p);
            ppasswd = game_options.getPassword(p);

            if (!ignore_network) {
                //In network mode, test if all player sections are present
                if (pname == "") {
                    cout<<"Missing or incomlpete section for player "<<p<<endl;
                    cleanexit = true;
                    winsys_exit( 1 );
                }
            }
            playername.push_back( pname );
            playerpasswd.push_back( ppasswd );
        }
        float  credits = XMLSupport::parse_float( mission->getVariable( "credits", "0" ) );
        g_game.difficulty = XMLSupport::parse_float( mission->getVariable( "difficulty", "1" ) );
        string savegamefile = mission->getVariable( "savegame", "" );
        vector< SavedUnits > savedun;
        vector< string >     playersaveunit;
        vector< StarSystem* >ss;
        vector< string >     starsysname;
        vector< QVector >    playerNloc;


        vector< vector< std::string > >savefiles;

        _Universe->SetupCockpits( playername );

        vector< std::string >::iterator it, jt;
        unsigned int k = 0;

        Cockpit    *cp = _Universe->AccessCockpit( k );
        SetStartupView( cp );
        bool        setplayerXloc = false;
        std::string psu;
        if (k == 0) {
            QVector     myVec;
            if ( SetPlayerLoc( myVec, false ) )
              _Universe->AccessCockpit( 0 )->savegame->SetPlayerLocation( myVec );
            std::string st;
            if ( SetPlayerSystem( st, false ) )
              _Universe->AccessCockpit( 0 )->savegame->SetStarSystem( st );
          }
        vector< SavedUnits >saved;
        vector< string > packedInfo;

        if (game_options.load_last_savegame) {
            _Universe->AccessCockpit( k )->savegame->ParseSaveGame( savegamefile,
                                                                    mysystem,
                                                                    mysystem,
                                                                    pos,
                                                                    setplayerXloc,
                                                                    credits,
                                                                    packedInfo,
                                                                    k );
        } else {
          _Universe->AccessCockpit( k )->savegame->SetOutputFileName( savegamefile );
        }

        _Universe->AccessCockpit( k )->UnpackUnitInfo(packedInfo);
        CopySavedShips( playername[k], k, packedInfo, true );
        playersaveunit.push_back( _Universe->AccessCockpit( k )->GetUnitFileName() );
        _Universe->AccessCockpit( k )->credits = credits;
        ss.push_back( _Universe->Init( mysystem, Vector( 0, 0, 0 ), planetname ) );
        if (setplayerXloc)
          playerNloc.push_back( pos );
        else
          playerNloc.push_back( QVector( FLT_MAX, FLT_MAX, FLT_MAX ) );
        for (unsigned int j = 0; j < saved.size(); j++)
          savedun.push_back( saved[j] );

        SetStarSystemLoading( true );
        InitializeInput();

        vs_config->bindKeys();         //gotta do this before we do ai

        createObjects( playersaveunit, ss, playerNloc, savefiles );
        while ( !savedun.empty() ) {
            AddUnitToSystem( &savedun.back() );
            savedun.pop_back();
        }
        forcefeedback = new ForceFeedback();

        UpdateTime();
        FactionUtil::LoadContrabandLists();
        {
            if ( !game_options.intro1.empty() ) {
                UniverseUtil::IOmessage( 0, "game", "all", game_options.intro1 );
                if ( !game_options.intro2.empty() ) {
                    UniverseUtil::IOmessage( 4, "game", "all", game_options.intro2 );
                    if ( !game_options.intro3.empty() ) {
                        UniverseUtil::IOmessage( 8, "game", "all", game_options.intro3 );
                        if ( !game_options.intro4.empty() ) {
                            UniverseUtil::IOmessage( 12, "game", "all",game_options.intro4 );
                            if ( !game_options.intro5.empty() )
                                UniverseUtil::IOmessage( 16, "game", "all", game_options.intro5 );
                        }
                    }
                }
            }
        }

        if ( mission->getVariable( "savegame",
                                     "" ).length() != 0
            && game_options.dockOnLoad) {
            for (size_t i = 0; i < _Universe->numPlayers(); i++) {
                QVector vec;
                DockToSavedBases( i, vec );
            }
        }

        if (game_options.load_last_savegame) {
            //Don't write if we didn't load...
            for (unsigned int i = 0; i < _Universe->numPlayers(); ++i)
                WriteSaveGame( _Universe->AccessCockpit( i ), false );
        }
        cur_check = getNewTime();
        for (unsigned int i = 0; i < _Universe->numPlayers(); ++i)
            _Universe->AccessCockpit( i )->savegame->LoadSavedMissions();
        _Universe->Loop( main_loop );
        ///return to idle func which now should call main_loop mohahahah
        if ( game_options.auto_hide )
            UniverseUtil::hideSplashScreen();
    }
    ///Draw Texture
}

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
    "\n";
std::string ParseCommandLine( int argc, char **lpCmdLine )
{
    std::string st;
    std::string retstr;
    std::string datatmp;
    g_game.vsdebug = '0';
    QVector     PlayerLocation;
    for (int i = 1; i < argc; i++) {
        if (lpCmdLine[i][0] == '-') {
            cerr<<"ARG #"<<i<<" = "<<lpCmdLine[i]<<endl;
            switch (lpCmdLine[i][1])
            {
            case 'd':
            case 'D':
                //Specifying data directory
                if (lpCmdLine[i][2] == 0) {
                    cout<<"Option -D requires an argument"<<endl;
                    exit( 1 );
                }
                datatmp = &lpCmdLine[i][2];
                if ( VSFileSystem::DirectoryExists( datatmp ) ) {
                    VSFileSystem::datadir = datatmp;
                } else {
                    cout<<"Specified data directory not found... exiting"<<endl;
                    exit( 1 );
                }
                cout<<"Using data dir specified on command line : "<<datatmp<<endl;
                break;
            case 'r':
            case 'R':
                break;
            case 'N':
            case 'n':
                if ( !(lpCmdLine[i][2] == '1' && lpCmdLine[i][3] == '\0') ) {
                    CONFIGFILE = new char[40+strlen( lpCmdLine[i] )+1];
                    sprintf( CONFIGFILE, "vegastrike.config.%splayer", lpCmdLine[i]+2 );
                }
                break;
            case 'M':
            case 'm':
                retstr = string( lpCmdLine[i]+2 );
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
                if ( 3 == sscanf( lpCmdLine[i]+2, "%lf,%lf,%lf", &PlayerLocation.i, &PlayerLocation.j, &PlayerLocation.k ) )
                    SetPlayerLoc( PlayerLocation, true );
                break;
            case 'J':
            case 'j':             //low rez
                st = string( (lpCmdLine[i])+2 );
                SetPlayerSystem( st, true );
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
                if (strcmp( lpCmdLine[i], "--benchmark" ) == 0) {
                    benchmark = atof( lpCmdLine[i+1] );
                    i++;
                } else if (strcmp( lpCmdLine[i], "--net" ) == 0) {
                    //don't ignore the network section of the config file
                    ignore_network = false;
                } else if (strcmp( lpCmdLine[i], "--help" ) == 0) {
                    cout<<helpmessage;
                    exit( 0 );
                } else if (strncmp( lpCmdLine[i], "--debug", 7 ) == 0) {
                    if (lpCmdLine[i][7] == 0) {
                        g_game.vsdebug = 1;
                    } else if (lpCmdLine[i][8] == 0) {
                        cout<<helpmessage;
                        exit( 0 );
                    }
                    g_game.vsdebug = lpCmdLine[i][8] - '0';
                    ++i;
                }
                break;
            }
        } else {
            //no "-" before it - it's the mission name
            strncpy( mission_name, lpCmdLine[i], 1023 );
            mission_name[1023] = '\0';
        }
    }
    return retstr;
}
#undef main
int readCommandLineOptions(int argc, char ** argv)
{
    for (int i=1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            if (strcmp("--test-audio", argv[i])==0) {
                return Audio::Test::main(argc, argv);
            }
        }
    }
    return -1;
}
