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

#include "jvoipsilencesuppression.h"
#include <iostream>

#include "debugnew.h"

#define JVOIPSILENCESUPPRESSOR_ONEBPS_MINAMP								5
#define JVOIPSILENCESUPPRESSOR_TWOBPS_MINAMP								256

JVOIPSilenceSuppression::JVOIPSilenceSuppression(JVOIPSession *sess) : JVOIPCompressionModule(sess)
{
}

JVOIPSilenceSuppression::~JVOIPSilenceSuppression()
{
}

int JVOIPSilenceSuppression::InitCompressor(int sampinterval,int inputsamprate,int inputbytespersample,const JVOIPComponentParams *componentparams)
{
	// NOTE: nothing to do
	return 0;
}

int JVOIPSilenceSuppression::CleanupCompressor()
{
	// NOTE: nothing to do
	return 0;
}

int JVOIPSilenceSuppression::InitDecompressor()
{
	// NOTE: nothing to do
	return 0;
}

int JVOIPSilenceSuppression::CleanupDecompressor()
{
	// NOTE: nothing to do
	return 0;
}

int JVOIPSilenceSuppression::Compress(VoIPFramework::VoiceBlock *vb)
{
	if (IsSilent(vb))
		vb->SetIsSilence(true);
	else
		vb->SetIsSilence(false);
	return 0;
}

int JVOIPSilenceSuppression::Decompress(VoIPFramework::VoiceBlock *vb,VoIPFramework::VOIPuint64 sourceid)
{
	// NOTE: this shouldn't do anything: in the compression manager, a silent block will simply be set
	//       to 'no compression'. The transmitter can then determine whether to send the data or not
	std::cerr << "JVOIPSilenceSuppression::Decompress --  Hey !? We shouldn't be here!" << std::endl;
	return 0;
}

bool JVOIPSilenceSuppression::SupportsSampleInterval(int ival)
{
	return true;
}

int JVOIPSilenceSuppression::SetSampleInterval(int ival)
{
	return 0;
}

bool JVOIPSilenceSuppression::SupportsInputSamplingRate(int irate)
{
	return true;
}

int JVOIPSilenceSuppression::SetInputSamplingRate(int irate)
{
	return 0;
}

bool JVOIPSilenceSuppression::SupportsInputBytesPerSample(int inputbytespersample)
{
	return true;
}

int JVOIPSilenceSuppression::SetInputBytesPerSample(int inputbytespersample)
{
	return 0;
}

int JVOIPSilenceSuppression::GetComponentState(JVOIPComponentState **compstate)
{
	// NOTE: nothing to do
	*compstate = NULL;
	return 0;
}

int JVOIPSilenceSuppression::SetComponentState(JVOIPComponentState *compstate)
{
	// NOTE: nothing to do
	return 0;
}

std::string JVOIPSilenceSuppression::GetComponentName()
{
	return std::string("JVOIPSilenceSuppression");
}

std::string JVOIPSilenceSuppression::GetComponentDescription()
{
	return std::string("JVOIPLIB Internal silence suppression module");
}

std::vector<JVOIPCompParamInfo> *JVOIPSilenceSuppression::GetComponentParameters() throw (JVOIPException)
{
	// NOTE: no parameters
	return NULL;
}

bool JVOIPSilenceSuppression::IsSilent(VoIPFramework::VoiceBlock *vb)
{
	// NOTE: since this is voice input, we assume that the block is NOT in stereo
	//       also note that everything is in big endian and unsigned

	unsigned char *data;
	int len,i;
	int val;

	data = vb->GetSamples(false);
	len = vb->GetNumBytes();

	if (data == NULL || len == 0) // just to make sure
		return true;

	if (vb->GetBytesPerSample() == 1)
	{
		for (i = 0 ; i < len ; i++)
		{
			val = (int)data[i];
			val -= 127;
			if (val < 0)
				val = -val;
                       	if (val > JVOIPSILENCESUPPRESSOR_ONEBPS_MINAMP)
				return false;
		}
	}
	else // two bytes per sample
	{
		for (i = 0 ; i < len ; i += 2)
		{
			val = ((((int)data[i])<<8)|((int)data[i+1]));
			val -= 32767;
			if (val < 0)
				val = -val;
                       	if (val > JVOIPSILENCESUPPRESSOR_TWOBPS_MINAMP)
				return false;
		}
	}
	return true;
}

