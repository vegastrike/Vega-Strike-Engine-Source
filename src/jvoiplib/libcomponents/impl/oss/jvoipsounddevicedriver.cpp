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

#include "jvoipsounddevicedriver.h"
#include "jvoipsounddevice.h"
#include "jvoipsounddevdrvthread.h"
#include "jvoiperrors.h"
#include <sys/soundcard.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

#include "debugnew.h"

JVOIPSoundDeviceDriver::JVOIPSoundDeviceDriver()
{
	filedesc = -1;
	inputthread = NULL;
	outputthread = NULL;
	sdevread = NULL;
	sdevwrite = NULL;
	currentsamplingrate = 8000;
	readrequestrate = 8000;
	writerequestrate = 8000;
	reopenerror = false;
}

JVOIPSoundDeviceDriver::~JVOIPSoundDeviceDriver()
{
	if (inputthread)
		delete inputthread;
	if (outputthread)
		delete outputthread;
	if (filedesc > 0)
		close(filedesc);	
}

#define ERRCLOSE(x) 	{\
				close(f);\
				return x;\
			}
			

int JVOIPSoundDeviceDriver::Init(const std::string &devicename,int requestedrate)
{
	int f;
	
	if (filedesc > 0)
		return ERR_JVOIPLIB_SOUNDCARDIO_DRIVERALREADYINIT;

	// NOTE: when we get at this point, it might be possible that the input and
	// output threads already exist: after an unsuccessful attempt to reopen the
	// soundcard device, the filedesc is also < 0.
	
	if (reopenerror)
		return ERR_JVOIPLIB_SOUNDCARDIO_DRIVERSHOULDNTBEUSEDAFTERBADREOPEN;

	f = OpenDevice(devicename,requestedrate);
	if (f < 0)
		return f; // contains error code
	devname = devicename;
	filedesc = f;
	return 0;
}

int JVOIPSoundDeviceDriver::OpenDevice(const std::string &devicename,int reqsamprate)
{
	int f,val;
	int flags;
	
	// NOTE: Here, we've set the O_NONBLOCK, so the soundcard
	//       driver won't block till de device becomes available
	f = open(devicename.c_str(),O_RDWR/*|O_SYNC*/|O_NONBLOCK);
	if (f < 0)
		return ERR_JVOIPLIB_SOUNDCARDIO_CANTOPENDEVICE;

	// Now that we've opened the file, we can set the blocking
	// mode again
	flags = fcntl(f,F_GETFL);
	if (flags < 0)
		ERRCLOSE(ERR_JVOIPLIB_SOUNDCARDIO_CANTSETDRIVERPARAM);
	flags &= ~O_NONBLOCK; // disable non-blocking flags
	if (fcntl(f,F_SETFL,flags) < 0)
		ERRCLOSE(ERR_JVOIPLIB_SOUNDCARDIO_CANTSETDRIVERPARAM);

	// set internal dma block size and number of fragments
	val = 7 | (128<<16);
	if (ioctl(f,SNDCTL_DSP_SETFRAGMENT,&val) < 0)
		ERRCLOSE(ERR_JVOIPLIB_SOUNDCARDIO_CANTSETDRIVERPARAM);
	
	// reset sound device
	if (ioctl(f,SNDCTL_DSP_SYNC,NULL) < 0)
		ERRCLOSE(ERR_JVOIPLIB_SOUNDCARDIO_CANTSETDRIVERPARAM);
	
	// set to stereo
	val = 1;
	if (ioctl(f,SNDCTL_DSP_STEREO,&val) < 0)
		ERRCLOSE(ERR_JVOIPLIB_SOUNDCARDIO_CANTSETDRIVERPARAM);

	// set to sixteen bit samples
	val = 16;
	if(ioctl(f,SNDCTL_DSP_SAMPLESIZE,&val) < 0)
		ERRCLOSE(ERR_JVOIPLIB_SOUNDCARDIO_CANTSETDRIVERPARAM);
		
	// set sampling rate
	currentsamplingrate = reqsamprate;
	if (ioctl(f,SNDCTL_DSP_SPEED,&currentsamplingrate) < 0)
		ERRCLOSE(ERR_JVOIPLIB_SOUNDCARDIO_CANTSETDRIVERPARAM);
	ioctl(f,SOUND_PCM_READ_RATE,&currentsamplingrate);
		
	// set sample encoding to little endian and signed
	val = AFMT_S16_LE;
	if (ioctl(f,SNDCTL_DSP_SETFMT,&val) < 0)
		ERRCLOSE(ERR_JVOIPLIB_SOUNDCARDIO_CANTSETDRIVERPARAM);
	
	return f;
}

