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

#if defined(HAVE_SDL)
#include <SDL/SDL.h>
#endif
#include "vs_globals.h"
#include "gfxlib.h"
#include "in_kb.h"
#include "lin_time.h"
#include "main_loop.h"
#include "config_xml.h"
#include "mission.h"
#include "audiolib.h"
#include "vs_path.h"



#include "python/init.h"
/*
 * Globals 
 */
game_data_t g_game;

Universe *_Universe;
FILE * fpread=NULL;
/* 
 * Function definitions
 */

void setup_game_data ( ){ //pass in config file l8r??
  g_game.audio_frequency_mode=4;//22050/16
  g_game.sound_enabled =1;
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
void ParseCommandLine(int argc, char ** CmdLine);
void cleanup(void)
{
  fprintf( stderr, "\n"
	     "Graceful exit\n" );
  printf ("Thank you for playing!\n");

  //    write_config_file();
  //  write_saved_games();
  AUDDestroy();
  destroyObjects();
  delete _Universe;
}

VegaConfig *vs_config;
Mission *mission;
double benchmark=-1.0;

char mission_name[1024];

int main( int argc, char *argv[] ) 
{

  
    /* Print copyright notice */
  fprintf( stderr, "Vega Strike "  " \n"
	     "See http://www.gnu.org/copyleft/gpl.html for license details.\n\n" );
    /* Seed the random number generator */

    strcpy(mission_name,"test1.mission");

    ParseCommandLine(argc,argv);

    if(benchmark<0.0){
      srand( time(NULL) );
    }
    else{
      // in benchmark mode, always use the same seed
      srand(171070);
    }

    setup_game_data(); 

    mission=new Mission(mission_name);


    // loads the configuration file .vegastrikerc from home dir if such exists
    initpaths();

#ifdef HAVE_PYTHON

	Python::init();

#endif

    float col[4];
    vs_config->gethColor("default","testcolor",col,0xff00ff00);
    printf("hexcol: %f %f %f %f\n",col[0],col[1],col[2],col[3]);

    string olds=vs_config->getVariable("general","testvar","def-value");
    vs_config->setVariable("general","testvar","foobar");
    string news=vs_config->getVariable("general","testvar","new-def-value");

    cout << "old " << olds << " new " << news << endl;


    string varstr=vs_config->getVariable("graphics","mesh","testvar","1024");
    cout << "testvar: " << varstr << endl;
    //            exit(0);

    //read_vs_config_file();
    //init_debug("");

#if defined(HAVE_SDL)
    // && defined(HAVE_SDL_MIXER)
  if (  SDL_Init( SDL_INIT_JOYSTICK )) {
        fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }

#endif
#if 0
    InitTime();
    UpdateTime();
#endif

    AUDInit();

    /* Set up a function to clean up when program exits */
    if ( atexit( cleanup ) != 0 ) {
	perror( "atexit" );
    }
    /*
#if defined(HAVE_SDL) && defined(HAVE_SDL_MIXER)

    init_audio_data();
    init_audio();
    init_joystick();

#endif
    */
    _Universe= new Universe(argc,argv);   

    Vector pos;
    string planetname;
  
    mission->GetOrigin(pos,planetname);

    _Universe->Init (mission->getVariable("system","sol.system"),pos,planetname);
    createObjects();

       InitializeInput();

       vs_config->bindKeys();

       InitTime();
       UpdateTime();

    _Universe->Loop(main_loop);

    // never makes it here
    return 0;
} 






void ParseCommandLine(int argc, char ** lpCmdLine) {
  for (int i=1;i<argc;i++) {
    if(lpCmdLine[i][0]=='-') {
      switch(lpCmdLine[i][1]){
      case 'r':
      case 'R':
	break;
      case 'M':
      case 'm':
	g_game.music_enabled=1;
	break;
      case 'S':
      case 's':
	g_game.sound_enabled=1;
	break;
      case '1':
	break;
      case '3':
	break; 
      case 'f':
      case 'F':
	break;
      case 'L':
      case 'l'://low rez
	g_game.y_resolution = 480;
	g_game.x_resolution = 640;
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
      case 'D':
      case 'd':
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
      }
    }
    else{
      // no "-" before it - it's the mission name
      strncpy (mission_name,lpCmdLine[i],1023);
	  mission_name[1023]='\0';
    }
  }
  //FILE *fp = fopen("vid.cfg", "rb");
  //  GUID temp;
  //fread(&temp, sizeof(GUID), 1, fp);
  //fread(&temp, sizeof(GUID), 1, fp);
  //fread(&_ViewPortWidth, sizeof(DWORD), 1, fp);
  //fread(&_ViewPortHeight, sizeof(DWORD), 1, fp);
  //fread(&_ColDepth,sizeof(DWORD),1,fp);
  //fclose(fp);
}
