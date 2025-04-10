/**
* debug_vs.cpp
*
* Copyright (c) 2001-2002 Daniel Horn
* Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
* Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
*
* https://github.com/vegastrike/Vega-Strike-Engine-Source
*
* This file is part of Vega Strike.
*
* Vega Strike is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* Vega Strike is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
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

