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

#include "jvoipsimplelocalisation.h"
#include <math.h>

#include "debugnew.h"

#define JVOIPSIMPLELOCALISATION_HEADRADIUS				0.15
#define JVOIPSIMPLELOCALISATION_SPEEDOFSOUND				343.0 // meters/second

JVOIPSimpleLocalisation::JVOIPSimpleLocalisation(JVOIPSession *sess):JVOIPSessionLocalisation(sess)
{
	init = false;
}

JVOIPSimpleLocalisation::~JVOIPSimpleLocalisation()
{
	Cleanup();
}

int JVOIPSimpleLocalisation::Init(int outputsamprate,int outputbytespersample,const JVOIPComponentParams *componentparams)
{
	if (init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTALREADYINIT;
	samplerate = outputsamprate;
	bytespersample = outputbytespersample;
	init = true;
	return 0;
}

int JVOIPSimpleLocalisation::Cleanup()
{
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	init = false;
	return 0;
}

int JVOIPSimpleLocalisation::Create3DEffect(double local_x,double local_y,double local_z,
                                            double righteardir_x,double righteardir_y,double righteardir_z,
        		                    double frontdir_x,double frontdir_y,double frontdir_z,
					    double updir_x,double updir_y,double updir_z,
                                            double remote_x,double remote_y,double remote_z,
                                            VoIPFramework::VoiceBlock *vb,
                                            VoIPFramework::VOIPuint64 sourceid)
{
	double vectlen;
	int status,len,newlen,maxsampoff;
	int rightadd,leftadd;
	int i,val;

	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;

	if (vb->IsStereo()) // we can't handle this (and it shouldn't happen either
	{
		// Silence!
		vb->Clear();
		return 0;
	}

	// First, we'll get the necessary data

	data = vb->GetSamples(false);
	if (data == NULL)
		return 0;
	datalen = vb->GetNumBytes();
	if (datalen <= 0)
		return 0;

	vblock = vb;

	localpos[0] = local_x;
	localpos[1] = local_y;
	localpos[2] = local_z;

	vectlen = sqrt(righteardir_x*righteardir_x + righteardir_y*righteardir_y + righteardir_z*righteardir_z);
	righteardir[0] = righteardir_x/vectlen;
	righteardir[1] = righteardir_y/vectlen;
	righteardir[2] = righteardir_z/vectlen;

	remotepos[0] = remote_x;
	remotepos[1] = remote_y;
	remotepos[2] = remote_z;

	// Calculate distance to each ear (also time-distance)

	CalculateDistances();

	// Calculate the Interaural Time Difference (ITD), expressed in samples

	leftearsampleoffset = (int)((lefteartimedist*(double)samplerate)+0.5);
	rightearsampleoffset = (int)((righteartimedist*(double)samplerate)+0.5);

	// Calculate the amplitude factors, used to calculate the Interaural Intensity Difference (IID)

	leftearampfactor = JVOIPSIMPLELOCALISATION_HEADRADIUS/lefteardist;
	rightearampfactor = JVOIPSIMPLELOCALISATION_HEADRADIUS/righteardist;

	// Allocate a new buffer
	if ((status = AllocateNewBuffer()) < 0)
	{
		vb->Clear();
		return status;
	}

	// Convert the original data to the right samplerate and bytes per sample

	sampconv.SetConversionParams((int)vb->GetSampleRate(),false,(int)vb->GetBytesPerSample(),false,false,
	                             samplerate,true,bytespersample,false,false);

	maxsampoff = (leftearsampleoffset > rightearsampleoffset)?leftearsampleoffset:rightearsampleoffset;
	newlen = newdatalen-maxsampoff*bytespersample*2; // the '2' is for stereo
	len = sampconv.Convert(data,datalen,newdata,newlen);
	if (len < 2)
	{
		vb->Clear();
		return 0;
	}

	if (len < newlen)
	{
		if (bytespersample == 1)
		{
			unsigned char copybyte;

			copybyte = newdata[len-1];
			while (len < newlen)
				newdata[len++] = copybyte;
		}
		else // two bytes per sample
		{
			unsigned char copybyte1,copybyte2;

			copybyte1 = newdata[len-2];
			copybyte2 = newdata[len-1];
			while (len < newlen)
			{
				newdata[len++] = copybyte1;
				newdata[len++] = copybyte2;
			}
		}
	}

	// Insert the right offsets and apply the amplitude factors

	rightadd = rightearsampleoffset*2*bytespersample;
	leftadd = leftearsampleoffset*2*bytespersample;

	if (bytespersample == 1)
	{
		for (i = newlen-1 ; i >= 0 ; i -= 2)
		{
			// for the right ear
			val = ((int)newdata[i])-127;
			val = (int)((((double)val)*rightearampfactor)+0.5);
			val += 127;
			if (val > 255)
				val = 255;
			else if (val < 0)
				val = 0;
			newdata[i+rightadd] = (unsigned char)val;

			// for the left ear
			val = ((int)newdata[i-1])-127;
			val = (int)((((double)val)*leftearampfactor)+0.5);
			val += 127;
			if (val > 255)
				val = 255;
			else if (val < 0)
				val = 0;
			newdata[i-1+leftadd] = (unsigned char)val;
		}

		// set some stuff to silence
		for (i = newlen+leftadd ; i < newdatalen ; i += 2)
			newdata[i] = 127;
		for (i = newlen+1+rightadd ; i < newdatalen ; i += 2)
			newdata[i] = 127;
		for (i = 0 ; i < leftadd ; i += 2)
			newdata[i] = 127;
		for (i = 1 ; i < rightadd+1 ; i += 2)
			newdata[i] = 127;
	}
	else // two bytes per sample
	{
		for (i = newlen-2 ; i >= 0 ; i -= 4)
		{
			// for the right ear
			val = (((int)newdata[i+1])|(((int)newdata[i])<<8))-32767;
			val = (int)((((double)val)*rightearampfactor)+0.5);
			val += 32767;
			if (val > 65535)
				val = 65535;
			else if (val < 0)
				val = 0;
			newdata[i+rightadd] = (unsigned char)((val>>8)&255);
			newdata[i+rightadd+1] = (unsigned char)(val&255);

			// for the left ear

			val = (((int)newdata[i-1])|(((int)newdata[i-2])<<8))-32767;
			val = (int)((((double)val)*rightearampfactor)+0.5);
			val += 32767;
			if (val > 65535)
				val = 65535;
			else if (val < 0)
				val = 0;
			newdata[i-2+leftadd] = (unsigned char)((val>>8)&255);
			newdata[i-1+leftadd] = (unsigned char)(val&255);
		}

		// set some stuff to silence
		for (i = newlen+leftadd ; i < newdatalen ; i += 4)
		{
			newdata[i] = 127;
			newdata[i+1] = 255;
		}
		for (i = newlen+2+rightadd ; i < newdatalen ; i += 4)
		{
			newdata[i] = 127;
			newdata[i+1] = 255;
		}
		for (i = 0 ; i < leftadd ; i += 4)
		{
			newdata[i] = 127;
			newdata[i+1] = 255;
		}
		for (i = 2 ; i < rightadd+2 ; i += 4)
		{
			newdata[i] = 127;
			newdata[i+1] = 255;
		}
	}

	vb->SetSamples(newdata,newdatalen);
	vb->SetNumSamples(newnumsamples);
	vb->SetStereo(true);
	vb->SetSampleRate(samplerate);
	vb->SetBytesPerSample(bytespersample);

	return 0;
}

bool JVOIPSimpleLocalisation::SupportsOutputSamplingRate(int orate)
{
	if (!init)
		return false;
	return true;
}

int JVOIPSimpleLocalisation::SetOutputSamplingRate(int orate)
{
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	samplerate = orate;
	return 0;
}

bool JVOIPSimpleLocalisation::SupportsOutputBytesPerSample(int outputbytespersample)
{
	if (!init)
		return false;
	return true;
}

int JVOIPSimpleLocalisation::SetOutputBytesPerSample(int outputbytespersample)
{
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	bytespersample = outputbytespersample;
	return 0;
}

int JVOIPSimpleLocalisation::GetComponentState(JVOIPComponentState **compstate)
{
	// NOTE: nothing to do...
	*compstate = NULL;
	return 0;
}

int JVOIPSimpleLocalisation::SetComponentState(JVOIPComponentState *compstate)
{
	// NOTE: nothing to do...
	return 0;
}

std::string JVOIPSimpleLocalisation::GetComponentName()
{
	return std::string("JVOIPSimpleLocalisation");
}

std::string JVOIPSimpleLocalisation::GetComponentDescription()
{
	return std::string("JVOIPLIB Internal basic localisation module");
}

std::vector<JVOIPCompParamInfo> *JVOIPSimpleLocalisation::GetComponentParameters() throw (JVOIPException)
{
	return NULL;
}

void JVOIPSimpleLocalisation::CalculateDistances()
{
	double pos[3];
	double diff;
	int i;

	// calculate the right ear distance to the sound source

	for (i = 0 ; i < 3 ; i++)
		pos[i] = localpos[i]+righteardir[i]*JVOIPSIMPLELOCALISATION_HEADRADIUS;

	righteardist = 0;
        for (i = 0 ; i < 3 ; i++)
	{
		diff = pos[i]-remotepos[i];
		righteardist += diff*diff;
	}
	righteardist = sqrt(righteardist);
	if (righteardist < JVOIPSIMPLELOCALISATION_HEADRADIUS)
		righteardist = JVOIPSIMPLELOCALISATION_HEADRADIUS;

	// calculate the left ear distance to the sound source

	for (i = 0 ; i < 3 ; i++)
		pos[i] = localpos[i]-righteardir[i]*JVOIPSIMPLELOCALISATION_HEADRADIUS;

	lefteardist = 0;
        for (i = 0 ; i < 3 ; i++)
	{
		diff = pos[i]-remotepos[i];
		lefteardist += diff*diff;
	}
	lefteardist = sqrt(lefteardist);
	if (lefteardist < JVOIPSIMPLELOCALISATION_HEADRADIUS)
		lefteardist = JVOIPSIMPLELOCALISATION_HEADRADIUS;

	// calculate the time distance

	lefteartimedist = lefteardist/JVOIPSIMPLELOCALISATION_SPEEDOFSOUND;
	righteartimedist = righteardist/JVOIPSIMPLELOCALISATION_SPEEDOFSOUND;
}

int JVOIPSimpleLocalisation::AllocateNewBuffer()
{
	int maxaddsamples;
	double rateratio;

	rateratio = ((double)samplerate)/((double)vblock->GetSampleRate());
	newnumsamples = (int)((((double)vblock->GetNumSamples())*rateratio)+0.5);
        maxaddsamples = (rightearsampleoffset > leftearsampleoffset)?rightearsampleoffset:leftearsampleoffset;
	newnumsamples += maxaddsamples;

	newdatalen = newnumsamples*2; // we'll be using stereo
	if (bytespersample != 1)
		newdatalen *= 2;

	newdata = new unsigned char[newdatalen];
	if (newdata == NULL)
		return ERR_JVOIPLIB_GENERAL_OUTOFMEM;
	return 0;
}
