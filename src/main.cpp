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
#include <Python.h>
#include "python/init.h"
#include "config.h"
#if defined(HAVE_SDL)
#include <SDL/SDL.h>
#endif
#include "cmd/role_bitmask.h"
#if defined(WITH_MACOSX_BUNDLE)
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
#include "savegame.h"
#include "force_feedback.h"
#include "gfx/hud.h"
#include "gldrv/winsys.h"
#include "universe_util.h"
#include "networking/netclient.h"
#include "universe.h"
#include "save_util.h"
#include "gfx/masks.h"
#include "cmd/music.h"

#include <time.h>
#ifndef _WIN32
#include <sys/signal.h>
#endif

#if defined (_MSC_VER) && defined(_DEBUG)
#include <crtdbg.h>
#endif

#if defined(CG_SUPPORT)
#include "cg_global.h"
#endif

commandI CommandInterpretor;

int nadanixnuthin() {
  float a=0;
  int test=0;
  Delta(a, a);
  flipbit(test,test);
  checkbit(test,test);
  dosetbit(test,test);
  unsetbit(test,test);
  return 0;
}
/*
 * Globals 
 */
Universe * _Universe;
// GameUniverse _Universe;
TextPlane *bs_tp=NULL;
char SERVER=0;

// false if command line option --net is given to start without network
static bool ignore_network = true;

/* 
 * Function definitions
 */

void setup_game_data ( ){ //pass in config file l8r??
  g_game.audio_frequency_mode=4;//22050/16
  g_game.sound_enabled =1;
  g_game.use_textures =1;
  g_game.use_ship_textures =0;
  g_game.use_planet_textures =0;
  g_game.use_sprites =1;
  g_game.use_animations =1;
  g_game.use_videos =1;
  g_game.use_logos =1;
  g_game.music_enabled=1;
  g_game.sound_volume=1;
  g_game.music_volume=1;
  g_game.warning_level=20;
  g_game.capture_mouse=GFXFALSE;
  g_game.y_resolution = 768;
  g_game.x_resolution = 1024;
  g_game.fov=78;
  g_game.MouseSensitivityX=2;
  g_game.MouseSensitivityY=4;

}
VegaConfig * createVegaConfig( char * file)
{
	return new GameVegaConfig( file);
}

extern QVector DockToSavedBases (int playernum);
extern bool soundServerPipes();
extern void InitUnitTables(); // universe_generic.cpp

std::string ParseCommandLine(int argc, char ** CmdLine);
void VSExit( int code)
{
   if (muzak) {
      delete muzak;
      muzak=NULL;
    }
	winsys_exit(code);
}

void cleanup(void)
{
  VSFileSystem::vs_fprintf( stdout, "\n\nLoop average : %g\n\n", avg_loop);
  VSFileSystem::vs_fprintf( stderr, "\n\nLoop average : %g\n\n", avg_loop);
  STATIC_VARS_DESTROYED=true;
  printf ("Thank you for playing!\n");

  // In network mode, we may not do the save since it is useless
  if( _Universe != NULL && Network==NULL)
	  _Universe->WriteSaveGame(true);
#ifdef _WIN32
#if defined (_MSC_VER) && defined(_DEBUG)
  if (!cleanexit) {
    _RPT0(_CRT_ERROR, "WARNING: Vega Strike exit not clean\n");
  }
  return;
#endif
#else
  while (!cleanexit) 
    int i=1;
#endif
  if( Network!=NULL)
  {
		cout<<"Number of players"<<_Universe->numPlayers()<<endl;
		for( int i=0; i<_Universe->numPlayers(); i++)
			if( Network[i].isInGame())
				Network[i].logout();
		delete [] Network;
  }

#if defined(CG_SUPPORT)
    if (cloak_cg->vertexProgram) cgDestroyProgram(cloak_cg->vertexProgram);
    if (cloak_cg->shaderContext)  cgDestroyContext(cloak_cg->shaderContext);
#endif

  if (muzak) {
    delete muzak;
    muzak=NULL;
  }
  winsys_shutdown();
  //    write_config_file();
  AUDDestroy();
  //destroyObjects();
  //Unit::ProcessDeleteQueue();
  //delete _Universe;
    delete [] CONFIGFILE;

  
}

