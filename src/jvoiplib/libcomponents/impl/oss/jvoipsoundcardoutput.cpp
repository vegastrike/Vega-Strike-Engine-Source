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

#include "jvoipsoundcardoutput.h"
#include "jvoipsounddevice.h"
#include "jvoiperrors.h"
#include "jvoipsoundcardparams.h"
#include "jvoipsounddeviceblock.h"

#include "debugnew.h"

#define JVOIPSOUNDCARDOUTPUT_TOTALBUFFERTIME					10000
#define JVOIPSOUNDCARDOUTPUT_EXTRABLOCKSIZE					32

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
	JVOIPSoundcardParams defparams,*usrparams;
	int status;

	if (init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTALREADYINIT;
	
	// Get the soundcard parameters	
	if (componentparams == NULL)
		usrparams = &defparams;
	else if ((usrparams = dynamic_cast<JVOIPSoundcardParams *>(const_cast<JVOIPComponentParams *>(componentparams))) == NULL)
		usrparams = &defparams;

	status = JVOIPSoundDevice::OpenDevice(&sounddev,usrparams->GetSoundDeviceName(),false,outputsamprate);
	if (status < 0)
		return status;
		
	sampleinterval = sampinterval;
	samprate = outputsamprate;
	bytespersample = outputbytespersample;
	needstereo = stereo;
	
	sounddev->RequestSamplingRate(samprate);

	if ((status = InitBuffers()) < 0)
	{
		delete sounddev;
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
	delete sounddev;
	ClearBuffers();
	init = false;
	return 0;
}

int JVOIPSoundcardOutput::Play(VoIPFramework::VoiceBlock *vb)
{
	bool playsilence;
	unsigned char *data;
	double diff;
	int numsamp,size;
	int status;
	
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
	}
	
	wantedplaytime += (double)sampleinterval;
	diff = wantedplaytime-playtime;
	diff /= 1000.0; // convert to seconds
	numsamp = (int)(diff*(double)drvsamprate+0.5);
	size = numsamp*drvstereo*drvbytespersample;
	playtime += ((double)numsamp)/((double)drvsamprate)*1000.0;

	if (playsilence)
	{
		// Here, we got to deliver samples to the driver, so the endianness
		// and signedness have to be taken into account
		
		if (drvbytespersample == 1)
		{
			if (!sounddev->IsSampleSigned())
				memset(sampleblocks[blockpos].data,127,size);
			else
				memset(sampleblocks[blockpos].data,0,size);
		}
		else // two bytes per sample
		{
			if (!sounddev->IsSampleSigned())
			{
				if (!sounddev->IsSampleLittleEndian()) // big endian
				{
					for (int i = 0 ; i < size ; i += 2)
					{
						sampleblocks[blockpos].data[i] = 127;
						sampleblocks[blockpos].data[i+1] = 255;
					}
				}
				else // little endian
				{
					for (int i = 0 ; i < size ; i += 2)
					{
						sampleblocks[blockpos].data[i] = 255;
						sampleblocks[blockpos].data[i+1] = 127;
					}
				}
			}
			else // signed !
				memset(sampleblocks[blockpos].data,0,size);
		}
	}
	else
	{
		int actualsize;

		actualsize = sampconvert.Convert(data,vb->GetNumBytes(),sampleblocks[blockpos].data,size);
		if (actualsize < size)
		{
			if (drvstereo == 1)
			{
				if (drvbytespersample == 1)
				{
					unsigned char b;

					b = sampleblocks[blockpos].data[actualsize-1];
					for (     ; actualsize < size ; actualsize++)
						sampleblocks[blockpos].data[actualsize] = b;
				}
				else // two bytes per sample
				{
					unsigned char b1,b2;

					b1 = sampleblocks[blockpos].data[actualsize-2];
					b2 = sampleblocks[blockpos].data[actualsize-1];
					for (     ; actualsize < size ; actualsize += 2)
					{
						sampleblocks[blockpos].data[actualsize] = b1;
						sampleblocks[blockpos].data[actualsize+1] = b2;
					}
				}
			}
			else // stereo
			{
				if (drvbytespersample == 1)
				{
					unsigned char b1,b2;

					b1 = sampleblocks[blockpos].data[actualsize-2];
					b2 = sampleblocks[blockpos].data[actualsize-1];
					for (     ; actualsize < size ; actualsize += 2)
					{
						sampleblocks[blockpos].data[actualsize] = b1;
						sampleblocks[blockpos].data[actualsize+1] = b2;
					}
				}
				else // two bytes per sample
				{
					unsigned char b1,b2,b3,b4;

					b1 = sampleblocks[blockpos].data[actualsize-4];
					b2 = sampleblocks[blockpos].data[actualsize-3];
					b3 = sampleblocks[blockpos].data[actualsize-2];
					b4 = sampleblocks[blockpos].data[actualsize-1];
					for (     ; actualsize < size ; actualsize += 4)
					{
						sampleblocks[blockpos].data[actualsize] = b1;
						sampleblocks[blockpos].data[actualsize+1] = b2;
						sampleblocks[blockpos].data[actualsize+2] = b3;
						sampleblocks[blockpos].data[actualsize+3] = b4;
					}
				}
			}
		}
	}
	
	sampleblocks[blockpos].datalen = size;
	if ((status = sounddev->AddBuffer(&sampleblocks[blockpos])) < 0)
		return status;
	
	blockpos++;
	if (blockpos >= numblocks)
		blockpos = 0;
	return 0;
}

