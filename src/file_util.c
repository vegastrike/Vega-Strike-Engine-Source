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
#include "file_util.h"

#if defined( __CYGWIN__ )
#  include <sys/cygwin.h>
#endif

bool_t file_exists( char *filename )
{
#if defined( WIN32 ) && !defined( __CYGWIN__ )

    /* Test existence by opening file -- I'm not a Win32 programmer,
       so if there's a better way let me know */
    FILE *file;
    file = fopen( filename, "r" );

    if ( file == NULL ) {
	return False;
    } else {
	if ( fclose( file ) != 0 ) {
	    handle_error( 1, "error closing file %s", filename );
	}
	return True;
    }

#else

    /* Unix/Linux/Cygwin */
    
    struct stat stat_info;
    bool_t file_exists = False;

#  if defined( __CYGWIN__ )

    /* If cygwin we need to convert from DOS paths (used in data files)
       to posix paths */
    char filename_copy[MAX_PATH];
    cygwin_conv_to_posix_path( filename, filename_copy );

#  else

    /* Create a dummy copy to keep things consistent with the cygwin
       case -- yuck! */
    char *filename_copy;
    filename_copy = string_copy( filename );

#  endif /* defined( __CYGWIN__ ) */

    if ( stat( filename_copy, &stat_info ) != 0 ) {
	if ( errno != ENOENT ) {
	    handle_system_error(1, "couldn't stat %s", filename_copy);
	}
	file_exists = False;
    } else {
	file_exists = True;
    }

#  if !defined( __CYGWIN__ )

    /* Free up dummy copy -- double yuck! */
    free( filename_copy );

#  endif /* !defined( __CYGWIN__ ) */

    return file_exists;

#endif /* defined( WIN32 ) && !defined( __CYGWIN__ ) */
}