//Mission *mission;
LeakVector<Mission *> active_missions;

char mission_name[1024];

void bootstrap_main_loop();
void bootstrap_first_loop();

#if defined(WITH_MACOSX_BUNDLE)
 #undef main
#endif
void nothinghappens (unsigned int, unsigned int, bool,int,int) {

}
extern void InitUnitTables();
int main( int argc, char *argv[] ) 
{
#if defined(WITH_MACOSX_BUNDLE)||defined(_WIN32)
  {
    char *parentdir;
    int pathlen=strlen(argv[0]);
    parentdir=new char[pathlen+1];
    char *c;
    strncpy ( parentdir, argv[0], pathlen+1 );
    c = (char*) parentdir;
    while (*c != '\0')     /* go to end */
      c++;
    
    while ((*c != '/')&&(*c != '\\')&&c>parentdir)      /* back up to parent */
      c--;
    
    *c = '\0';             /* cut off last part (binary name) */
    if (strlen (parentdir)>0) {  
      chdir (parentdir);/* chdir to the binary app's parent */
    }
    delete []parentdir;
  }    
#endif
	CONFIGFILE=0;
	mission_name[0]='\0';
#if defined (WITH_MACOSX_BUNDLE)
        {
          //chdir("../Resources");
          char pwd[8192]="";
          getcwd(pwd,8191);
          printf (" In path %s\n",pwd);
        }
#endif
    /* Print copyright notice */
	printf("Vega Strike "  " \n"
		   "See http://www.gnu.org/copyleft/gpl.html for license details.\n\n" );
    /* Seed the random number generator */
    

    if(benchmark<0.0){
      srand( time(NULL) );
    }
    else{
      // in benchmark mode, always use the same seed
      srand(171070);
    }
    //this sets up the vegastrike config variable
    setup_game_data(); 
    // loads the configuration file .vegastrike/vegastrike.config from home dir if such exists
	{
	    string subdir=ParseCommandLine(argc,argv);
		cerr<<"GOT SUBDIR ARG = "<<subdir<<endl;
		if (CONFIGFILE==0) {
			CONFIGFILE=new char[42];
			sprintf(CONFIGFILE,"vegastrike.config");
		}

		// Specify the config file and the possible mod subdir to play
		VSFileSystem::InitPaths( CONFIGFILE, subdir);
	}
    //can use the vegastrike config variable to read in the default mission

  g_game.music_enabled = XMLSupport::parse_bool (vs_config->getVariable ("audio","Music","true"));
#if !defined( _WIN32)
  if (g_game.music_enabled&&!soundServerPipes()) {
    int pid=fork();
    if (!pid) {
	  string soundserver_path = VSFileSystem::datadir+"/bin/soundserver";
      pid=execlp(soundserver_path.c_str() , soundserver_path.c_str(),NULL);
      soundserver_path = VSFileSystem::datadir+"/soundserver";
      pid=execlp(soundserver_path.c_str() , soundserver_path.c_str(),NULL);
      g_game.music_enabled=false;
      VSFileSystem::vs_fprintf(stderr,"Unable to spawn music player server\n");
      exit (0);
    } else {
      if (pid==-1) {
	g_game.music_enabled=false;
      }
    }
  }
#endif
    if (mission_name[0]=='\0')
	{
      strcpy(mission_name,vs_config->getVariable ("general","default_mission","test/test1.mission").c_str());
	  cerr<<"MISSION_NAME is empty using : "<<mission_name<<endl;
	}
    //might overwrite the default mission with the command line
	InitUnitTables();
#ifdef HAVE_PYTHON
    Python::init();

    Python::test();
#endif
    std::vector<std::vector <char > > temp = ROLES::getAllRolePriorities();
#if defined(HAVE_SDL)
#ifndef NO_SDL_JOYSTICK
    // && defined(HAVE_SDL_MIXER)
  if (  SDL_InitSubSystem( SDL_INIT_JOYSTICK )) {
        VSFileSystem::vs_fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
        winsys_exit(1);
    }
    // signal( SIGSEGV, SIG_DFL );
#endif
#endif
#if 0
    InitTime();
    UpdateTime();
#endif

    AUDInit();
    AUDListenerGain (XMLSupport::parse_float(vs_config->getVariable ("audio","sound_gain",".5")));   
    /* Set up a function to clean up when program exits */
    winsys_atexit( cleanup );
    /*
#if defined(HAVE_SDL) && defined(HAVE_SDL_MIXER)

    init_audio_data();
    init_audio();
    init_joystick();

#endif
    */
    _Universe= new GameUniverse(argc,argv,vs_config->getVariable ("general","galaxy","milky_way.xml").c_str());
	//_Universe.Init(argc, argv, vs_config->getVariable ("general","galaxy","milky_way.xml").c_str());
    //    winsys_set_keyboard_func(nothinghappens);
    _Universe->Loop(bootstrap_first_loop);
    return 0;
}
  static Animation * SplashScreen = NULL;
