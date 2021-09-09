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
 *
 * Incorporated into Vega Strike
 *
 * Copyright (C) Daniel Horn
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike contributors
 * Copyright (C) 2021 Stephen G. Tuggy
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */


#include <assert.h>
#include <sstream>

#include "gl_globals.h"
#include "winsys.h"
#include "vs_globals.h"
#include "xml_support.h"
#include "config_xml.h"
#include "vs_globals.h"
#include "vs_logging.h"
#include "options.h"



/*
 * Windowing System Abstraction Layer
 * Abstracts creation of windows, handling of events, etc.
 */

#if defined (SDL_WINDOWING) && defined (HAVE_SDL)

/*
 * *---------------------------------------------------------------------------
 * *---------------------------------------------------------------------------
 * SDL version
 *******************************---------------------------------------------------------------------------
 *******************************---------------------------------------------------------------------------
 */

static SDL_Surface *screen = NULL;

static winsys_display_func_t  display_func  = NULL;
static winsys_idle_func_t     idle_func     = NULL;
static winsys_reshape_func_t  reshape_func  = NULL;
static winsys_keyboard_func_t keyboard_func = NULL;
static winsys_mouse_func_t    mouse_func    = NULL;
static winsys_motion_func_t   motion_func   = NULL;
static winsys_motion_func_t   passive_motion_func = NULL;
static winsys_atexit_func_t   atexit_func   = NULL;

static bool redisplay = false;
static bool keepRunning = true;

/*---------------------------------------------------------------------------*/
/*!
 *  Requests that the screen be redrawn
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_post_redisplay()
{
    redisplay = true;
}

/*---------------------------------------------------------------------------*/
/*!
 *  Sets the display callback
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_set_display_func( winsys_display_func_t func )
{
    display_func = func;
}

/*---------------------------------------------------------------------------*/
/*!
 *  Sets the idle callback
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_set_idle_func( winsys_idle_func_t func )
{
    idle_func = func;
}

/*---------------------------------------------------------------------------*/
/*!
 *  Sets the reshape callback
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_set_reshape_func( winsys_reshape_func_t func )
{
    reshape_func = func;
}

/*---------------------------------------------------------------------------*/
/*!
 *  Sets the keyboard callback
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_set_keyboard_func( winsys_keyboard_func_t func )
{
    keyboard_func = func;
}

/*---------------------------------------------------------------------------*/
/*!
 *  Sets the mouse button-press callback
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_set_mouse_func( winsys_mouse_func_t func )
{
    mouse_func = func;
}

/*---------------------------------------------------------------------------*/
/*!
 *  Sets the mouse motion callback (when a mouse button is pressed)
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_set_motion_func( winsys_motion_func_t func )
{
    motion_func = func;
}

/*---------------------------------------------------------------------------*/
/*!
 *  Sets the mouse motion callback (when no mouse button is pressed)
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_set_passive_motion_func( winsys_motion_func_t func )
{
    passive_motion_func = func;
}

/*---------------------------------------------------------------------------*/
/*!
 *  Copies the OpenGL back buffer to the front buffer
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_swap_buffers()
{
    SDL_GL_SwapBuffers();
}

/*---------------------------------------------------------------------------*/
/*!
 *  Moves the mouse pointer to (x,y)
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_warp_pointer( int x, int y )
{
    SDL_WarpMouse( x, y );
}

/*---------------------------------------------------------------------------*/
/*!
 *  Sets up the SDL OpenGL rendering context
 *  \author  jfpatry
 *  \date    Created:  2000-10-20
 *  \date    Modified: 2021-09-07 - stephengtuggy
 */
