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

#include "jvoipsounddevice.h"

#include "debugnew.h"

JVOIPSoundDeviceDriver *JVOIPSoundDevice::firstsdevdriver = NULL;

JVOIPSoundDevice::JVOIPSoundDevice(JVOIPSoundDeviceDriver *s,bool read)
{
	devdrv = s;
	devdrv->Register(this,read);
	forreading = read;
}

JVOIPSoundDevice::~JVOIPSoundDevice()
{
	CloseDevice();
}

int JVOIPSoundDevice::OpenDevice(JVOIPSoundDevice **sdev,const std::string &devicename,bool read,int requestedrate)
{
	JVOIPSoundDeviceDriver *tmpdrv;
	JVOIPSoundDevice *sounddev;
	bool found;
	
	// look if the device is already opened
	found = false;
	tmpdrv = firstsdevdriver;
	while (!found && tmpdrv)
	{
		if (tmpdrv->IsSameDevice(devicename))
			found = true;
		else
			tmpdrv = tmpdrv->next;
	}
	
	if (found)
	{
		if (read && tmpdrv->IsSomeoneReading())
			return ERR_JVOIPLIB_SOUNDCARDIO_ALREADYOPENEDFORREADING;
		if (!read && tmpdrv->IsSomeoneWriting())
			return ERR_JVOIPLIB_SOUNDCARDIO_ALREADYOPENEDFORWRITING;
		sounddev = new JVOIPSoundDevice(tmpdrv,read);
		if (sounddev == NULL)
			return ERR_JVOIPLIB_GENERAL_OUTOFMEM;
	}
	else // open new device
	{
		int status;
		
		tmpdrv = new JVOIPSoundDeviceDriver();
		if (tmpdrv == NULL)
			return ERR_JVOIPLIB_GENERAL_OUTOFMEM;
		status = tmpdrv->Init(devicename,requestedrate);
		if (status < 0)
		{
			delete tmpdrv;
			return status;
		}
		sounddev = new JVOIPSoundDevice(tmpdrv,read);
		if (sounddev == NULL)
		{
			delete tmpdrv;
			return ERR_JVOIPLIB_GENERAL_OUTOFMEM;
		}
		
		// link the driver in the list
		tmpdrv->next = firstsdevdriver;
		firstsdevdriver = tmpdrv;
	}
	
	*sdev = sounddev;
	return 0;	
}

void JVOIPSoundDevice::CloseDevice()
{
	JVOIPSoundDeviceDriver *tmp,*prevtmp;
	bool found;
	
	if (devdrv == NULL)
		return;

	devdrv->Unregister(this);
	if (devdrv->IsReferenced())
	{
		devdrv = NULL;
		return;
	}
	
	// the driver is no longer used: remove it from the list
	
	tmp = firstsdevdriver;
	prevtmp = NULL;
	found = false;
	while (tmp && !found)
	{
		if (tmp == devdrv)
			found = true;
		else
		{
			prevtmp = tmp;
			tmp = tmp->next;
		}
	}
	if (!found)
		std::cerr << "JVOIPSoundDevice::CloseDevice -- Warning: device not found in list !\n";
	else
	{
		if (prevtmp)
			prevtmp->next = tmp->next;
		else
			firstsdevdriver = tmp->next;
	}
	delete tmp;
	devdrv = NULL;
}

