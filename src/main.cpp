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


#include "vegastrike.h"
#include "fonts.h"
#include "gfxlib.h"
#include "render_util.h"
#include "in_kb.h"
#include "fog.h"
#include "sdl_audio_data.h"
#include "sdl_audio.h"
#include "sdl_init.h"
#include "joystick.h"
#include "lin_time.h"
#include "main_loop.h"

/*
 * Globals 
 */

game_data_t g_game;
WrapGFX *_GFX;
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
  g_game.capture_mouse=False;
  g_game.fullscreen = 0;
  g_game.color_depth = 16;
  g_game.y_resolution = 480;
  g_game.x_resolution = 640;
  g_game.fov=60;
  g_game.PaletteExt=1;
  
}
void ParseCommandLine(int argc, char ** CmdLine);
void cleanup(void)
{
  //    write_config_file();
  //  write_saved_games();
  destroyObjects();
  delete _GFX;
}
int main( int argc, char **argv ) 
{

    /* Print copyright notice */
    fprintf( stderr, "Vega Strike " VERSION " -- Final Conflict\n"
	     "See http://www.gnu.org/copyleft/gpl.html for license details.\n\n" );
    /* Seed the random number generator */
    srand( time(NULL) );
    setup_game_data(); 
    //read_config_file();
    //init_debug("");

#if defined(HAVE_SDL) && defined(HAVE_SDL_MIXER)
    sdl_init();
#endif
    InitTime();
    ParseCommandLine(argc,argv);


    /* Set up a function to clean up when program exits */
    if ( atexit( cleanup ) != 0 ) {
	perror( "atexit" );
    }
    //init_textures();
    //init_fonts();
#if defined(HAVE_SDL) && defined(HAVE_SDL_MIXER)

    init_audio_data();
    init_audio();
    init_joystick();

#endif

    _GFX= new WrapGFX(argc,argv);    
    //    glutSetCursor(GLUT_CURSOR_NONE); 
    InitInput();
    createObjects();

    InitializeInput();
    _GFX->Loop(main_loop);

    // never makes it here
    return 0;
} 










void ParseCommandLine(int argc, char ** lpCmdLine) {
 
  for (int i=1;i<argc;i++) {
    if(lpCmdLine[i][0]=='-') {
      switch(lpCmdLine[i][1]){
      case 'M':
      case 'm':
	g_game.music_enabled=1;
	break;
      case 'S':
      case 's':
	g_game.sound_enabled=1;
	break;
      case '1':
	g_game.color_depth = 16;
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
	//viddrv = "D3DDRV.DLL";
	break;
      case 'G':
      case 'g':
	//viddrv = "GLDRV.DLL";
	break;
      }
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
