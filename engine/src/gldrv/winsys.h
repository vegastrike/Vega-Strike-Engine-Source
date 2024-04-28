/*
 * winsys.h
 *
 * Incorporated into Vega Strike from Tux Racer
 *
 * Copyright (C) 1999-2024 Jasmin F. Patry, Daniel Horn, pyramid3d,
 * Benjamen R. Meyer, Stephen G. Tuggy, and other Vega Strike contributors.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef VEGA_STRIKE_ENGINE_GLDRV_WINSYS_H
#define VEGA_STRIKE_ENGINE_GLDRV_WINSYS_H
#define WINSYS_H 1
#ifndef UCHAR_MAX
#define UCHAR_MAX 255
#endif
#define HAVE_GLUT
#ifndef HAVE_SDL
#undef SDL_WINDOWING
#endif

#if defined( SDL_WINDOWING ) && defined (HAVE_SDL)
#   include "SDL2/SDL.h"
#elif defined( HAVE_GLUT )
#if defined(__APPLE__) && defined(__MACH__)
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

typedef enum {
    WSK_NOT_AVAIL = SDLK_UNKNOWN,

    /* Numeric keypad */
    WSK_KP0 = SDLK_KP_0,
    WSK_KP1 = SDLK_KP_1,
    WSK_KP2 = SDLK_KP_2,
    WSK_KP3 = SDLK_KP_3,
    WSK_KP4 = SDLK_KP_4,
    WSK_KP5 = SDLK_KP_5,
    WSK_KP6 = SDLK_KP_6,
    WSK_KP7 = SDLK_KP_7,
    WSK_KP8 = SDLK_KP_8,
    WSK_KP9 = SDLK_KP_9,
    WSK_KP_PERIOD = SDLK_KP_PERIOD,
    WSK_KP_DIVIDE = SDLK_KP_DIVIDE,
    WSK_KP_MULTIPLY = SDLK_KP_MULTIPLY,
    WSK_KP_MINUS = SDLK_KP_MINUS,
    WSK_KP_PLUS = SDLK_KP_PLUS,
    WSK_KP_ENTER = SDLK_KP_ENTER,
    WSK_KP_EQUALS = SDLK_KP_EQUALS,

    WSK_RETURN = 13,
    WSK_TAB = '\t',
    WSK_ESCAPE = 27,
    WSK_BACKSPACE =
#if defined(__APPLE__) && defined(__MACH__)
    127,
#else
    8,
#endif
    WSK_DELETE =
#if defined(__APPLE__) && defined(__MACH__)
    8,
#else
    127,
#endif
    /* Arrows + Home/End pad */
    WSK_UP = SDLK_UP,
    WSK_DOWN = SDLK_DOWN,
    WSK_RIGHT = SDLK_RIGHT,
    WSK_LEFT = SDLK_LEFT,
    WSK_INSERT = SDLK_INSERT,
    WSK_HOME = SDLK_HOME,
    WSK_END = SDLK_END,
    WSK_PAGEUP = SDLK_PAGEUP,
    WSK_PAGEDOWN = SDLK_PAGEDOWN,

    /* Function keys */
    WSK_F1 = SDLK_F1,
    WSK_F2 = SDLK_F2,
    WSK_F3 = SDLK_F3,
    WSK_F4 = SDLK_F4,
    WSK_F5 = SDLK_F5,
    WSK_F6 = SDLK_F6,
    WSK_F7 = SDLK_F7,
    WSK_F8 = SDLK_F8,
    WSK_F9 = SDLK_F9,
    WSK_F10 = SDLK_F10,
    WSK_F11 = SDLK_F11,
    WSK_F12 = SDLK_F12,
    WSK_F13 = SDLK_F13,
    WSK_F14 = SDLK_F14,
    WSK_F15 = SDLK_F15,

    /* Key state modifier keys */
    WSK_NUMLOCK = SDLK_NUMLOCKCLEAR,
    WSK_CAPSLOCK = SDLK_CAPSLOCK,
    WSK_SCROLLOCK = SDLK_SCROLLLOCK,
    WSK_RSHIFT = SDLK_RSHIFT,
    WSK_LSHIFT = SDLK_LSHIFT,
    WSK_RCTRL = SDLK_RCTRL,
    WSK_LCTRL = SDLK_LCTRL,
    WSK_RALT = SDLK_RALT,
    WSK_LALT = SDLK_LALT,
    WSK_RMETA = SDLK_RGUI,
    WSK_LMETA = SDLK_LGUI,
    WSK_BREAK = SDLK_PAUSE,
    WSK_PAUSE = SDLK_PAUSE,
    WSK_LAST = SDL_NUM_SCANCODES // Could be an issue. Needs investigating. See https://wiki.libsdl.org/SDL2/MigrationGuide.

} winsys_keysym_t;
typedef enum {
    WSK_MOD_NONE = KMOD_NONE,
    WSK_MOD_LSHIFT = KMOD_LSHIFT,
    WSK_MOD_RSHIFT = KMOD_RSHIFT,
    WSK_MOD_LCTRL = KMOD_LCTRL,
    WSK_MOD_RCTRL = KMOD_RCTRL,
    WSK_MOD_LALT = KMOD_LALT,
    WSK_MOD_RALT = KMOD_RALT,
    WSK_MOD_LMETA = SDLK_LGUI, // This is an issue for these two entries. We are ignoring the modifier.
    WSK_MOD_RMETA = SDLK_RGUI, // Need to figure out how to do modifier in sdl2.
    WSK_MOD_NUM = KMOD_NUM,
    WSK_MOD_CAPS = KMOD_CAPS,
    WSK_MOD_MODE = KMOD_MODE
} winsys_modifiers;

