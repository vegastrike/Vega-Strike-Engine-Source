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
 * Portions of this code from Tux Racer by Jasmin F. Patry www.tuxracer.com
 */

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef _VEGASTRIKE_H_
#define _VEGASTRIKE_H_

#ifdef HAVE_CONFIG_H
#   include <config.h>
#endif

#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <string.h>
#include <limits.h>
#include <stdarg.h>

#if defined( WIN32 ) || defined( __CYGWIN__ )
    /* Note that this will define WIN32 for us, if it isn't defined already
     */
#  include <windows.h>
#endif /* defined( WIN32 ) || defined( __CYGWIN__ ) */

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#ifdef HAVE_GL_GLX_H
#   include <GL/glx.h>
#endif

#include TCL_HEADER

#ifndef M_PI
#   define M_PI 3.1415926535
#endif
#define TRUE 1
#define FALSE 0
#define EPS 1e-13
  //#define GL_TEXTURE0_ARB 0x84C0
  //#define GL_TEXTURE1_ARB 0x84C1
#include "string_util.h"
#include "file_util.h"
#include "vs_types.h"
#include "alglib.h"
#include "debug.h"
#include "error_util.h"

#define PROG_NAME "vegastrike"

/* Macros and include files for non-standard math routines */
#ifdef HAVE_IEEEFP_H
#   include <ieeefp.h>
#endif
#include <float.h>

#ifdef HAVE_FINITE
#   define FINITE(x) (finite(x))
#elif HAVE__FINITE
#   define FINITE(x) (_finite(x))
#elif HAVE_ISNAN
#   define FINITE(x) (!isnan(x))
#elif HAVE__ISNAN
#   define FINITE(x) (!_isnan(x))
#else
#   error "You don't have finite(), _finite(), isnan(), or _isnan() on your system!"
#endif

/* Macros for swapping bytes */
#define SWAP_WORD(x) \
{ \
unsigned long tmp; \
tmp  = ((x) >> 24) & 0x000000ff; \
tmp |= ((x) >> 8)  & 0x0000ff00; \
tmp |= ((x) << 8)  & 0x00ff0000; \
tmp |= ((x) << 24) & 0xff000000; \
(x) = tmp; \
}

#define SWAP_SHORT(x) \
{ \
unsigned short tmp; \
tmp  = ((x) << 8)  & 0xff00; \
tmp |= ((x) >> 8)  & 0x00ff; \
(x) = tmp; \
}


/* define this to turn off all debugging checks and messages */
/* #define VEGASTRIKE_NO_DEBUG */

/* Directory separator */
#ifdef WIN32
#   define DIR_SEPARATOR "\\"
#else
#   define DIR_SEPARATOR "/"
#endif

#define BUFF_LEN 512

/* Multiplayer is not yet supported */
#define MAX_PLAYERS 1

/* Game state */
typedef enum {
    ALL_MODES = -2,
    NO_MODE = -1,
    SPLASH = 0,
    GAME_TYPE_SELECT,
    EVENT_SELECT,
    INTRO,
    RACING,
    GAME_OVER,
    PAUSED,
    NUM_GAME_MODES
} game_mode_t;


/* View mode */
typedef enum {
    BEHIND,
    FOLLOW,
    ABOVE,
    NUM_VIEW_MODES
} view_mode_t;

/* View point */
typedef struct {
    view_mode_t mode;                   /* View mode */
    point_t pos;                        /* position of camera */
    point_t plyr_pos;                   /* position of player */
    vector_t dir;                       /* viewing direction */
    vector_t up;                        /* up direction */
    matrixgl_t inv_view_mat;            /* inverse view matrix */
    bool_t initialized;                 /* has view been initialized? */
} view_t;

/* Control mode */
typedef enum {
    KEYBOARD = 0,
    MOUSE = 1,
    JOYSTICK = 2
} control_mode_t;

/* Control data */
typedef struct {
    control_mode_t mode;                /* control mode */
    scalar_t turn_fact;                 /* turning [-1,1] */
    scalar_t turn_animation;            /* animation step [-1,1] */
    bool_t is_braking;                  /* is player braking? */
    bool_t is_paddling;                 /* is player paddling? */
    scalar_t paddle_time;
    bool_t begin_jump;
    bool_t jumping;
    bool_t jump_charging;
    scalar_t jump_amt;
    scalar_t jump_start_time;
    bool_t barrel_roll_left;
    bool_t barrel_roll_right;
    scalar_t barrel_roll_factor;
    bool_t front_flip;
    bool_t back_flip;
    scalar_t flip_factor;
} control_t;

/* All global data is stored in a variable of this type */
typedef struct {
  int audio_frequency_mode; //0==11025/8 1==22050/8 2==44100/8  3==11025/16 4==22050/16 5==44100/16  
  int sound_enabled;
  int music_enabled;
  int sound_volume;
  int music_volume;
  int warning_level;
  int capture_mouse;
  int fullscreen;
  int Multitexture;
  int PaletteExt;
  int color_depth;
  int y_resolution;
  int x_resolution;
  
  int fov;
  game_mode_t mode;
  game_mode_t prev_mode;
  scalar_t time;                      /* game time */
  scalar_t time_step;                 /* size of current time step */
  scalar_t secs_since_start;          /* seconds since game was started */
} game_data_t;

extern game_data_t g_game;

#define get_player_data( plyr ) ( & g_game.player[ (plyr) ] )
typedef int BOOL;
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif
