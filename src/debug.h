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
 * This code taken from Tux Racer by Jasmin F. Patry www.tuxracer.com
 */

#ifdef __cplusplus
extern "C"
{
#endif

#include "vegastrike.h"

#ifndef _DEBUG_H_
#define _DEBUG_H_

typedef enum {
    DEBUG_ODE,
    DEBUG_QUADTREE,
    DEBUG_CONTROL,
    DEBUG_HEALTH,
    DEBUG_SOUND,
    DEBUG_TEXTURE,
    DEBUG_VIEW,
    DEBUG_GL_EXT,
    DEBUG_FONT,
    DEBUG_UI,
    DEBUG_GAME_LOGIC,
    DEBUG_SAVE,
    DEBUG_JOYSTICK,
    NUM_DEBUG_MODES
} debug_mode_t;

void init_debug(char * t);
bool_t debug_mode_is_active( debug_mode_t mode );
void print_debug( debug_mode_t mode, char *fmt, ... );

#ifdef TUXRACER_NO_DEBUG

#define check_assertion( condition, desc )  /* noop */

#else

#define check_assertion( condition, desc ) \
    if ( condition ) {} else { \
        fprintf( stderr, "!!! " PROG_NAME " unexpected error [%s:%d]: %s\n", \
		 __FILE__, __LINE__, desc ); \
        abort(); \
    }


#endif /* TUXRACER_NO_DEBUG */

#define code_not_reached() \
    check_assertion( 0, "supposedly unreachable code reached!" )

#endif /* _DEBUG_H_ */

#ifdef __cplusplus
} /* extern "C" */
#endif
