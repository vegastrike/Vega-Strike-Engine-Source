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
#include "in_kb.h"
static double firsttime;
#ifdef WIN32
#include <windows.h>
static LONGLONG ttime;
static LONGLONG newtime = 0;
static LONGLONG freq;
static double dblnewtime;
#else
#if defined( HAVE_SDL )
#   include <SDL/SDL.h>
#endif /* defined( HAVE_SDL ) */
static double newtime;
static double lasttime;

#include <sys/time.h>
#include <sys/types.h>
#endif
static double elapsedtime=.1;
static double timecompression=1;
double getNewTime() {
#ifdef _WIN32
	return dblnewtime-firsttime;
#else
	return newtime-firsttime;
#endif
}

int timecount;

void inc_time_compression (int i, KBSTATE a) {
  if (a==PRESS) {
    timecompression*=1.5;
    timecount++;
  }
}
void dec_time_compression (int i, KBSTATE a) {
  if (a==PRESS) {
    timecompression/=1.5;
    timecount--;
  }
}
void reset_time_compression (int i, KBSTATE a) {
  if (a==PRESS) {
    timecompression=1;
    timecount=0;
  }
}
void pause_key (int i, KBSTATE a) {
  static bool paused=false;
  if (a==PRESS) {
    if (paused==false) {
      timecompression=.0000001;
      timecount=0;
      paused=true;
    }else {
      paused=false;
      reset_time_compression(i,a);
    }
  }
}

float getTimeCompression () {
  return timecompression;
}
void setTimeCompression(float tc){
  timecompression=tc;
  timecount=0;//to avoid any problems with time compression sounds... use getTimeCompression() instead
}

#ifdef _WIN32
#include <windows.h>


void micro_sleep(unsigned int n) {
	Sleep(n / 1000);

	return;
}
#elif defined(IRIX)
#include <unistd.h>

void micro_sleep(unsigned int n) {
	(void) usleep((useconds_t)n);
	return;
}

#else

void micro_sleep(unsigned int n) {
	struct timeval tv = { 0, 0 };

	tv.tv_usec = n%1000000;
	tv.tv_sec=n/1000000;
	select(0, NULL, NULL, NULL, &tv);

	return;
}
#endif

void InitTime () {
#ifdef WIN32
  QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
  QueryPerformanceCounter((LARGE_INTEGER*)&ttime);

#elif defined(HAVE_GETTIMEOFDAY)
  struct timeval tv;
  (void) gettimeofday(&tv, NULL);

  newtime = (double)tv.tv_sec + (double)tv.tv_usec * 1.e-6;
  lasttime = newtime -.001;

#elif defined(HAVE_SDL)
  newtime = SDL_GetTicks() * 1.e-3;
  lasttime = newtime -.001;

#else
# error "We have no way to determine the time on this system."
#endif
  elapsedtime = .001;
}

double GetElapsedTime() {
  return elapsedtime;
}

void UpdateTime() {
#ifdef WIN32
  QueryPerformanceCounter((LARGE_INTEGER*)&newtime);
  elapsedtime = ((double)(newtime-ttime))/freq;
  ttime = newtime;
  if( freq==0)
	  dblnewtime = 0.;
  else
	  dblnewtime = ((double)newtime)/((double)freq);
  static double ftime = firsttime = dblnewtime;
#elif defined(HAVE_GETTIMEOFDAY)
  struct timeval tv;
  (void) gettimeofday(&tv, NULL);

  lasttime = newtime;
  newtime = (double)tv.tv_sec + (double)tv.tv_usec * 1.e-6;
  elapsedtime = newtime-lasttime;
  static double ftime = firsttime = newtime;
#elif defined(HAVE_SDL)
  lasttime = newtime;
  newtime = SDL_GetTicks() * 1.e-3;
  elapsedtime = newtime-lasttime;
  static double ftime = firsttime = newtime;
#else
# error "We have no way to determine the time on this system."
#endif

  elapsedtime *=timecompression;
}
