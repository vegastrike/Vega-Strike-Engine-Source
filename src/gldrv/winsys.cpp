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
#include <assert.h>
#include <sstream>

#include "config.h"
#include "gl_globals.h"
#include "winsys.h"
#include "vs_globals.h"
#include "xml_support.h"
#include "config_xml.h"
#include "vs_globals.h"
#include "vsfilesystem.h"
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

static SDL_Window   *g_window    = NULL;
static SDL_GLContext g_glcontext = NULL;

/* SDL 3 has no SDL_EnableKeyRepeat; repeats arrive as flagged keydown events
 * (event.key.repeat). winsys_enable_key_repeat() sets this flag and the event
 * loop filters repeats (and their text input) when it is false - matching
 * SDL 1.2's SDL_EnableKeyRepeat(0,0) which disabled repeats entirely. */
static bool keyRepeatEnabled = true;

/* SDL 3 delivers the translated character in a separate SDL_EVENT_TEXT_INPUT
 * that always follows its SDL_EVENT_KEY_DOWN; SDL 1.2 carried it inside the
 * keydown event (keysym.unicode). Keydowns are stashed here until the text
 * event arrives, then dispatched with the unicode value attached. */
struct PendingKey
{
    bool         active;
    unsigned int sym;
    unsigned int mod;
    int          x, y;
};
static PendingKey pendingKey;

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
    SDL_GL_SwapWindow( g_window );
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
    SDL_WarpMouseInWindow( g_window, x, y );
}

/*---------------------------------------------------------------------------*/
/*!
 *  Sets up the SDL OpenGL rendering context
 *  \author  jfpatry
 *  \date    Created:  2000-10-20
 *  \date    Modified: 2000-10-20
 */
/* SDL 3 splits window and GL context creation; SDL 1.2's SDL_SetVideoMode did
 * both. Retry loop mirrors the original: on failure walk the depth size down
 * (32->24->16) then flip the color depth (16<->32) with reset attributes.
 * SDL 3 GL attributes are minimums/best-effort, which covers SDL 1.2's
 * SDL_ANYFORMAT. */
static bool create_window_and_context( const char *window_title, int width, int height, SDL_WindowFlags flags )
{
    g_window = SDL_CreateWindow( window_title, width, height, flags );
    if (g_window != NULL) {
        g_glcontext = SDL_GL_CreateContext( g_window );
        if (g_glcontext != NULL) {
            SDL_GL_MakeCurrent( g_window, g_glcontext );
        } else {
            SDL_DestroyWindow( g_window );
            g_window = NULL;
        }
    }
    return g_window != NULL && g_glcontext != NULL;
}

