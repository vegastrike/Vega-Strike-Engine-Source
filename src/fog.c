/* 
 * Vega Strike 
 * Copyright (C) 2001-2002 Daniel
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

#include "vegastrike.h"
#include "fog.h"
#include "gl_util.h"


static fog_t fog_settings;
static bool_t enabled = True;

void reset_fog()
{
    fog_settings.is_on = True;
    fog_settings.mode = GL_LINEAR;
    fog_settings.density = 0.005;
    fog_settings.start = 0.0;
    fog_settings.end = getparam_forward_clip_distance();
    init_glfloat_array( 4, fog_settings.colour, 1.0, 1.0, 1.0, 1.0 );;
}

void setup_fog()
{
    if ( !fog_settings.is_on ) {
	glDisable( GL_FOG );
	enabled = False;
	return;
    }

    glEnable( GL_FOG );
    enabled = True;

    glFogi( GL_FOG_MODE, fog_settings.mode );
    glFogf( GL_FOG_DENSITY, fog_settings.density );
    glFogf( GL_FOG_START, fog_settings.start );
    glFogf( GL_FOG_END, fog_settings.end );
    glFogfv( GL_FOG_COLOR, fog_settings.colour );

    if ( getparam_nice_fog() ) {
	glHint( GL_FOG_HINT, GL_NICEST );
    } else {
	glHint( GL_FOG_HINT, GL_FASTEST );
    }
}

void disable_fog()
{
    glDisable( GL_FOG );
    enabled = False;
}

bool_t is_fog_on()
{
    return fog_settings.is_on && enabled;
}

GLfloat* get_fog_colour()
{
    return fog_settings.colour;
}

/**
static int fog_cb (ClientData cd, Tcl_Interp *ip, 
		   int argc, char *argv[]) 
{
    scalar_t tmp_arr[4];
    double tmp_dbl;
    bool_t error = False;
    
    if (argc < 2) {
	error = True;
    }

    NEXT_ARG;

    while ( !error && argc > 0 ) {

	if ( strcmp( "-on", *argv ) == 0 ) {
	    fog_settings.is_on = True;
	} else if ( strcmp( "-off", *argv ) == 0 ) {
	    fog_settings.is_on = False;
	} else if ( strcmp( "-mode", *argv ) == 0 ) {
	    NEXT_ARG;
	    if ( argc == 0 ) {
		error = True;
		break;
	    }

	    if ( strcmp( "exp", *argv ) == 0 ) {

		fog_settings.mode = GL_EXP;

	    } else if ( strcmp( "exp2", *argv ) == 0 ) {

		fog_settings.mode = GL_EXP2;

	    } else if ( strcmp( "linear", *argv ) == 0 ) {

		fog_settings.mode = GL_LINEAR;

	    } else {
		print_warning( TCL_WARNING, "tux_fog: mode must be one of "
			       "`exp', `exp2', or `linear'" );
		error = True;
	    }

	} else if ( strcmp( "-density", *argv ) == 0 ) {
	    NEXT_ARG;
	    if ( argc == 0 ) {
		error = True;
		break;
	    }
	    if ( Tcl_GetDouble ( ip, *argv, &tmp_dbl ) == TCL_ERROR ) {
		error = True;
		break;
	    }
	    fog_settings.density = tmp_dbl;
	} else if ( strcmp( "-start", *argv ) == 0 ) {
	    NEXT_ARG;
	    if ( argc == 0 ) {
		error = True;
		break;
	    }
	    if ( Tcl_GetDouble ( ip, *argv, &tmp_dbl ) == TCL_ERROR ) {
		error = True;
		break;
	    }
	    fog_settings.start = tmp_dbl;
	} else if ( strcmp( "-end", *argv ) == 0 ) {
	    NEXT_ARG;
	    if ( argc == 0 ) {
		error = True;
		break;
	    }
	    if ( Tcl_GetDouble ( ip, *argv, &tmp_dbl ) == TCL_ERROR ) {
		error = True;
		break;
	    }
	    fog_settings.end = tmp_dbl;
	} else if ( strcmp( "-colour", *argv ) == 0 ||
	     strcmp( "-color",  *argv ) == 0 ) 
	{
	    NEXT_ARG;
	    if ( argc == 0 ) {
		error = True;
		break;
	    }
	    if ( get_tcl_tuple ( ip, *argv, tmp_arr, 4 ) == TCL_ERROR ) {
		error = True;
		break;
	    }
	    copy_to_glfloat_array( fog_settings.colour, tmp_arr, 4 );
	} else {
	    print_warning( TCL_WARNING, "tux_fog: unrecognized "
			   "parameter `%s'", *argv );
	}

	NEXT_ARG;
    }

    if ( error ) {
	print_warning( TCL_WARNING, "error in call to tux_fog" );
	Tcl_AppendResult(
	    ip, 
	    "\nUsage: tux_fog [-on|-off] "
	    "[-mode [exp|exp2|linear]] "
	    "[-density <value>] "
	    "[-start <value>] "
	    "[-end <value>] "
	    "[-colour { r g b a }] ",
	    (char *) 0 );
	return TCL_ERROR;
    }
    
    return TCL_OK;
}

void register_fog_callbacks( Tcl_Interp *ip )
{
    Tcl_CreateCommand (ip, "tux_fog", fog_cb,  0,0);
}
*/
