/*

    This file is a part of JVOIPLIB, a library designed to facilitate
    the use of Voice over IP (VoIP).

    Copyright (C) 2000-2002  Jori Liesenborgs (jori@lumumba.luc.ac.be)

    This library (JVOIPLIB) is based upon work done for my thesis at
    the School for Knowledge Technology (Belgium/The Netherlands)

    The full GNU Library General Public License can be found in the
    file LICENSE.LGPL which is included in the source code archive.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
    USA

*/

#include "jvoipsimpletimer.h"
#include "jvoiperrors.h"
#include <time.h>
#include <iostream>

#ifndef WIN32
	#include <sys/time.h>

	inline void JVOIPSimpleTimer::GetTheCurrentTime(struct timeval *tv)
	{
		gettimeofday(tv,NULL);
	}

#else
	#include <sys/timeb.h>
	#include <windows.h>

	inline void JVOIPSimpleTimer::GetTheCurrentTime(struct timeval *tv)
	{
		struct _timeb tb;
		
	        _ftime(&tb);
       		tv->tv_sec = tb.time;
		tv->tv_usec = tb.millitm*1000;
	}

#endif // !WIN32

#include "debugnew.h"

#define JVOIPSIMPLETIMER_DEFAULTINTERVAL					10

JVOIPSimpleTimer::JVOIPSimpleTimer() throw (JVOIPException)
{
	if (countmutex.Init() < 0)
		throw JVOIPException(ERR_JVOIPLIB_GENERAL_CANTINITMUTEX);
	if (stopmutex.Init() < 0)
		throw JVOIPException(ERR_JVOIPLIB_GENERAL_CANTINITMUTEX);
	if (loopmutex.Init() < 0)
		throw JVOIPException(ERR_JVOIPLIB_GENERAL_CANTINITMUTEX);
	if (sigwait.Init() < 0)
		throw JVOIPException(ERR_JVOIPLIB_GENERAL_CANTINITSIGWAIT);
	interval = JVOIPSIMPLETIMER_DEFAULTINTERVAL;
}

JVOIPSimpleTimer::~JVOIPSimpleTimer()
{
	StopTimer();
}

int JVOIPSimpleTimer::SetSamplingInterval(int millisec)
{
	loopmutex.Lock();
	
	sigwait.ClearSignalBuffers();
	interval = millisec;
	firsttime = true;
	
	loopmutex.Unlock();
	return 0;
}

bool JVOIPSimpleTimer::HasTimeOut()
{
	bool val;
	
	if (!IsRunning())
		return true;
	countmutex.Lock();
	val = (timercount > 0)?true:false;
	if (val)
		timercount--;
	countmutex.Unlock();
	return val;
}

int JVOIPSimpleTimer::RestartTimer()
{
	if (!IsRunning())
	{
		stopthread = false;
		sigwait.ClearSignalBuffers();
		timercount = 0;
		firsttime = true;
		
		if (Start() < 0)
			return ERR_JVOIPLIB_SIMPLETIMER_CANTSTARTTHREAD;
	}
	return 0;
}

int JVOIPSimpleTimer::StopTimer()
{
	time_t tm;	

	if (!IsRunning())
		return 0;
	
	stopmutex.Lock();
	stopthread = true;
	stopmutex.Unlock();
	
	tm = time(NULL);
	while (IsRunning() && (time(NULL)-tm) < 2) // wait at most 2 seconds
		;
	if (IsRunning())
	{
		Kill();
		std::cerr << "JVOIPSimpleTimer::StopTimer -- Warning: killing thread" << std::endl;
	}
	return 0;
}

void *JVOIPSimpleTimer::Thread()
{
	bool stop;
	struct timeval starttv,curtime,nexttime,waittime;
	int timinginterval;
	double tm1,tm2,diff,diff2;
	
	stopmutex.Lock();
	stop = stopthread;
	stopmutex.Unlock();
	while (!stop)
	{
		loopmutex.Lock();
		if (firsttime)
		{
			firsttime = false;
			GetTheCurrentTime(&starttv);
			timercount = 0;
			timinginterval = (interval > 1)?interval:1;
			nexttime = starttv;
			nexttime.tv_usec += timinginterval*1000;
			while (nexttime.tv_usec >= 1000000)
			{
				nexttime.tv_usec -= 1000000;
				nexttime.tv_sec++;
			}
		}
		GetTheCurrentTime(&curtime);
		tm1 = (double)nexttime.tv_sec+((double)nexttime.tv_usec)/1000000.0;
		tm2 = (double)curtime.tv_sec+((double)curtime.tv_usec)/1000000.0;
		diff = tm1-tm2;
		if ((diff >= 0 && diff < 0.000500) || diff < 0)
		{
			nexttime.tv_usec += timinginterval*1000;
			while (nexttime.tv_usec >= 1000000)
			{
				nexttime.tv_usec -= 1000000;
				nexttime.tv_sec++;
			}
			countmutex.Lock();
			timercount++;
			countmutex.Unlock();
			sigwait.Signal();
		}
		else
		{
			if (diff > 0.100) // to make sure we loop at least every 100 ms
				diff = 0.100;
				
			waittime.tv_sec = (int)diff;
			diff2 = diff-waittime.tv_sec;
			waittime.tv_usec = (int)(diff2*1000000.0);
			while (waittime.tv_usec > 1000000)
			{
				waittime.tv_usec -= 1000000;
				waittime.tv_sec++;
			}
			select(FD_SETSIZE,NULL,NULL,NULL,&waittime);
		}
		
		loopmutex.Unlock();
	
		stopmutex.Lock();
		stop = stopthread;
		stopmutex.Unlock();
	}
	return NULL;
}