static bool setup_sdl_video_mode( const char *window_title )
{
    SDL_WindowFlags video_flags = SDL_WINDOW_OPENGL;
    int    bpp = 0;
    int    width, height;
    if (gl_options.fullscreen) {
        video_flags |= SDL_WINDOW_FULLSCREEN;
    } else {
#ifndef _WIN32
        video_flags |= SDL_WINDOW_RESIZABLE;
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
    } else {
        otherattributes = 5;
        otherbpp = 16;
    }
    /* SDL 3's SDL_GL_SetAttribute returns bool; SDL 1.2's returned int and the
     * original code ignored it. The stencil attribute is set in winsys_init. */
    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, rs );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, gs );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, bs );
    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, game_options.z_pixel_format );
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    if (game_options.gl_accelerated_visual)
        SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1 );
    width  = g_game.x_resolution;
    height = g_game.y_resolution;
    if ( !create_window_and_context( window_title, width, height, video_flags ) ) {
        VSFileSystem::vs_dprintf( 1, "Couldn't initialize video: %s",
                                 SDL_GetError() );
        for (int counter = 0; g_window == NULL && counter < 2; ++counter) {
            for (int bpd = 4; bpd > 1; --bpd) {
                SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, bpd*8 );
                if ( !create_window_and_context( window_title, width, height, video_flags ) )
                    VSFileSystem::vs_dprintf( 1, "Couldn't initialize video bpp %d depth %d: %s\n",
                                             bpp, bpd*8, SDL_GetError() );
                else
                    break;
            }
            if (g_window == NULL) {
                SDL_GL_ResetAttributes();
                SDL_GL_SetAttribute( SDL_GL_RED_SIZE, otherattributes );
                SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, otherattributes );
                SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, otherattributes );
                SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
                if (game_options.gl_accelerated_visual)
                    SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1 );
                gl_options.color_depth = bpp = otherbpp;
            }
        }
        if (g_window == NULL) {
            VSFileSystem::vs_fprintf( stderr, "FAILED to initialize video\n" );
            exit( 1 );
        }
    }

    std::string version = (const char*)glGetString(GL_RENDERER);
    if (version == "GDI Generic")
    {
        if (game_options.gl_accelerated_visual) {
            VSFileSystem::vs_fprintf( stderr, "GDI Generic software driver reported, trying to reset.\n" );
            SDL_Quit();
	    game_options.gl_accelerated_visual = false;
            return false;
        } else {
            VSFileSystem::vs_fprintf( stderr, 
                "GDI Generic software driver reported, reset failed.\n "
                "Please make sure a graphics card driver is installed and functioning properly.\n" );
        }
    }

    int actual_w = 0, actual_h = 0;
    SDL_GetWindowSize( g_window, &actual_w, &actual_h );
    VSFileSystem::vs_dprintf( 3, "Setting Screen to w %d h %d\n", actual_w, actual_h );

    /* On Wayland (and some X11 setups) the compositor engages fullscreen
     * asynchronously; relying on the SDL_WINDOW_FULLSCREEN creation flag alone
     * leaves the first few frames (the intro splash screens) rendering windowed
     * before the compositor switches the window to fullscreen. Setting the
     * fullscreen mode explicitly with NULL = desktop mode forces the switch to
     * happen synchronously, before any frame is drawn. */
    if (gl_options.fullscreen) {
        SDL_SetWindowFullscreenMode( g_window, NULL );
    }

    return true;
}

/*---------------------------------------------------------------------------*/
/*!
 *  Initializes the OpenGL rendering context, and creates a window (or
 *  sets up fullscreen mode if selected)
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
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
     * (SDL 3 returns bool true on success; SDL 1.2 returned 0 on success)
     */
    if (!SDL_Init( sdl_flags )) {
        VSFileSystem::vs_fprintf( stderr, "Couldn't initialize SDL: %s", SDL_GetError() );
        exit( 1 );
    }

    //signal( SIGSEGV, SIG_DFL );
    /*
     * Init video
     */
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

#if defined (USE_STENCIL_BUFFER)
    /* Not sure if this is sufficient to activate stencil buffer  */
    SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );
#endif

    if (!setup_sdl_video_mode( window_title )) {
        winsys_init(argc, argv, window_title, icon_title);
    } else {
        /* The HUD/text renderer (gfx/hud.cpp) draws fonts via freeglut's
         * glutStrokeCharacter/glutBitmapCharacter even in the SDL build, so
         * GLUT must be initialized (master called glutInit here too). */
        glutInit( argc, argv );
        /* SDL 3 has no SDL_WM_SetIcon and the window must exist before the
         * icon can be attached (SDL 1.2 set it before SDL_SetVideoMode); the
         * window title was passed to SDL_CreateWindow. SDL_StartTextInput
         * replaces SDL 1.2's SDL_EnableUNICODE(1) (int'l keyboards); the game
         * still gates use of the translations via game_options.enable_unicode. */
        SDL_Surface *icon = NULL;
#if 1
        if (icon_title) icon = SDL_LoadBMP( icon_title );
        if (icon) {
            SDL_SetSurfaceColorKey( icon, true, ( (Uint32*) (icon->pixels) )[0] );
            SDL_SetWindowIcon( g_window, icon );
            SDL_DestroySurface( icon );
        }
#endif
        SDL_StartTextInput( g_window );
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
    /* SDL 3 has no SDL_EnableKeyRepeat; repeats arrive as flagged keydown
     * events and are filtered in winsys_process_events when disabled. */
    keyRepeatEnabled = enabled;
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
        /* SDL 3 split SDL_ShowCursor(int toggle) into SDL_ShowCursor() and
         * SDL_HideCursor() */
        if (visible)
            SDL_ShowCursor();
        else
            SDL_HideCursor();
        vis = visible;
    }
}

