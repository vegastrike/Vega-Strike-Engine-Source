/* 
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn
 * 
 * http://vegastrike.sourceforge.net/
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
 */


#include "vegastrike.h"

#ifdef WIN32
#include <windows.h>
static LONGLONG ttime;
static LONGLONG newtime = 0;
static LONGLONG freq;
#else
#if defined( HAVE_SDL )
#   include "SDL.h"
#endif /* defined( HAVE_SDL ) */
static double newtime;
static double lasttime;
#endif
static double elapsedtime;







double get_clock_time()
{
#if defined( HAVE_GETTIMEOFDAY )

    struct timeval tv;
    gettimeofday( &tv, NULL );

    return (double) tv.tv_sec + (double) tv.tv_usec * 1.e-6;
#elif defined (WIN32)
	return 0;
	//We're cool
#elif defined( HAVE_SDL ) 

    return SDL_GetTicks() * 1.e-3;

#else

#   error "We have no way to determine the time on this system."

#endif /* defined( HAVE_GETTIMEOFDAY ) */
} 

void InitTime () {
#ifdef WIN32
  QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
  QueryPerformanceCounter((LARGE_INTEGER*)&ttime);
#else
  newtime = get_clock_time();
  lasttime = newtime -.001;
#endif
  elapsedtime = .001;
}
double GetElapsedTime() {
  return elapsedtime;
}
void UpdateTime() {
#ifdef WIN32
  QueryPerformanceCounter((LARGE_INTEGER*)&newtime);
  elapsedtime = ((float)(newtime-ttime))/freq;
  ttime = newtime;
#else
  lasttime = newtime;
  newtime = get_clock_time();
  elapsedtime =newtime-lasttime;
#endif
}
