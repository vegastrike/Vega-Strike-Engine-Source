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

#include "jvoipgsmcompression.h"
#include "jvoiperrors.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <list>
#ifdef WIN32
	#include <winsock2.h>
#else
	#include <sys/time.h>
	#include <sys/types.h>
	#include <unistd.h>
#endif // WIN32

#include "debugnew.h"

#define JVOIPGSMCOMPRESSION_SAMPLEINTERVAL			20
#define JVOIPGSMCOMPRESSION_SAMPLERATE				8000
#define JVOIPGSMCOMPRESSION_FRAMESIZE				33
#define JVOIPGSMCOMPRESSION_EIGHTBITSCALE			256
#define JVOIPGSMCOMPRESSION_CHECKDELAY				(60*4)
#define JVOIPGSMCOMPRESSION_TIMEOUTDELAY			(60*10)

JVOIPGSMCompression::JVOIPGSMCompression(JVOIPSession *sess) : JVOIPCompressionModule(sess)
{
	compinit = false;
	localgsmstate = NULL;
	stopthreadmutex.Init();
	statemutex.Init();
}

JVOIPGSMCompression::~JVOIPGSMCompression()
{
	CleanupCompressor();
	CleanupDecompressor();
}

int JVOIPGSMCompression::InitCompressor(int sampinterval,int inputsamprate,int inputbytespersample,const JVOIPComponentParams *componentparams)
{
	if (compinit)
		return ERR_JVOIPLIB_GENERAL_COMPONENTALREADYINIT;

	if (sampinterval <= 0 || sampinterval%JVOIPGSMCOMPRESSION_SAMPLEINTERVAL != 0)
		return ERR_JVOIPLIB_GSMCOMP_UNSUPPORTEDSAMPLEINTERVAL;

	if (inputsamprate != JVOIPGSMCOMPRESSION_SAMPLERATE)
		return ERR_JVOIPLIB_GSMCOMP_UNSUPPORTEDSAMPLERATE;

	localgsmstate = gsm_create();
	if (localgsmstate == NULL)
		return ERR_JVOIPLIB_GENERAL_OUTOFMEM;
		
	compinit = true;
	return 0;
}

int JVOIPGSMCompression::CleanupCompressor()
{
	if (!compinit)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	
	gsm_destroy(localgsmstate);
	compinit = false;
	return 0;
}

int JVOIPGSMCompression::InitDecompressor()
{
	stopthread = false;
	if (Start() < 0) // Start the thread which cleans the GSM state info map from time to time
		return ERR_JVOIPLIB_GSMCOMP_CANTSTARTTHREAD;
	return 0;
}

int JVOIPGSMCompression::CleanupDecompressor()
{
	// We have to clean up the gsm states

	std::map<VoIPFramework::VOIPuint64,GSMStateInfo *>::const_iterator it;

	StopThread();
	
	for (it = gsmstates.begin() ; it != gsmstates.end() ; ++it)
		delete it->second; // delete the GSMStateInfo part;
	gsmstates.clear();
	return 0;
}

int JVOIPGSMCompression::Compress(VoIPFramework::VoiceBlock *vb)
{
	unsigned char *frame,*data;
	gsm_signal signalbuf[160];
	int datalen,numsamples,numframes;
	int i,j,offset,frameoffset;

	offset = 0;
	frameoffset = 0;
	
	data = vb->GetSamples(false);
	datalen = vb->GetNumBytes();
	numsamples = vb->GetNumSamples();
	numframes = numsamples / 160;
	if (numsamples % 160 != 0)
		numframes++;

	frame = new unsigned char[JVOIPGSMCOMPRESSION_FRAMESIZE*numframes];
	if (frame == NULL)
		return ERR_JVOIPLIB_GENERAL_OUTOFMEM;
			
	while (offset < numsamples)
	{
		int samp;

		samp = numsamples-offset;
		if (samp > 160)
			samp = 160;
			
		memset(signalbuf,0,sizeof(gsm_signal) * 160);
		// fill in the signal buffer

		if (vb->GetBytesPerSample() == 1)
		{
			for (i = 0 ; i < samp ; i++)
			{
				int val;
	
				val = (((int)data[i+offset])-127)*JVOIPGSMCOMPRESSION_EIGHTBITSCALE;
				signalbuf[i] = (gsm_signal)val;
			}
		}
		else // two bytes per sample
		{
			for (i = 0,j = offset*2 ; i < samp ; i++,j += 2)
			{
				int val;
			
				val = (((int)data[j])<<8)|((int)data[j+1]);
				val -= 32767;
				signalbuf[i] = (gsm_signal)val;
			}
		}
	
	
		gsm_encode(localgsmstate,signalbuf,frame+frameoffset);
		frameoffset += JVOIPGSMCOMPRESSION_FRAMESIZE;
		offset += 160;
	}

	vb->SetSamples(frame,JVOIPGSMCOMPRESSION_FRAMESIZE*numframes);

	return 0;
}

