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

#include "jvoipsoundcardinput.h"
#include "jvoiperrors.h"

#include "debugnew.h"

#define JVOIPSOUNDCARDINPUT_TOTALBUFFERTIME				10000
#define JVOIPSOUNDCARDINPUT_EXTRABLOCKSIZE				32

#define JVOIPSOUNDCARDINPUT_DRVBYTESPERSAMPLE					2
#define JVOIPSOUNDCARDINPUT_DRVSTEREO						1
#define JVOIPSOUNDCARDINPUT_DRVLITTLEENDIAN					true
#define JVOIPSOUNDCARDINPUT_DRVSIGNED						true

JVOIPSoundcardInput::JVOIPSoundcardInput(JVOIPSession *sess) throw (JVOIPException) : JVOIPVoiceInput(sess)
{
	init = false;
	sounddev = NULL;
	if (sigwait.Init() < 0)
		throw JVOIPException(ERR_JVOIPLIB_GENERAL_CANTINITSIGWAIT);
}

JVOIPSoundcardInput::~JVOIPSoundcardInput()
{
	Cleanup();
}

int JVOIPSoundcardInput::Init(int sampinterval,int inputsamprate,int inputbytespersample,const JVOIPComponentParams *componentparams)
{
	JVOIPSoundcardInputParams defparams,*usrparams;
	int status;

	if (init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTALREADYINIT;
	
	usrparams = dynamic_cast<JVOIPSoundcardInputParams*>(const_cast<JVOIPComponentParams*>(componentparams));
	if (usrparams == NULL)
		usrparams = &defparams;

	nastyresetbug = usrparams->GetNastyResetBugFix();

	if ((status = OpenSoundDevice(CalculateDriveSampleRate(inputsamprate))) < 0)
		return status;

	sampleinterval = sampinterval;
	samprate = inputsamprate;
	bytespersample = inputbytespersample;
	
	if ((status = InitBuffers()) < 0)
	{
		if (sounddev != NULL)
			waveInClose(sounddev);
		sounddev = NULL;
		return status;
	}

	InitSampleConverter();		
	sigwait.ClearSignalBuffers();
		
	init = true;
	firsttime = true;
	resetting = false;
	return 0;
}

int JVOIPSoundcardInput::Cleanup()
{
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;

	Reset();
	ClearBuffers();
	if (sounddev != NULL)
		waveInClose(sounddev);
	sounddev = NULL;
	init = false;
	return 0;
}

int JVOIPSoundcardInput::StartSampling()
{
	int i,size,numsamp;
	double diff;
	
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	
	if (firsttime)
	{
		firsttime = false;
		srcsampletime = 0;
		wantedsrcsampletime = 0;
		
		for (i = 0 ; i < numblocks ; i++)
		{
			wantedsrcsampletime += (double)sampleinterval;
			diff = wantedsrcsampletime-srcsampletime;
			diff /= 1000.0; // convert to seconds
			numsamp = (int)(diff*(double)drvsamprate+0.5);
			size = numsamp*JVOIPSOUNDCARDINPUT_DRVSTEREO*JVOIPSOUNDCARDINPUT_DRVBYTESPERSAMPLE;
			
			sampleblocks[i].dwBufferLength = size;
			sampleblocks[i].dwBytesRecorded = 0;
			sampleblocks[i].dwUser = 0;
			sampleblocks[i].dwFlags = 0;
			sampleblocks[i].dwLoops = 0;
			waveInPrepareHeader(sounddev,&sampleblocks[i],sizeof(WAVEHDR));
			waveInAddBuffer(sounddev,&sampleblocks[i],sizeof(WAVEHDR));
			
			srcsampletime += ((double)numsamp)/((double)drvsamprate)*1000.0;
		}
		waveInStart(sounddev);
		blockpos = 0;
	}
	else
	{
		wantedsrcsampletime += (double)sampleinterval;
		diff = wantedsrcsampletime-srcsampletime;
		diff /= 1000.0; // convert to seconds
		numsamp = (int)(diff*(double)drvsamprate+0.5);
		size = numsamp*JVOIPSOUNDCARDINPUT_DRVSTEREO*JVOIPSOUNDCARDINPUT_DRVBYTESPERSAMPLE;
			
		waveInUnprepareHeader(sounddev,&sampleblocks[blockpos],sizeof(WAVEHDR));
		sampleblocks[blockpos].dwBufferLength = size;
		sampleblocks[blockpos].dwBytesRecorded = 0;
		sampleblocks[blockpos].dwUser = 0;
		sampleblocks[blockpos].dwFlags = 0;
		sampleblocks[blockpos].dwLoops = 0;
		waveInPrepareHeader(sounddev,&sampleblocks[blockpos],sizeof(WAVEHDR));
		waveInAddBuffer(sounddev,&sampleblocks[blockpos],sizeof(WAVEHDR));
			
		srcsampletime += ((double)numsamp)/((double)drvsamprate)*1000.0;
		
		blockpos++;
		if (blockpos >= numblocks)
			blockpos = 0;
	}
	
	return 0;
}

void JVOIPSoundcardInput::Reset()
{
	int status;
	bool wasfirsttime;

	if (!init)
		return;

	wasfirsttime = firsttime;
	firsttime = true;
	if (nastyresetbug)
		status = mixstat.LoadStatus();

	waveInStop(sounddev);
	resetting = true;
	waveInReset(sounddev);
	resetting = false;

	if (nastyresetbug)
	{
		if (status >= 0)
			mixstat.RestoreStatus();
	}

	if (!wasfirsttime)
	{
		int i;
               	
		for (i = 0 ; i < numblocks ; i++)
			waveInUnprepareHeader(sounddev,&sampleblocks[i],sizeof(WAVEHDR));
	}

	sigwait.ClearSignalBuffers();
}

int JVOIPSoundcardInput::GetSampleBlock(VoIPFramework::VoiceBlock *vb)
{
	int numsamp,size;
	double diff;
	
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	
	if (firsttime) // this will happen only once, and 'firsttime' will be set
	{              // to 'false', in 'StartSampling' (which is called after this function)
		dstsampletime = 0;
		wanteddstsampletime = 0;
	}		
	
	wanteddstsampletime += (double)sampleinterval;
	diff = wanteddstsampletime-dstsampletime;
	diff /= 1000.0; // convert to seconds
	numsamp = (int)(diff*(double)samprate+0.5);
	size = numsamp*bytespersample;
	dstsampletime += ((double)numsamp)/((double)samprate)*1000.0;
	
	if (firsttime) // we'll have to return silence, since we haven't sampled anything yet
	{
		// Here, we follow the assumption that we deliver sample in big endian and unsigned
	
		if (bytespersample == 1)
			memset(convertbuffer,127,numsamp);
		else // two bytes per sample
		{
			for (int i = 0 ; i < size ; i += 2)
			{
				convertbuffer[i] = 127;
				convertbuffer[i+1] = 255;
			}
		}
	}
	else
	{
		int actualsize;

		actualsize = sampconvert.Convert((unsigned char *)sampleblocks[blockpos].lpData,sampleblocks[blockpos].dwBufferLength,convertbuffer,size);
		if (actualsize < size) // we'll have to fill a bit
		{
			if (bytespersample == 1)
			{
				unsigned char b;
				
				b = convertbuffer[actualsize-1];
				for (    ;  actualsize < size ; actualsize++)
					convertbuffer[actualsize] = b;
			}
			else // two bytes per sample
			{
				unsigned char b1,b2;
				
				b1 = convertbuffer[actualsize-2];
				b2 = convertbuffer[actualsize-1];
				for (    ;  actualsize < size ; actualsize += 2)
				{
					convertbuffer[actualsize] = b1;
					convertbuffer[actualsize + 1] = b2;
				}
			}
		}
	}
	
	// Ok, now we have what we want, and we'll just have to fill in the info in the
	// voice block

	vb->SetSamples(convertbuffer,size);
	vb->SetNumSamples(numsamp);
	vb->SetStereo(false);
	vb->SetBytesPerSample(bytespersample);
	vb->SetSampleRate(samprate);
	vb->SetDataDeleteHandler(BlockDeleteHandler,this);
	
	return 0;
}

bool JVOIPSoundcardInput::SupportsSampleInterval(int ival)
{
	return true;
}

bool JVOIPSoundcardInput::SupportsInputSamplingRate(int irate)
{
	return true;
}

bool JVOIPSoundcardInput::SupportsInputBytesPerSample(int inputbytespersample)
{
	return true;	
}

int JVOIPSoundcardInput::SetSampleInterval(int ival)
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
			waveInClose(sounddev);
		sounddev = NULL;
		init = false;
		return status;
		
	}
	InitSampleConverter();	
	return 0;
}