/* SDL 1.2's SDL_EnableUNICODE(bool); SDL 3 toggles text-input events. Used by
 * the console (command.cpp). */
void winsys_set_text_input( bool enabled )
{
    if (g_window != NULL) {
        if (enabled)
            SDL_StartTextInput( g_window );
        else
            SDL_StopTextInput( g_window );
    }
}

/*---------------------------------------------------------------------------*/
/*!
 *  Processes and dispatches events.  This function never returns.
 *  \return  No.
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 *  \date    Modified: 2005-8-16 - Rogue
 *  \date    Modified: 2005-12-24 - ace123
 */
extern int shiftdown( int );
extern int shiftup( int );

/* SDL 3 -> SDL 1 keycode translation (see the comment on the frozen WSK enum
 * in winsys.h). SDL 3 kept the SDL 1 ASCII-range keycodes unchanged (letters,
 * digits, punctuation, Return/Tab/Escape/Backspace/Delete); only the special
 * keys were renumbered to SDL_SCANCODE_TO_KEYCODE() values. */
static unsigned int sdl3key_to_wsk( SDL_Keycode key )
{
    switch (key)
    {
    case SDLK_KP_DIVIDE:   return WSK_KP_DIVIDE;
    case SDLK_KP_MULTIPLY: return WSK_KP_MULTIPLY;
    case SDLK_KP_MINUS:    return WSK_KP_MINUS;
    case SDLK_KP_PLUS:     return WSK_KP_PLUS;
    case SDLK_KP_ENTER:    return WSK_KP_ENTER;
    case SDLK_KP_1: return WSK_KP1;
    case SDLK_KP_2: return WSK_KP2;
    case SDLK_KP_3: return WSK_KP3;
    case SDLK_KP_4: return WSK_KP4;
    case SDLK_KP_5: return WSK_KP5;
    case SDLK_KP_6: return WSK_KP6;
    case SDLK_KP_7: return WSK_KP7;
    case SDLK_KP_8: return WSK_KP8;
    case SDLK_KP_9: return WSK_KP9;
    case SDLK_KP_0:      return WSK_KP0;
    case SDLK_KP_PERIOD: return WSK_KP_PERIOD;
    case SDLK_KP_EQUALS: return WSK_KP_EQUALS;
    case SDLK_UP:    return WSK_UP;
    case SDLK_DOWN:  return WSK_DOWN;
    case SDLK_RIGHT: return WSK_RIGHT;
    case SDLK_LEFT:  return WSK_LEFT;
    case SDLK_INSERT: return WSK_INSERT;
    case SDLK_HOME:   return WSK_HOME;
    case SDLK_END:    return WSK_END;
    case SDLK_PAGEUP:   return WSK_PAGEUP;
    case SDLK_PAGEDOWN: return WSK_PAGEDOWN;
    case SDLK_F1:  return WSK_F1;
    case SDLK_F2:  return WSK_F2;
    case SDLK_F3:  return WSK_F3;
    case SDLK_F4:  return WSK_F4;
    case SDLK_F5:  return WSK_F5;
    case SDLK_F6:  return WSK_F6;
    case SDLK_F7:  return WSK_F7;
    case SDLK_F8:  return WSK_F8;
    case SDLK_F9:  return WSK_F9;
    case SDLK_F10: return WSK_F10;
    case SDLK_F11: return WSK_F11;
    case SDLK_F12: return WSK_F12;
    case SDLK_F13: return WSK_F13;
    case SDLK_F14: return WSK_F14;
    case SDLK_F15: return WSK_F15;
    case SDLK_NUMLOCKCLEAR: return WSK_NUMLOCK;
    case SDLK_CAPSLOCK:     return WSK_CAPSLOCK;
    case SDLK_SCROLLLOCK:   return WSK_SCROLLOCK;
    case SDLK_RSHIFT: return WSK_RSHIFT;
    case SDLK_LSHIFT: return WSK_LSHIFT;
    case SDLK_RCTRL:  return WSK_RCTRL;
    case SDLK_LCTRL:  return WSK_LCTRL;
    case SDLK_RALT:   return WSK_RALT;
    case SDLK_LALT:   return WSK_LALT;
    case SDLK_RGUI:   return WSK_RMETA;   /* SDL 1.2 KMOD_RMETA */
    case SDLK_LGUI:   return WSK_LMETA;   /* SDL 1.2 KMOD_LMETA */
    case SDLK_PAUSE:  return WSK_PAUSE;
    default:
        /* ASCII-range keys are identical in SDL 1.2 and SDL 3; unmapped SDL 3
         * special keys report WSK_NOT_AVAIL like SDL 1.2's SDLK_UNKNOWN */
        return (key < 256) ? (unsigned int)key : WSK_NOT_AVAIL;
    }
}