int JVOIPGSMCompression::Decompress(VoIPFramework::VoiceBlock *vb,VoIPFramework::VOIPuint64 sourceid)
{
	unsigned char *frame;
	unsigned char *speech;
	gsm_signal signalbuf[160];
	int framelen,i,j,frameoff,numframes,off;
	GSMStateInfo *stateinf;

	frame = vb->GetSamples(false);
	if (frame == NULL)
		return 0;
	framelen = vb->GetNumBytes();
	if (framelen <= 0 || (framelen % JVOIPGSMCOMPRESSION_FRAMESIZE) != 0)
	{
		vb->Clear();
		return 0;
	}
	numframes = framelen/JVOIPGSMCOMPRESSION_FRAMESIZE;

	// Obtain the gsm state info for this participant. This is done by the
	// map. If the key isn't found, the map creates a default entry, so
	// in that case the gsmstate will be NULL.

	statemutex.Lock();
	
	stateinf = gsmstates[sourceid];
	if (stateinf == NULL)
	{
		stateinf = new GSMStateInfo();
		if (stateinf == NULL)
		{
			statemutex.Unlock();
			return ERR_JVOIPLIB_GENERAL_OUTOFMEM;
		}
		gsmstates[sourceid] = stateinf;
	}
	
	if (stateinf->gsmstate == NULL)
	{
		stateinf->gsmstate = gsm_create();
		if (stateinf->gsmstate == NULL)
		{
			statemutex.Unlock();
			return ERR_JVOIPLIB_GENERAL_OUTOFMEM;
		}
	}
	else
	{
		// update the time so we can delete a member after a long
		// time of inactivity...
		stateinf->lasttime = time(NULL);
	}

	// We'll encode it in 16 bit values. This way we have more precision.
	// Conversion to the right output rate and sample type will be done
	// in the mixer.

	speech = new unsigned char[160*2*numframes];
	if (speech == NULL)
	{
		statemutex.Unlock();
		return ERR_JVOIPLIB_GENERAL_OUTOFMEM;
	}

	// Now, we simply have to decode the frame and put it in the voiceblock

	for (frameoff = 0,off = 0 ; frameoff < framelen ; frameoff += JVOIPGSMCOMPRESSION_FRAMESIZE,off += 160*2)
	{
		gsm_decode(stateinf->gsmstate,frame+frameoff,signalbuf);
  		
		for (i = 0,j = off ; i < 160 ; i++,j += 2)
		{
			int val;

			val = (int)signalbuf[i];
			val = val+32767;
			if (val < 0)
				val = 0;
			else if (val > 65535)
				val = 65535;
			speech[j] = (unsigned char)((val>>8)&0xFF);
			speech[j+1] = (unsigned char)(val&0xFF);
		}
	}
	
	statemutex.Unlock();

	// Now, we simply have to store the decodes samples in the voiceblock

	vb->SetSamples(speech,160*2*numframes);
	vb->SetNumSamples(160*numframes);
	vb->SetBytesPerSample(2);
	
	return 0;
}

bool JVOIPGSMCompression::SupportsSampleInterval(int ival)
{
	if (!compinit)
		return false;
	if (ival <= 0 || ival%JVOIPGSMCOMPRESSION_SAMPLEINTERVAL != 0)
		return false;
	return true;
}