static bool BootstrapMyStarSystemLoading=true;
void SetStarSystemLoading (bool value) {
  BootstrapMyStarSystemLoading=value;
}
bool GetStarSystemLoading () {
  return BootstrapMyStarSystemLoading;
}
void SetSplashScreen(Animation * ss) {
  SplashScreen=ss;
}
void bootstrap_draw (const std::string &message, Animation * newSplashScreen) {

  static Animation *ani=NULL;
  if (!BootstrapMyStarSystemLoading) {
    return;
  }
  if(SplashScreen==NULL&&newSplashScreen==NULL){
    // if there's no splashscreen, we don't draw on it
    // this happens, when the splash screens texture is loaded
    return;
  }

  if (newSplashScreen!=NULL) {
    ani = newSplashScreen;
  }
  UpdateTime();

  Matrix tmp;
  Identity (tmp);
  BootstrapMyStarSystemLoading=false;
  static Texture dummy ("white.bmp",0,MIPMAP,TEXTURE2D,TEXTURE_2D, 1 );
  BootstrapMyStarSystemLoading=true;
  dummy.MakeActive();
  GFXDisable(LIGHTING);
  GFXDisable(DEPTHTEST);
  GFXBlendMode (ONE,ZERO);
  GFXEnable (TEXTURE0);
  GFXDisable (TEXTURE1);
  GFXColor4f (1,1,1,1);
  GFXClear (GFXTRUE);
  GFXLoadIdentity (PROJECTION);
  GFXLoadIdentity(VIEW);
  GFXLoadMatrixModel (tmp);
  GFXBeginScene();

  bs_tp->SetPos (-.99,-.97); // Leave a little bit of room for the bottoms of characters.
  bs_tp->SetCharSize (.4,.8);
  ScaleMatrix (tmp,Vector (6500,6500,0));
  GFXLoadMatrixModel (tmp);
  GFXHudMode (GFXTRUE);
  if (ani) {
    ani->UpdateAllFrame();
    ani->DrawNow(tmp);
  }
  static std::string defaultbootmessage=vs_config->getVariable("graphics","default_boot_message","");
  
  bs_tp->Draw (defaultbootmessage.length()>0?defaultbootmessage:message);  
  GFXHudMode (GFXFALSE);
  GFXEndScene();

}
extern Unit **fighters;


