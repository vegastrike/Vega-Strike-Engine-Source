/* 
 * Tux Racer 
 * Copyright (C) 1999-2001 Jasmin F. Patry
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

#include "gl_globals.h"
#include "winsys.h"
#include "vs_globals.h"
#include "xml_support.h"
#include "config_xml.h"
#include "vs_globals.h"
#include <assert.h>
/* Windowing System Abstraction Layer */
/* Abstracts creation of windows, handling of events, etc. */

#if defined( SDL_WINDOWING ) && defined (HAVE_SDL)


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* SDL version */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

static SDL_Surface *screen = NULL;

static winsys_display_func_t display_func = NULL;
static winsys_idle_func_t idle_func = NULL;
static winsys_reshape_func_t reshape_func = NULL;
static winsys_keyboard_func_t keyboard_func = NULL;
static winsys_mouse_func_t mouse_func = NULL;
static winsys_motion_func_t motion_func = NULL;
static winsys_motion_func_t passive_motion_func = NULL;
static winsys_atexit_func_t atexit_func = NULL;

static bool redisplay = false;


/*---------------------------------------------------------------------------*/
/*! 
  Requests that the screen be redrawn
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_post_redisplay() 
{
    redisplay = true;
}


/*---------------------------------------------------------------------------*/
/*! 
  Sets the display callback
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_display_func( winsys_display_func_t func )
{
    display_func = func;
}


/*---------------------------------------------------------------------------*/
/*! 
  Sets the idle callback
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_idle_func( winsys_idle_func_t func )
{
    idle_func = func;
}


/*---------------------------------------------------------------------------*/
/*! 
  Sets the reshape callback
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_reshape_func( winsys_reshape_func_t func )
{
    reshape_func = func;
}


/*---------------------------------------------------------------------------*/
/*! 
  Sets the keyboard callback
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_keyboard_func( winsys_keyboard_func_t func )
{
    keyboard_func = func;
}


/*---------------------------------------------------------------------------*/
/*! 
  Sets the mouse button-press callback
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_mouse_func( winsys_mouse_func_t func )
{
    mouse_func = func;
}


/*---------------------------------------------------------------------------*/
/*! 
  Sets the mouse motion callback (when a mouse button is pressed)
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_motion_func( winsys_motion_func_t func )
{
    motion_func = func;
}


/*---------------------------------------------------------------------------*/
/*! 
  Sets the mouse motion callback (when no mouse button is pressed)
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_passive_motion_func( winsys_motion_func_t func )
{
    passive_motion_func = func;
}



/*---------------------------------------------------------------------------*/
/*! 
  Copies the OpenGL back buffer to the front buffer
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_swap_buffers()
{
    SDL_GL_SwapBuffers();
}


/*---------------------------------------------------------------------------*/
/*! 
  Moves the mouse pointer to (x,y)
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_warp_pointer( int x, int y )
{
    SDL_WarpMouse( x, y );
}


/*---------------------------------------------------------------------------*/
/*! 
  Sets up the SDL OpenGL rendering context
  \author  jfpatry
  \date    Created:  2000-10-20
  \date    Modified: 2000-10-20
*/
static void setup_sdl_video_mode()
{
    Uint32 video_flags = SDL_OPENGL; 
    int bpp = 0;
    int width, height;

    if ( gl_options.fullscreen ) {
	video_flags |= SDL_FULLSCREEN;
    } else {
#ifndef _WIN32
	video_flags |= SDL_RESIZABLE;
#endif
    }

    bpp = gl_options.color_depth;

    width = g_game.x_resolution;
    height =g_game.y_resolution  ;

    if ( ( screen = SDL_SetVideoMode( width, height, bpp, video_flags ) ) == 
	 NULL ) 
    {
	fprintf( stderr, "Couldn't initialize video: %s", 
		 SDL_GetError() );
	exit(1);
    }
}


/*---------------------------------------------------------------------------*/
/*! 
  Initializes the OpenGL rendering context, and creates a window (or 
  sets up fullscreen mode if selected)
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_init( int *argc, char **argv, char *window_title, 
		  char *icon_title )
{
	Uint32 sdl_flags = SDL_INIT_AUDIO|SDL_INIT_VIDEO|SDL_INIT_JOYSTICK;
    g_game.x_resolution = XMLSupport::parse_int (vs_config->getVariable ("graphics","x_resolution","1024"));     
    g_game.y_resolution = XMLSupport::parse_int (vs_config->getVariable ("graphics","y_resolution","768"));     
    gl_options.fullscreen = XMLSupport::parse_bool (vs_config->getVariable ("graphics","fullscreen","false"));
    gl_options.color_depth = XMLSupport::parse_int (vs_config->getVariable ("graphics","colordepth","16"));
    /*
     * Initialize SDL
     */
    if ( SDL_Init( sdl_flags ) < 0 ) {
	fprintf( stderr, "Couldn't initialize SDL: %s", SDL_GetError() );
	exit(1);
    }


    /* 
     * Init video 
     */
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

#if defined( USE_STENCIL_BUFFER )
    /* Not sure if this is sufficient to activate stencil buffer  */
    SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );
#endif

    setup_sdl_video_mode();

    SDL_WM_SetCaption( window_title, icon_title );

}


/*---------------------------------------------------------------------------*/
/*! 
  Deallocates resources in preparation for program termination
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_shutdown()
{
  static bool shutdown=false;
  if (!shutdown) {
    shutdown=true;
    SDL_Quit();
  }
}

/*---------------------------------------------------------------------------*/
/*! 
  Enables/disables key repeat messages from being generated
  \return  
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_enable_key_repeat( bool enabled )
{
    if ( enabled ) {
	SDL_EnableKeyRepeat( SDL_DEFAULT_REPEAT_DELAY,
			     SDL_DEFAULT_REPEAT_INTERVAL );
    } else {
	SDL_EnableKeyRepeat( 0, 0 );
    }
}


/*---------------------------------------------------------------------------*/
/*! 
  Shows/hides mouse cursor
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_show_cursor( bool visible )
{
    SDL_ShowCursor( visible );
}

/*---------------------------------------------------------------------------*/
/*! 
  Processes and dispatches events.  This function never returns.
  \return  No.
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_process_events()
{
    SDL_Event event; 
    unsigned int key;
    int x, y;

    while (true) {

	SDL_LockAudio();
	SDL_UnlockAudio();
	while ( SDL_PollEvent( &event ) ) {
	    
	    switch ( event.type ) {
	    case SDL_KEYDOWN:
		if ( keyboard_func ) {
		    SDL_GetMouseState( &x, &y );
		    key = event.key.keysym.sym; 
		    (*keyboard_func)( key,
				      event.key.keysym.mod,
				      false,
				      x, y );
		}
		break;

	    case SDL_KEYUP:
		if ( keyboard_func ) {
		    SDL_GetMouseState( &x, &y );
		    key = event.key.keysym.sym; 
		    (*keyboard_func)( key,
				      event.key.keysym.mod,
				      true,
				      x, y );
		}
		break;

	    case SDL_MOUSEBUTTONDOWN:
	    case SDL_MOUSEBUTTONUP:
		if ( mouse_func ) {
		    (*mouse_func)( event.button.button,
				   event.button.state,
				   event.button.x,
				   event.button.y );
		}
		break;

	    case SDL_MOUSEMOTION:
		if ( event.motion.state ) {
		    /* buttons are down */
		    if ( motion_func ) {
			(*motion_func)( event.motion.x,
					event.motion.y );
		    }
		} else {
		    /* no buttons are down */
		    if ( passive_motion_func ) {
			(*passive_motion_func)( event.motion.x,
						event.motion.y );
		    }
		}
		break;

	    case SDL_VIDEORESIZE:
#if !(defined(_WIN32)&&defined(SDL_WINDOWING))
		g_game.x_resolution=event.resize.w;
		g_game.y_resolution=event.resize.h;
		setup_sdl_video_mode();
		if ( reshape_func ) {
		    (*reshape_func)( event.resize.w,
				     event.resize.h );
		}