static bool setup_sdl_video_mode()
{
    Uint32 video_flags = SDL_OPENGL;
    int    bpp = 0;
    int    width, height;
    if (gl_options.fullscreen) {
        video_flags |= SDL_FULLSCREEN;
    } else {
#ifndef _WIN32
        video_flags |= SDL_RESIZABLE;
#endif
    }
    bpp = gl_options.color_depth;

    int    rs, gs, bs;
    rs  = gs = bs = (bpp == 16) ? 5 : 8;
    if(game_options.rgb_pixel_format.compare("undefined") == 0){
	game_options.rgb_pixel_format = ((bpp == 16) ? "555" : "888");
    }
    if ( (game_options.rgb_pixel_format.length() == 3) && isdigit( game_options.rgb_pixel_format[0] ) && isdigit( game_options.rgb_pixel_format[1] ) && isdigit( game_options.rgb_pixel_format[2] ) ) {
        rs = game_options.rgb_pixel_format[0]-'0';
        gs = game_options.rgb_pixel_format[1]-'0';
        bs = game_options.rgb_pixel_format[2]-'0';
    }
    int otherbpp;
    int otherattributes;
    if (bpp == 16) {
        otherattributes = 8;
        otherbpp = 32;
        SDL_GL_SetAttribute( SDL_GL_RED_SIZE, rs );
        SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, gs );
        SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, bs );
        SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, game_options.z_pixel_format );
        SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    } else {
        otherattributes = 5;
        otherbpp = 16;
        SDL_GL_SetAttribute( SDL_GL_RED_SIZE, rs );
        SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, gs );
        SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, bs );
        SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, game_options.z_pixel_format );
        SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    }
#if SDL_VERSION_ATLEAST( 1, 2, 10 )
    if (game_options.gl_accelerated_visual) {
        SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1 );
    }
#endif
    width  = g_game.x_resolution;
    height = g_game.y_resolution;
    if ( ( screen = SDL_SetVideoMode( width, height, bpp, video_flags ) ) == NULL ) {
        VS_LOG(info, (boost::format("Couldn't initialize video: %1%") % SDL_GetError()));
        for (int counter = 0; screen == NULL && counter < 2; ++counter) {
            for (int bpd = 4; bpd > 1; --bpd) {
                SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, bpd*8 );
                if ( ( screen = SDL_SetVideoMode( width, height, bpp, video_flags|SDL_ANYFORMAT ) )
                        == NULL ) {
                    VS_LOG_AND_FLUSH(error,
                            (boost::format("Couldn't initialize video bpp %1% depth %2%: %3%")
                                    % bpp
                                    % (bpd * 8)
                                    % SDL_GetError()));
                } else {
                    break;
                }
            }
            if (screen == NULL) {
                SDL_GL_SetAttribute( SDL_GL_RED_SIZE, otherattributes );
                SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, otherattributes );
                SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, otherattributes );
                gl_options.color_depth = bpp = otherbpp;
            }
        }
        if (screen == NULL) {
            VS_LOG_AND_FLUSH(fatal, "FAILED to initialize video");
            VSExit( 1 );
        }
    }

    std::string version = (const char*)glGetString(GL_RENDERER);
    if (version == "GDI Generic") {
        if (game_options.gl_accelerated_visual) {
            VS_LOG(error, "GDI Generic software driver reported, trying to reset.");
            SDL_Quit();
            game_options.gl_accelerated_visual = false;
            return false;
        } else {
            VS_LOG(error, "GDI Generic software driver reported, reset failed.\n");
            VS_LOG(error, "Please make sure a graphics card driver is installed and functioning properly.\n");
        }
    }

    VS_LOG(trace,
            (boost::format("Setting Screen to w %1% h %2% and pitch of %3% and %4% bpp %5% bytes per pix mode")
                    % screen->w
                    % screen->h
                    % screen->pitch
                    % screen->format->BitsPerPixel
                    % screen->format->BytesPerPixel));

    return true;
}

/*---------------------------------------------------------------------------*/
/*!
 *  Initializes the OpenGL rendering context, and creates a window (or
 *  sets up fullscreen mode if selected)
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2020-07-27 stephengtuggy
 */

void winsys_init( int *argc, char **argv, char const *window_title, char const *icon_title )
{
    keepRunning = true;

    //SDL_INIT_AUDIO|
    Uint32 sdl_flags = SDL_INIT_VIDEO|SDL_INIT_JOYSTICK;
    g_game.x_resolution    = game_options.x_resolution;
    g_game.y_resolution    = game_options.y_resolution;
    gl_options.fullscreen  = game_options.fullscreen;
    gl_options.color_depth = game_options.colordepth;
    /*
     * Initialize SDL
     */
    if (SDL_Init( sdl_flags ) < 0) {
        VS_LOG_AND_FLUSH(fatal, (boost::format("Couldn't initialize SDL: %1%") % SDL_GetError()));
        exit( 1 );              // stephengtuggy 2020-07-27 - I would use VSExit here, but that calls winsys_exit, which I'm not sure will work if winsys_init hasn't finished yet.
    }
    SDL_EnableUNICODE( 1 );     //supposedly fixes int'l keyboards.

    //signal( SIGSEGV, SIG_DFL );
    SDL_Surface *icon = NULL;
#if 1
    if (icon_title) icon = SDL_LoadBMP( icon_title );
    if (icon) {
        SDL_SetColorKey( icon, SDL_SRCCOLORKEY, ( (Uint32*) (icon->pixels) )[0] );
    }
#endif
    /*
     * Init video
     */
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

#if defined (USE_STENCIL_BUFFER)
    /* Not sure if this is sufficient to activate stencil buffer  */
    SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );
#endif

    SDL_WM_SetCaption( window_title, window_title );
    if (icon) SDL_WM_SetIcon( icon, 0 );

    if (!setup_sdl_video_mode()) {
        winsys_init(argc, argv, window_title, icon_title);
    } else {
        glutInit( argc, argv );
    }
}

/*---------------------------------------------------------------------------*/
/*!
 *  Deallocates resources in preparation for program termination
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_cleanup()
{
    static bool cleanup = false;
    if (!cleanup) {
        cleanup = true;
        SDL_Quit();
    }
}

void winsys_shutdown()
{
    keepRunning = false;
}

/*---------------------------------------------------------------------------*/
/*!
 *  Enables/disables key repeat messages from being generated
 *  \return
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_enable_key_repeat( bool enabled )
{
    if (enabled) {
        SDL_EnableKeyRepeat( SDL_DEFAULT_REPEAT_DELAY,
                             SDL_DEFAULT_REPEAT_INTERVAL );
    } else {
        SDL_EnableKeyRepeat( 0, 0 );
    }
}

/*---------------------------------------------------------------------------*/
/*!
 *  Shows/hides mouse cursor
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_show_cursor( bool visible )
{
    static bool vis = true;
    if (visible != vis) {
        SDL_ShowCursor( visible );
        vis = visible;
    }
}

/*---------------------------------------------------------------------------*/
/*!
 *  Processes and dispatches events.  This function never returns.
 *  \return  No.
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 *  \date    Modified: 2005-08-16 - Rogue
 *  \date    Modified: 2005-12-24 - ace123
 *  \date    Modified: 2021-09-07 - stephengtuggy
 */
extern int shiftdown( int );
extern int shiftup( int );