void JVOIPSoundcardOutput::ResetDevice()
{
	if (!init)
		return;
	
	sounddev->Reset();
	firsttime = true;
}

void JVOIPSoundcardOutput::Reset()
{
	int status;

	if (!init)
		return;
	
	sounddev->Reset();
	firsttime = true;
	ClearBuffers();
	if ((status = InitBuffers()) < 0)
	{
		delete sounddev;
		init = false;
		std::cerr << "JVOIPSoundcardOutput: couldn't init buffers during Reset, device closed" << std::endl;
 		return;
	}
	InitSampleConverter();	
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
	ResetDevice();
	ClearBuffers();
	if ((status = InitBuffers()) < 0)
	{
		delete sounddev;
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
	ResetDevice();
	ClearBuffers();
	if ((status = InitBuffers()) < 0)
	{
		delete sounddev;
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
	ResetDevice();
	ClearBuffers();
	if ((status = InitBuffers()) < 0)
	{
		delete sounddev;
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
	ResetDevice();
	ClearBuffers();
	if ((status = InitBuffers()) < 0)
	{
		delete sounddev;
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
	return std::string("JVOIPLIB Internal soundcard output module (OSS version)");
}

std::vector<JVOIPCompParamInfo> *JVOIPSoundcardOutput::GetComponentParameters() throw (JVOIPException)
{
	std::vector<JVOIPCompParamInfo> *paraminfo;
	
	paraminfo = new std::vector<JVOIPCompParamInfo>(1);
	if (paraminfo == NULL)
		throw JVOIPException(ERR_JVOIPLIB_GENERAL_OUTOFMEM);
	(*paraminfo)[0].SetParameterName(std::string("Device"));
	if (sounddev == NULL)
		(*paraminfo)[0].SetParameterValue(std::string("None"));
	else
		(*paraminfo)[0].SetParameterValue(sounddev->GetDeviceName());
	return paraminfo;
}

void JVOIPSoundcardOutput::InitSampleConverter()
{
	int srcrate,srcbytespersample,dstrate,dstbytespersample;
	bool srcstereo,srcsigned,srcLE,dststereo,dstsigned,dstLE;

        dstrate = sounddev->GetSamplingRate();
        dstbytespersample = sounddev->GetBytesPerSample();
        dststereo = sounddev->IsStereo();
        dstsigned = sounddev->IsSampleSigned();
        dstLE = sounddev->IsSampleLittleEndian();

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
	int i;

	// Request the most appropriate sampling rate

	sounddev->RequestSamplingRate(samprate);

	// allocate buffers for the sounddevice
		
	drvsamprate = sounddev->GetSamplingRate();
	drvbytespersample = sounddev->GetBytesPerSample();
	drvstereo = (sounddev->IsStereo())?2:1;
	
	numblocks = (int)(((double)JVOIPSOUNDCARDOUTPUT_TOTALBUFFERTIME)/((double)sampleinterval)+0.5);
	blocksize = (int)((((double)drvsamprate*(double)sampleinterval)/1000.0)*(double)drvbytespersample*(double)drvstereo+0.5);
	blocksize += JVOIPSOUNDCARDOUTPUT_EXTRABLOCKSIZE;
	
	sampleblocks = new JVOIPSoundDeviceBlock [numblocks];
	if (sampleblocks == NULL)
		return ERR_JVOIPLIB_GENERAL_OUTOFMEM;

	for (i = 0 ; i < numblocks ; i++)
	{
		sampleblocks[i].data = new unsigned char[blocksize];
		if (sampleblocks[i].data == NULL)
		{
			for (int j = 0 ; j < i ; j++)
				delete [] sampleblocks[j].data;
			delete [] sampleblocks;
			return ERR_JVOIPLIB_GENERAL_OUTOFMEM;
		}
	}

	return 0;
}

void JVOIPSoundcardOutput::ClearBuffers()
{
	for (int i = 0 ; i < numblocks ; i++)
		delete [] sampleblocks[i].data;
	delete [] sampleblocks;
}

