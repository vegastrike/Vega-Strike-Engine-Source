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

#include "vegastrike.h"
#include "string_util.h"

char *string_copy( char *src ) 
{
    char *dest;

    check_assertion( src != NULL, "string NULL in string_copy" );

    dest = (char *) malloc( strlen( src ) + 1 );

    if ( dest == NULL ) {
	handle_system_error( 1, "malloc failed" );
    }

    strcpy( dest, src );
    return dest;
}

int string_cmp_no_case( char *s1, char *s2 )
{
    char *s1c, *s2c;
    int retval;

    check_assertion( s1 != NULL && s2 != NULL, 
		     "string NULL in string_cmp_no_case" );

    s1c = string_copy( s1 );
    s2c = string_copy( s2 );
    string_to_lower( s1c );
    string_to_lower( s2c );
    retval = strcmp( s1c, s2c );
    free( s1c );
    free( s2c );
    return retval;
}

void string_to_lower( char *s )
{
    int i;
    int l;

    check_assertion( s != NULL, "string NULL in string_to_lower" );

    l = strlen(s);

    for (i=0; i<l; i++) {
	s[i] = tolower(s[i]);
    }
}