int JVOIPSoundcardInput::SetInputSamplingRate(int irate)
{
	int status;
	
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	if (irate == samprate)
		return 0;
	
	samprate = irate;
	Reset();
	ClearBuffers();
	if ((status = InitBuffers()) < 0)
	{
		if (sounddev != NULL)
			waveInClose(sounddev);
		sounddev = NULL;
		init = false;
		return status;
	}
	InitSampleConverter();	
	return 0;
}

int JVOIPSoundcardInput::SetInputBytesPerSample(int inputbytespersample)
{
	int status;
	
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	if (inputbytespersample == bytespersample)
		return 0;
	
	bytespersample = inputbytespersample;
	Reset();
	ClearBuffers();
	if ((status = InitBuffers()) < 0)
	{
		if (sounddev != NULL)
			waveInClose(sounddev);
		sounddev = NULL;
		init = false;
		return status;
	}
	InitSampleConverter();	
	return 0;
}

bool JVOIPSoundcardInput::HasTimeOut()
{
	if (!init)
		return false;
	if (firsttime)
		return true;
	return ((sampleblocks[blockpos].dwFlags&WHDR_DONE) == WHDR_DONE)?true:false;
}

int JVOIPSoundcardInput::RestartTimer()
{
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	
	// NOTE: nothing needs to be done here
	return 0;
}