#endif
		break;
	    }

	    SDL_LockAudio();
	    SDL_UnlockAudio();
	}

	if ( redisplay && display_func ) {
	    redisplay = false;
	    (*display_func)();
	} else if ( idle_func ) {
	    (*idle_func)();
	}

	/* Delay for 1 ms.  This allows the other threads to do some
	   work (otherwise the audio thread gets starved). */
	SDL_Delay(1);

    }

    /* Never exits */
#define CODE_NOT_REACHED 0
    assert(CODE_NOT_REACHED);
    //    code_not_reached();
}


/*---------------------------------------------------------------------------*/
/*! 
  Sets the function to be called when program ends.  Note that this
  function should only be called once.
  \author  jfpatry
  \date    Created:  2000-10-20
  \date Modified: 2000-10-20 */
void winsys_atexit( winsys_atexit_func_t func )
{
      static bool called = false;

  if ( called != false)
     fprintf (stderr,"winsys_atexit called twice" );

  called = true;
  // atexit_func = func;
  atexit (func);
}


/*---------------------------------------------------------------------------*/
/*! 
  Exits the program
  \author  jfpatry
  \date    Created:  2000-10-20
  \date    Modified: 2000-10-20
*/
void winsys_exit( int code )
{
  winsys_shutdown();
    if ( atexit_func ) {
	(*atexit_func)();
    }
    
    exit( code );
}

#else

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* GLUT version */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

static winsys_keyboard_func_t keyboard_func = NULL;

static bool redisplay = false;


/*---------------------------------------------------------------------------*/
/*! 
  Requests that the screen be redrawn
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_post_redisplay() 
{
    redisplay = true;
}


/*---------------------------------------------------------------------------*/
/*! 
  Sets the display callback
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_display_func( winsys_display_func_t func )
{
    glutDisplayFunc( func );
}


/*---------------------------------------------------------------------------*/
/*! 
  Sets the idle callback
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_idle_func( winsys_idle_func_t func )
{
    glutIdleFunc( func );
}


/*---------------------------------------------------------------------------*/
/*! 
  Sets the reshape callback
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_reshape_func( winsys_reshape_func_t func )
{
    glutReshapeFunc( func );
}


/* Keyboard callbacks */
static void glut_keyboard_cb( unsigned char ch, int x, int y ) 
{
  
    if ( keyboard_func ) {
	(*keyboard_func)( ch, glutGetModifiers(), false, x, y );
    }
}

static void glut_special_cb( int key, int x, int y ) 
{
    if ( keyboard_func ) {
	(*keyboard_func)( key+128, glutGetModifiers(), false, x, y );
    }
}

static void glut_keyboard_up_cb( unsigned char ch, int x, int y ) 
{
    if ( keyboard_func ) {
	(*keyboard_func)( ch, glutGetModifiers(), true, x, y );
    }
}

static void glut_special_up_cb( int key, int x, int y ) 
{
    if ( keyboard_func ) {
	(*keyboard_func)( key+128, glutGetModifiers(), true, x, y );
    }
}


