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

#include "jvoipnoinput.h"
#include "jvoiperrors.h"
#include "voiceblock.h"

#include "debugnew.h"

JVOIPNoInput::JVOIPNoInput(JVOIPSession *sess) throw (JVOIPException) : JVOIPVoiceInput(sess)
{
	init = false;
}

JVOIPNoInput::~JVOIPNoInput()
{
}

int JVOIPNoInput::Init(int sampinterval,int inputsamprate,int inputbytespersample,const JVOIPComponentParams *componentparams)
{
	int status;

	if (init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTALREADYINIT;

	if ((status = thetimer.SetSamplingInterval(sampinterval)) < 0)
		return status;

	sampleinterval = sampinterval;
	samprate = inputsamprate;
	bytespersample = inputbytespersample;

	init = true;
	firsttime = true;
	return 0;
}

int JVOIPNoInput::Cleanup()
{
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	thetimer.StopTimer();
	init = false;
	return 0;
}

int JVOIPNoInput::StartSampling()
{
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	// NOTE: nothing to do here...
	return 0;
}

void JVOIPNoInput::Reset()
{
	if (!init)
		return;
	thetimer.StopTimer();
	firsttime = true;
}

int JVOIPNoInput::GetSampleBlock(VoIPFramework::VoiceBlock *vb)
{
	unsigned char *buffer;
	double diff;
	int numsamp,size;

	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;

	if (firsttime)
	{
		firsttime = false;
		dstsampletime = 0;
		wanteddstsampletime = 0;
	}		

	wanteddstsampletime += (double)sampleinterval;
	diff = wanteddstsampletime-dstsampletime;
	diff /= 1000.0; // convert to seconds
	numsamp = (int)(diff*(double)samprate+0.5);
	size = numsamp*bytespersample;
	dstsampletime += ((double)numsamp)/((double)samprate)*1000.0;
	
	buffer = new unsigned char [size];
	if (buffer == NULL)
		return ERR_JVOIPLIB_GENERAL_OUTOFMEM;
	
	// Here, we follow the assumption that we deliver sample in big endian and unsigned

	if (bytespersample == 1)
		memset(buffer,127,numsamp);
	else // two bytes per sample
	{
		for (int i = 0 ; i < size ; i += 2)
		{
			buffer[i] = 127;
			buffer[i+1] = 255;
		}
	}

	vb->SetSamples(buffer,size);
	vb->SetNumSamples(numsamp);
	vb->SetStereo(false);
	vb->SetBytesPerSample(bytespersample);
	vb->SetSampleRate(samprate);

	return 0;
}

bool JVOIPNoInput::SupportsSampleInterval(int ival)
{
	if (!init)
		return false;
	return true;
}

bool JVOIPNoInput::SupportsInputSamplingRate(int irate)
{
	if (!init)
		return false;
	return true;
}

bool JVOIPNoInput::SupportsInputBytesPerSample(int inputbytespersample)
{
	if (!init)
		return false;
	return true;
}

int JVOIPNoInput::SetSampleInterval(int ival)
{
	int status;

	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	Reset();
	sampleinterval = ival;
	if ((status = thetimer.SetSamplingInterval(ival)) < 0)
		return status;
	return 0;
}

int JVOIPNoInput::SetInputSamplingRate(int irate)
{
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;

	Reset();
	samprate = irate;
	return 0;
}

int JVOIPNoInput::SetInputBytesPerSample(int inputbytespersample)
{
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	Reset();
	bytespersample = inputbytespersample;
	return 0;
}

int JVOIPNoInput::GetComponentState(JVOIPComponentState **compstate)
{
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	*compstate = NULL;
	return 0;
}

int JVOIPNoInput::SetComponentState(JVOIPComponentState *compstate)
{
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	return 0;
}

std::string JVOIPNoInput::GetComponentName()
{
	return std::string("JVOIPNoInput");
}

std::string JVOIPNoInput::GetComponentDescription()
{
	return std::string("JVOIPLIB Internal 'no input' module");
}

std::vector<JVOIPCompParamInfo> *JVOIPNoInput::GetComponentParameters() throw (JVOIPException)
{
	return NULL;
}

