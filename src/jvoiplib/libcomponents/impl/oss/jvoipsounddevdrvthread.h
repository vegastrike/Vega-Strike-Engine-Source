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

#ifndef JVOIPSOUNDDEVDRVTHREAD_H

#define JVOIPSOUNDDEVDRVTHREAD_H

#include "jthread.h"
#include "jmutex.h"
#include "jvoipexception.h"
#include "jvoipsigwait.h"

class JVOIPSoundDeviceBlock;

class JVOIPSoundDevDrvThread : public JThread
{
public:
	JVOIPSoundDevDrvThread(int filedesc,int currentsamprate) throw (JVOIPException);
	virtual ~JVOIPSoundDevDrvThread();
	void SetSoundDeviceDescriptor(int filedesc)							{ filedescriptor = filedesc; }
	int AddBuffer(JVOIPSoundDeviceBlock *blk);
	void StopThread();
	void Reset();
	void SetCurrentSamplingRate(int currentsamprate)						{ CalculateFragmentLength(currentsamprate); }
protected:
	void ClearBuffers();
	void CalculateFragmentLength(int samprate);

	int filedescriptor;
	int fragmentlength;
	JVOIPSignalWait sigwait;
	JMutex flagmutex;
	JMutex stopmutex;
	JMutex buffermutex;
	JMutex loopmutex;
	JMutex resetmutex;
	bool stopthread,resetting;
	JVOIPSoundDeviceBlock *firstbuffer,*lastbuffer;
};

class JVOIPSoundDevDrvInputThread : public JVOIPSoundDevDrvThread
{
public:
	JVOIPSoundDevDrvInputThread(int filedesc,int currentsamprate) throw (JVOIPException);
	virtual ~JVOIPSoundDevDrvInputThread() { }
	void *Thread();
	void SetInputCallback(void (*f)(void *param),void *param);
private:
	void (*inputcallback)(void *param);
	void *inputcallbackparam;
	JMutex callbackmutex;
};

class JVOIPSoundDevDrvOutputThread : public JVOIPSoundDevDrvThread
{
public:
	JVOIPSoundDevDrvOutputThread(int filedesc,int currentsamprate) throw (JVOIPException) : JVOIPSoundDevDrvThread(filedesc,currentsamprate) { }
	virtual ~JVOIPSoundDevDrvOutputThread() { }
	void *Thread();
};

#endif // JVOIPSOUNDDEVDRVTHREAD_H
