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

#include "jvoipdpcmcompression.h"
#include "jvoipsilencesuppression.h"
#include "jvoiperrors.h"
#include <stdio.h>
#include <string.h>

#include "debugnew.h"

#define JVOIPDPCMCOMPRESSION_TWOBPSQUANTIZER				256

JVOIPDPCMCompression::JVOIPDPCMCompression(JVOIPSession *sess) : JVOIPCompressionModule(sess)
{
	compinit = false;
}

JVOIPDPCMCompression::~JVOIPDPCMCompression()
{
	CleanupCompressor();
	CleanupDecompressor();
}

int JVOIPDPCMCompression::InitCompressor(int sampinterval,int inputsamprate,int inputbytespersample,const JVOIPComponentParams *componentparams)
{
	JVOIPDPCMCompressionParams *dpcmparams,defparams;

	if (compinit)
		return ERR_JVOIPLIB_GENERAL_COMPONENTALREADYINIT;

	dpcmparams = dynamic_cast<JVOIPDPCMCompressionParams *>(const_cast<JVOIPComponentParams *>(componentparams));
	if (dpcmparams == NULL)
		dpcmparams = &defparams;

	numencodebits = dpcmparams->GetNumEncodeBits();

	maxvalue = (1<<(numencodebits-2));
	minvalue = -(1<<(numencodebits-1));
	compinit = true;
	return 0;
}

int JVOIPDPCMCompression::CleanupCompressor()
{
	if (!compinit)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	// NOTE: not much to do...
	compinit = false;
	return 0;
}

int JVOIPDPCMCompression::InitDecompressor()
{
	// NOTE: nothing needs to be done here
	return 0;
}

int JVOIPDPCMCompression::CleanupDecompressor()
{
	// NOTE: nothing needs to be done here
	return 0;
}

int JVOIPDPCMCompression::Compress(VoIPFramework::VoiceBlock *vb)
{
	int previousvalue;
	int currentvalue;
	int status,len;
	unsigned char *newdata;

	if (!compinit)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;

        // We'll detect silence so the transmitter can discard the packet if desired.
	// note that we still need to compress the data in case the transmitter decides
	// to send the packet anyway
	if (JVOIPSilenceSuppression::IsSilent(vb))
		vb->SetIsSilence(true);
	else
		vb->SetIsSilence(false);

	ResetEncoder(vb);
	previousvalue = EncodeFirstValue();
	if (previousvalue < 0) // error
		return previousvalue;

	while ((status = GetNextValue(&currentvalue)) > 0)
	{
		int diff;

		diff = currentvalue-previousvalue;
		if ((status = EncodeValue(diff,&previousvalue)) < 0)
			return status;
	}
	if (status < 0)
		return status;

	len = GetEncodeLength();
	newdata = new unsigned char[len];
	if (newdata == NULL)
		return ERR_JVOIPLIB_GENERAL_OUTOFMEM;

	memcpy(newdata,buffer,len);
	vb->SetSamples(newdata,len);

	return 0;
}

int JVOIPDPCMCompression::Decompress(VoIPFramework::VoiceBlock *vb,VoIPFramework::VOIPuint64 sourceid)
{
	int prevval,diff,len;
	bool done;
	unsigned char *newdata;

	if (vb->GetSamples(false) == NULL || vb->GetNumBytes() <= 0) // no data
		return 0;

	// NOTE: the decoder should ignore bad packets and should not return an
	// error -> otherwise the session will terminate

	if (!ResetDecoder(vb))
	{
		vb->Clear();
		return 0;
	}
	
	if (!GetFirstSampleValue(&prevval))
	{
		vb->Clear();
		return 0;
	}
	
	if (!StoreDecodeValue(prevval))
		done = true;
	else
		done = false;	
	
	while (!done && GetNextSampleDifference(&diff))
	{
		prevval += diff;
		if (!StoreDecodeValue(prevval))
			done = true;
	}
	
	len = GetDecodeLength();	
	if (len <= 0)
	{
		vb->Clear();
		return 0;
	}
	
	newdata = new unsigned char[len];
	if (newdata == NULL)
		return ERR_JVOIPLIB_GENERAL_OUTOFMEM;
	memcpy(newdata,buffer,len);
	vb->SetSamples(newdata,len);
	
	return 0;
}

