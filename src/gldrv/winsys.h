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
#ifndef WINSYS_H
#define WINSYS_H 1
#include "config.h"
#ifndef UCHAR_MAX
#define UCHAR_MAX 255
#endif
#define HAVE_GLUT
#ifndef HAVE_SDL
#undef SDL_WINDOWING
#endif

#if defined( SDL_WINDOWING ) && defined (HAVE_SDL)
#   include <SDL3/SDL.h>
#elif defined( HAVE_GLUT )
#if defined(__APPLE__) || defined(MACOSX)
    #include <GLUT/glut.h>
#else
    #include <GL/glut.h>
#endif
#else
#   error "Neither SDL nor GLUT are present."
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/* Keysyms */

#if defined( SDL_WINDOWING ) && defined (HAVE_SDL) 
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* SDL version */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/* SDL 3 port (from scratch, 2026-08-14):
 *
 * The WSK_* keycode values below are FROZEN at their SDL 1.2 values (verified
 * against sdl12-compat's SDL_keysym.h). They are the game's internal key
 * identity: baked into the keyBindings[][]/keyState[][] array sizes
 * (in_kb.cpp KEYMAP_SIZE), the key_map name table (config_xml.cpp) and every
 * user key binding. SDL 3 renumbered all special keys (SDLK_UP: 273 ->
 * 0x40000052) and removed SDLK_LAST, so this enum must NOT track SDL 3
 * constants. Translation from SDL 3 keycodes happens at the boundary in
 * winsys.cpp (sdl3key_to_wsk); ASCII-range keys are identical in both SDLs
 * and pass through unchanged.
 */
typedef enum {
    WSK_NOT_AVAIL = 0,

    /* Numeric keypad */
    WSK_KP0 = 256,
    WSK_KP1 = 257,
    WSK_KP2 = 258,
    WSK_KP3 = 259,
    WSK_KP4 = 260,
    WSK_KP5 = 261,
    WSK_KP6 = 262,
    WSK_KP7 = 263,
    WSK_KP8 = 264,
    WSK_KP9 = 265,
    WSK_KP_PERIOD = 266,
    WSK_KP_DIVIDE = 267,
    WSK_KP_MULTIPLY = 268,
    WSK_KP_MINUS = 269,
    WSK_KP_PLUS = 270,
    WSK_KP_ENTER = 271,
    WSK_KP_EQUALS = 272,
	
	WSK_RETURN = 13,
	WSK_TAB = '\t',
	WSK_ESCAPE = 27,
	WSK_BACKSPACE =
#ifdef __APPLE__
    127,
#else
    8,
#endif
	WSK_DELETE = 
#ifdef __APPLE__
    8,
#else
    127,
#endif
    /* Arrows + Home/End pad */
    WSK_UP = 273,
    WSK_DOWN = 274,
    WSK_RIGHT = 275,
    WSK_LEFT = 276,
    WSK_INSERT = 277,
    WSK_HOME = 278,
    WSK_END = 279,
    WSK_PAGEUP = 280,
    WSK_PAGEDOWN = 281,

    /* Function keys */
    WSK_F1 = 282,
    WSK_F2 = 283,
    WSK_F3 = 284,
    WSK_F4 = 285,
    WSK_F5 = 286,
    WSK_F6 = 287,
    WSK_F7 = 288,
    WSK_F8 = 289,
    WSK_F9 = 290,
    WSK_F10 = 291,
    WSK_F11 = 292,
    WSK_F12 = 293,
    WSK_F13 = 294,
    WSK_F14 = 295,
    WSK_F15 = 296,

    /* Key state modifier keys */
    WSK_NUMLOCK = 300,
    WSK_CAPSLOCK = 301,
    WSK_SCROLLOCK = 302,
    WSK_RSHIFT = 303,
    WSK_LSHIFT = 304,
    WSK_RCTRL = 305,
    WSK_LCTRL = 306,
    WSK_RALT = 307,
    WSK_LALT = 308,
    WSK_RMETA = 309,
    WSK_LMETA = 310,
    WSK_BREAK = 318,      /* SDL 1.2 SDLK_BREAK; no SDL 3 keycode exists - binding never fires */
	WSK_PAUSE = 19,       /* SDL 1.2 SDLK_PAUSE */
    WSK_LAST = 323        /* SDL 1.2 SDLK_LAST; SDL 3 removed SDLK_LAST */

} winsys_keysym_t;
typedef enum {
    /* SDL 3 SDL_KMOD_* bits are identical to SDL 1.2 KMOD_* (verified); only
     * the names gained the SDL_ prefix and LMETA/RMETA became LGUI/RGUI. */
        WSK_MOD_NONE=SDL_KMOD_NONE,
	WSK_MOD_LSHIFT=SDL_KMOD_LSHIFT,
	WSK_MOD_RSHIFT=SDL_KMOD_RSHIFT,
	WSK_MOD_LCTRL=SDL_KMOD_LCTRL ,
	WSK_MOD_RCTRL=SDL_KMOD_RCTRL ,
	WSK_MOD_LALT=SDL_KMOD_LALT  ,
	WSK_MOD_RALT=SDL_KMOD_RALT  ,
	WSK_MOD_LMETA=SDL_KMOD_LGUI ,
	WSK_MOD_RMETA=SDL_KMOD_RGUI ,
	WSK_MOD_NUM=SDL_KMOD_NUM   ,
	WSK_MOD_CAPS=SDL_KMOD_CAPS  ,
	WSK_MOD_MODE=SDL_KMOD_MODE  
} winsys_modifiers;