int JVOIPGSMCompression::SetSampleInterval(int ival)
{
	if (!compinit)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	if (ival <= 0 || ival%JVOIPGSMCOMPRESSION_SAMPLEINTERVAL != 0)
		return ERR_JVOIPLIB_GSMCOMP_UNSUPPORTEDSAMPLEINTERVAL;
	return 0;
}

bool JVOIPGSMCompression::SupportsInputSamplingRate(int irate)
{
	if (!compinit)
		return false;
	if (irate != JVOIPGSMCOMPRESSION_SAMPLERATE)
		return false;
	return true;
}

int JVOIPGSMCompression::SetInputSamplingRate(int irate)
{
	if (!compinit)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	if (irate != JVOIPGSMCOMPRESSION_SAMPLERATE)
		return ERR_JVOIPLIB_GSMCOMP_UNSUPPORTEDSAMPLERATE;
	return 0;
}

bool JVOIPGSMCompression::SupportsInputBytesPerSample(int inputbytespersample)
{
	if (!compinit)
		return false;
	return true;
}

int JVOIPGSMCompression::SetInputBytesPerSample(int inputbytespersample)
{
	if (!compinit)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	return 0;
}

int JVOIPGSMCompression::GetComponentState(JVOIPComponentState **compstate)
{
	if (!compinit)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	*compstate = NULL;
	return 0;
}

int JVOIPGSMCompression::SetComponentState(JVOIPComponentState *compstate)
{
	if (!compinit)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	return 0;
}

std::string JVOIPGSMCompression::GetComponentName()
{
	return std::string("JVOIPGSMCompression");
}

std::string JVOIPGSMCompression::GetComponentDescription()
{
	return std::string("JVOIPLIB Internal GSM 06.10 13kbps compression module (uses libgsm)");
}

std::vector<JVOIPCompParamInfo> *JVOIPGSMCompression::GetComponentParameters() throw (JVOIPException)
{
	return NULL;
}

void *JVOIPGSMCompression::Thread()
{
	bool stop;
	time_t prevchecktime;

	prevchecktime = time(NULL);

	stopthreadmutex.Lock();
	stop = stopthread;
	stopthreadmutex.Unlock();
	while (!stop)
	{
		time_t curtime;
		struct timeval tv;
		
		tv.tv_sec = 0;
		tv.tv_usec = 100000; // 100 millisecods -> make sure we don't have to wait too long
		select(FD_SETSIZE,NULL,NULL,NULL,&tv);

		curtime = time(NULL);
		if ((curtime - prevchecktime) > JVOIPGSMCOMPRESSION_CHECKDELAY)
		{
			std::map<VoIPFramework::VOIPuint64,GSMStateInfo *>::iterator it;
			std::list<VoIPFramework::VOIPuint64> sourceids;
			std::list<VoIPFramework::VOIPuint64>::const_iterator it2;
			
			statemutex.Lock();
			it = gsmstates.begin();
			while (it != gsmstates.end())
			{
				if ((curtime - it->second->lasttime) > JVOIPGSMCOMPRESSION_TIMEOUTDELAY)
				{
					// NOTE: we cannot simply call an 'erase'
					// function because the map::erase function
					// does not return a new iterator. So, we'll
					// store the keys which need to be erased
					// in a list and delete all entries afterwards
					// Also note that we do need to erase the
					// GSMStateInfo* here
					delete it->second; 
					sourceids.push_back(it->first);
				}
				it++;
			}
			for (it2 = sourceids.begin() ; it2 != sourceids.end() ; ++it2)
				gsmstates.erase(*it2);
			statemutex.Unlock();
		
			prevchecktime = curtime;
		}

		
		stopthreadmutex.Lock();
		stop = stopthread;
		stopthreadmutex.Unlock();
	}

	return NULL;
}

void JVOIPGSMCompression::StopThread()
{
	time_t t = time(NULL);

	stopthreadmutex.Lock();
	stopthread = true;
	stopthreadmutex.Unlock();
	while (IsRunning() && (time(NULL) - t) < 5)
		;
	if (IsRunning())
	{
		std::cerr << "JVOIPGSMCompression::StopThread -- warning: having to kill thread" << std::endl;
		Kill();
	}
}