int JVOIPSoundcardInput::GetComponentState(JVOIPComponentState **compstate)
{
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	
	*compstate = NULL;
	return 0;
}

int JVOIPSoundcardInput::SetComponentState(JVOIPComponentState *compstate)
{
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	// NOTE: nothing needs to be done here
	return 0;
}

std::string JVOIPSoundcardInput::GetComponentName()
{
	return std::string("JVOIPSoundcardInput");
}

std::string JVOIPSoundcardInput::GetComponentDescription()
{
	return std::string("JVOIPLIB Internal soundcard input module (Win32 version)");
}

std::vector<JVOIPCompParamInfo> *JVOIPSoundcardInput::GetComponentParameters() throw (JVOIPException)
{
	return NULL;
}

int JVOIPSoundcardInput::InitBuffers()
{
	int i;

	// allocate buffers for the sounddevice
		
	numblocks = (int)(((double)JVOIPSOUNDCARDINPUT_TOTALBUFFERTIME)/((double)sampleinterval)+0.5);
	blocksize = (int)((((double)drvsamprate*(double)sampleinterval)/1000.0)*(double)JVOIPSOUNDCARDINPUT_DRVBYTESPERSAMPLE*(double)JVOIPSOUNDCARDINPUT_DRVSTEREO+0.5);
	blocksize += JVOIPSOUNDCARDINPUT_EXTRABLOCKSIZE;
	
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
	}

	// allocate a buffer to hold the converted data
		
	convertbufsize = (int)((((double)samprate*(double)sampleinterval)/1000.0)*(double)bytespersample+0.5);
	convertbufsize += JVOIPSOUNDCARDINPUT_EXTRABLOCKSIZE;
	
	convertbuffer = new unsigned char [convertbufsize];
	if (convertbuffer == NULL)
	{
		for (i = 0 ; i < numblocks ; i++)
			delete [] sampleblocks[i].lpData;
		delete [] sampleblocks;
	}
	
	return 0;
}