/* SDL 1.2 delivered the mouse wheel as button presses (SDL_BUTTON_WHEELUP=4,
 * SDL_BUTTON_WHEELDOWN=5). SDL 3 has a dedicated SDL_EVENT_MOUSE_WHEEL and
 * renumbered buttons 4/5 to X1/X2, so keep the wheel values as frozen SDL 1
 * literals: winsys.cpp synthesizes 4/5 press+release pairs from the wheel
 * event and renumbers real X1/X2 (4/5) to the SDL 1 X1/X2 values (6/7). */
typedef enum {
    WS_LEFT_BUTTON = SDL_BUTTON_LEFT,      /* 1 */
    WS_MIDDLE_BUTTON = SDL_BUTTON_MIDDLE,  /* 2 */
    WS_RIGHT_BUTTON = SDL_BUTTON_RIGHT,    /* 3 */
	WS_WHEEL_UP = 4,
	WS_WHEEL_DOWN = 5
} winsys_mouse_button_t;

typedef enum {
    WS_MOUSE_DOWN = SDL_PRESSED,   /* 1 */
    WS_MOUSE_UP = SDL_RELEASED     /* 0 */
} winsys_button_state_t;

#else
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* GLUT version */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/* GLUT doesn't define as many keysyms as SDL; we map those to 
   WSK_NOT_AVAIL (0) */

