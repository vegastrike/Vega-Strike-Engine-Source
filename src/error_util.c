/* 
 * Vega Strike 
 * Copyright (C) 2000-2001 Daniel Horn
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

#include <stdarg.h>
#include "error_util.h"
#include "vegastrike.h"
#include "string_util.h"

/* A note on warning levels:
   A warning level of 0 prints nothing
   A warning level of 100 prints everything

   1 is reserved for critical warnings -- things that the user really
   shouldn't do but that we can continue from anyway. */

void print_warning( int warning_level, char *fmt, ... )
{
    va_list args;

    /* We enforce the fact that warning level 0 prints no warnings */
    check_assertion( warning_level > 0, "warning levels must be > 0" );
    check_assertion( warning_level <= 100, "warning levels must be <= 100" );

    if ( warning_level > g_game.warning_level ) {
	return;
    }

    va_start( args, fmt );

    fprintf( stderr, "%%%%%% " PROG_NAME " warning: " );
    vfprintf( stderr, fmt, args );
    fprintf( stderr, "\n" );

    va_end( args );
}

void handle_error( int exit_code, char *fmt, ... )
{
    va_list args;

    va_start( args, fmt );

    fprintf( stderr, "*** " PROG_NAME " error: " );
    vfprintf( stderr, fmt, args );
    fprintf( stderr, "\n" );

    va_end( args );

    exit( exit_code );
}

void handle_system_error( int exit_code, char *fmt, ... )
{
    va_list args;

    va_start( args, fmt );

    fprintf( stderr, "*** " PROG_NAME " error: " );
    vfprintf( stderr, fmt, args );
    fprintf( stderr, " (%s)\n", strerror( errno ) );

    va_end( args );

    exit( exit_code );
}
