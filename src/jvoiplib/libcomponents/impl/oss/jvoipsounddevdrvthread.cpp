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

#include "jvoipsounddevdrvthread.h"
#include "jvoipsounddeviceblock.h"
#include "jvoiperrors.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <iostream>

#include "debugnew.h"

#define JVOIPSOUNDDEVDRVTHREAD_FRAGMENTTIME					10 // ten milliseconds

// Make sure these are multiples of 4 !!!
#define JVOIPSOUNDDEVDRVTHREAD_FRAGMENTMIN					128
#define JVOIPSOUNDDEVDRVTHREAD_FRAGMENTMAX					4096

JVOIPSoundDevDrvThread::JVOIPSoundDevDrvThread(int filedesc,int currentsamplingrate) throw (JVOIPException)
{
	filedescriptor = filedesc;
	stopthread = false;
	firstbuffer = NULL;
	lastbuffer = NULL;
	if (sigwait.Init() < 0)
		throw JVOIPException(ERR_JVOIPLIB_GENERAL_CANTINITSIGWAIT);
	if (flagmutex.Init() < 0)
		throw JVOIPException(ERR_JVOIPLIB_GENERAL_CANTINITMUTEX);
	if (stopmutex.Init() < 0)
		throw JVOIPException(ERR_JVOIPLIB_GENERAL_CANTINITMUTEX);
	if (buffermutex.Init() < 0)
		throw JVOIPException(ERR_JVOIPLIB_GENERAL_CANTINITMUTEX);
	if (loopmutex.Init() < 0)
		throw JVOIPException(ERR_JVOIPLIB_GENERAL_CANTINITMUTEX);
	if (resetmutex.Init() < 0)
		throw JVOIPException(ERR_JVOIPLIB_GENERAL_CANTINITMUTEX);
	resetting = false;	

	CalculateFragmentLength(currentsamplingrate);
}

JVOIPSoundDevDrvThread::~JVOIPSoundDevDrvThread()
{
	StopThread();
}

int JVOIPSoundDevDrvThread::AddBuffer(JVOIPSoundDeviceBlock *blk)
{
	buffermutex.Lock();
	if (firstbuffer == NULL)
		firstbuffer = blk;
	else
		lastbuffer->next = blk;
	lastbuffer = blk;
	blk->done = false;
	blk->flagmutex = &flagmutex;
	blk->next = NULL;
	sigwait.Signal();
	buffermutex.Unlock();
	return 0;
}

void JVOIPSoundDevDrvThread::StopThread()
{
	time_t tm;

	if (!IsRunning())
		return;
	
	Reset();
		
	stopmutex.Lock();
	stopthread = true;
	stopmutex.Unlock();
	sigwait.Signal(); // to make sure that the thread loops
	tm = time(NULL);
	while (IsRunning() && (time(NULL)-tm) < 5) // wait max 5 seconds
		;
	
	if (IsRunning())
	{
		Kill();
		std::cerr << "JVOIPSoundDevDrvThread::StopThread -- Warning: killing thread" << std::endl;
	}
		
	ClearBuffers();
}

void JVOIPSoundDevDrvThread::Reset()
{
	stopmutex.Lock(); // to make sure the thread waits for a loop

	resetmutex.Lock();
	resetting = true;
	resetmutex.Unlock();
	
	sigwait.Signal(); // to avoid being trapped
	loopmutex.Lock(); // wait till we're at the end of the loop
	loopmutex.Unlock(); // the stopmutex will still pause the thread

	// Clear all buffers
	ClearBuffers();
	
	sigwait.ClearSignalBuffers();
	
	
	resetmutex.Lock();
	resetting = false;
	resetmutex.Unlock();
	
	stopmutex.Unlock();
}

void JVOIPSoundDevDrvThread::ClearBuffers()
{
	JVOIPSoundDeviceBlock *tmp;	
	
	buffermutex.Lock(); 	
	tmp = firstbuffer;
	while (tmp)
	{
		flagmutex.Lock();
		tmp->done = true;
		flagmutex.Unlock();
		tmp = tmp->next;
	}
	firstbuffer = NULL;
	lastbuffer = NULL;
	buffermutex.Unlock();
}

