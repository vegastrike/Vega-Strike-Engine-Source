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
#include "gl_util.h"
#include "gfxlib.h"
#include "render_util.h"
//#include "keyboard.h"
#include "in_kb.h"
#include "fog.h"
#include "audio_data.h"
#include "audio.h"
#include "joystick.h"




/*
 * Globals 
 */

game_data_t g_game;


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
  g_game.mode = RACING;
  g_game.prev_mode=RACING;
  g_game.fov=60;
  g_game.PaletteExt=1;
}

/* This function is called on exit */
void cleanup(void)
{
  //    write_config_file();
  GFXShutdown();
}


#if defined (HAVE_SDL) && defined (HAVE_SDL_MIXER)

void setup_sdl() 
{
    int hz, channels, buffer;
    Uint16 format;
    Uint32 flags;

    flags = 0;

    if ( g_game.sound_enabled ) {
	flags |= SDL_INIT_AUDIO;
    }
 
#ifdef HAVE_SDL_JOYSTICKOPEN
    flags |= SDL_INIT_JOYSTICK;
#endif

    /*
     * Initialize SDL
     */
    if ( SDL_Init(flags) < 0 ) {
	handle_error(1, "Couldn't initialize SDL: %s\n",SDL_GetError());
    }

    if ( g_game.sound_enabled) {
	/* Open the audio device */
	switch (g_game.audio_frequency_mode%3) {
	case 0:
	    hz = 11025;
	    break;
	case 1:
	    hz = 22050;
	    break;
	case 2:
	    hz = 44100;
	    break;
	default:
	    hz = 22050;
	}

	switch ( (g_game.audio_frequency_mode/3)%2 ) {
	case 0:
	    format = AUDIO_U8;
	    break;
	case 1:
	    format = AUDIO_S16SYS;
	    break;
	default:
	    format = AUDIO_S16SYS;
	}

	if ( g_game.audio_frequency_mode/6 ) {
	    channels = 1;
	} else {
	    channels = 2;
	}

	buffer = 2048;//getparam_audio_buffer_size();

	if ( Mix_OpenAudio(hz, format, channels, buffer) < 0 ) {
	    print_warning( 1,
			   "Warning: Couldn't set %d Hz %d-bit audio\n"
			   "  Reason: %s\n", 
			   hz,  
			   (g_game.audio_frequency_mode/3)%2 == 0 ? 8 : 16,
			   SDL_GetError());
	} else {
	    print_debug( DEBUG_SOUND,
			 "Opened audio device at %d Hz %d-bit audio",
			 hz, 
			 (g_game.audio_frequency_mode/3)%2 == 0 ? 8 : 16);
			 
	}
    }

    atexit(SDL_Quit);
}

#endif /* defined (HAVE_SDL) && defined (HAVE_SDL_MIXER) */


void displayCB(void)            /* function called whenever redisplay needed */
{
  glClear(GL_COLOR_BUFFER_BIT);         /* clear the display */
  glColor3f(1.0, 1.0, 1.0);             /* set current color to white */
  //  glBegin(GL_POLYGON);                  /* draw filled triangle */
  //glVertex3f(.30,.50,.5);                  /* specify each vertex of triangle */
  //glVertex3f(.70,.50,.5);
  //glVertex3f(.50,.70,.5);
  //glEnd();                              /* OpenGL draws the filled triangle */
  glFlush();                            /* Complete any pending operations */
  glutSwapBuffers();
}



int main( int argc, char **argv ) 
{

    /* Print copyright notice */
    fprintf( stderr, "Vega Strike " VERSION " -- Final Conflict "
	     "See http://www.gnu.org/copyleft/gpl.html for license details.\n\n" );
    
    /* Init the game clock */
    g_game.secs_since_start = 0;

    /* Seed the random number generator */
    srand( time(NULL) );


    /*
     * Set up the game configuration
     */


    /* Setup the configuration variables and read the ~/.tuxracer/options file */
     //init_game_configuration();
    setup_game_data(); 
    //read_config_file();


    /* Set up the debugging modes */
    init_debug("");

    /* Let GLUT process its command-line options */
    GFXInit(argc,argv);


    /*
     * Initialize SDL
     */

#if defined(HAVE_SDL) && defined(HAVE_SDL_MIXER)
    setup_sdl();
#endif

    /* Set up a function to clean up when program exits */
    if ( atexit( cleanup ) != 0 ) {
	perror( "atexit" );
    }

    /* 
     * Initial OpenGL settings 
     */
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );



    //init_textures();

    //FIXME VEGASTRIKE
//init_fonts();


    init_audio_data();
    init_audio();
    init_joystick();

    //g_game.player[local_player()].view.pos = make_point( 0., 0., 0. );

    /* Placeholder name until we give players way to enter name */
    //g_game.player[0].name = "noname";

    //init_preview();

    //splash_screen_register();
    //intro_register();
    //racing_register();
    //game_over_register();
    //paused_register();
    //reset_register();
    //game_type_select_register();
    //event_select_register();
    //race_select_register();
    //credits_register();

    g_game.mode = NO_MODE;
    //set_game_mode( SPLASH );

    InitKB();
    

    glutSetCursor( GLUT_CURSOR_NONE );

    /* We use this to "prime" the GLUT loop */
    //    glutIdleFunc( main_loop );
    //    glutDisplayFunc (main_loop);
    glClearColor(0.0,0.0,0.0,0.0);        /* set background to black */
    glutDisplayFunc(displayCB);           /* set window's display callback */
    //glutKeyboardFunc(keyCB);              /* set window's key callback */

 
   
    /* 
     * ...and off we go!
     */
    glutMainLoop();

    return 0;
} 