void JVOIPSoundcardInput::ClearBuffers()
{
	for (int i = 0 ; i < numblocks ; i++)
		delete [] sampleblocks[i].lpData;
	delete [] sampleblocks;
	delete [] convertbuffer;
}

void JVOIPSoundcardInput::InputCallback(HWAVEIN hwi,UINT uMsg,DWORD dwInstance,DWORD dwParam1,DWORD dwParam2)
{
	JVOIPSoundcardInput *inst;
	
	if (!dwInstance)
		return;
	
	inst = (JVOIPSoundcardInput *)dwInstance;
	if (inst->resetting)
		return;

	inst->sigwait.Signal();
}

void JVOIPSoundcardInput::BlockDeleteHandler(unsigned char **data,void *param)
{
	*data = NULL;
}

void JVOIPSoundcardInput::InitSampleConverter()
{
	int srcrate,srcbytespersample,dstrate,dstbytespersample;
	bool srcstereo,srcsigned,srcLE,dststereo,dstsigned,dstLE;

	srcrate = drvsamprate;
	srcbytespersample = JVOIPSOUNDCARDINPUT_DRVBYTESPERSAMPLE;
	srcstereo = (JVOIPSOUNDCARDINPUT_DRVSTEREO == 1)?false:true;
	srcsigned = JVOIPSOUNDCARDINPUT_DRVSIGNED;
	srcLE = JVOIPSOUNDCARDINPUT_DRVLITTLEENDIAN;

	dstrate = samprate;
	dstbytespersample = bytespersample;
	dststereo = false;
	dstsigned = false;
	dstLE = false;
        	
	sampconvert.SetConversionParams(srcrate,srcstereo,srcbytespersample,srcsigned,srcLE,
                                        dstrate,dststereo,dstbytespersample,dstsigned,dstLE);
}

int JVOIPSoundcardInput::OpenSoundDevice(int samplerate)
{
	int fact;
	WAVEFORMATEX format;
	
	fact = 1;
	if (JVOIPSOUNDCARDINPUT_DRVSTEREO != 1)
		fact *= 2;
	if (JVOIPSOUNDCARDINPUT_DRVBYTESPERSAMPLE != 1)
		fact *= 2;

	format.nAvgBytesPerSec = samplerate*fact;
	format.nBlockAlign = fact;
	format.nChannels = JVOIPSOUNDCARDINPUT_DRVSTEREO;
	format.nSamplesPerSec = samplerate;
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.cbSize = 0;
	format.wBitsPerSample = JVOIPSOUNDCARDINPUT_DRVBYTESPERSAMPLE*8;
	
	if (waveInOpen((LPHWAVEIN)&sounddev, WAVE_MAPPER,(LPWAVEFORMATEX)&format,(DWORD)InputCallback,(DWORD)this,CALLBACK_FUNCTION))
		return ERR_JVOIPLIB_SOUNDCARDIO_CANTOPENDEVICE;
	
	drvsamprate = samplerate;
	return 0;
}

int JVOIPSoundcardInput::CalculateDriveSampleRate(int requestedrate)
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

int JVOIPSoundcardInput::RequestSamplingRate(int rate)
{
	int usedrate;
	int status;

	usedrate = CalculateDriveSampleRate(rate);
	if (usedrate == drvsamprate) // don't have to adjust
		return 0;
	waveInClose(sounddev);
	sounddev = NULL;
	if ((status = OpenSoundDevice(usedrate)) < 0)
		return status;
	return 0;
}
