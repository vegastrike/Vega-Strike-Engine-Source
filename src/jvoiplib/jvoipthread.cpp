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

#include "jvoipthread.h"
#include "jvoipsession.h"
#include "jvoiperrors.h"
#include "jvoipsigwait.h"
#include "voicecall.h"
#include <stdlib.h>
#include <time.h>
#include <iostream>

#include "debugnew.h"

using namespace VoIPFramework;

JVOIPThread::JVOIPThread(JVOIPSession &sess) throw (JVOIPException) : voipsession(sess)
{
	if (stopmutex.Init() < 0)
		throw JVOIPException(ERR_JVOIPLIB_GENERAL_CANTINITMUTEX);
	if (sigwaitmutex.Init() < 0)
		throw JVOIPException(ERR_JVOIPLIB_GENERAL_CANTINITMUTEX);
	waitingforsignal = false;
}

JVOIPThread::~JVOIPThread()
{
}

void *JVOIPThread::Thread()
{
	VoiceCall &voicecall = voipsession.GetVoiceCall();
	JMutex &componentmutex = voipsession.GetComponentMutex();
	bool done,intervalpassed;
	int componenterr,status;

	stopmutex.Lock();
	stopthread = false;
	stopmutex.Unlock();
	
	sigwaitmutex.Lock();
	waitingforsignal = false;
	sigwaitmutex.Unlock();
		
	done = false;
	while (1)
	{
		intervalpassed = false;
		
		componentmutex.Lock();
		
		stopmutex.Lock();
		done = stopthread;
		stopmutex.Unlock();
		if (done)
		{
			componentmutex.Unlock();
			return NULL;
		}
		
		status = voicecall.Step(&componenterr,&intervalpassed);
		if (status < 0)
		{
			voipsession.ThreadFinished(ERR_JVOIPLIB_VOIPTHREAD_VOICECALLERROR,status,componenterr);
			componentmutex.Unlock();
			return NULL;
		}
		
		// perform interval action if necessary
		if (intervalpassed)
		{
			bool ok,waitforsignal;
			
			waitforsignal = true;
			ok = voipsession.IntervalAction(&waitforsignal);
			if (!ok)
			{
				voipsession.ThreadFinished(ERR_JVOIPLIB_VOIPTHREAD_INTERVALACTIONERROR);
				componentmutex.Unlock();
				return NULL;
			}
			
			// wait till we get a signal that the timing interval passed
			sigwait = voipsession.GetSignalWaiter();
			if (sigwait && waitforsignal)
			{
				sigwaitmutex.Lock();
				waitingforsignal = true;
				sigwaitmutex.Unlock();
			
				// we unlock it here to make sure that 'waitingforsignal'
				// is set when we obtain a lock to 'componentmutex' in
				// JVOIPSession
				componentmutex.Unlock();
				
				sigwait->WaitForSignal();
				
				sigwaitmutex.Lock();
				waitingforsignal = false;
				sigwaitmutex.Unlock();
			}
			else
				componentmutex.Unlock();
		}
		else
			componentmutex.Unlock();
	}
	return NULL;
}

void JVOIPThread::SignalStop()
{
	if (!IsRunning())
		return;

	stopmutex.Lock();
	stopthread = true;
	stopmutex.Unlock();
}

void JVOIPThread::WaitForStop()
{
	time_t starttime;
	
	if (!IsRunning())
		return;

	starttime = time(NULL);	
	while (IsRunning() && (time(NULL)-starttime) < 5) // wait maximum 5 seconds for thread to finish
	{
		if (IsWaitingForSignal())
			sigwait->Signal();
	}
	
	// if it isn't finished by now, it probably never will. So we'll just kill the thread...
	if (IsRunning())
	{
		Kill();
		std::cerr << "JVOIPThread::WaitForStop -- Warning: killing thread!" << std::endl;
	}
}

bool JVOIPThread::IsWaitingForSignal()
{
	bool v;
	
	sigwaitmutex.Lock();
	v = waitingforsignal;
	sigwaitmutex.Unlock();
	return v;
}