bool JVOIPDPCMCompression::SupportsSampleInterval(int ival)
{
	if (!compinit)
		return false;
	return true;
}

int JVOIPDPCMCompression::SetSampleInterval(int ival)
{
	if (!compinit)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	return 0;
}

bool JVOIPDPCMCompression::SupportsInputSamplingRate(int irate)
{
	if (!compinit)
		return false;
	return true;
}

int JVOIPDPCMCompression::SetInputSamplingRate(int irate)
{
	if (!compinit)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	return 0;
}

bool JVOIPDPCMCompression::SupportsInputBytesPerSample(int inputbytespersample)
{
	if (!compinit)
		return false;
	return true;
}

int JVOIPDPCMCompression::SetInputBytesPerSample(int inputbytespersample)
{
	if (!compinit)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	return 0;
}

int JVOIPDPCMCompression::GetComponentState(JVOIPComponentState **compstate)
{
	if (!compinit)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	*compstate = NULL;
	return 0;
}

int JVOIPDPCMCompression::SetComponentState(JVOIPComponentState *compstate)
{
	if (!compinit)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	return 0;
}

std::string JVOIPDPCMCompression::GetComponentName()
{
	return std::string("JVOIPDPCMCompression");
}

std::string JVOIPDPCMCompression::GetComponentDescription()
{
	return std::string("JVOIPLIB Internal DPCM compression module");
}

std::vector<JVOIPCompParamInfo> *JVOIPDPCMCompression::GetComponentParameters() throw (JVOIPException)
{
	std::vector<JVOIPCompParamInfo> *params;
	char buf[256];

	params = new std::vector<JVOIPCompParamInfo>(1);
	(*params)[0].SetParameterName("Number of encoding bits");
	sprintf(buf,"%d",numencodebits);
	(*params)[0].SetParameterValue(buf);
	return params;
}

inline void JVOIPDPCMCompression::ResetEncoder(VoIPFramework::VoiceBlock *vb)
{
	bytepos = 0;
	bitpos = 0;
	datapos = 0;
	bytespersample = vb->GetBytesPerSample();
	data = vb->GetSamples(false);
	datalen = vb->GetNumBytes();
}

inline int JVOIPDPCMCompression::EncodeFirstValue()
{
	if (datalen < bytespersample) // one extra byte to encode the number of encoding bits
		return ERR_JVOIPLIB_DPCMCOMP_BADFORMAT;

	buffer[bytepos++] = (unsigned char)numencodebits;
	buffer[bytepos++] = (unsigned char)((datalen>>8)&0xFF);
	buffer[bytepos++] = (unsigned char)(datalen&0xFF);
	buffer[bytepos++] = data[datapos++];
        if (bytespersample == 1)
		return (int)data[0];
	else
	{
		buffer[bytepos++] = data[datapos++];
		return ((((int)data[0])<<8)|((int)data[1]));
	}

	return 0;
}

inline int JVOIPDPCMCompression::GetNextValue(int *value)
{
	int val;

	if (datapos >= datalen)
		return 0;
        if ((datalen-datapos) < bytespersample)
		return ERR_JVOIPLIB_DPCMCOMP_BADFORMAT;

	if (bytespersample == 1)
		val = (int)data[datapos++];
        else // two bytes per sample
	{
		val = ((((int)data[datapos])<<8)|((int)data[datapos+1]));
		datapos += 2;
        }
	*value = val;
	return 1;
}

inline int JVOIPDPCMCompression::EncodeValue(int diff,int *prevvalue)
{
	int val;
	int storeval;
	int i;
	unsigned char thebyte;

	
	val = *prevvalue;
	if (bytespersample != 1)
		diff /= JVOIPDPCMCOMPRESSION_TWOBPSQUANTIZER;
	
	if (diff > maxvalue)
		diff = maxvalue;
	else if (diff < minvalue)
		diff = minvalue;

	if (bytespersample != 1)
		val += diff*JVOIPDPCMCOMPRESSION_TWOBPSQUANTIZER;
	else
		val += diff;
	*prevvalue = val;

	if (diff < 0)
	{
		storeval = -diff;
		storeval--;
		storeval |= (1<<(numencodebits-1));
	}
	else // diff >= 0
		storeval = diff;

	if (bitpos == 8)
	{
		bitpos = 0;
		bytepos++;
		if (bytepos >= JVOIPDPCMCOMPRESSION_BUFFERSIZE)
			return ERR_JVOIPLIB_DPCMCOMP_TOOMUCHDATA;
		thebyte = 0;
	}
	else if (bitpos == 0)
		thebyte = 0;
	else
		thebyte = buffer[bytepos];


	for (i = 0 ; i < numencodebits ; i++)
	{
		if (bitpos == 8)
		{
			bitpos = 0;
			buffer[bytepos++] = thebyte;
			if (bytepos >= JVOIPDPCMCOMPRESSION_BUFFERSIZE)
				return ERR_JVOIPLIB_DPCMCOMP_TOOMUCHDATA;
			thebyte = 0;
		}
		if (storeval&(1<<i)) // test bit i
			thebyte |= (1<<bitpos);
		bitpos++;
	}
	buffer[bytepos] = thebyte;
	return 0;
}

inline int JVOIPDPCMCompression::GetEncodeLength()
{
	if (bitpos == 0)
		return bytepos;
	return (bytepos+1);
}

inline bool JVOIPDPCMCompression::ResetDecoder(VoIPFramework::VoiceBlock *vb)
{
	bytepos = 0;
	bitpos = 0;
	datapos = 0;
	bytespersample = vb->GetBytesPerSample();
	data = vb->GetSamples(false);
	datalen = vb->GetNumBytes();
	if (datalen < 3)
		return false;
	numdecodebits = (int)data[bytepos++];
	totallen = (((int)data[bytepos])<<8)|((int)data[bytepos+1]);
	bytepos += 2;
	return true;
}

inline bool JVOIPDPCMCompression::GetFirstSampleValue(int *val)
{
	if (datalen-bytepos < bytespersample)
		return false;
	if (bytespersample == 1)
		*val = (int)data[bytepos++];
	else
	{
		*val = (((int)data[bytepos])<<8)|((int)data[bytepos+1]);
		bytepos += 2;
	}
	return true;
}

inline bool JVOIPDPCMCompression::StoreDecodeValue(int val)
{
	if (JVOIPDPCMCOMPRESSION_BUFFERSIZE-datapos < bytespersample ||
	    totallen-datapos < bytespersample)
		return false;
	if (bytespersample == 1)
	{
		if (val > 255)
			val = 255;
		else if (val < 0)
			val = 0;
		buffer[datapos++] = (unsigned char)val;
	}
	else // two bytes per sample
	{
		unsigned char b1,b2;
	
		if (val > 65535)
			val = 65535;
		else if (val < 0)
			val = 0;
		
		b1 = (unsigned char)((val>>8)&0xFF);
		b2 = (unsigned char)(val&0xFF);
		buffer[datapos++] = b1;
		buffer[datapos++] = b2;
	}
	return true;
}

inline bool JVOIPDPCMCompression::GetNextSampleDifference(int *diff)
{
	unsigned char curbyte;
	int i;
	int val;
	
	val = 0;
	
	if (bytepos >= datalen)
		return false;
	curbyte = data[bytepos];
	for (i = 0 ; i < numdecodebits ; i++)
	{
		if (bitpos == 8)
		{
			bitpos = 0;
			bytepos++;
			if (bytepos >= datalen)
				return false;
			curbyte = data[bytepos];
		}
		if (curbyte & (1<<bitpos))
			val |= (1 << i);
		bitpos++;
	}
	
	if (val & (1<<(numdecodebits-1))) // negative number
	{
		val ^= (1<<(numdecodebits-1)); // set that bit to zero
		val = -val-1;
	}

	if (bytespersample != 1)
		val *= JVOIPDPCMCOMPRESSION_TWOBPSQUANTIZER;		
	*diff = val;
	return true;
}

inline int JVOIPDPCMCompression::GetDecodeLength()
{
	return datapos;
}