/* First UTF-8 code point of a text-input string as a 16-bit unicode value
 * (SDL 1.2's keysym.unicode was 16-bit). Returns 0 for empty or 4-byte
 * sequences, mirroring SDL 1.2 delivering no translation in those cases. */
static unsigned int utf8_first_codepoint( const char *s )
{
    if (s == NULL || s[0] == '\0')
        return 0;
    const unsigned char *p = (const unsigned char *) s;
    unsigned int cp;
    if (p[0] < 0x80) {
        cp = p[0];
    } else if ( (p[0]&0xE0) == 0xC0 && (p[1]&0xC0) == 0x80 ) {
        cp = ( (p[0]&0x1F) << 6 )|( p[1]&0x3F );
    } else if ( (p[0]&0xF0) == 0xE0 && (p[1]&0xC0) == 0x80 && (p[2]&0xC0) == 0x80 ) {
        cp = ( (p[0]&0x0F) << 12 )|( (p[1]&0x3F) << 6 )|( p[2]&0x3F );
    } else {
        return 0;
    }
    return (cp <= 0xFFFF) ? cp : 0;
}

/* The keyboard callback body, unchanged in intent from the SDL 1.2 version:
 * unicode/keysym selection, ctrl fixup, keysym_to_unicode release-event
 * translation, intl-keyboard shift hack. 'sym' is already a WSK (SDL 1)
 * value; 'unicode' comes from SDL_EVENT_TEXT_INPUT (or 0). */
static void dispatch_key( unsigned int sym, unsigned int unicode, SDL_Keymod mod, bool release, int x, int y )
{
    static unsigned int keysym_to_unicode[256];
    static bool keysym_to_unicode_init = false;
    if (!keysym_to_unicode_init) {
        keysym_to_unicode_init = true;
        memset( keysym_to_unicode, 0, sizeof (keysym_to_unicode) );
    }
    unsigned int modbits = (unsigned int) mod;

    bool maybe_unicode = game_options.enable_unicode && !(sym&~0xFF);
    bool is_unicode = maybe_unicode && unicode;

    //Fix up ctrl unicode codes
    if (is_unicode && unicode <= 0x1a && (sym&0xFF) > 0x1a && modbits & (SDL_KMOD_LCTRL|SDL_KMOD_RCTRL))
        unicode += 0x60; // 0x01 (^A) --> 0x61 (A)

    //Translate untranslated release events
    if (release && maybe_unicode
        && keysym_to_unicode[sym&0xFF])
        unicode = keysym_to_unicode[sym&0xFF];

    //Remember translation for translating release events
    if (is_unicode)
        keysym_to_unicode[sym&0xFF] = unicode;

    //Ugly hack: prevent shiftup/shiftdown screwups on intl keyboard
    //Note: Thank god we'll have OIS for 0.5.x
    bool shifton = modbits&(SDL_KMOD_LSHIFT|SDL_KMOD_RSHIFT|SDL_KMOD_CAPS);

    VSFileSystem::vs_dprintf(2,
        "Kbd: %s mod:%x sym:%x unicode:%x sh:%c u:%c mu:%c\n",
        release ? "KEYUP" : "KEYDOWN",
        modbits,
        sym,
        unicode,
        (shifton) ? 't' : 'f',
        (is_unicode) ? 't' : 'f',
        (maybe_unicode) ? 't' : 'f'
    );

    if (shifton && is_unicode
        && (unsigned int) shiftup( shiftdown( unicode ) ) != unicode) {
        modbits = modbits&~(SDL_KMOD_LSHIFT|SDL_KMOD_RSHIFT|SDL_KMOD_CAPS);
        shifton = false;
    }
    //Choose unicode or symbolic, depending on whether there is or not a unicode
    //code (unicode codes must be postprocessed to make sure application of the
    //shiftup modifier does not destroy it)
    unsigned int key = is_unicode
                       ? ( (shifton)
                          ? shiftdown( unicode )
                          : unicode
                          ) : sym;
    //Send the event
    (*keyboard_func)(key,
                     modbits,
                     release,
                     x, y);
}