void JVOIPSoundDevDrvThread::CalculateFragmentLength(int rate)
{
	double samples;	
	int diff;
	
	samples = (double)rate;
	samples /= 1000.0;
	samples *= (double)JVOIPSOUNDDEVDRVTHREAD_FRAGMENTTIME;

	fragmentlength = (int)(samples*4.0); // *2 for sixteen bit; again *2 for stereo
	if (fragmentlength < JVOIPSOUNDDEVDRVTHREAD_FRAGMENTMIN)
		fragmentlength = JVOIPSOUNDDEVDRVTHREAD_FRAGMENTMIN;
	else if (fragmentlength > JVOIPSOUNDDEVDRVTHREAD_FRAGMENTMAX)
		fragmentlength = JVOIPSOUNDDEVDRVTHREAD_FRAGMENTMAX;

	// Make sure the framgmentlength is a multiple of four, since we're working with
	// sixteen bit samples and with stereo sound! 
	if ((diff = fragmentlength%4) != 0)
		fragmentlength -= diff;
}

JVOIPSoundDevDrvInputThread::JVOIPSoundDevDrvInputThread(int filedesc,int currentsamplingrate) throw (JVOIPException) : JVOIPSoundDevDrvThread(filedesc,currentsamplingrate)
{
	if (callbackmutex.Init() < 0)
		throw JVOIPException(ERR_JVOIPLIB_GENERAL_CANTINITMUTEX);
	inputcallback = NULL;
	inputcallbackparam = NULL;
}

void JVOIPSoundDevDrvInputThread::SetInputCallback(void (*f)(void *param),void *param)
{
	callbackmutex.Lock();
	inputcallback = f;
	inputcallbackparam = param;
	callbackmutex.Unlock();
}

void *JVOIPSoundDevDrvInputThread::Thread()
{
	bool stop;
	int blklen;
	JVOIPSoundDeviceBlock *blk;
	int bytestoread,bytesleft;
	unsigned char *buf;
	bool done;

	stopmutex.Lock();
	stop = stopthread;
	stopmutex.Unlock();
	while (!stop)
	{
		loopmutex.Lock();
		
		sigwait.WaitForSignal(); // wait until a buffer is available
		buffermutex.Lock();
		blk = firstbuffer;
		if (firstbuffer)
		{
			firstbuffer = firstbuffer->next;
			if (!firstbuffer)
				lastbuffer = NULL;
		}
		buffermutex.Unlock();
		if (blk)
		{
			if ((blklen = blk->datalen) > 0 && blk->data != NULL)
			{
				bytesleft = blklen;
				buf = blk->data;
				while (bytesleft > 0)
				{
					resetmutex.Lock();
					done = resetting;
					resetmutex.Unlock();
					if (done)
						break;
					
					bytestoread = (bytesleft > fragmentlength)?fragmentlength:bytesleft;
					read(filedescriptor,buf,bytestoread);
					buf += bytestoread;
					bytesleft -= bytestoread;
				}
			}

			flagmutex.Lock();
			blk->done = true;
			flagmutex.Unlock();
			callbackmutex.Lock();
			if (inputcallback)
				inputcallback(inputcallbackparam);
			callbackmutex.Unlock();
		}
		loopmutex.Unlock();
		
		stopmutex.Lock();
		stop = stopthread;
		stopmutex.Unlock();
	}
	return NULL;
}

void *JVOIPSoundDevDrvOutputThread::Thread()
{
	bool stop;
	int blklen;
	JVOIPSoundDeviceBlock *blk;
	int bytestowrite,bytesleft;
	unsigned char *buf;
	bool done;
	
	stopmutex.Lock();
	stop = stopthread;
	stopmutex.Unlock();
	while (!stop)
	{
		loopmutex.Lock();
		
		sigwait.WaitForSignal(); // wait until a buffer is available
		buffermutex.Lock();
		blk = firstbuffer;
		if (firstbuffer)
		{
			firstbuffer = firstbuffer->next;
			if (!firstbuffer)
				lastbuffer = NULL;
		}
		buffermutex.Unlock();
		if (blk)
		{
			if ((blklen = blk->datalen) > 0 && blk->data != NULL)
			{
				bytesleft = blklen;
				buf = blk->data;
				while (bytesleft > 0)
				{
					resetmutex.Lock();
					done = resetting;
					resetmutex.Unlock();
					if (done)
						break;
					
					bytestowrite = (bytesleft > fragmentlength)?fragmentlength:bytesleft;
					write(filedescriptor,buf,bytestowrite);
					buf += bytestowrite;
					bytesleft -= bytestowrite;
				}
			}
			flagmutex.Lock();
			blk->done = true;
			flagmutex.Unlock();
		}
		loopmutex.Unlock();
	
		stopmutex.Lock();
		stop = stopthread;
		stopmutex.Unlock();
	}
	return NULL;
}