typedef enum {
    WSK_NOT_AVAIL = 0,

    /* Numeric keypad */
    WSK_KP0 = 0,
    WSK_KP1 = 0,
    WSK_KP2 = 0,
    WSK_KP3 = 0,
    WSK_KP4 = 0,
    WSK_KP5 = 0,
    WSK_KP6 = 0,
    WSK_KP7 = 0,
    WSK_KP8 = 0,
    WSK_KP9 = 0,
    WSK_KP_PERIOD = 0,
    WSK_KP_DIVIDE = 0,
    WSK_KP_MULTIPLY = 0,
    WSK_KP_MINUS = 0,
    WSK_KP_PLUS = 0,
    WSK_KP_ENTER = 0,
    WSK_KP_EQUALS = 0,

	WSK_RETURN = 13,
	WSK_TAB = '\t',
	WSK_ESCAPE = 27,
	WSK_BACKSPACE =
#ifdef __APPLE__
    127,
#else
    8,
#endif
	WSK_DELETE = 
#ifdef __APPLE__
    8,
#else
    127,
#endif

    /* Arrows + Home/End pad */
    WSK_UP = GLUT_KEY_UP+128,
    WSK_DOWN = GLUT_KEY_DOWN+128,
    WSK_RIGHT = GLUT_KEY_RIGHT+128,
    WSK_LEFT = GLUT_KEY_LEFT+128,
    WSK_INSERT = GLUT_KEY_INSERT+128,
    WSK_HOME = GLUT_KEY_HOME+128,
    WSK_END = GLUT_KEY_END+128,
    WSK_PAGEUP = GLUT_KEY_PAGE_UP+128,
    WSK_PAGEDOWN = GLUT_KEY_PAGE_DOWN+128,

    /* Function keys */
    WSK_F1 = GLUT_KEY_F1+128,
    WSK_F2 = GLUT_KEY_F2+128,
    WSK_F3 = GLUT_KEY_F3+128,
    WSK_F4 = GLUT_KEY_F4+128,
    WSK_F5 = GLUT_KEY_F5+128,
    WSK_F6 = GLUT_KEY_F6+128,
    WSK_F7 = GLUT_KEY_F7+128,
    WSK_F8 = GLUT_KEY_F8+128,
    WSK_F9 = GLUT_KEY_F9+128,
    WSK_F10 = GLUT_KEY_F10+128,
    WSK_F11 = GLUT_KEY_F11+128,
    WSK_F12 = GLUT_KEY_F12+128,
    WSK_F13 = 0,
    WSK_F14 = 0,
    WSK_F15 = 0,

    /* Key state modifier keys */
    WSK_NUMLOCK = 0,
    WSK_CAPSLOCK = 0,
    WSK_SCROLLOCK = 0,
    WSK_RSHIFT = 0,
    WSK_LSHIFT = 0,
    WSK_RCTRL = 0,
    WSK_LCTRL = 0,
    WSK_RALT = 0,
    WSK_LALT = 0,
    WSK_RMETA = 0,
    WSK_LMETA = 0,
    WSK_BREAK = 0,
 	WSK_PAUSE = 0,
   WSK_LAST = UCHAR_MAX /* GLUT doesn't define a max key, but this is more
			    than enough as of version 3.7 */
} winsys_keysym_t;
typedef enum {
        WSK_MOD_NONE=0,
	WSK_MOD_LSHIFT=GLUT_ACTIVE_SHIFT,
	WSK_MOD_RSHIFT=GLUT_ACTIVE_SHIFT,
	WSK_MOD_LCTRL=GLUT_ACTIVE_CTRL ,
	WSK_MOD_RCTRL=GLUT_ACTIVE_CTRL ,
	WSK_MOD_LALT=GLUT_ACTIVE_ALT  ,
	WSK_MOD_RALT=GLUT_ACTIVE_ALT  ,
	WSK_MOD_LMETA=0 ,
	WSK_MOD_RMETA=0,
	WSK_MOD_NUM=0,
	WSK_MOD_CAPS=0,
	WSK_MOD_MODE=0
} winsys_modifiers;

typedef enum {
    WS_LEFT_BUTTON = GLUT_LEFT_BUTTON,
    WS_MIDDLE_BUTTON = GLUT_MIDDLE_BUTTON,
    WS_RIGHT_BUTTON = GLUT_RIGHT_BUTTON,
	WS_WHEEL_UP,
	WS_WHEEL_DOWN
} winsys_mouse_button_t;

typedef enum {
    WS_MOUSE_DOWN = GLUT_DOWN,
    WS_MOUSE_UP = GLUT_UP
} winsys_button_state_t;

#endif /* defined( SDL_WINDOWING ) */


typedef void (*winsys_display_func_t)();
typedef void (*winsys_idle_func_t)();
typedef void (*winsys_reshape_func_t)( int w, int h );
typedef void (*winsys_keyboard_func_t)( unsigned int key, unsigned int mod,
					bool release, int x, int y );
typedef void (*winsys_mouse_func_t)( int button, int state, int x, int y );
typedef void (*winsys_motion_func_t)( int x, int y );

typedef void (*winsys_atexit_func_t)( void );

void winsys_post_redisplay();
void winsys_set_display_func( winsys_display_func_t func );
void winsys_set_idle_func( winsys_idle_func_t func );
void winsys_set_reshape_func( winsys_reshape_func_t func );
void winsys_set_keyboard_func( winsys_keyboard_func_t func );
void winsys_set_mouse_func( winsys_mouse_func_t func );
void winsys_set_motion_func( winsys_motion_func_t func );
void winsys_set_passive_motion_func( winsys_motion_func_t func );

void winsys_swap_buffers();
void winsys_enable_key_repeat( bool enabled );
void winsys_set_text_input( bool enabled );
void winsys_warp_pointer( int x, int y );
void winsys_show_cursor( bool visible );

void winsys_init( int *argc, char **argv, char const *window_title,
		  char const *icon_title );
void winsys_shutdown();

void winsys_process_events(); /* Never returns */

void winsys_atexit( winsys_atexit_func_t func );

void winsys_exit( int code );

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* WINSYS_H */

/* Emacs Customizations
;;; Local Variables: ***
;;; c-basic-offset:0 ***
;;; End: ***
*/

/* EOF */