void winsys_process_events()
{
    SDL_Event    event;
    unsigned int key;
    int  x, y;
    bool state;

    static unsigned int keysym_to_unicode[256];
    static bool keysym_to_unicode_init = false;
    if (!keysym_to_unicode_init) {
        keysym_to_unicode_init = true;
        memset( keysym_to_unicode, 0, sizeof (keysym_to_unicode) );
    }
    while (keepRunning)
    {
        SDL_LockAudio();
        SDL_UnlockAudio();
        while ( SDL_PollEvent( &event ) ) {
            state = false;
            switch (event.type)
            {
            case SDL_KEYUP:
                state = true;
            //does same thing as KEYDOWN, but with different state.
            case SDL_KEYDOWN:
                if (keyboard_func) {
                    SDL_GetMouseState( &x, &y );

                    bool maybe_unicode = game_options.enable_unicode && !(event.key.keysym.sym&~0xFF);
                    bool is_unicode = maybe_unicode && event.key.keysym.unicode;

                    //Fix up ctrl unicode codes
                    if (is_unicode && event.key.keysym.unicode <= 0x1a && (event.key.keysym.sym&0xFF) > 0x1a && event.key.keysym.mod & (KMOD_LCTRL|KMOD_RCTRL)) {
                        event.key.keysym.unicode += 0x60; // 0x01 (^A) --> 0x61 (A)
                    }

                    //Translate untranslated release events
                    if (state && maybe_unicode
                        && keysym_to_unicode[event.key.keysym.sym&0xFF]) {
                        event.key.keysym.unicode = keysym_to_unicode[event.key.keysym.sym&0xFF];
                    }

                    //Remember translation for translating release events
                    if (is_unicode) {
                        keysym_to_unicode[event.key.keysym.sym&0xFF] = event.key.keysym.unicode;
                    }

                    //Ugly hack: prevent shiftup/shiftdown screwups on intl keyboard
                    //Note: Thank god we'll have OIS for 0.5.x
                    bool shifton = event.key.keysym.mod&(KMOD_LSHIFT|KMOD_RSHIFT|KMOD_CAPS);

                    VS_LOG(debug, (boost::format("Kbd: %1$s mod:%2$x sym:%3$x unicode:%4$x sh:%5$c u:%6$c mu:%7$c")
                            % ((event.type == SDL_KEYUP) ? "KEYUP" : "KEYDOWN")
                            % event.key.keysym.mod
                            % event.key.keysym.sym
                            % event.key.keysym.unicode
                            % ((shifton) ? 't' : 'f')
                            % ((is_unicode) ? 't' : 'f')
                            % ((maybe_unicode) ? 't' : 'f')));

                    if (shifton && is_unicode
                        && shiftup( shiftdown( event.key.keysym.unicode ) ) != event.key.keysym.unicode) {
                        event.key.keysym.mod = SDLMod( event.key.keysym.mod&~(KMOD_LSHIFT|KMOD_RSHIFT|KMOD_CAPS) );
                        shifton = false;
                    }
                    //Choose unicode or symbolic, depending on whether ther is or not a unicode code
                    //(unicode codes must be postprocessed to make sure application of the shiftup
                    //modifier does not destroy it)
                    key = is_unicode
                          ? ( (shifton)
                             ? shiftdown( event.key.keysym.unicode )
                             : event.key.keysym.unicode
                              ) : event.key.keysym.sym;
                    //Send the event
                    (*keyboard_func)(key,
                                     event.key.keysym.mod,
                                     state,
                                     x, y);
                }
                break;

            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                if (mouse_func) {
                    (*mouse_func)(event.button.button,
                                  event.button.state,
                                  event.button.x,
                                  event.button.y);
                }
                break;

            case SDL_MOUSEMOTION:
                if (event.motion.state) {
                    /* buttons are down */
                    if (motion_func) {
                        (*motion_func)(event.motion.x,
                                       event.motion.y);
                    }
                } else
                /* no buttons are down */
                if (passive_motion_func) {
                    (*passive_motion_func)(event.motion.x,
                                           event.motion.y);
                }
                break;

            case SDL_VIDEORESIZE:
#if !(defined (_WIN32) && defined (SDL_WINDOWING ) )
                g_game.x_resolution = event.resize.w;
                g_game.y_resolution = event.resize.h;
                setup_sdl_video_mode();
                if (reshape_func) {
                    (*reshape_func)(event.resize.w,
                                    event.resize.h);
                }
#endif
                break;
            }
            SDL_LockAudio();
            SDL_UnlockAudio();
        }
        if (redisplay && display_func) {
            redisplay = false;
            (*display_func)();
        } else if (idle_func) {
            (*idle_func)();
            /* Delay for 1 ms.  This allows the other threads to do some
             *  work (otherwise the audio thread gets starved). */
        }
        SDL_Delay( 1 );
    }
    winsys_cleanup();
}

/*---------------------------------------------------------------------------*/
/*!
 *  Sets the function to be called when program ends.  Note that this
 *  function should only be called once.
 *  \author  jfpatry
 *  \date    Created:  2000-10-20
 *  \date    Modified: 2021-09-06 - stephengtuggy
 */
void winsys_atexit( winsys_atexit_func_t func )
{
    static bool called = false;
    if (called != false) {
        VS_LOG_AND_FLUSH(error, "winsys_atexit called twice");
    }
    called = true;
}

/*---------------------------------------------------------------------------*/
/*!
 *  Exits the program
 *  \author  jfpatry
 *  \date    Created:  2000-10-20
 *  \date    Modified: 2000-10-20
 */
void winsys_exit( int code )
{
    winsys_shutdown();
    if (atexit_func) {
        (*atexit_func)();
    }
    // exit( code );
}

#else

/*
 * *---------------------------------------------------------------------------
 * *---------------------------------------------------------------------------
 * GLUT version
 *******************************---------------------------------------------------------------------------
 *******************************---------------------------------------------------------------------------
 */

static winsys_keyboard_func_t keyboard_func = NULL;

static bool redisplay = false;

