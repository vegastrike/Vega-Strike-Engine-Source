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

    ACKNOWLEDGEMENTS
    ----------------
    This 3D sound module uses Head-Related Impulse Response data, which
    was obtained from http://sound.media.mit.edu/KEMAR.html (compact.zip).
    The measurements were made by Bill Gardner and Keith Martin, so many
    thanks to them.
*/

#include "jvoiphrtflocalisation.h"
#include <math.h>

#include "debugnew.h"

#define JVOIPHRTFLOCALISATION_HEADRADIUS				0.15
#define JVOIPHRTFLOCALISATION_SPEEDOFSOUND				343.0 // meters/second
#define JVOIPHRTFLOCALISATION_SAMPLERATE				44100

// NOTE: The head related impulse responses were taken at 1.4 meters from the KEMAR model
#define JVOIPHRTFLOCALISATION_ORIGDIST					1.4

JVOIPHRTFLocalisation::JVOIPHRTFLocalisation(JVOIPSession *sess):JVOIPSessionLocalisation(sess)
{
	init = false;
}

JVOIPHRTFLocalisation::~JVOIPHRTFLocalisation()
{
	Cleanup();
}

int JVOIPHRTFLocalisation::Init(int outputsamprate,int outputbytespersample,const JVOIPComponentParams *componentparams)
{
	int status;
	
	if (init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTALREADYINIT;
	
	if (outputsamprate != JVOIPHRTFLOCALISATION_SAMPLERATE)
		return ERR_JVOIPLIB_HRTFLOC_UNSUPPORTEDSAMPLERATE;

	bytespersample = outputbytespersample;
	
	if ((status = CreateConvolutionFilters()) < 0)
		return status;
	
	init = true;
	return 0;
}

int JVOIPHRTFLocalisation::Cleanup()
{
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	DestroyConvolutionFilters();
	init = false;
	return 0;
}

int JVOIPHRTFLocalisation::Create3DEffect(double local_x,double local_y,double local_z,
                                          double righteardir_x,double righteardir_y,double righteardir_z,
        		                  double frontdir_x,double frontdir_y,double frontdir_z,
					  double updir_x,double updir_y,double updir_z,
                                          double remote_x,double remote_y,double remote_z,
                                          VoIPFramework::VoiceBlock *vb,
                                          VoIPFramework::VOIPuint64 sourceid)
{
	double vectlen;
	int status,len,newlen;
	
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

	vectlen = sqrt(frontdir_x*frontdir_x + frontdir_y*frontdir_y + frontdir_z*frontdir_z);
	frontdir[0] = frontdir_x/vectlen;
	frontdir[1] = frontdir_y/vectlen;
	frontdir[2] = frontdir_z/vectlen;

	vectlen = sqrt(updir_x*updir_x + updir_y*updir_y + updir_z*updir_z);
	updir[0] = updir_x/vectlen;
	updir[1] = updir_y/vectlen;
	updir[2] = updir_z/vectlen;

	remotepos[0] = remote_x;
	remotepos[1] = remote_y;
	remotepos[2] = remote_z;

	// Do some necessary calculations

	CalculateAzimuthAndElevation();
	CalculateDistances();

	// Allocate a new buffer
	if ((status = AllocateNewBuffer()) < 0)
	{
		vb->Clear();
		return status;
	}

	// Convert the original data to the right samplerate and bytes per sample

	sampconv.SetConversionParams((int)vb->GetSampleRate(),false,(int)vb->GetBytesPerSample(),false,false,
	                             JVOIPHRTFLOCALISATION_SAMPLERATE,true,bytespersample,false,false);

	newlen = newdatalen-128*bytespersample*2; // the '2' is for stereo
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

	// Do the convolution

	// first, we'll obtain the right convolution filters
	int index,numazimuths;
	double stepsize;
	float *leftfilt,*rightfilt;
	AzimuthData *azidat;

	index = elevation/10+4;
	azidat = filterdata[index].azimuths;
	numazimuths = filterdata[index].numazimuths;
	
	if (numazimuths <= 1)
		index = 0;
	else
	{
		stepsize = 180.0/(double)(numazimuths-1);
		index  = (int)((((double)azimuth)+stepsize/2.0)/((double)stepsize));
		if (index >= numazimuths)
			index = numazimuths-1;
		else if (index < 0)
			index = 0;
	}

	if (leftear)
	{
		leftfilt = azidat[index].rightvalues;
		rightfilt = azidat[index].leftvalues;
	}
	else
	{
		leftfilt = azidat[index].leftvalues;
		rightfilt = azidat[index].rightvalues;
	}
	
	// now we have the filter for the left samples in 'leftfilt' and in 'rightfilt' for the right samples
	
	int i,j,numsamp;
	float leftamp,rightamp;

	numsamp = newnumsamples-128;
	
	for (i = 0 ; i < newnumsamples ; i++)
	{
		samplebuf[i] = 0;
		samplebuf2[i] = 0;
	}
	
	// determine the amplitude factor for each ear... 

	leftamp = JVOIPHRTFLOCALISATION_ORIGDIST/(float)lefteardist;
	rightamp = JVOIPHRTFLOCALISATION_ORIGDIST/(float)righteardist;
	leftamp *= 0.5;
	rightamp *= 0.5;
	
	// process left ear samples

	if (bytespersample == 1)
	{
		for (j = 0,i = 0 ; i < numsamp ; i++,j += 2)
		{
			int val,val2,k,l;

			val = ((int)newdata[j])-127;
			val2 = ((int)newdata[j+1])-127;
			for (l = 0,k = i ; l < 128 ; k++,l++)
			{
				samplebuf[k] += (float)val*leftfilt[l];
				samplebuf2[k] += (float)val2*rightfilt[l];
			}
			val = ((int)(samplebuf[i]*leftamp))+127;
			val2 = ((int)(samplebuf2[i]*rightamp))+127;
			if (val > 255)
				val = 255;
			else if (val < 0)
				val = 0;
			if (val2 > 255)
				val2 = 255;
			else if (val2 < 0)
				val2 = 0;
			newdata[j] = (unsigned char)val;
			newdata[j+1] = (unsigned char)val2;
		}
		for (j = numsamp*2, i = numsamp ; i < newnumsamples ; i++,j += 2)
		{
			int val;

			val = ((int)(samplebuf[i]*leftamp))+127;
			if (val > 255)
				val = 255;
			else if (val < 0)
				val = 0;
			newdata[j] = (unsigned char)val;
			val = ((int)(samplebuf2[i]*rightamp))+127;
			if (val > 255)
				val = 255;
			else if (val < 0)
				val = 0;
			newdata[j+1] = (unsigned char)val;
		}
	}
	else // two bytes per sample
	{
		for (j = 0,i = 0 ; i < numsamp ; i++,j += 4)
		{
			int val,val2,k,l;

			val = ((((int)newdata[j])<<8)|((int)newdata[j+1]))-32767;
			val2 = ((((int)newdata[j+2])<<8)|((int)newdata[j+3]))-32767;
			for (l = 0,k = i ; l < 128 ; k++,l++)
			{
				samplebuf[k] += (float)val*leftfilt[l];
				samplebuf2[k] += (float)val2*rightfilt[l];
			}
			val = ((int)(samplebuf[i]*leftamp))+32767;
			val2 = ((int)(samplebuf2[i]*rightamp))+32767;
			if (val > 65535)
				val = 65535;
			else if (val < 0)
				val = 0;
			if (val2 > 65535)
				val2 = 65535;
			else if (val2 < 0)
				val2 = 0;
			newdata[j] = (unsigned char)((val>>8)&0xFF);
			newdata[j+1] = (unsigned char)(val&0xFF);
			newdata[j+2] = (unsigned char)((val2>>8)&0xFF);
			newdata[j+3] = (unsigned char)(val2&0xFF);
		}
		for (j = numsamp*4, i = numsamp ; i < newnumsamples ; i++,j += 4)
		{
			int val;

			val = ((int)(samplebuf[i]*leftamp))+32767;
			if (val > 65535)
				val = 65535;
			else if (val < 0)
				val = 0;
			newdata[j] = (unsigned char)((val>>8)&0xFF);
			newdata[j+1] = (unsigned char)(val&0xFF);
		
			val = ((int)(samplebuf2[i]*rightamp))+32767;
			if (val > 65535)
				val = 65535;
			else if (val < 0)
				val = 0;
			newdata[j+2] = (unsigned char)((val>>8)&0xFF);
			newdata[j+3] = (unsigned char)(val&0xFF);
		}
	}

	vb->SetSamples(newdata,newdatalen);
	vb->SetNumSamples(newnumsamples);
	vb->SetStereo(true);
	vb->SetSampleRate(JVOIPHRTFLOCALISATION_SAMPLERATE);
	vb->SetBytesPerSample(bytespersample);

	delete [] samplebuf;
	delete [] samplebuf2;

	return 0;
}

bool JVOIPHRTFLocalisation::SupportsOutputSamplingRate(int orate)
{
	if (!init)
		return false;
	if (orate != JVOIPHRTFLOCALISATION_SAMPLERATE)
		return false;
	return true;
}

int JVOIPHRTFLocalisation::SetOutputSamplingRate(int orate)
{
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	if (orate != JVOIPHRTFLOCALISATION_SAMPLERATE)
		return ERR_JVOIPLIB_HRTFLOC_UNSUPPORTEDSAMPLERATE;
	return 0;
}

bool JVOIPHRTFLocalisation::SupportsOutputBytesPerSample(int outputbytespersample)
{
	if (!init)
		return false;
	return true;
}

int JVOIPHRTFLocalisation::SetOutputBytesPerSample(int outputbytespersample)
{
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	bytespersample = outputbytespersample;
	return 0;
}

int JVOIPHRTFLocalisation::GetComponentState(JVOIPComponentState **compstate)
{
	// NOTE: nothing to do...
	*compstate = NULL;
	return 0;
}

int JVOIPHRTFLocalisation::SetComponentState(JVOIPComponentState *compstate)
{
	// NOTE: nothing to do...
	return 0;
}

std::string JVOIPHRTFLocalisation::GetComponentName()
{
	return std::string("JVOIPHRTFLocalisation");
}

std::string JVOIPHRTFLocalisation::GetComponentDescription()
{
	return std::string("JVOIPLIB Internal basic localisation module");
}

std::vector<JVOIPCompParamInfo> *JVOIPHRTFLocalisation::GetComponentParameters() throw (JVOIPException)
{
	return NULL;
}

void JVOIPHRTFLocalisation::CalculateDistances()
{
	double pos[3];
	double diff;
	int i;

	// calculate the right ear distance to the sound source

	for (i = 0 ; i < 3 ; i++)
		pos[i] = localpos[i]+righteardir[i]*JVOIPHRTFLOCALISATION_HEADRADIUS;

	righteardist = 0;
        for (i = 0 ; i < 3 ; i++)
	{
		diff = pos[i]-remotepos[i];
		righteardist += diff*diff;
	}
	righteardist = sqrt(righteardist);
	if (righteardist < JVOIPHRTFLOCALISATION_HEADRADIUS)
		righteardist = JVOIPHRTFLOCALISATION_HEADRADIUS;

	// calculate the left ear distance to the sound source

	for (i = 0 ; i < 3 ; i++)
		pos[i] = localpos[i]-righteardir[i]*JVOIPHRTFLOCALISATION_HEADRADIUS;

	lefteardist = 0;
        for (i = 0 ; i < 3 ; i++)
	{
		diff = pos[i]-remotepos[i];
		lefteardist += diff*diff;
	}
	lefteardist = sqrt(lefteardist);
	if (lefteardist < JVOIPHRTFLOCALISATION_HEADRADIUS)
		lefteardist = JVOIPHRTFLOCALISATION_HEADRADIUS;
}

int JVOIPHRTFLocalisation::AllocateNewBuffer()
{
	double rateratio;

	rateratio = ((double)JVOIPHRTFLOCALISATION_SAMPLERATE)/((double)vblock->GetSampleRate());
	newnumsamples = (int)((((double)vblock->GetNumSamples())*rateratio)+0.5);
	newnumsamples += 128; // for the 'tail' of the convolution filter

	newdatalen = newnumsamples*2; // we'll be using stereo
	if (bytespersample != 1)
		newdatalen *= 2;

	newdata = new unsigned char[newdatalen];
	if (newdata == NULL)
		return ERR_JVOIPLIB_GENERAL_OUTOFMEM;

	samplebuf = new float[newnumsamples];
	if (samplebuf == NULL)
	{
		delete [] newdata;
		return ERR_JVOIPLIB_GENERAL_OUTOFMEM;
	}
	samplebuf2 = new float[newnumsamples];
	if (samplebuf2 == NULL)
	{
		delete [] samplebuf;
		delete [] newdata;
	}
	return 0;
}

int JVOIPHRTFLocalisation::CreateConvolutionFilters()
{
	int elev,i,j,num;
	
	filterdata = new JVOIPHRTFLocalisation::ElevationData[14];
	if (filterdata == NULL)
		return ERR_JVOIPLIB_GENERAL_OUTOFMEM;
	
	for (i = 0, elev = -40 ; elev <= 90 ; i++,elev += 10)
	{
		JVOIPHRTFLocalisation::AzimuthData *data;
		JVOIPHRTFLocalisation::OrigAzimuthData *origdata;
		
		num = elevationdata[i].numazimuths;
		filterdata[i].elevation = elev;
		filterdata[i].numazimuths = num;
		
		data = new JVOIPHRTFLocalisation::AzimuthData[num];
		if (data == NULL)
		{
			delete [] filterdata;
			return ERR_JVOIPLIB_GENERAL_OUTOFMEM;
		}
		filterdata[i].azimuths = data;
		
		origdata = elevationdata[i].azimuthdata;
		for (j = 0 ; j < num ; j++)
		{
			data[j].azimuth = origdata[j].azimuth;
			data[j].numvalues = 128;
			data[j].leftvalues = new float[128];
			if (data[j].leftvalues == NULL)
			{
				delete [] filterdata;
				return ERR_JVOIPLIB_GENERAL_OUTOFMEM;
			}
			data[j].rightvalues = new float[128];
			if (data[j].rightvalues == NULL)
			{
				delete [] filterdata;
				return ERR_JVOIPLIB_GENERAL_OUTOFMEM;
			}

			int *src;
			float *dst;
			int k;

			src = origdata[j].leftresponse;
			dst = data[j].leftvalues;
			for (k = 0 ; k < 128 ; k++)
			{
				float v;

				v = (float)src[k];
				v /= 32768.0;
				dst[k] = v;
			}

			src = origdata[j].rightresponse;
			dst = data[j].rightvalues;
			for (k = 0 ; k < 128 ; k++)
			{
				float v;

				v = (float)src[k];
				v /= 32768.0;
				dst[k] = v;
			}
		}
	}
	
	return 0;
}

void JVOIPHRTFLocalisation::DestroyConvolutionFilters()
{
	delete [] filterdata;
}

#define TORAD(x) ((x)*3.1415926535897932384626433832795/180.0)
#define FROMRAD(x) ((x)*180.0/3.1415926535897932384626433832795)

double ArcCos(double x)
{
	double angle;

	if (x > 0.999999)
		angle = 0.0;
	else if (x < -0.999999)
		angle = 180.0;
	else
		angle = FROMRAD(acos(x));
	
	if (angle < 0.00001)
		angle = 0;

	return angle;
}

void JVOIPHRTFLocalisation::CalculateAzimuthAndElevation()
{
	double cosangle,prjlen;
	double angle,elev;
	double vect[3],vect2[3],remoteprj[3];
	double length;

	// get vector to remote position, starting from local position

	vect[0] = remotepos[0]-localpos[0];
	vect[1] = remotepos[1]-localpos[1];
	vect[2] = remotepos[2]-localpos[2];

	// get the vector length

	length = sqrt(vect[0]*vect[0]+vect[1]*vect[1]+vect[2]*vect[2]);

	// calculate the angle with the updir vector

	vect2[0] = vect[0]/length;
	vect2[1] = vect[1]/length;
	vect2[2] = vect[2]/length;

	cosangle = vect2[0]*updir[0]+vect2[1]*updir[1]+vect2[2]*updir[2];
	angle = ArcCos(cosangle);
	elev = 90.0-angle;
	
	elevation = 10*((int)((elev/10.0)+0.5));
	if (elevation < -40)
		elevation = -40;
	else if (elevation > 90)
		elevation = 90;

	// calculate the azimuth

	// first, we get the projection of the remote soundsource position into the
	// azimuth plane, which is formed by the righteardir and frontdir vectors

	prjlen = vect[0]*updir[0]+vect[1]*updir[1]+vect[2]*updir[2];
	remoteprj[0] = vect[0]-prjlen*updir[0];
	remoteprj[1] = vect[1]-prjlen*updir[1];
	remoteprj[2] = vect[2]-prjlen*updir[2];

	// now, we calculate the angle of this vector with the frontdir vector
	length = sqrt(remoteprj[0]*remoteprj[0]+remoteprj[1]*remoteprj[1]+remoteprj[2]*remoteprj[2]);
	vect2[0] = remoteprj[0]/length;
	vect2[1] = remoteprj[1]/length;
	vect2[2] = remoteprj[2]/length;

	cosangle = vect2[0]*frontdir[0]+vect2[1]*frontdir[1]+vect2[2]*frontdir[2];
	angle = ArcCos(cosangle);
	azimuth = (int)angle;
	if (azimuth < 0)
		azimuth = 0;
	else if (azimuth > 180)
		azimuth = 180;
	
	// now, we check if its the azimuth at the right ear side or at the left ear side
	cosangle = vect2[0]*righteardir[0]+vect2[1]*righteardir[1]+vect2[2]*righteardir[2];
	if (cosangle < 0)
		leftear = true;
	else
		leftear = false;
}