#undef ERRCLOSE

int JVOIPSoundDeviceDriver::Register(JVOIPSoundDevice *sdev,bool read)
{
	if (filedesc < 0)
	{
		if (reopenerror)
			return ERR_JVOIPLIB_SOUNDCARDIO_COULDNTREOPENDEVICE;		
		return ERR_JVOIPLIB_SOUNDCARDIO_DRIVERNOTINIT;
	}
	
	if (read)
	{
		if (sdevread)
			return ERR_JVOIPLIB_SOUNDCARDIO_DRIVERALREADYREGISTERED;
		inputthread = new JVOIPSoundDevDrvInputThread(filedesc,currentsamplingrate);
		if (inputthread == NULL)
			return ERR_JVOIPLIB_GENERAL_OUTOFMEM;
		if (inputthread->Start() < 0)
		{
			delete inputthread;
			return ERR_JVOIPLIB_SOUNDCARDIO_CANTSTARTTHREAD;
		}
		sdevread = sdev;
		readrequestrate = currentsamplingrate;
	}
	else
	{
		if (sdevwrite)
			return ERR_JVOIPLIB_SOUNDCARDIO_DRIVERALREADYREGISTERED;
		outputthread = new JVOIPSoundDevDrvOutputThread(filedesc,currentsamplingrate);
		if (outputthread == NULL)
			return ERR_JVOIPLIB_GENERAL_OUTOFMEM;
		if (outputthread->Start() < 0)
		{
			delete outputthread;
			return ERR_JVOIPLIB_SOUNDCARDIO_CANTSTARTTHREAD;
		}
		sdevwrite = sdev;
		writerequestrate = currentsamplingrate;
	}
	return 0;
}

bool JVOIPSoundDeviceDriver::Unregister(JVOIPSoundDevice *sdev)
{
	if (sdev == NULL)
		return false;
	if (sdevread == sdev)
	{
		sdevread = NULL;
		delete inputthread;
		inputthread = NULL;
	}
	else if (sdevwrite == sdev)
	{
		sdevwrite = NULL;
		delete outputthread;
		outputthread = NULL;
	}
	else
		return false;
		
	// After unregistering, check if we can set the sampling rate to the requested one
	CheckRateAdjustments();
	
	return true;
}

bool JVOIPSoundDeviceDriver::IsReferenced()
{
	if (!sdevread && !sdevwrite)
		return false;
	return true;
}

bool JVOIPSoundDeviceDriver::IsSameDevice(const std::string &devicename)
{
	if (devname == devicename)
		return true;
	return false;
}

int JVOIPSoundDeviceDriver::SetInputCallback(void (*f)(void *param),void *param)
{
	if (filedesc < 0)
	{
		if (reopenerror)
			return ERR_JVOIPLIB_SOUNDCARDIO_COULDNTREOPENDEVICE;
		return ERR_JVOIPLIB_SOUNDCARDIO_DRIVERNOTINIT;
	}
	
	if (!inputthread)
		return ERR_JVOIPLIB_SOUNDCARDIO_DRIVERHASNOINPUTTHREAD;
	inputthread->SetInputCallback(f,param);
	return 0;
}

