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

#ifndef JVOIPSOUNDDEVICE_H

#define JVOIPSOUNDDEVICE_H

#include "jvoipsounddevicedriver.h"
#include "jvoipsounddeviceblock.h"
#include "jvoiperrors.h"
#include <stdlib.h>
#include <string>

class JVOIPSoundDevice
{
private:
	JVOIPSoundDevice(JVOIPSoundDeviceDriver *s,bool read);
public:
	~JVOIPSoundDevice();
	static int OpenDevice(JVOIPSoundDevice **sdev,const std::string &devicename,bool read,int requestedrate);
	void CloseDevice();
	bool IsOpened()									{ return (devdrv == NULL)?false:true; }
	inline std::string GetDeviceName();
	inline int SetInputCallback(void (*f)(void *param),void *param);
	inline int AddBuffer(JVOIPSoundDeviceBlock *blk);
	inline int Reset();
	inline int RequestSamplingRate(int samprate);
	
	// get features
	inline int GetSamplingRate();
	inline int GetBytesPerSample();
	inline bool IsStereo();
	inline bool IsSampleLittleEndian();
	inline bool IsSampleSigned();
private:
	JVOIPSoundDeviceDriver *devdrv;
	bool forreading;
	
	static JVOIPSoundDeviceDriver *firstsdevdriver;
};

inline std::string JVOIPSoundDevice::GetDeviceName()
{
	if (devdrv == NULL)
		return std::string("Device closed");
	return devdrv->GetDeviceName();
}

inline int JVOIPSoundDevice::SetInputCallback(void (*f)(void *param),void *param)
{
	if (!forreading)
		return ERR_JVOIPLIB_SOUNDCARDIO_DEVICENOTOPENEDFORINPUT;
	if (!devdrv)
		return ERR_JVOIPLIB_SOUNDCARDIO_DEVICECLOSED;
	return devdrv->SetInputCallback(f,param);
}

inline int JVOIPSoundDevice::AddBuffer(JVOIPSoundDeviceBlock *blk)
{
	if (!devdrv)
		return ERR_JVOIPLIB_SOUNDCARDIO_DEVICECLOSED;
	if (forreading)
		return devdrv->AddInputBuffer(blk);
	return devdrv->AddOutputBuffer(blk);
}

inline int JVOIPSoundDevice::Reset()
{
	if (!devdrv)
		return ERR_JVOIPLIB_SOUNDCARDIO_DEVICECLOSED;
	if (forreading)
		return devdrv->ResetInput();
	return devdrv->ResetOutput();
}

inline int JVOIPSoundDevice::GetSamplingRate()
{
	if (!devdrv)
		return ERR_JVOIPLIB_SOUNDCARDIO_DEVICECLOSED;
	return devdrv->GetSamplingRate();
}

inline int JVOIPSoundDevice::GetBytesPerSample()
{
	if (!devdrv)
		return ERR_JVOIPLIB_SOUNDCARDIO_DEVICECLOSED;
	return devdrv->GetBytesPerSample();
}

inline bool JVOIPSoundDevice::IsStereo()
{
	if (!devdrv)
		return false;
	return devdrv->IsStereo();
}

inline bool JVOIPSoundDevice::IsSampleLittleEndian()
{
	if (!devdrv)
		return false;
	return devdrv->IsSampleLittleEndian();
}

inline bool JVOIPSoundDevice::IsSampleSigned()
{
	if (!devdrv)
		return false;
	return devdrv->IsSampleSigned();
}

inline int JVOIPSoundDevice::RequestSamplingRate(int samprate)
{
	if (!devdrv)
		return ERR_JVOIPLIB_SOUNDCARDIO_DEVICECLOSED;
	return devdrv->RequestSamplingRate(samprate,forreading);
}
#endif /* SOUNDDEVICE_H */
