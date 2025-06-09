/*
 * debug_vs.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
 *
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef VS_DEBUG

#else
#include <stdio.h>

#include "src/hashtable.h"
#if defined (_MSC_VER) && defined (_DEBUG)
#include <crtdbg.h>
#endif
Hashtable< long, char, 65535 >constructed;

Hashtable< long, char, 65535 >destructed;
bool DEBUG_ERROR_IN_MY_CODE = true;
void VS_DEBUG_ERROR()
{
    VSFileSystem::Fprintf( stderr, "WARNING: invalid refcount in vegastrike object\n" );
#if defined (_MSC_VER) && defined (_DEBUG)
    if (DEBUG_ERROR_IN_MY_CODE)
        _RPT0( _CRT_ERROR, "WARNING: invalid refcount in vegastrike object\n" );
    return;
#endif
    while (DEBUG_ERROR_IN_MY_CODE)
        printf( "ack" );
}
char * Constructed( void *v )
{
    return constructed.Get( (long) v );
}
char * Destructed( void *v )
{
    return destructed.Get( (long) v );
}
#endif