int JVOIPSoundDeviceDriver::AddInputBuffer(JVOIPSoundDeviceBlock *blk)
{
	if (filedesc < 0)
	{
		if (reopenerror)
			return ERR_JVOIPLIB_SOUNDCARDIO_COULDNTREOPENDEVICE;
		return ERR_JVOIPLIB_SOUNDCARDIO_DRIVERNOTINIT;
	}
	
	if (!inputthread)
		return ERR_JVOIPLIB_SOUNDCARDIO_DRIVERHASNOINPUTTHREAD;
	return inputthread->AddBuffer(blk);
}

int JVOIPSoundDeviceDriver::AddOutputBuffer(JVOIPSoundDeviceBlock *blk)
{
	if (filedesc < 0)
	{
		if (reopenerror)
			return ERR_JVOIPLIB_SOUNDCARDIO_COULDNTREOPENDEVICE;
		return ERR_JVOIPLIB_SOUNDCARDIO_DRIVERNOTINIT;
	}	
	
	if (!outputthread)
		return ERR_JVOIPLIB_SOUNDCARDIO_DRIVERHASNOOUTPUTTHREAD;
	return outputthread->AddBuffer(blk);
}

int JVOIPSoundDeviceDriver::ResetInput()
{
	if (filedesc < 0)
	{
		if (reopenerror)
			return ERR_JVOIPLIB_SOUNDCARDIO_COULDNTREOPENDEVICE;
		return ERR_JVOIPLIB_SOUNDCARDIO_DRIVERNOTINIT;
	}	
	
	if (!inputthread)
		return ERR_JVOIPLIB_SOUNDCARDIO_DRIVERHASNOINPUTTHREAD;
	inputthread->Reset();
	return 0;
}

int JVOIPSoundDeviceDriver::ResetOutput()
{
	if (filedesc < 0)
	{
		if (reopenerror)
			return ERR_JVOIPLIB_SOUNDCARDIO_COULDNTREOPENDEVICE;
		return ERR_JVOIPLIB_SOUNDCARDIO_DRIVERNOTINIT;
	}	
	
	if (!outputthread)
		return ERR_JVOIPLIB_SOUNDCARDIO_DRIVERHASNOOUTPUTTHREAD;
	outputthread->Reset();
	return 0;
}

int JVOIPSoundDeviceDriver::RequestSamplingRate(int samprate,bool read)
{
	if (read)
		readrequestrate = samprate;
	else
		writerequestrate = samprate;
	CheckRateAdjustments();
	return 0;
}

void JVOIPSoundDeviceDriver::CheckRateAdjustments()
{
	int newrate;

	if (!sdevread && !sdevwrite) // no devices registered
		return;
	
	if (sdevread && sdevwrite) // two devices registered, use the highest requested sampling rate
		newrate = (readrequestrate > writerequestrate)?readrequestrate:writerequestrate;		
	else // only one device asked for a specific rate: grant its request
	{
		if (sdevread)
			newrate = readrequestrate;
		else
			newrate = writerequestrate;
	}

	if (newrate != currentsamplingrate)
	{
		ResetInput();
		ResetOutput();

		close(filedesc);
		filedesc = OpenDevice(devname,newrate);
		if (filedesc < 0)
		{
			reopenerror = true;

			// The threads use the file descriptor, make sure they are
			// killed
			if (outputthread)
			{
				delete outputthread;
				outputthread = NULL;
			}
			if (inputthread)
			{
				delete inputthread;
				inputthread = NULL;
			}
			return;
		}

		// Adjust the parameters in the threads
		// Also: set the new file descriptor!
		if (outputthread)
		{
			outputthread->SetSoundDeviceDescriptor(filedesc);
			outputthread->SetCurrentSamplingRate(currentsamplingrate);
		}
		if (inputthread)
		{
			inputthread->SetSoundDeviceDescriptor(filedesc);
			inputthread->SetCurrentSamplingRate(currentsamplingrate);
		}
	}
}
