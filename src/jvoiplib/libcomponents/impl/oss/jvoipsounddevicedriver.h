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

#ifndef JVOIPSOUNDDEVICEDRIVER_H

#define JVOIPSOUNDDEVICEDRIVER_H

#include <stdlib.h>
#include <string>

class JVOIPSoundDevice;
class JVOIPSoundDeviceBlock;
class JVOIPSoundDevDrvInputThread;
class JVOIPSoundDevDrvOutputThread;

class JVOIPSoundDeviceDriver
{
public:
	JVOIPSoundDeviceDriver();
	~JVOIPSoundDeviceDriver();
	int Init(const std::string &devicename,int requesterate);
	int Register(JVOIPSoundDevice *sdev,bool read);
	bool Unregister(JVOIPSoundDevice *sdev);
	bool IsReferenced();
	inline std::string GetDeviceName();
	bool IsSameDevice(const std::string &devicename);
	bool IsSomeoneReading()							{ return (sdevread == NULL)?false:true; }
	bool IsSomeoneWriting()							{ return (sdevwrite == NULL)?false:true; }
	int SetInputCallback(void (*f)(void *param),void *param);
	int AddInputBuffer(JVOIPSoundDeviceBlock *blk);
	int AddOutputBuffer(JVOIPSoundDeviceBlock *blk);
	int ResetInput();
	int ResetOutput();
	int RequestSamplingRate(int samprate,bool read);
	
	// get characteristics	
	int GetSamplingRate()							{ return currentsamplingrate; }
	int GetBytesPerSample()							{ return 2; }
	bool IsStereo()								{ return true; }
	bool IsSampleLittleEndian()						{ return true; }
	bool IsSampleSigned()							{ return true; }
private:
	int filedesc;
	int inputrate,outputrate;
	bool inputstereo,outputstereo;
	int inputbytespersample,outputbytespersample;
	int currentsamplingrate;
	std::string devname;
	JVOIPSoundDevDrvInputThread *inputthread;
	JVOIPSoundDevDrvOutputThread *outputthread;
	
	JVOIPSoundDevice *sdevread;
	JVOIPSoundDevice *sdevwrite;
	int readrequestrate,writerequestrate;

	bool reopenerror;

	int OpenDevice(const std::string &devicename,int reqsamprate);
	void CheckRateAdjustments();
public:
	JVOIPSoundDeviceDriver *next;
};

inline std::string JVOIPSoundDeviceDriver::GetDeviceName()
{
	if (filedesc < 0)
		return std::string("No device opened");
	return devname;
}

#endif // JVOIPSOUNDDEVICEDRIVER_H
