/*

    This file is a part of JVOIPLIB, a library designed to facilitate
    the use of Voice over IP (VoIP).

    Copyright (C) 2000-2002  Jori Liesenborgs (jori@lumumba.luc.ac.be)

    This library (JVOIPLIB) is based upon work done for my thesis at
    the School for Knowledge Technology (Belgium/The Netherlands)

    This file was developed at the 'Expertise Centre for Digital
    Media' (EDM) in Diepenbeek, Belgium (http://www.edm.luc.ac.be).
    The EDM is a research institute of the 'Limburgs Universitair
    Centrum' (LUC) (http://www.luc.ac.be).

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

#include "jvoipsoundcardoutput.h"
#include "jvoiperrors.h"
#include "voiceblock.h"
#include "debugnew.h"

#define JVOIPSOUNDCARDOUTPUT_TOTALBUFFERTIME					10000
#define JVOIPSOUNDCARDOUTPUT_EXTRABLOCKSIZE					32

#define JVOIPSOUNDCARDOUTPUT_DRVBYTESPERSAMPLE					2
#define JVOIPSOUNDCARDOUTPUT_DRVSTEREO						2
#define JVOIPSOUNDCARDOUTPUT_DRVLITTLEENDIAN					true
#define JVOIPSOUNDCARDOUTPUT_DRVSIGNED						true					

JVOIPSoundcardOutput::JVOIPSoundcardOutput(JVOIPSession *sess):JVOIPVoiceOutput(sess)
{
	init = false;
	sounddev = NULL;
}

JVOIPSoundcardOutput::~JVOIPSoundcardOutput()
{
	Cleanup();
}

int JVOIPSoundcardOutput::Init(int sampinterval,int outputsamprate,int outputbytespersample,bool stereo,const JVOIPComponentParams *componentparams)
{
	int status;

	if (init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTALREADYINIT;
	
	status = OpenSoundDevice(CalculateDriveSampleRate(outputsamprate));
	if (status < 0)
		return status;
	
	sampleinterval = sampinterval;
	samprate = outputsamprate;
	bytespersample = outputbytespersample;
	needstereo = stereo;
	
	if ((status = InitBuffers()) < 0)
	{
		if (sounddev != NULL)
			waveOutClose(sounddev);
		sounddev = NULL;
		return status;
	}

	InitSampleConverter();		
	init = true;
	firsttime = true;			
	return 0;
}

int JVOIPSoundcardOutput::Cleanup()
{
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	
	Reset();
	ClearBuffers();
	if (sounddev != NULL)
		waveOutClose(sounddev);
	sounddev = NULL;
	init = false;
	return 0;
}

int JVOIPSoundcardOutput::Play(VoIPFramework::VoiceBlock *vb)
{
	bool playsilence;
	unsigned char *data;
	double diff;
	int numsamp,size;

	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;

	playsilence = false;

	if ((data = vb->GetSamples(false)) == NULL)
		playsilence = true;
	if (vb->IsStereo() != needstereo)
		playsilence = true;
	else if (vb->GetBytesPerSample() != bytespersample)
		playsilence = true;
	if ((int)vb->GetSampleRate() != samprate)
		playsilence = true;
			
	if (firsttime)
	{
		firsttime = false;
		blockpos = 0;
		wantedplaytime = 0;
		playtime = 0;
		cycled = false;
	}
	
	if (cycled) // we should unprepare the header first
	{
		if ((sampleblocks[blockpos].dwFlags&WHDR_DONE) != WHDR_DONE)
		{
			// This means we're seriously out of sync and continuing
			// would create resource problems since waveOutUnprepareHeader
			// would not release the internal buffers of the wave device 
			Reset();
			return 0;
		}
		waveOutUnprepareHeader(sounddev,&sampleblocks[blockpos],sizeof(WAVEHDR));
	}

	wantedplaytime += (double)sampleinterval;
	diff = wantedplaytime-playtime;
	diff /= 1000.0; // convert to seconds
	numsamp = (int)(diff*(double)drvsamprate+0.5);
	size = numsamp*JVOIPSOUNDCARDOUTPUT_DRVSTEREO*JVOIPSOUNDCARDOUTPUT_DRVBYTESPERSAMPLE;
	playtime += ((double)numsamp)/((double)drvsamprate)*1000.0;

	if (playsilence)
	{
		if (JVOIPSOUNDCARDOUTPUT_DRVBYTESPERSAMPLE == 1)
		{
			if (JVOIPSOUNDCARDOUTPUT_DRVSIGNED)
				memset(sampleblocks[blockpos].lpData,0,size);
			else
				memset(sampleblocks[blockpos].lpData,127,size);
		}
		else // two bytes per sample
		{
			if (JVOIPSOUNDCARDOUTPUT_DRVSIGNED)
				memset(sampleblocks[blockpos].lpData,0,size);
			else
			{
				if (JVOIPSOUNDCARDOUTPUT_DRVLITTLEENDIAN)
				{
					for (int i = 0 ; i < size ; i += 2)
					{
						sampleblocks[blockpos].lpData[i] = -1; // ( -1 is 0xFF)
						sampleblocks[blockpos].lpData[i+1] = 127;
					}
				}
				else // big endian
				{
					for (int i = 0 ; i < size ; i += 2)
					{
						sampleblocks[blockpos].lpData[i] = 127;
						sampleblocks[blockpos].lpData[i+1] = -1; // ( -1 is 0xFF)
					}
				}
			}
		}
	}
	else
	{
		int actualsize;
		
		actualsize = sampconvert.Convert(data,vb->GetNumBytes(),(unsigned char *)sampleblocks[blockpos].lpData,size);
		if (actualsize < size)
		{
			if (JVOIPSOUNDCARDOUTPUT_DRVSTEREO == 1) // no stereo
			{
				if (JVOIPSOUNDCARDOUTPUT_DRVBYTESPERSAMPLE == 1)
				{
					unsigned char b;
					
					b = sampleblocks[blockpos].lpData[actualsize-1];
					for (     ; actualsize < size ; actualsize++)
						sampleblocks[blockpos].lpData[actualsize] = b;
				}
				else // two bytes per sample
				{
					unsigned char b1,b2;
					
					b1 = sampleblocks[blockpos].lpData[actualsize-2];
					b2 = sampleblocks[blockpos].lpData[actualsize-1];
					for (     ; actualsize < size ; actualsize += 2)
					{
						sampleblocks[blockpos].lpData[actualsize] = b1;
						sampleblocks[blockpos].lpData[actualsize+1] = b2;
					}
				}
			}
			else // stereo
			{
				if (JVOIPSOUNDCARDOUTPUT_DRVBYTESPERSAMPLE == 1)
				{
					unsigned char b1,b2;
					
					b1 = sampleblocks[blockpos].lpData[actualsize-2];
					b2 = sampleblocks[blockpos].lpData[actualsize-1];
					for (     ; actualsize < size ; actualsize += 2)
					{
						sampleblocks[blockpos].lpData[actualsize] = b1;
						sampleblocks[blockpos].lpData[actualsize+1] = b2;
					}
				}
				else // two bytes per sample
				{
					unsigned char b1,b2,b3,b4;
					
					b1 = sampleblocks[blockpos].lpData[actualsize-4];
					b2 = sampleblocks[blockpos].lpData[actualsize-3];
					b3 = sampleblocks[blockpos].lpData[actualsize-2];
					b4 = sampleblocks[blockpos].lpData[actualsize-1];
					for (     ; actualsize < size ; actualsize += 4)
					{
						sampleblocks[blockpos].lpData[actualsize] = b1;
						sampleblocks[blockpos].lpData[actualsize+1] = b2;
						sampleblocks[blockpos].lpData[actualsize+2] = b3;
						sampleblocks[blockpos].lpData[actualsize+3] = b4;
					}
				}
			}
		}
	}
	
	sampleblocks[blockpos].dwBufferLength = size;
	sampleblocks[blockpos].dwFlags = 0;
	sampleblocks[blockpos].dwLoops = 0;
	sampleblocks[blockpos].dwUser = 0;

	waveOutPrepareHeader(sounddev,&sampleblocks[blockpos],sizeof(WAVEHDR));

	waveOutWrite(sounddev,&sampleblocks[blockpos],sizeof(WAVEHDR));

	blockpos++;
	if (blockpos >= numblocks)
	{
		blockpos = 0;
		cycled = true;
	}
	return 0;
}

void JVOIPSoundcardOutput::Reset()
{
	int i;

	if (!init)
		return;
	
	firsttime = true;
	waveOutReset(sounddev);
	if (cycled)
	{
		for (i = 0 ; i < numblocks ; i++)
			waveOutUnprepareHeader(sounddev,&sampleblocks[i],sizeof(WAVEHDR));
	}
	else
	{
		for (i = 0 ; i < blockpos ; i++)
			waveOutUnprepareHeader(sounddev,&sampleblocks[i],sizeof(WAVEHDR));
	}
}

bool JVOIPSoundcardOutput::SupportsSampleInterval(int ival)
{
	return true;
}

bool JVOIPSoundcardOutput::SupportsOutputSamplingRate(int orate)
{
	return true;
}

bool JVOIPSoundcardOutput::SupportsOutputBytesPerSample(int outputbytespersample)
{
	return true;
}

int JVOIPSoundcardOutput::SetSampleInterval(int ival)
{
	int status;

	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	if (ival == sampleinterval)
		return 0;

	sampleinterval = ival;
	Reset();
	ClearBuffers();
	if ((status = InitBuffers()) < 0)
	{
		if (sounddev != NULL)
			waveOutClose(sounddev);
		sounddev = NULL;
		init = false;
		return status;
	}
	InitSampleConverter();	
	return 0;
}

int JVOIPSoundcardOutput::SetOutputSamplingRate(int orate)
{
	int status;

	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	if (orate == samprate)
		return 0;
	
	samprate = orate;
	Reset();
	ClearBuffers();
	if ((status = InitBuffers()) < 0)
	{
		if (sounddev != NULL)
			waveOutClose(sounddev);
		sounddev = NULL;
		init = false;
		return status;
	}
	InitSampleConverter();	
	return 0;
}

int JVOIPSoundcardOutput::SetOutputBytesPerSample(int outputbytespersample)
{
	int status;

	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	if (outputbytespersample == bytespersample)
		return 0;
	
	bytespersample = outputbytespersample;
	Reset();
	ClearBuffers();
	if ((status = InitBuffers()) < 0)
	{
		if (sounddev != NULL)
			waveOutClose(sounddev);
		sounddev = NULL;
		init = false;
		return status;
	}
	InitSampleConverter();	
	return 0;
}

int JVOIPSoundcardOutput::SetStereo(bool s)
{
	int status;

	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	if (s == needstereo)
		return 0;
	
	needstereo = s;
	Reset();
	ClearBuffers();
	if ((status = InitBuffers()) < 0)
	{
		if (sounddev != NULL)
			waveOutClose(sounddev);
		sounddev = NULL;
		init = false;
		return status;
	}
	InitSampleConverter();	
	return 0;
}

int JVOIPSoundcardOutput::GetComponentState(JVOIPComponentState **compstate)
{
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	*compstate = NULL;
	return 0;
}

int JVOIPSoundcardOutput::SetComponentState(JVOIPComponentState *compstate)
{
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	// NOTE: Nothing needs to be done here
	return 0;
}

std::string JVOIPSoundcardOutput::GetComponentName()
{
	return std::string("JVOIPSoundcardOutput");
}

std::string JVOIPSoundcardOutput::GetComponentDescription()
{
	return std::string("JVOIPLIB Internal soundcard output module (Win32 version)");
}

std::vector<JVOIPCompParamInfo> *JVOIPSoundcardOutput::GetComponentParameters()
{
	return NULL;
}

void JVOIPSoundcardOutput::InitSampleConverter()
{
	int srcrate,srcbytespersample,dstrate,dstbytespersample;
	bool srcstereo,srcsigned,srcLE,dststereo,dstsigned,dstLE;

	dstrate = drvsamprate;
	dstbytespersample = JVOIPSOUNDCARDOUTPUT_DRVBYTESPERSAMPLE;
	dststereo = (JVOIPSOUNDCARDOUTPUT_DRVSTEREO == 2)?true:false;
	dstsigned = JVOIPSOUNDCARDOUTPUT_DRVSIGNED;
	dstLE = JVOIPSOUNDCARDOUTPUT_DRVLITTLEENDIAN;

	srcrate = samprate;
	srcbytespersample = bytespersample;
	srcstereo = needstereo;
	srcsigned = false;
	srcLE = false;
        	
	sampconvert.SetConversionParams(srcrate,srcstereo,srcbytespersample,srcsigned,srcLE,
                                    dstrate,dststereo,dstbytespersample,dstsigned,dstLE);
}

int JVOIPSoundcardOutput::InitBuffers()
{
	int i,status;

	if ((status = RequestSamplingRate(samprate)) < 0)
		return status;

	// allocate buffers for the sounddevice
		
	numblocks = (int)(((double)JVOIPSOUNDCARDOUTPUT_TOTALBUFFERTIME)/((double)sampleinterval)+0.5);
	blocksize = (int)((((double)drvsamprate*(double)sampleinterval)/1000.0)*(double)JVOIPSOUNDCARDOUTPUT_DRVBYTESPERSAMPLE*(double)JVOIPSOUNDCARDOUTPUT_DRVSTEREO+0.5);
	blocksize += JVOIPSOUNDCARDOUTPUT_EXTRABLOCKSIZE;
	
	sampleblocks = new WAVEHDR[numblocks];
	if (sampleblocks == NULL)
		return ERR_JVOIPLIB_GENERAL_OUTOFMEM;

	for (i = 0 ; i < numblocks ; i++)
	{
		sampleblocks[i].lpData = new char[blocksize];
		if (sampleblocks[i].lpData == NULL)
		{
			for (int j = 0 ; j < i ; j++)
				delete [] sampleblocks[j].lpData;
			delete [] sampleblocks;
			return ERR_JVOIPLIB_GENERAL_OUTOFMEM;
		}
		sampleblocks[i].dwFlags = 0; // clear all flags
	}

	return 0;
}

void JVOIPSoundcardOutput::ClearBuffers()
{
	for (int i = 0 ; i < numblocks ; i++)
		delete [] sampleblocks[i].lpData;
	delete [] sampleblocks;
}

int JVOIPSoundcardOutput::OpenSoundDevice(int samplerate)
{
	int fact;
	WAVEFORMATEX format;

	fact = 1;
	if (JVOIPSOUNDCARDOUTPUT_DRVSTEREO != 1)
		fact *= 2;
	if (JVOIPSOUNDCARDOUTPUT_DRVBYTESPERSAMPLE != 1)
		fact *= 2;

	format.nAvgBytesPerSec = samplerate*fact;
	format.nBlockAlign = fact;
	format.nChannels = JVOIPSOUNDCARDOUTPUT_DRVSTEREO;
	format.nSamplesPerSec = samplerate;
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.cbSize = 0;
	format.wBitsPerSample = JVOIPSOUNDCARDOUTPUT_DRVBYTESPERSAMPLE*8;
	
	if (waveOutOpen((LPHWAVEOUT)&sounddev, WAVE_MAPPER,(LPWAVEFORMATEX)&format,0,0,CALLBACK_NULL))
		return ERR_JVOIPLIB_SOUNDCARDIO_CANTOPENDEVICE;

	drvsamprate = samplerate;
	return 0;
}

int JVOIPSoundcardOutput::CalculateDriveSampleRate(int requestedrate)
{
	int usedsamplerate;

	if (requestedrate <= 5000)
		usedsamplerate = 5000;
	else if (requestedrate <= 11025)
		usedsamplerate = 11025;
	else if (requestedrate <= 22050)
		usedsamplerate = 22050;
	else 
		usedsamplerate = 44100;

	return usedsamplerate;
}

int JVOIPSoundcardOutput::RequestSamplingRate(int rate)
{
	int usedrate;
	int status;

	usedrate = CalculateDriveSampleRate(rate);
	if (usedrate == drvsamprate) // don't have to adjust
		return 0;
	waveOutClose(sounddev);
	sounddev = NULL;
	if ((status = OpenSoundDevice(usedrate)) < 0)
		return status;
	return 0;
}
