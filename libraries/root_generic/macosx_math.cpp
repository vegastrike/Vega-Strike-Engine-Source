/*
 * Copyright (C) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike contributors.
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
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */

#include <macosx_math.h>
#include <string>
#include "posh.h"
#if defined (__APPLE__)
//these stuffs are included in OSX 10.4 and above--so just check for x86
#include <stdio.h>
    #include <string.h>
    #include <assert.h>
    #include <sys/param.h>
    #include <stdlib.h>
    #include <crt_externs.h>
    #include <errno.h>
    #include <mach-o/dyld.h>

typedef int (*NSGetExecutablePathProcPtr)( char *buf, size_t *bufsize );

static int NSGetExecutablePathOnTenOneAndEarlierOnly( char *execPath, size_t *execPathSize )
{
    int    err = 0;
    char **cursor;
    char  *possiblyRelativePath;
    char   absolutePath[MAXPATHLEN];
    size_t absolutePathSize;

    assert( execPath != NULL );
    assert( execPathSize != NULL );

    cursor = (char**) ( *( _NSGetArgv() )+*( _NSGetArgc() ) );
    //There should be a NULL after the argv array.
    //If not, error out.
    if (*cursor != 0)
        err = -1;
    if (err == 0) {
        //Skip past the NULL after the argv array.

        cursor += 1;
        //Now, skip over the entire kernel-supplied environment,
        //which is an array of char * terminated by a NULL.
        while (*cursor != 0)
            cursor += 1;
        //Skip over the NULL terminating the environment.

        cursor += 1;

        //Now we have the path that was passed to exec
        //(not the argv[0] path, but the path that the kernel
        //actually executed).

        possiblyRelativePath = *cursor;
        //Convert the possibly relative path to an absolute
        //path.  We use realpath for expedience, although
        //the real implementation of _NSGetExecutablePath
        //uses getcwd and can return a path with symbolic links
        //etc in it.
        if (realpath( possiblyRelativePath, absolutePath ) == NULL)
            err = -1;
    }
    //Now copy the path out into the client's buffer, returning
    //an error if the buffer isn't big enough.
    if (err == 0) {
        absolutePathSize = (strlen( absolutePath )+1);
        if (absolutePathSize <= *execPathSize)
            strcpy( execPath, absolutePath );
        else
            err = -1;
        *execPathSize = absolutePathSize;
    }
    return err;
}

extern "C" {
int XNSGetExecutablePath( char *execPath, size_t *execPathSize )
{
    if ( NSIsSymbolNameDefined( "__NSGetExecutablePath" ) )
        return ( (NSGetExecutablePathProcPtr) NSAddressOfSymbol( NSLookupAndBindSymbol( "__NSGetExecutablePath" ) ) )( execPath,
                                                                                                                       execPathSize );
    else
        //The function _NSGetExecutablePath() is new in Jaguar, so use this custom version when running on 10.1.x and earlier.
        return NSGetExecutablePathOnTenOneAndEarlierOnly( execPath, execPathSize );
}
}

#endif

int float_to_int(float a) {
    int maxint = 0x7ffffff;
    int minint = -0x8000000;
    if ((a < maxint) && (a > minint)) {
        return int(a);
    }
    if (a > 0) {
        return maxint;
    }
    if (a < 0) {
        return minint;
    }
    return 0;
}

int double_to_int(double a) {
    int maxint = 0x7ffffff;
    int minint = -0x8000000;
    if ((a < maxint) && (a > minint)) {
        return int(a);
    }
    if (a > 0) {
        return maxint;
    }
    if (a < 0) {
        return minint;
    }
    return 0;
}

#ifdef WITH_MACOSX_BUNDLE
#if (defined (__APPLE__) || defined (MACOSX ) ) && !defined (POSH_LITTLE_ENDIAN)
extern "C" {
char * ctermid_r( char *buf )
{
    if (buf) {
        buf[0] = '/';
        buf[1] = 'd';
        buf[2] = 'e';
        buf[3] = 'v';
        buf[4] = '/';
        buf[5] = 't';
        buf[6] = 't';
        buf[7] = 'y';
        buf[8] = '\0';
    } else {
        static char ret[] = "/dev/tty";
        return ret;
    }
}

float acosf( float a )
{
    return acos( a );
}
float asinf( float a )
{
    return asin( a );
}
long lrintf( float a )
{
    return (long) round( a );
}
long lrint( double a )
{
    return (long) round( a );
}
long long llrintf( float a )
{
    return (long) round( a );
}
long long llrint( double a )
{
    return (long) round( a );
}
long long llrintl( long double a )
{
    return (long) round( a );
}

float sqrtf( float v )
{
    return (float) sqrt( (double) v );
}

float cosf( float v )
{
    return (float) cos( (double) v );
}

float sinf( float v )
{
    return (float) sin( (double) v );
}

float tanf( float v )
{
    return (float) tan( (double) v );
}

float powf( float v, float p )
{
    return (float) pow( (double) v, (double) p );
}
long long atoll( const char *a )
{
    long long retval = 0;
    bool negatory    = false;
    if (a[0] == '-') negatory = true;
    ++a;
    while (*a) {
        retval *= 10;
        retval += *a-'0';
        ++a;
    }
    return retval;
}
#define sockaddr void
#define socklen_t size_t
#define restrict
int getnameinfo( const sockaddr*restrict sa,
                 socklen_t salen,
                 char*restrict node,
                 socklen_t nodelen,
                 char*restrict service,
                 socklen_t servicelen,
                 int flags )
{
    return 1;
}
}
#endif
#endif

