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

#include <string.h>
#include <stdarg.h>
#include "debug.h"
#include "vegastrike.h"
#include "string_util.h"

static bool_t debug_setting[ NUM_DEBUG_MODES ];
static char* debug_desc[ NUM_DEBUG_MODES ] = {
    "ode",
    "quadtree",
    "control",
    "health",
    "sound",
    "texture",
    "view",
    "gl_ext",
    "font",
    "ui",
    "game_logic",
    "save",
    "joystick"
};

/* Parse the debug parameter, fill in the debug_setting array */
void init_debug(char *debug_str)
{

#ifndef TUXRACER_NO_DEBUG

    char *tmp_str;
    char *p;
    int i;
    bool_t new_setting;

    for ( i=0; i<NUM_DEBUG_MODES; i++ ) {
	debug_setting[i] = False;
    }

    //    debug_str = getparam_debug(); passed in
    tmp_str = debug_str;

    while ( (p = strtok( tmp_str, " " )) != NULL ) {
	tmp_str = NULL;

	new_setting = True;

	if ( *p == '-' ) {
	    p++;
	    new_setting = False;

	    if ( *p == '\0' ) {
		print_warning( CONFIGURATION_WARNING, 
			       "solitary `-' in debug parameter -- ignored." );
		continue;
	    }
	}

	if ( *p == '\0' ) {
	    continue;
	}


	if ( string_cmp_no_case( p, "all" ) == 0 ) {
	    for (i=0; i<NUM_DEBUG_MODES; i++) {
		debug_setting[i] = new_setting;
	    }
	} else {
	    for ( i=0; i<NUM_DEBUG_MODES; i++ ) {
		if ( string_cmp_no_case( p, debug_desc[i] ) == 0 ) {
		    debug_setting[i] = new_setting;
		    break;
		}
	    }

	    if ( i == NUM_DEBUG_MODES ) {
		print_warning( CONFIGURATION_WARNING,
			       "unrecognized debug mode `%s'", p );
	    }
	}
    }

#endif /* TUXRACER_NO_DEBUG */

}

bool_t debug_mode_is_active( debug_mode_t mode )
{
    return debug_setting[ mode ];
}

void print_debug( debug_mode_t mode, char *fmt, ... )
{
#ifndef TUXRACER_NO_DEBUG

    va_list args;

    check_assertion( 0 <= mode && mode < NUM_DEBUG_MODES,
		     "invalid debugging mode" );

    if ( ! debug_mode_is_active( mode ) ) {
	return;
    }

    va_start( args, fmt );

    fprintf( stderr, PROG_NAME " debug (%s): ", debug_desc[ mode ] );
    vfprintf( stderr, fmt, args );
    fprintf( stderr, "\n" );

    va_end( args );

#endif /* TUXRACER_NO_DEBUG */
}

