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

#include "voiceblock.h"
#include "voicecall.h"
#include "debugnew.h"

// start namespace
namespace VoIPFramework
{

#define VOICEBLOCK_INDEX_SAMPLEINPUT						0
#define VOICEBLOCK_INDEX_SAMPLEOUTPUT						1
#define VOICEBLOCK_INDEX_COMPRESSION						2
#define VOICEBLOCK_INDEX_DECOMPRESSION						3
#define VOICEBLOCK_INDEX_LOCATION3D						4
#define VOICEBLOCK_INDEX_TRANSFORM3D						5
#define VOICEBLOCK_INDEX_TRANSMISSION						6
#define VOICEBLOCK_INDEX_MIXER							7

VoiceBlock::VoiceBlock()
{
	block = NULL;
	stereo = false;
	sampleoffset = 0;
	numsamp = 0;
	numbytes = 0;
	bytespersample = 1;
	samplerate = 8000;
	sampdelhandler = NULL;
	samphandlerparam = NULL;
	issilence = false;
	info3d = NULL;
	info3dlen = 0;
	inf3ddelhandler = NULL;
	inf3dhandlerparam = NULL;
	majorcompression = 0;
	minorcompression = 0;
	for (int i = 0 ; i < VOICEBLOCK_NUMCOMPONENTS ; i++)
		vbparams[i] = NULL;
}

VoiceBlock::~VoiceBlock()
{
	ClearAllVoiceBlockParams();
	if (sampdelhandler)
		sampdelhandler(&block,samphandlerparam);
	if (block != NULL)
		delete [] block;
	if (inf3ddelhandler)
		inf3ddelhandler(&info3d,inf3dhandlerparam);
	if (info3d != NULL)
		delete [] info3d;
}

void VoiceBlock::Clear()
{
	SetSampleOffset(0);
	SetSamples(NULL,0);
	SetNumSamples(0);
	SetStereo(false);
	SetBytesPerSample(1);
	SetSampleRate(0);
	SetDataDeleteHandler(NULL,NULL);
	SetIsSilence(true);
	SetInfo3D(NULL,0);
	SetInfo3DDeleteHandler(NULL,NULL);
	SetMajorCompressionID(0);
	SetMinorCompressionID(0);
}

int VoiceBlock::SetVoiceBlockParams(ComponentType t,VoiceBlockParams *params)
{
	int index = -1;
	
	switch(t)
	{
	case SampleInput:
		index = VOICEBLOCK_INDEX_SAMPLEINPUT;
		break;
	case SampleOutput:
		index = VOICEBLOCK_INDEX_SAMPLEOUTPUT;
		break;
	case Compression:
		index = VOICEBLOCK_INDEX_COMPRESSION;
		break;
	case Decompression:
		index = VOICEBLOCK_INDEX_DECOMPRESSION;
		break;
	case Location3D:
		index = VOICEBLOCK_INDEX_LOCATION3D;
		break;
	case Transform3D:
		index = VOICEBLOCK_INDEX_TRANSFORM3D;
		break;
	case Transmission:
		index = VOICEBLOCK_INDEX_TRANSMISSION;
		break;
	case Mixer:
		index = VOICEBLOCK_INDEX_MIXER;
	}

	if (index >= 0)
	{
		if (params)
		{
			VoiceBlockParams *p;

			p = params->CreateCopy();
			if (p == NULL)
				return ERR_VOICECALL_OUTOFMEM;
			if (vbparams[index])
				delete vbparams[index];
			vbparams[index] = p;
		}
		else
		{
			if (vbparams[index])
			{
				delete vbparams[index];
				vbparams[index] = NULL;
			}
		}
	}
	return 0;
}

void VoiceBlock::ClearAllVoiceBlockParams()
{
	int i;
	
	for (i = 0 ; i < VOICEBLOCK_NUMCOMPONENTS ; i++)
	{
		if (vbparams[i])
		{
			delete vbparams[i];
			vbparams[i] = NULL;
		}
	}
}

// end namespace
};

