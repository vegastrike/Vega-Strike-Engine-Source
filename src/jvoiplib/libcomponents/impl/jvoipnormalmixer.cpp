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

#include "jvoipnormalmixer.h"
#include "jvoiperrors.h"
#include "voiceblock.h"
#include <iostream>
#include <string.h>

#include "debugnew.h"

#define JVOIPNORMMIX_MAXFUTUREDIST						10.0 // ten seconds

// the component state
	
class JVOIPNormalMixerState : public JVOIPComponentState
{
public:
	JVOIPNormalMixerState(VoIPFramework::VOIPdouble off) { offset = off; }
	
	VoIPFramework::VOIPdouble Offset() { return offset; }
private:
	VoIPFramework::VOIPdouble offset;
};
	
// the component

JVOIPNormalMixer::JVOIPNormalMixer(JVOIPSession *sess) : JVOIPMixer(sess)
{
	init = false;
}

JVOIPNormalMixer::~JVOIPNormalMixer()
{
	Cleanup();
}

int JVOIPNormalMixer::Init(int sampinterval,int outputsamprate,int outputbytespersample,bool stereo,const JVOIPComponentParams *componentparams)
{
	if (init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTALREADYINIT;

	sampleinterval = sampinterval;
	samplerate = outputsamprate;
	bytespersample = outputbytespersample;
	isstereo = stereo;	
	
	sampleoffset = 0;
	realtotaltime = 0;
	wantedtotaltime = 0;
	
	init = true;
	return 0;
}

int JVOIPNormalMixer::Cleanup()
{
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	
	DeleteMixerBlocks();
	init = false;
	return 0;
}

int JVOIPNormalMixer::AddBlock(VoIPFramework::VoiceBlock *vb,VoIPFramework::VOIPuint64 sourceid)
{
	VoIPFramework::VOIPuint64 samplenum;
	std::list<MixerBlock*>::iterator it;
	int status;

	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;

	if (vb->GetSamples(false) == NULL)
		return 0;
	
	// if the samples are too far into the future, we're gonna ignore them (to avoid memory shortage)
	if ((vb->GetSampleOffset() - sampleoffset) > JVOIPNORMMIX_MAXFUTUREDIST)
		return 0;
	
	// if the sample block is too late, we'll ignore it too
	if (IsTooLate(vb))
		return 0;
	
	// convert to the right sample rate etc
	if ((status = AdjustSamples(vb)) < 0)
		return status;
	
	if (vb->GetSamples(false) == NULL)
		return 0;
		
	// search for the mixer block where we have to start inserting the data
	samplenum = CalculateSampleNumber(vb->GetSampleOffset());
	if ((status = GetStartMixerBlock(samplenum,&it)) < 0)
		return status;
	if ((status = CreateAllMemoryBlocks(it,samplenum+vb->GetNumSamples()-1)) < 0)
		return status;
		
	FillInSamples(it,vb,samplenum);
	return 0;
}

int JVOIPNormalMixer::GetSampleBlock(VoIPFramework::VoiceBlock *vb)
{
	std::list<MixerBlock*>::iterator it;
	unsigned char *data;
	int num,len;

	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;

	vb->SetSampleOffset(sampleoffset);
				
	it = mixerblocks.begin();
	if (it == mixerblocks.end())
	{
		num = CalculateNewNumSamples();
		len = num;
		if (isstereo)
			len *= 2;
		if (bytespersample != 1)
			len *= 2;
			
		data = new unsigned char[len];
		if (data == NULL)
			return ERR_JVOIPLIB_GENERAL_OUTOFMEM;
		SetSilence(data,num);
		
		sampleoffset += ((VoIPFramework::VOIPdouble)num)/((VoIPFramework::VOIPdouble)samplerate);
	}
	else
	{
		data = (*it)->data;
		num = (*it)->numsamples;
		len = (*it)->datalen;

#ifdef WIN32
		// the conversion from uint64 to double isn't implemented in Visual C++ 6
		sampleoffset = VoIPFramework::GetVOIPdouble((*it)->samplenum+(*it)->numsamples)/((VoIPFramework::VOIPdouble)samplerate);
#else
		sampleoffset = ((VoIPFramework::VOIPdouble)((*it)->samplenum+(*it)->numsamples))/((VoIPFramework::VOIPdouble)samplerate);
#endif // WIN32
		
		(*it)->data = NULL; // to avoid it being deleted in the destructor of mixerblock
		delete (*it);
		mixerblocks.erase(it);
		
		if (data == NULL)
		{
			len = num;
			if (isstereo)
				len *= 2;
			if (bytespersample != 1)
				len *= 2;
			
			data = new unsigned char[len];
			if (data == NULL)
				return ERR_JVOIPLIB_GENERAL_OUTOFMEM;
			SetSilence(data,num);
		}
	}

	vb->SetSamples(data,len);
	vb->SetNumSamples(num);
	vb->SetStereo(isstereo);
	vb->SetBytesPerSample(bytespersample);
	vb->SetSampleRate(samplerate);
	vb->SetIsSilence(false);
	vb->SetInfo3D(NULL,0);
	vb->SetMajorCompressionID(0);
	vb->SetMinorCompressionID(0);
	
	return 0;
}

int JVOIPNormalMixer::GetSampleOffset(VoIPFramework::VOIPdouble *offset)
{
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	*offset = sampleoffset;
	return 0;
}

bool JVOIPNormalMixer::SupportsSampleInterval(int ival)
{
	if (!init)
		return false;
	return true;
}

bool JVOIPNormalMixer::SupportsOutputSamplingRate(int orate)
{
	if (!init)
		return false;
	return true;
}

bool JVOIPNormalMixer::SupportsOutputBytesPerSample(int outputbytespersample)
{
	if (!init)
		return false;
	return true;
}

int JVOIPNormalMixer::SetSampleInterval(int ival)
{
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	
	DeleteMixerBlocks();
	sampleinterval = ival;
	return 0;
}

int JVOIPNormalMixer::SetOutputSamplingRate(int orate)
{
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	
	DeleteMixerBlocks();
	samplerate = orate;
	return 0;
}

int JVOIPNormalMixer::SetOutputBytesPerSample(int outputbytespersample)
{
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	
	DeleteMixerBlocks();
	bytespersample = outputbytespersample;
	return 0;
}

int JVOIPNormalMixer::SetStereo(bool s)
{
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	
	DeleteMixerBlocks();
	isstereo = s;
	return 0;
}

int JVOIPNormalMixer::GetComponentState(JVOIPComponentState **compstate)
{
	JVOIPNormalMixerState *state;
	
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	
	state = new JVOIPNormalMixerState(sampleoffset);
	if (state == NULL)
		return ERR_JVOIPLIB_GENERAL_OUTOFMEM;
	*compstate = state;
	return 0;
}

int JVOIPNormalMixer::SetComponentState(JVOIPComponentState *compstate)
{
	JVOIPNormalMixerState *state;
	
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	if (compstate == NULL)
		return ERR_JVOIPLIB_GENERAL_ILLEGALSTATEPARAMETER;
	
	state = dynamic_cast<JVOIPNormalMixerState *>(compstate);
	if (state == NULL)
		return ERR_JVOIPLIB_GENERAL_ILLEGALSTATEPARAMETER;
	
	sampleoffset = state->Offset();
	return 0;
}

std::string JVOIPNormalMixer::GetComponentName()
{
	return std::string("JVOIPNormalMixer");
}

std::string JVOIPNormalMixer::GetComponentDescription()
{
	return std::string("JVOIPLIB Internal mixer");
}

std::vector<JVOIPCompParamInfo> *JVOIPNormalMixer::GetComponentParameters() throw (JVOIPException)
{
	// NOTE: this mixer doens't have any options
	return NULL;
}

void JVOIPNormalMixer::DeleteMixerBlocks()
{
	std::list<MixerBlock*>::const_iterator it;
	
	for (it = mixerblocks.begin() ; it != mixerblocks.end() ; ++it)
		delete (*it);
	mixerblocks.clear();
}

inline VoIPFramework::VOIPuint64 JVOIPNormalMixer::CalculateSampleNumber(VoIPFramework::VOIPdouble offset)
{
	VoIPFramework::VOIPuint64 id;
	
	// we're gonna calculate the sample number at this offset
	id = (VoIPFramework::VOIPuint64)((offset*((VoIPFramework::VOIPdouble)samplerate))+0.5);
	return id;
}

int JVOIPNormalMixer::AdjustSamples(VoIPFramework::VoiceBlock *vb)
{
	int len,newlen,numsamples;
	unsigned char *data;
	
	sampconv.SetConversionParams((int)vb->GetSampleRate(),vb->IsStereo(),vb->GetBytesPerSample(),false,false,
				     samplerate,isstereo,bytespersample,false,false);
	
	numsamples = (int)((((double)vb->GetNumSamples())*((double)samplerate))/((double)vb->GetSampleRate()));
	len = numsamples;
	if (isstereo)
		len *= 2;
	if (bytespersample != 1)
		len *= 2;

	data = new unsigned char[len];
	if (data == NULL)
		return ERR_JVOIPLIB_GENERAL_OUTOFMEM;
	if ((newlen = sampconv.Convert(vb->GetSamples(false),vb->GetNumBytes(),data,len)) < 0)
	{
		delete [] data;
		vb->SetSamples(NULL,0);
		return 0;
	}
	
	numsamples = newlen;
	if (isstereo)
		numsamples /= 2;
	if (bytespersample != 1)
		numsamples /= 2;
	
	vb->SetSamples(data,newlen);
	vb->SetNumSamples(numsamples);
	vb->SetSampleRate(samplerate);
	vb->SetBytesPerSample(bytespersample);
	vb->SetStereo(isstereo);
	return 0;
}

inline bool JVOIPNormalMixer::IsTooLate(VoIPFramework::VoiceBlock *vb)
{
	VoIPFramework::VOIPdouble lastsampleoffset;
	
	lastsampleoffset = vb->GetSampleOffset();
	lastsampleoffset += ((VoIPFramework::VOIPdouble)vb->GetNumSamples())/((VoIPFramework::VOIPdouble)vb->GetSampleRate());
	if (lastsampleoffset < sampleoffset)
		return true;
	return false;
}

int JVOIPNormalMixer::GetStartMixerBlock(VoIPFramework::VOIPuint64 samplenum,std::list<MixerBlock*>::iterator *i)
{
	std::list<MixerBlock*>::iterator it;
	VoIPFramework::VOIPuint64 lastsamplenum,startsamplenum;
	bool found,gotlastsamplenum,done;
		
	found = false;
	gotlastsamplenum = false;
	it = mixerblocks.begin();
	while (!found && it != mixerblocks.end())
	{
		if (samplenum < (*it)->samplenum)
			found = true;
		else if (samplenum >= (*it)->samplenum && samplenum < ((*it)->samplenum+(*it)->numsamples))
			found = true;
		else
		{
			gotlastsamplenum = true;
			lastsamplenum = (*it)->samplenum+(*it)->numsamples-1;
			++it;
		}
	}
	if (found)
	{
		*i = it;
		return 0;
	}
	
	// In this case, we have to build the list

	if (gotlastsamplenum)
		startsamplenum = lastsamplenum+1;
	else
		startsamplenum = CalculateSampleNumber(sampleoffset);
	
	done = false;
	while (!done)
	{
		int num;
		MixerBlock *blk;
		
		num = CalculateNewNumSamples();
		blk = new MixerBlock(startsamplenum);
		if (blk == NULL)
			return ERR_JVOIPLIB_GENERAL_OUTOFMEM;
		blk->numsamples = num;
		
		mixerblocks.push_back(blk);
		if (samplenum < startsamplenum || (samplenum >= startsamplenum && samplenum < (startsamplenum + num)))
		{
			it = mixerblocks.end();
			--it;
			done = true;
		}
		else
			startsamplenum += num;
	}
	*i = it;
	return 0;
}

int JVOIPNormalMixer::CalculateNewNumSamples()
{
	double diff;
	int numsamples;

	wantedtotaltime += ((double)sampleinterval)/1000.0;
	diff = wantedtotaltime - realtotaltime;
	
	numsamples = (int)((((double)samplerate)*diff)+0.5);
	if (numsamples < 1)
		numsamples = 1;
	realtotaltime += ((double)numsamples)/((double)samplerate);
	
	return numsamples;
}

int JVOIPNormalMixer::CreateAllMemoryBlocks(std::list<MixerBlock*>::iterator i,VoIPFramework::VOIPuint64 endsamplenum)
{
	bool done;
	VoIPFramework::VOIPuint64 lastsampnum;
	std::list<MixerBlock*>::iterator it = i;
	int multiplier;
	
	multiplier = 1;
	if (isstereo)
		multiplier *= 2;
	if (bytespersample != 1)
		multiplier *= 2;
	
	done = false;
	while (!done)
	{
		lastsampnum = (*it)->samplenum+(*it)->numsamples-1;
		
		if ((*it)->data == NULL)
		{
			int len;
			unsigned char *data;
			
			len = (*it)->numsamples*multiplier;
			data = new unsigned char[len];
			if (data == NULL)
				return ERR_JVOIPLIB_GENERAL_OUTOFMEM;
			(*it)->data = data;
			(*it)->datalen = len;
			SetSilence(data,(*it)->numsamples);
		}
		
		if ((*it)->samplenum >= endsamplenum && endsamplenum < ((*it)->samplenum+(*it)->numsamples))
			done = true;
		else
		{
			++it;
			if (it == mixerblocks.end())
			{
				MixerBlock *blk;
				
				blk = new MixerBlock(lastsampnum+1);
				if (blk == NULL)
					return ERR_JVOIPLIB_GENERAL_OUTOFMEM;
				blk->numsamples = CalculateNewNumSamples();
				mixerblocks.push_back(blk);
				it = mixerblocks.end();
				--it;
			}
		}
	}
	return 0;
}

void JVOIPNormalMixer::SetSilence(unsigned char *data,int numsamples)
{
	int steps;
	
	steps = numsamples;
	if (isstereo)
		steps *= 2;
	
	if (bytespersample == 1)
	{
		// unsigned!
		memset(data,127,steps);
	}
	else
	{
		int i,j;
		
		for (i = 0,j = 0 ; i < steps ; i++,j += 2)
		{
			// big endian and unsigned!
			data[j] = 127;
			data[j+1] = 255;
		}
	}
}

void JVOIPNormalMixer::FillInSamples(std::list<MixerBlock*>::iterator it2,VoIPFramework::VoiceBlock *vb,VoIPFramework::VOIPuint64 startsamplenum)
{
	std::list<MixerBlock*>::iterator it = it2;
	VoIPFramework::VOIPuint64 cursamplenum,endsamplenum;
	bool done;
	int offset,loops,i,j;
	int val,val2;
	unsigned char *data,*data2;
	
	if (isstereo)
		loops = 2;
	else
		loops = 1;
	
	endsamplenum = startsamplenum+vb->GetNumSamples()-1;
	if (startsamplenum < (*it)->samplenum)
	{
		cursamplenum = (*it)->samplenum;
		offset = 0;
		
		j = cursamplenum-startsamplenum;
		if (isstereo)
			j *= 2;
		if (bytespersample != 1)
			j *= 2;
	}
	else
	{
		cursamplenum = startsamplenum;
		
		offset = cursamplenum-(*it)->samplenum;
		if (isstereo)
			offset *= 2;
		if (bytespersample != 1)
			offset *= 2;
		
		j = 0;
	}
	
	data = (*it)->data;
	data2 = vb->GetSamples(false);
	done = false;
	while(!done)
	{
		for (i = 0 ; i < loops ; i++)
		{
			if (bytespersample == 1)
			{
				val = (int)data[offset];
				val -= 127;
				val2 = (int)data2[j];
				val2 -= 127;
				
				val += val2;
				val += 127;
				if (val < 0)
					val = 0;
				else if (val > 255)
					val = 255;
				
				data[offset] = (unsigned char)val;
				offset++;
				j++;
			}
			else
			{
				val = (int)((((int)data[offset])<<8)|((int)data[offset+1]));
				val -= 32767;
				val2 = (int)((((int)data2[j])<<8)|((int)data2[j+1]));
				val2 -= 32767;
			
				val += val2;
				val += 32767;
				if (val < 0)
					val = 0;
				else if (val > 65535)
					val = 65535;
					
				data[offset] = (unsigned char)((val>>8)&0xFF);
				data[offset+1] = (unsigned char)(val&0xFF);
				offset += 2;
				j += 2;
			}
		}
		
		cursamplenum++;
		if (cursamplenum > endsamplenum)
			done = true;
		else
		{
			if (offset >= (*it)->datalen)
			{
				++it;
				offset = 0;
				data = (*it)->data;
			}	
		}
	}
}
