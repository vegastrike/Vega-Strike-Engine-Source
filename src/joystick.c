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
 * This code adapted from Tux Racer by Jasmin F. Patry www.tuxracer.com
 */

#include "vegastrike.h"
#include "joystick.h"

#if defined( HAVE_SDL ) && defined( HAVE_SDL_JOYSTICKOPEN )

#include "SDL.h"
#include "SDL_joystick.h"

SDL_Joystick *joystick = NULL;
int num_buttons = 0;

void init_joystick()
{
    int num_joysticks = 0;
    char *js_name;

    num_joysticks = SDL_NumJoysticks();

    print_debug( DEBUG_JOYSTICK, "Found %d joysticks", num_joysticks );

    if ( num_joysticks == 0 ) {
	joystick = NULL;
	return;
    }

    js_name = (char*) SDL_JoystickName( 0 );

    print_debug( DEBUG_JOYSTICK, "Using joystick `%s'", js_name );

    joystick = SDL_JoystickOpen( 0 );

    if ( joystick == NULL ) {
	print_debug( DEBUG_JOYSTICK, "Cannot open joystick" );
	return;
    }

    num_buttons = SDL_JoystickNumButtons( joystick );

    print_debug( DEBUG_JOYSTICK, "Joystick has %d buttons", num_buttons );
}

bool_t is_joystick_active()
{
    return (bool_t) ( joystick != NULL );
}

void update_joystick()
{
    SDL_JoystickUpdate();
}

scalar_t get_joystick_x_axis()
{
    check_assertion( joystick != NULL,
		     "joystick is null" );

    return SDL_JoystickGetAxis( joystick, 0 )/32768.0;
}

scalar_t get_joystick_y_axis()
{
    check_assertion( joystick != NULL,
		     "joystick is null" );

    return SDL_JoystickGetAxis( joystick, 1 )/32768.0;
}

bool_t is_joystick_button_down( int button ) 
{
    check_assertion( joystick != NULL,
		     "joystick is null" );

    check_assertion( button >= 0, "button is negative" );

    if ( button >= num_buttons ) {
	print_debug( DEBUG_JOYSTICK,
		     "WARNING: State of button %d requested, but "
		     "joystick only has %d buttons", button, num_buttons );
	return False;
    }

    return (bool_t) SDL_JoystickGetButton( joystick, button );
}

bool_t is_joystick_continue_button_down()
{
    if ( joystick == NULL ) {
	return False;
    }
    /*
    if ( getparam_joystick_continue_button() < 0 ) {
	return False;
    }

    return is_joystick_button_down( getparam_joystick_continue_button() );
    */
    return False;
}

#else

/* Stub functions */

void init_joystick()
{
}

bool_t is_joystick_active()
{
    return False;
}

void update_joystick()
{
}

scalar_t get_joystick_x_axis()
{
    return 0.0;
}

scalar_t get_joystick_y_axis()
{
    return 0.0;
}

bool_t is_joystick_button_down( int button ) 
{
    return False;
}

bool_t is_joystick_continue_button_down()
{
    return False;
}

#endif

/* EOF */