/*---------------------------------------------------------------------------*/
/*!
 *  Requests that the screen be redrawn
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_post_redisplay()
{
    redisplay = true;
}

/*---------------------------------------------------------------------------*/
/*!
 *  Sets the display callback
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_set_display_func( winsys_display_func_t func )
{
    glutDisplayFunc( func );
}

/*---------------------------------------------------------------------------*/
/*!
 *  Sets the idle callback
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_set_idle_func( winsys_idle_func_t func )
{
    glutIdleFunc( func );
}

/*---------------------------------------------------------------------------*/
/*!
 *  Sets the reshape callback
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_set_reshape_func( winsys_reshape_func_t func )
{
    glutReshapeFunc( func );
}

char AdjustKeyCtrl( char ch )
{
    if (ch == '\0') {
        ch = '2';
    } else if (ch >= '0' && ch <= '9') {} else if (ch >= 27 && ch <= 31) {
        ch = ch+'0'-24;
    } else if (ch == 127) {
        ch = '8';
    } else if (ch <= 26) {
        ch += 'a'-1;
    }
    return ch;
}

/* Keyboard callbacks */
static void glut_keyboard_cb( unsigned char ch, int x, int y )
{
    if (keyboard_func) {
        int gm = glutGetModifiers();
        if (gm) {
            VS_LOG(trace, (boost::format("Down Modifier %d for char %d %c") % gm % (int)ch % ch));
        }
        if (gm&GLUT_ACTIVE_CTRL) {
            ch = AdjustKeyCtrl( ch );
        }
        (*keyboard_func)(ch, gm, false, x, y);
    }
}

static void glut_special_cb( int key, int x, int y )
{
    if (keyboard_func) {
        (*keyboard_func)(key+128, glutGetModifiers(), false, x, y);
    }
}

static void glut_keyboard_up_cb( unsigned char ch, int x, int y )
{
    if (keyboard_func) {
        int gm = glutGetModifiers();
        if (gm) {
            VS_LOG(trace, (boost::format("Up Modifier %d for char %d %c") % gm % (int)ch % ch));
        }
        if (gm&GLUT_ACTIVE_CTRL) {
            ch = AdjustKeyCtrl( ch );
        }
        (*keyboard_func)(ch, gm, true, x, y);
    }
}

static void glut_special_up_cb( int key, int x, int y )
{
    if (keyboard_func) {
        (*keyboard_func)(key+128, glutGetModifiers(), true, x, y);
    }
}

/*---------------------------------------------------------------------------*/
/*!
 *  Sets the keyboard callback
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_set_keyboard_func( winsys_keyboard_func_t func )
{
    keyboard_func = func;
}

/*---------------------------------------------------------------------------*/
/*!
 *  Sets the mouse button-press callback
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_set_mouse_func( winsys_mouse_func_t func )
{
    glutMouseFunc( func );
}

/*---------------------------------------------------------------------------*/
/*!
 *  Sets the mouse motion callback (when a mouse button is pressed)
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_set_motion_func( winsys_motion_func_t func )
{
    glutMotionFunc( func );
}

/*---------------------------------------------------------------------------*/
/*!
 *  Sets the mouse motion callback (when no mouse button is pressed)
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_set_passive_motion_func( winsys_motion_func_t func )
{
    glutPassiveMotionFunc( func );
}

/*---------------------------------------------------------------------------*/
/*!
 *  Copies the OpenGL back buffer to the front buffer
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_swap_buffers()
{
    glutSwapBuffers();
}

/*---------------------------------------------------------------------------*/
/*!
 *  Moves the mouse pointer to (x,y)
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_warp_pointer( int x, int y )
{
    glutWarpPointer( x, y );
}

/*---------------------------------------------------------------------------*/
/*!
 *  Initializes the OpenGL rendering context, and creates a window (or
 *  sets up fullscreen mode if selected)
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2021-09-07 - stephengtuggy
 */