bool SetPlayerLoc (QVector &sys, bool set) {
  static QVector mysys;
  static bool isset=false;
  if (set) {
    isset = true;
    mysys = sys;
    return true;
  }else {
    if (isset)
      sys =mysys;
    return isset;
  }  
  
}
bool SetPlayerSystem (std::string &sys, bool set) {
  static std::string mysys;
  static bool isset=false;
  if (set) {
    isset = true;
    mysys = sys;
    return true;
  }else {
    if (isset)
      sys =mysys;
    return isset;
  }
}
vector<string> parse_space_string(std::string s) {
	vector<string> ret;
	string::size_type index=0;
	while ((index =s.find(" "))!=string::npos) {
		ret.push_back (s.substr(0,index));
		s = s.substr (index+1);
	}
	ret.push_back(s);
	return ret;
}
void bootstrap_first_loop() {
  static int i=0;
  static std::string ss=vs_config->getVariable ("graphics","splash_screen","vega_splash.ani");
  if (i==0) {
	vector<string> s = parse_space_string(ss);
    SplashScreen = new Animation (s[time(NULL)%s.size()].c_str(),0);
    bs_tp=new TextPlane();
  }
  bootstrap_draw ("Vegastrike Loading...",SplashScreen);
  
  if (i++>4) {
    _Universe->Loop(bootstrap_main_loop);
  }
}
void SetStartupView(Cockpit * cp) {
      static std::string startupview = vs_config->getVariable("graphics","startup_cockpit_view","front");
      cp->SetView(startupview=="view_target"?CP_TARGET:(startupview=="back"?CP_BACK:(startupview=="chase"?CP_CHASE:CP_FRONT)));
}
void bootstrap_main_loop () {

  static bool LoadMission=true;
  InitTime();

  //  bootstrap_draw ("Beginning Load...",SplashScreen);
  //  bootstrap_draw ("Beginning Load...",SplashScreen);
  if (LoadMission) {
    LoadMission=false;
    active_missions.push_back(mission=new Mission(mission_name));

    mission->initMission();

    //    if (SplashScreen)
    //      delete SplashScreen;
    //    SplashScreen = new Animation (mission->getVariable ("splashscreen",vs_config->getVariable ("graphics","splash_screen","vega_splash.ani")).c_str(),0); 
    bootstrap_draw ("Vegastrike Loading...",SplashScreen);
    if (g_game.music_enabled&&!soundServerPipes()) {
#if defined( _WIN32) && !defined( __CYGWIN__)
      string ss_path = VSFileSystem::datadir+"/soundserver.exe";
      int pid=spawnl(P_NOWAIT,ss_path.c_str(),ss_path.c_str(),NULL);
      if (pid==-1) {
		ss_path = VSFileSystem::datadir+"/bin/soundserver.exe";
		chdir("bin");
		int pid=spawnl(P_NOWAIT,ss_path.c_str(),ss_path.c_str(),NULL);
		if (pid==-1) {
			g_game.music_enabled=false;
			VSFileSystem::vs_fprintf(stderr,"Unable to spawn music player server Error (%d)\n",pid);
		}
      }
#endif
    }
    QVector pos;
    string planetname;

    mission->GetOrigin(pos,planetname);
    bool setplayerloc=false;
    string mysystem = mission->getVariable("system","sol.system");
	string srvip = vs_config->getVariable("network","server_ip","");
    if( ignore_network ) srvip = "";

	int numplayers;
	/*
	string nbplayers = vs_config->getVariable("network","nbplayers","1");
	// Test if nb players if present in network section
	if( srvip != "" && nbplayers != "")
	{
		numplayers = atoi( nbplayers.c_str());
		cout<<numplayers<<" Players in Networking Mode"<<endl;
	}
	else
	{
	*/
		numplayers = XMLSupport::parse_int (mission->getVariable ("num_players","1"));
	//}
    vector <std::string>playername;
    vector <std::string>playerpasswd;
	string pname, ppasswd;
    for (int p=0;p<numplayers;p++) {
	  pname = vs_config->getVariable("player"+((p>0)?tostring(p+1):string("")),"callsign","");
	  ppasswd = vs_config->getVariable("player"+((p>0)?tostring(p+1):string("")),"password","");
	  if ( srvip != "")
	  {
		  // In network mode, test if all player sections are present
		  if( pname=="")
		  {
			  cout<<"Missing or incomlpete section for player "<<p<<endl;
			  cleanexit=true;
			  winsys_exit(1);
		  }
	  }
      playername.push_back( pname);
	  playerpasswd.push_back(ppasswd);
    }

    float credits=XMLSupport::parse_float (mission->getVariable("credits","0"));
    g_game.difficulty=XMLSupport::parse_float (mission->getVariable("difficulty","1"));
	string savegamefile = mission->getVariable ("savegame","");
    vector <SavedUnits> savedun;
    vector <string> playersaveunit;
	vector <StarSystem *> ss;
	vector <string> starsysname;
	vector <QVector> playerNloc;

	/************************* NETWORK INIT ***************************/
	  vector<vector<std::string> > savefiles;
	char * srvipadr;
	int port;
	if ( srvip != "")
	{
		cout<<"Number of local players = "<<numplayers<<endl;
		// Initiate the network if in networking play mode for each local player
		if( srvip != "")
		{
			setNewTime(time(NULL)); // FIXME: Try to read this from server instead.
			string srvport = vs_config->getVariable("network","server_port", "6777");
			// Get the number of local players
			Network = new NetClient[numplayers];

			cout<<endl<<"Server IP : "<<srvip<<" - port : "<<srvport<<endl<<endl;
			srvipadr = new char[srvip.length()+1];
			memcpy( srvipadr, srvip.c_str(), srvip.length());
			srvipadr[srvip.length()] = '\0';
			port = atoi( srvport.c_str());
			cout<<"Port : "<<port<<endl;
		// Here we say we want to only handle activity in 1 starsystem not more
			run_only_player_starsystem=true;
		}
		else
		{
			Network = NULL;
			cout<<"Non-networking mode"<<endl;
		// Here we say we want to only handle activity in 1 starsystem not more
			run_only_player_starsystem=true;
		}
	}

    _Universe->SetupCockpits(playername);
    
	/************************* NETWORK INIT ***************************/
    vector<std::string>::iterator it, jt;
    unsigned int k=0;
    for (k=0, it=playername.begin(), jt=playerpasswd.begin();k<(unsigned int)_Universe->numPlayers();k++, it++, jt++) {
      Cockpit *cp = _Universe->AccessCockpit(k);
      SetStartupView(cp);
      bool setplayerXloc=false;
      std::string psu;
      if (k==0) {
		QVector myVec;
		if (SetPlayerLoc (myVec,false)) {
		  _Universe->AccessCockpit(0)->savegame->SetPlayerLocation(myVec);
		}
		std::string st;
		if (SetPlayerSystem (st,false)) {
		  _Universe->AccessCockpit(0)->savegame->SetStarSystem(st);
		}
      }
	  vector <SavedUnits> saved;
		/************* NETWORK PART ***************/
	  if( Network!=NULL)
	  {
		bool ret = false;
		// Are we using the directly account server to identify us ?
		string use_acctserver = vs_config->getVariable("network","use_account_server", "false");
		if( use_acctserver=="true")
			ret = Network[k].init_acct( srvipadr, (unsigned short) port).valid();
		else
		// Or are we going through a game server to do so ?
			ret = Network[k].init( srvipadr, (unsigned short) port).valid();
		if( ret==false)
		{
			// If network initialization fails, exit
			cout<<"Network initialization error - exiting"<<endl;
			cleanexit=true;
			winsys_exit(1);
		}
		//sleep( 3);
        cout<<"Waiting for player "<<(k)<<" = "<<(*it)<<":"<<(*jt)<<"login response...";
        vector<string> loginResp;
        if( use_acctserver=="true")
            loginResp = Network[k].loginAcctLoop( (*it), (*jt));
        else
            loginResp = Network[k].loginLoop( (*it), (*jt));
        savefiles.push_back( loginResp );

        if( savefiles[k].empty() || savefiles[k][0]=="")
        {
            if( savefiles[k].empty() )
                cout << "Server must have closed connection without warning" << endl;
            else
                cout<<savefiles[k][1]<<endl;
            cout<<"QUITTING"<<endl;
            cleanexit=true;
            winsys_exit(1);
        }
        else
        {
            cout<<" logged in !"<<endl;
        }
	  }
		/************* NETWORK PART ***************/
	  if( Network!=NULL)
	  {
		_Universe->AccessCockpit(k)->savegame->ParseSaveGame ("",mysystem,mysystem,pos,setplayerXloc,credits,_Universe->AccessCockpit(k)->unitfilename,k, savefiles[k][0], false);
		/*
		cout<<"UNIT XML :"<<endl<<savefiles[k][0]<<endl<<endl;
		cout<<"UNIT FILE NAME = "<<_Universe->AccessCockpit(k)->unitfilename[0]<<endl;
		*/
	  }
	  else
		_Universe->AccessCockpit(k)->savegame->ParseSaveGame (savegamefile,mysystem,mysystem,pos,setplayerXloc,credits,_Universe->AccessCockpit()->unitfilename,k);
          CopySavedShips(playername[k],k,_Universe->AccessCockpit()->unitfilename,true);
	  playersaveunit.push_back(_Universe->AccessCockpit(k)->GetUnitFileName());
	  _Universe->AccessCockpit(k)->credits=credits;
		  ss.push_back (_Universe->Init (mysystem,Vector(0,0,0),planetname));
	  if (setplayerXloc) {
	   	  playerNloc.push_back(pos);
	  }else {
		  playerNloc.push_back(QVector(FLT_MAX,FLT_MAX,FLT_MAX));
	  }
	  setplayerloc=setplayerXloc;//FIX ME will only set first player where he was

      for (unsigned int j=0;j<saved.size();j++) {
		savedun.push_back(saved[j]);
      }

    }
    SetStarSystemLoading (true);
    InitializeInput();

    vs_config->bindKeys();//gotta do this before we do ai

    createObjects(playersaveunit,ss,playerNloc, savefiles);

    if (setplayerloc&&fighters) {
      if (fighters[0]) {
	//fighters[0]->SetPosition (Vector (0,0,0));
      }
    }
    
    while (!savedun.empty()) {
      AddUnitToSystem (&savedun.back());
      savedun.pop_back();
    }
    

    forcefeedback=new ForceFeedback();

    UpdateTime();
    delete SplashScreen;
    SplashScreen= NULL;
    SetStarSystemLoading (false);
    FactionUtil::LoadContrabandLists();
	{
		string str=vs_config->getVariable ("general","intro1","Welcome to Vega Strike! Use #8080FFTab#000000 to afterburn (#8080FF+,-#000000 cruise control), #8080FFarrows#000000 to steer.");
		if (!str.empty()) {
			UniverseUtil::IOmessage (0,"game","all",str);
			str=vs_config->getVariable ("general","intro2","The #8080FFt#000000 key targets objects; #8080FFspace#000000 fires at them & #8080FFa#000000 activates the SPEC drive. To");
			if (!str.empty()) {
				UniverseUtil::IOmessage (4,"game","all",str);
				str=vs_config->getVariable ("general","intro3","go to another star system, buy a jump drive for about 10000 credits, fly to a");
				if (!str.empty()) {
					UniverseUtil::IOmessage (8,"game","all",str);
					str = vs_config->getVariable ("general","intro4","wireframe jump-point and press #8080FFj#000000 to warp to a near star. Target a base or planet;");
					if (!str.empty()) {
						UniverseUtil::IOmessage (12,"game","all",str);
						str=vs_config->getVariable ("general","intro5","When you get close a green box will appear. Inside the box, #8080FFd#000000 will land.");
						if (!str.empty())
							UniverseUtil::IOmessage (16,"game","all",str);
					}
				}
			}
		}
	}
	// Always dock on load in networking if it was said so in the save file
	if (/*Network!=NULL ||*/ mission->getVariable("savegame","").length()!=0&&XMLSupport::parse_bool(vs_config->getVariable("AI","dockOnLoad","true"))) {
		for (int i=0;i<_Universe->numPlayers();i++) {
			DockToSavedBases(i);
		}
	}
	cout<<"Loading completed, now network init"<<endl;
	// Send a network msg saying we are ready and also send position info
	if( Network!=NULL) {
		int l;
		for(l=0; l<_Universe->numPlayers(); l++)
		{
			Network[l].downloadZoneInfo();
		}
		for(l=0; l<_Universe->numPlayers(); l++)
		{
			Network[l].inGame();
		}
	}
	{
		for (unsigned int i=0;i<_Universe->numPlayers();++i) {
			WriteSaveGame(_Universe->AccessCockpit(i),false);
		}
	}
	cur_check = getNewTime();
        for (unsigned int i=0;i<_Universe->numPlayers();++i) {
          _Universe->AccessCockpit(i)->savegame->LoadSavedMissions();
        }
        _Universe->Loop(main_loop);
    ///return to idle func which now should call main_loop mohahahah
  }
  ///Draw Texture
  
} 