// mouse wheel events are only available with SDL 1.2.5 or later
#ifndef SDL_BUTTON_WHEELUP
# define SDL_BUTTON_WHEELUP 254
#endif
#ifndef SDL_BUTTON_WHEELDOWN
# define SDL_BUTTON_WHEELDOWN 255
#endif
typedef enum {
    WS_LEFT_BUTTON = SDL_BUTTON_LEFT,
    WS_MIDDLE_BUTTON = SDL_BUTTON_MIDDLE,
    WS_RIGHT_BUTTON = SDL_BUTTON_RIGHT,
    WS_WHEEL_UP = SDL_BUTTON_WHEELUP,
    WS_WHEEL_DOWN = SDL_BUTTON_WHEELDOWN
} winsys_mouse_button_t;

typedef enum {
    WS_MOUSE_DOWN = SDL_PRESSED,
    WS_MOUSE_UP = SDL_RELEASED
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
#if defined(__APPLE__) && defined(__MACH__)
    127,
#else
    8,
#endif
    WSK_DELETE =
#if defined(__APPLE__) && defined(__MACH__)
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
typedef void (*winsys_reshape_func_t)(int w, int h);
typedef void (*winsys_keyboard_func_t)(unsigned int key, unsigned int mod,
        bool release, int x, int y);
typedef void (*winsys_mouse_func_t)(int button, int state, int x, int y);
typedef void (*winsys_motion_func_t)(int x, int y);

typedef void (*winsys_atexit_func_t)(void);

void winsys_post_redisplay();
void winsys_set_display_func(winsys_display_func_t func);
void winsys_set_idle_func(winsys_idle_func_t func);
void winsys_set_reshape_func(winsys_reshape_func_t func);
void winsys_set_keyboard_func(winsys_keyboard_func_t func);
void winsys_set_mouse_func(winsys_mouse_func_t func);
void winsys_set_motion_func(winsys_motion_func_t func);
void winsys_set_passive_motion_func(winsys_motion_func_t func);

void winsys_swap_buffers();
void winsys_enable_key_repeat(bool enabled);
void winsys_warp_pointer(int x, int y);
void winsys_show_cursor(bool visible);

void winsys_init(int *argc, char **argv, char const *window_title,
        char const *icon_title);
void winsys_shutdown();

void winsys_process_events(); /* Never returns */

void winsys_atexit(winsys_atexit_func_t func);

void winsys_exit(int code);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif //VEGA_STRIKE_ENGINE_GLDRV_WINSYS_H

/* Emacs Customizations
;;; Local Variables: ***
;;; c-basic-offset:0 ***
;;; End: ***
*/

/* EOF */