void winsys_init( int *argc, char **argv, char const *window_title, char const *icon_title )
{
    int width, height;
    int glutWindow;
    g_game.x_resolution    = game_options.x_resolution;
    g_game.y_resolution    = game_options.y_resolution;
    gl_options.fullscreen  = game_options.fullscreen;
    gl_options.color_depth = game_options.colordepth;
    glutInit( argc, argv );
    if (game_options.glut_stencil) {
#ifdef __APPLE__
        if ( !(glutInitDisplayMode( GLUT_RGBA|GLUT_DEPTH|GLUT_DOUBLE|GLUT_STENCIL ), 1) )
            glutInitDisplayMode( GLUT_RGBA|GLUT_DEPTH|GLUT_DOUBLE );
#else
        glutInitDisplayMode( GLUT_RGBA|GLUT_DEPTH|GLUT_DOUBLE|GLUT_STENCIL );
#endif
    } else {
        glutInitDisplayMode( GLUT_RGBA|GLUT_DEPTH|GLUT_DOUBLE );
    }
    char str[1024];
    sprintf( str, "%dx%d:%d@60", g_game.x_resolution, g_game.y_resolution, gl_options.color_depth );
    glutGameModeString( str );
    VS_LOG(trace, (boost::format("Game Mode Params %1%x%2% at depth %3% @ %4% Hz")
                                % glutGameModeGet(GLUT_GAME_MODE_WIDTH)
                                % glutGameModeGet(GLUT_GAME_MODE_HEIGHT)
                                % glutGameModeGet(GLUT_GAME_MODE_PIXEL_DEPTH)
                                % glutGameModeGet(GLUT_GAME_MODE_REFRESH_RATE)));
    /* Create a window */
    if ( gl_options.fullscreen && (glutGameModeGet( GLUT_GAME_MODE_POSSIBLE ) != -1) ) {
        glutInitWindowPosition( 0, 0 );
        glutEnterGameMode();
        VS_LOG(trace, (boost::format("Game Mode Params %1%x%2% at depth %3% @ %4% Hz")
                                    % glutGameModeGet(GLUT_GAME_MODE_WIDTH)
                                    % glutGameModeGet(GLUT_GAME_MODE_HEIGHT)
                                    % glutGameModeGet(GLUT_GAME_MODE_PIXEL_DEPTH)
                                    % glutGameModeGet(GLUT_GAME_MODE_REFRESH_RATE)));
    } else {
        /* Set the initial window size */
        glutInitWindowSize( g_game.x_resolution, g_game.y_resolution );

        glutWindow = glutCreateWindow( window_title );
        if (glutWindow == 0) {
            VS_LOG_AND_FLUSH(fatal, "Couldn't create a window.");
            exit( 1 );                  // stephengtuggy 2020-07-27 - I would use VSExit here, but that calls winsys_exit, which I'm not sure will work if winsys_init hasn't finished yet.
        }
    }
}

/*---------------------------------------------------------------------------*/
/*!
 *  Deallocates resources in preparation for program termination
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_shutdown()
{
    static bool shutdown = false;
    if (!shutdown) {
        shutdown = true;
        if (gl_options.fullscreen) {
            glutLeaveGameMode();
        }
    }
}

/*---------------------------------------------------------------------------*/
/*!
 *  Enables/disables key repeat messages from being generated
 *  \return
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_enable_key_repeat( bool enabled )
{
    glutIgnoreKeyRepeat( !enabled );
}

/*---------------------------------------------------------------------------*/
/*!
 *  Shows/hides mouse cursor
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_show_cursor( bool visible )
{
    static bool vis = true;
    if (visible != vis) {
        if (visible) {
            glutSetCursor( GLUT_CURSOR_LEFT_ARROW );
        } else {
            glutSetCursor( GLUT_CURSOR_NONE );
        }
        vis = visible;
    }
}

/*---------------------------------------------------------------------------*/
/*!
 *  Processes and dispatches events.  This function never returns.
 *  \return  No.
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
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
 *  Sets the function to be called when program ends.  Note that this
 *  function should only be called once.
 *  \author  jfpatry
 *  \date    Created:  2000-10-20
 *  \date    Modified: 2021-09-06 - stephengtuggy */
void winsys_atexit( winsys_atexit_func_t func )
{
    static bool called = false;
    if (called) {
        std::cerr << "winsys_atexit called twice\n";
        VS_LOG_AND_FLUSH(error, "winsys_atexit called twice\n");
    }
    called = true;

    //atexit(func);
}

/*---------------------------------------------------------------------------*/
/*!
 *  Exits the program
 *  \author  jfpatry
 *  \date    Created:  2000-10-20
 *  \date    Modified: 2000-10-20
 */
void winsys_exit( int code )
{
    winsys_shutdown();
    exit( code );
}

#endif /* defined( SDL_WINDOWING ) */

/* EOF */