const char helpmessage[] =
"Command line options for vegastrike\n"
"\n"
" -D -d     Specify data directory\n"
" -N -n     Number of players\n"
" -M -m     Specify a mod to play\n"
" -P -p     Specify player location\n"
" -J -j     Start in a specific system\n"
" -A -a     Normal resolution (800x600)\n"
" -H -h     High resolution (1024x768)\n"
" -V -v     Super high resolution (1280x1024)\n"
" --net     Networking Enabled (Experimental)\n"
"\n";

std::string ParseCommandLine(int argc, char ** lpCmdLine) {
  std::string st;
  std::string retstr;
  std::string datatmp;
  QVector PlayerLocation;
  for (int i=1;i<argc;i++) {
    if(lpCmdLine[i][0]=='-') {
		cerr<<"ARG #"<<i<<" = "<<lpCmdLine[i]<<endl;
      switch(lpCmdLine[i][1]){
    case 'd':
    case 'D': {
      // Specifying data directory
        if(lpCmdLine[i][2] == 0) {
            cout << "Option -D requires an argument" << endl;
            exit(1);
        }
 		datatmp = &lpCmdLine[i][2];
		if( VSFileSystem::DirectoryExists( datatmp))
			VSFileSystem::datadir = datatmp;
		else
		{
			cout<<"Specified data directory not found... exiting"<<endl;
			exit(1);
		}
        cout << "Using data dir specified on command line : " << datatmp << endl;
	  }
	  case 'r':
      case 'R':
	break;
      case 'N':
      case 'n':
		  if (!(lpCmdLine[i][2]=='1'&&lpCmdLine[i][3]=='\0')) {
	  		CONFIGFILE=new char[40+strlen(lpCmdLine[i])+1];
			sprintf(CONFIGFILE,"vegastrike.config.%splayer",lpCmdLine[i]+2);
		  }
	break;
      case 'M':
      case 'm':
		retstr=string(lpCmdLine[i]+2);
	break;
      case 'f':
      case 'F':
	break;
      case 'P':
      case 'p':
	if (3==sscanf (lpCmdLine[i]+2,"%lf,%lf,%lf",&PlayerLocation.i,&PlayerLocation.j,&PlayerLocation.k)) {
          SetPlayerLoc (PlayerLocation,true);
        }
	break;
      case 'J':
      case 'j'://low rez
	st=string ((lpCmdLine[i])+2);
	SetPlayerSystem (st ,true);
	break;
      case 'A'://average rez
      case 'a': 
	g_game.y_resolution = 600;
	g_game.x_resolution = 800;
	break;
      case 'H':
      case 'h'://high rez
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
	//viddrv = "GLDRV.DLL";
	break;
      case '-':
        // long options
        if(strcmp(lpCmdLine[i],"--benchmark")==0){
          //benchmark=30.0;
          benchmark=atof(lpCmdLine[i+1]);
          i++;
        }
        else if(strcmp(lpCmdLine[i], "--net")==0) {
          // don't ignore the network section of the config file
          ignore_network=false;
        }
        else if(strcmp(lpCmdLine[i], "--help")==0) {
          cout << helpmessage;
          exit(0);
        }
      }
    }
    else{
      // no "-" before it - it's the mission name
      strncpy (mission_name,lpCmdLine[i],1023);
	  mission_name[1023]='\0';
    }
  }
  return retstr;
}