/*---------------------------------------------------------------------------*/
/*! 
  Sets the keyboard callback
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_keyboard_func( winsys_keyboard_func_t func )
{
    keyboard_func = func;
}


/*---------------------------------------------------------------------------*/
/*! 
  Sets the mouse button-press callback
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_mouse_func( winsys_mouse_func_t func )
{
    glutMouseFunc( func );
}


/*---------------------------------------------------------------------------*/
/*! 
  Sets the mouse motion callback (when a mouse button is pressed)
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_motion_func( winsys_motion_func_t func )
{
    glutMotionFunc( func );
}


/*---------------------------------------------------------------------------*/
/*! 
  Sets the mouse motion callback (when no mouse button is pressed)
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_set_passive_motion_func( winsys_motion_func_t func )
{
    glutPassiveMotionFunc( func );
}



/*---------------------------------------------------------------------------*/
/*! 
  Copies the OpenGL back buffer to the front buffer
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_swap_buffers()
{
    glutSwapBuffers();
}


/*---------------------------------------------------------------------------*/
/*! 
  Moves the mouse pointer to (x,y)
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_warp_pointer( int x, int y )
{
    glutWarpPointer( x, y );
}


/*---------------------------------------------------------------------------*/
/*! 
  Initializes the OpenGL rendering context, and creates a window (or 
  sets up fullscreen mode if selected)
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_init( int *argc, char **argv, char *window_title, 
		  char *icon_title )
{
    int width, height;
    int glutWindow;
    g_game.x_resolution = XMLSupport::parse_int (vs_config->getVariable ("graphics","x_resolution","1024"));     
    g_game.y_resolution = XMLSupport::parse_int (vs_config->getVariable ("graphics","y_resolution","768"));     
    gl_options.fullscreen = XMLSupport::parse_bool (vs_config->getVariable ("graphics","fullscreen","false"));
    gl_options.color_depth = XMLSupport::parse_int (vs_config->getVariable ("graphics","colordepth","16"));
    glutInit( argc, argv );

#ifdef USE_STENCIL_BUFFER
    glutInitDisplayMode( GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE | GLUT_STENCIL );
#else
    glutInitDisplayMode( GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE );
#endif 
    char str [1024];
    sprintf (str, "%dx%d:%d",g_game.x_resolution,g_game.y_resolution,gl_options.color_depth); 
    glutGameModeString(str);

    /* Create a window */
    if ( gl_options.fullscreen &&(glutGameModeGet(GLUT_GAME_MODE_POSSIBLE)!=-1)) {
	glutInitWindowPosition( 0, 0 );
	glutEnterGameMode();
    } else {
	/* Set the initial window size */
	glutInitWindowSize( g_game.x_resolution,g_game.y_resolution );

	glutWindow = glutCreateWindow(window_title);

	if ( glutWindow == 0 ) {
	    (void) fprintf(stderr, "Couldn't create a window.\n");
	    exit(1);
	}
    }
}


/*---------------------------------------------------------------------------*/
/*! 
  Deallocates resources in preparation for program termination
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_shutdown()
{
  static bool shutdown=false;
  if (!shutdown) {
    shutdown=true;
    if ( gl_options.fullscreen ) {
	glutLeaveGameMode();
    }
  }
}

/*---------------------------------------------------------------------------*/
/*! 
  Enables/disables key repeat messages from being generated
  \return  
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_enable_key_repeat( bool enabled )
{
    glutIgnoreKeyRepeat(!enabled);
}

/*---------------------------------------------------------------------------*/
/*! 
  Shows/hides mouse cursor
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_show_cursor( bool visible )
{
    if ( visible ) {
	glutSetCursor( GLUT_CURSOR_LEFT_ARROW );
    } else {
	glutSetCursor( GLUT_CURSOR_NONE );
    }
}



/*---------------------------------------------------------------------------*/
/*! 
  Processes and dispatches events.  This function never returns.
  \return  No.
  \author  jfpatry
  \date    Created:  2000-10-19
  \date    Modified: 2000-10-19
*/
void winsys_process_events()
{
    /* Set up keyboard callbacks */
    glutKeyboardFunc( glut_keyboard_cb );
    glutKeyboardUpFunc( glut_keyboard_up_cb );
    glutSpecialFunc( glut_special_cb );
    glutSpecialUpFunc( glut_special_up_cb );

    glutMainLoop();
}

/*---------------------------------------------------------------------------*/
/*! 
  Sets the function to be called when program ends.  Note that this
  function should only be called once.
  \author  jfpatry
  \date    Created:  2000-10-20
  \date Modified: 2000-10-20 */
void winsys_atexit( winsys_atexit_func_t func )
{
    static bool called = false;

    if (called)
      fprintf (stderr,"winsys_atexit called twice\n" );

    called = true;

    atexit(func);
}


/*---------------------------------------------------------------------------*/
/*! 
  Exits the program
  \author  jfpatry
  \date    Created:  2000-10-20
  \date    Modified: 2000-10-20
*/
void winsys_exit( int code )
{
  winsys_shutdown();
    exit(code);
}

#endif /* defined( SDL_WINDOWING ) */

/* EOF */