/* Dispatch the stashed keydown (if any) with the given unicode value */
static void flushPendingKey( unsigned int unicode )
{
    if (!pendingKey.active)
        return;
    pendingKey.active = false;
    dispatch_key( pendingKey.sym, unicode, (SDL_Keymod) pendingKey.mod, false, pendingKey.x, pendingKey.y );
}

void winsys_process_events()
{
    SDL_Event event;
    int  x, y;
    float fx, fy;

    while (keepRunning)
    {
        while ( SDL_PollEvent( &event ) ) {
            switch (event.type)
            {
            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP:
                /* SDL 3 delivers key repeats as flagged keydowns; SDL 1.2's
                 * SDL_EnableKeyRepeat(0,0) (set in GFXInit) disabled them
                 * entirely, including their unicode, so filter them here. */
                if (!keyRepeatEnabled && event.key.repeat)
                    break;
                if (keyboard_func) {
                    SDL_GetMouseState( &fx, &fy );
                    x = (int) fx;
                    y = (int) fy;
                    unsigned int sym = sdl3key_to_wsk( event.key.key );
                    bool release = !event.key.down;
                    if (release) {
                        dispatch_key( sym, 0, event.key.mod, true, x, y );
                    } else {
                        /* SDL 1.2 carried the translated character inside the
                         * keydown event; SDL 3 delivers it in a following
                         * SDL_EVENT_TEXT_INPUT - stash until we see it. */
                        pendingKey.active = true;
                        pendingKey.sym    = sym;
                        pendingKey.mod    = (unsigned int) event.key.mod;
                        pendingKey.x      = x;
                        pendingKey.y      = y;
                    }
                }
                break;

            case SDL_EVENT_TEXT_INPUT:
                /* Attach the translated character to the stashed keydown. If
                 * no keydown is pending (repeat filtered, dead key, IME), this
                 * is a no-op - SDL 1.2 delivered no unicode in those cases
                 * either. */
                flushPendingKey( utf8_first_codepoint( event.text.text ) );
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP:
                flushPendingKey( 0 );
                if (mouse_func) {
                    /* SDL 3 renumbered X1/X2 to 4/5 (SDL 1.2 had them at 6/7
                     * with the wheel at 4/5); renumber so the game sees the
                     * same button space as SDL 1.2. */
                    unsigned int button = event.button.button;
                    if (button >= 4)
                        button += 2;
                    (*mouse_func)(button,
                                  event.button.down ? WS_MOUSE_DOWN : WS_MOUSE_UP,
                                  (int) event.button.x,
                                  (int) event.button.y);
                }
                break;

            case SDL_EVENT_MOUSE_MOTION:
                flushPendingKey( 0 );
                if (event.motion.state) {
                    /* buttons are down */
                    if (motion_func)
                        (*motion_func)((int) event.motion.x,
                                       (int) event.motion.y);
                } else
                /* no buttons are down */
                if (passive_motion_func) {
                    (*passive_motion_func)((int) event.motion.x,
                                           (int) event.motion.y);
                }
                break;

            case SDL_EVENT_MOUSE_WHEEL:
                flushPendingKey( 0 );
                if (mouse_func) {
                    /* SDL 1.2 delivered the wheel as button 4/5 press+release
                     * pairs; synthesize the same from the SDL 3 wheel event. */
                    int button = (event.wheel.y > 0) ? WS_WHEEL_UP : WS_WHEEL_DOWN;
                    int wx = (int) event.wheel.mouse_x;
                    int wy = (int) event.wheel.mouse_y;
                    (*mouse_func)(button, WS_MOUSE_DOWN, wx, wy);
                    (*mouse_func)(button, WS_MOUSE_UP, wx, wy);
                }
                break;

            case SDL_EVENT_WINDOW_RESIZED:
#if !(defined (_WIN32) && defined (SDL_WINDOWING ) )
                flushPendingKey( 0 );
                /* SDL 1.2 fired the resize event before resizing and re-called
                 * SDL_SetVideoMode here; SDL 3 has already resized the window,
                 * so just record the geometry and notify the app. */
                g_game.x_resolution = (int) event.window.data1;
                g_game.y_resolution = (int) event.window.data2;
                if (reshape_func)
                    (*reshape_func)(g_game.x_resolution,
                                    g_game.y_resolution);
#endif
                break;

            case SDL_EVENT_QUIT:
                /* SDL 1.2's loop ignored SDL_QUIT (window close); keep parity */
                break;
            }
        }
        flushPendingKey( 0 );
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
 *  \date Modified: 2000-10-20 */
void winsys_atexit( winsys_atexit_func_t func )
{
    static bool called = false;
    if (called != false)
        VSFileSystem::vs_dprintf( 1, "winsys_atexit called twice" );
    called = true;
    //atexit_func = func;
    //atexit (func);
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
    if (atexit_func)
        (*atexit_func)();
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
        if (gm)
            VSFileSystem::vs_dprintf('3', "Down Modifier %d for char %d %c\n", gm, (int) ch, ch );
        if (gm&GLUT_ACTIVE_CTRL)
            ch = AdjustKeyCtrl( ch );
        (*keyboard_func)(ch, gm, false, x, y);
    }
}

static void glut_special_cb( int key, int x, int y )
{
    if (keyboard_func)
        (*keyboard_func)(key+128, glutGetModifiers(), false, x, y);
}

static void glut_keyboard_up_cb( unsigned char ch, int x, int y )
{
    if (keyboard_func) {
        int gm = glutGetModifiers();
        if (gm)
            VSFileSystem::vs_dprintf('3',"Up Modifier %d for char %d %c\n", gm, (int) ch, ch );
        if (gm&GLUT_ACTIVE_CTRL)
            ch = AdjustKeyCtrl( ch );
        (*keyboard_func)(ch, gm, true, x, y);
    }
}

static void glut_special_up_cb( int key, int x, int y )
{
    if (keyboard_func)
        (*keyboard_func)(key+128, glutGetModifiers(), true, x, y);
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
 *  \date    Modified: 2000-10-19
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
    VSFileSystem::vs_dprintf('3', "Game Mode Params %dx%d at depth %d @ %d Hz\n", glutGameModeGet( GLUT_GAME_MODE_WIDTH ),
            glutGameModeGet( GLUT_GAME_MODE_WIDTH ), glutGameModeGet( GLUT_GAME_MODE_PIXEL_DEPTH ),
            glutGameModeGet( GLUT_GAME_MODE_REFRESH_RATE ) );
    /* Create a window */
    if ( gl_options.fullscreen && (glutGameModeGet( GLUT_GAME_MODE_POSSIBLE ) != -1) ) {
        glutInitWindowPosition( 0, 0 );
        glutEnterGameMode();
        VSFileSystem::vs_dprintf('3', "Game Mode Params %dx%d at depth %d @ %d Hz\n", glutGameModeGet(
                    GLUT_GAME_MODE_WIDTH ), glutGameModeGet( GLUT_GAME_MODE_WIDTH ), glutGameModeGet(
                    GLUT_GAME_MODE_PIXEL_DEPTH ), glutGameModeGet( GLUT_GAME_MODE_REFRESH_RATE ) );
    } else {
        /* Set the initial window size */
        glutInitWindowSize( g_game.x_resolution, g_game.y_resolution );

        glutWindow = glutCreateWindow( window_title );
        if (glutWindow == 0) {
            (void) VSFileSystem::vs_fprintf( stderr, "Couldn't create a window.\n" );
            exit( 1 );
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
        if (gl_options.fullscreen)
            glutLeaveGameMode();
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
        if (visible)
            glutSetCursor( GLUT_CURSOR_LEFT_ARROW );
        else
            glutSetCursor( GLUT_CURSOR_NONE );
        vis = visible;
    }
}

/* SDL 1.2's SDL_EnableUNICODE equivalent; GLUT has no text-input control */
void winsys_set_text_input( bool enabled )
{
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
 *  \date Modified: 2000-10-20 */
void winsys_atexit( winsys_atexit_func_t func )
{
    static bool called = false;
    if (called)
        VSFileSystem::vs_fprintf( stderr, "winsys_atexit called twice\n" );
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

