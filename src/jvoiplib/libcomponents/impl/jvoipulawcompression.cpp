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

#include "jvoipulawcompression.h"
#include "jvoiperrors.h"
#include <stdio.h>
#include <string.h>

#include "debugnew.h"

#define JVOIPULAWCOMPRESSION_CLIPVALUE						32635
#define JVOIPULAWCOMPRESSION_BIAS						0x84

unsigned char JVOIPULawCompression::expenc[256] = { 0,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
                                                    5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
						    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
						    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
                                                    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                                                    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                                                    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                                                    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7};

JVOIPULawCompression::JVOIPULawCompression(JVOIPSession *sess) : JVOIPCompressionModule(sess)
{
	compinit = false;
}

JVOIPULawCompression::~JVOIPULawCompression()
{
	CleanupCompressor();
	CleanupDecompressor();
}

int JVOIPULawCompression::InitCompressor(int sampinterval,int inputsamprate,int inputbytespersample,const JVOIPComponentParams *componentparams)
{
	if (compinit)
		return ERR_JVOIPLIB_GENERAL_COMPONENTALREADYINIT;

	compinit = true;
	return 0;
}

int JVOIPULawCompression::CleanupCompressor()
{
	if (!compinit)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	
	compinit = false;
	return 0;
}

int JVOIPULawCompression::InitDecompressor()
{
	// NOTE: nothing needs to be done here
	return 0;
}

int JVOIPULawCompression::CleanupDecompressor()
{
	// NOTE: nothing needs to be done here
	return 0;
}

int JVOIPULawCompression::Compress(VoIPFramework::VoiceBlock *vb)
{
	int i,j,num;
	unsigned char *data,*newdata;
	
	data = vb->GetSamples(false);
	num = vb->GetNumSamples();
	
	if (vb->GetBytesPerSample() == 1) // we'll use the same buffer here...
	{
		for (i = 0 ; i < num ; i++)
		{
			int val,mantpos;
			int signval,expon,mant;
			
			val = (data[i] << 8);
			val -= 32768;
			if (val < 0)
			{
				signval = (1<<7);
				if (val < -JVOIPULAWCOMPRESSION_CLIPVALUE)
					val = JVOIPULAWCOMPRESSION_CLIPVALUE;
				else
					val = -val;
			}
			else
				signval = 0;
			
			val += JVOIPULAWCOMPRESSION_BIAS;
			expon = expenc[val>>7];
			mantpos = (int)expon + 7 - 4;
			mant = ((val >> mantpos) & 0x0F);
			data[i] = ~((unsigned char)(signval | (expon<<4) | mant));
		}
	}
	else // two bytes per sample
	{
		newdata = new unsigned char[num];
		if (newdata == NULL)
			return ERR_JVOIPLIB_GENERAL_OUTOFMEM;
		
		for (i = 0,j = 0 ; i < num ; i++, j += 2)
		{
			int val,mantpos;
			int signval,expon,mant;
			
			val = (((int)data[j]) << 8)|((int)data[j+1]);
			val -= 32768;
			if (val < 0)
			{
				signval = (1<<7);
				if (val < -JVOIPULAWCOMPRESSION_CLIPVALUE)
					val = JVOIPULAWCOMPRESSION_CLIPVALUE;
				else
					val = -val;
			}
			else
				signval = 0;
			
			val += JVOIPULAWCOMPRESSION_BIAS;
			expon = expenc[val>>7];
			mantpos = (int)expon + 7 - 4;
			mant = ((val >> mantpos) & 0x0F);
			newdata[i] = ~((unsigned char)(signval | (expon<<4) | mant));
		}
		vb->SetSamples(newdata,num);
	}
	
	return 0;
}

int JVOIPULawCompression::Decompress(VoIPFramework::VoiceBlock *vb,VoIPFramework::VOIPuint64 sourceid)
{
	int num,i,newnum,j;
	unsigned char *data;
	unsigned char *newdata;

	data = vb->GetSamples(false);
	if (data == NULL)
		return 0;
	num = vb->GetNumBytes();
	if (num <= 0)
		return 0;

	newnum = num*2;
	newdata = new unsigned char[newnum];
	if (newdata == NULL)
		return ERR_JVOIPLIB_GENERAL_OUTOFMEM;

	for (i = 0,j = 0 ; i < num ; i++,j += 2)
	{
		int expon,mant,sign;
		int val;

		val = (int)(~data[i]);
		sign = val&(1<<7);
		mant = val&0x0F;
		expon = (val>>4)&0x07;

		val = (1 << (7+expon));
		val |= (mant << (3+expon));
		val -= JVOIPULAWCOMPRESSION_BIAS;
		if (val < 0)
			val = 0;
		else if (sign != 0)
			val = - val;
		val += 32768;
		if (val > 65535)
			val = 65535;
		else if (val < 0)
			val = 0;
		newdata[j] = (unsigned char)((val>>8)&0xFF);
		newdata[j+1] = (unsigned char)(val&0xFF);
	}

	vb->SetSamples(newdata,newnum);
	vb->SetNumSamples(num);
	vb->SetBytesPerSample(2);
	return 0;
}

bool JVOIPULawCompression::SupportsSampleInterval(int ival)
{
	if (!compinit)
		return false;
	return true;
}

int JVOIPULawCompression::SetSampleInterval(int ival)
{
	if (!compinit)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	return 0;
}

bool JVOIPULawCompression::SupportsInputSamplingRate(int irate)
{
	if (!compinit)
		return false;
	return true;
}

int JVOIPULawCompression::SetInputSamplingRate(int irate)
{
	if (!compinit)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	return 0;
}

bool JVOIPULawCompression::SupportsInputBytesPerSample(int inputbytespersample)
{
	if (!compinit)
		return false;
	return true;
}

int JVOIPULawCompression::SetInputBytesPerSample(int inputbytespersample)
{
	if (!compinit)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	return 0;
}

int JVOIPULawCompression::GetComponentState(JVOIPComponentState **compstate)
{
	if (!compinit)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	*compstate = NULL;
	return 0;
}

int JVOIPULawCompression::SetComponentState(JVOIPComponentState *compstate)
{
	if (!compinit)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	return 0;
}

std::string JVOIPULawCompression::GetComponentName()
{
	return std::string("JVOIPULawCompression");
}

std::string JVOIPULawCompression::GetComponentDescription()
{
	return std::string("JVOIPLIB Internal u-law encoding module");
}

std::vector<JVOIPCompParamInfo> *JVOIPULawCompression::GetComponentParameters() throw (JVOIPException)
{
	return NULL;
}

