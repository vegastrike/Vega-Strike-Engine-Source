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

#ifndef VOICEBLOCK_H

#define VOICEBLOCK_H

#include "voicetypes.h"
#include <stdlib.h>

// start namespace
namespace VoIPFramework
{

typedef void (*VoiceBlockDeleteHandler)(unsigned char **data,void *param);

class VoiceBlockParams
{
public:
	VoiceBlockParams()								{ }
	virtual ~VoiceBlockParams()							{ }
	virtual VoiceBlockParams *CreateCopy() = 0;
};

#define VOICEBLOCK_NUMCOMPONENTS							8

class VoiceBlock
{
public:
	enum ComponentType {SampleInput,SampleOutput,Compression,Decompression,Location3D,
	                    Transform3D,Transmission,Mixer};
	
	VoiceBlock();
	~VoiceBlock();
	void Clear();
	
	VOIPdouble GetSampleOffset()							{ return sampleoffset; }
	unsigned char *GetSamples(bool extract);
	unsigned long GetNumBytes()							{ return numbytes; }
	unsigned long GetNumSamples()							{ return numsamp; }
	bool IsStereo()									{ return stereo; }
	int GetBytesPerSample()								{ return bytespersample; }
	unsigned long GetSampleRate()							{ return samplerate; }
	VoiceBlockDeleteHandler GetSampleDataDeleteHandler(void **param)		{ *param = samphandlerparam; return sampdelhandler; }
	bool IsSilence()								{ return issilence; }
	bool Has3DInfo()								{ return (info3d != NULL); }
	VoiceBlockDeleteHandler GetInfo3DDeleteHandler(void **param)			{ *param = inf3dhandlerparam; return sampdelhandler; }
	unsigned char *GetInfo3D(bool extract);
	int GetInfo3DLength()								{ return info3dlen; }
	int GetMajorCompressionID()							{ return majorcompression; }
	int GetMinorCompressionID()							{ return minorcompression; }

	void SetSampleOffset(VOIPdouble offset)						{ sampleoffset = offset; }
	void SetSamples(unsigned char *samples,unsigned long nbytes);
	void SetNumSamples(unsigned long num)						{ numsamp = num; }
	void SetStereo(bool s)								{ stereo = s; }
	void SetBytesPerSample(int b)							{ bytespersample = b; }
	void SetSampleRate(unsigned long samprate)					{ samplerate = samprate; }
	void SetDataDeleteHandler(VoiceBlockDeleteHandler h,void *param)		{ sampdelhandler = h; samphandlerparam = param; }
	void SetIsSilence(bool val)							{ issilence = val; }
	void SetInfo3D(unsigned char *data,unsigned long len);
	void SetInfo3DDeleteHandler(VoiceBlockDeleteHandler h,void *param)		{ inf3ddelhandler = h; inf3dhandlerparam = param; }
	void SetMajorCompressionID(int id)						{ majorcompression = id; }
	void SetMinorCompressionID(int id)						{ minorcompression = id; }

	int SetVoiceBlockParams(ComponentType t,VoiceBlockParams *params);
	void ClearAllVoiceBlockParams();
private:
	unsigned char *block;
	bool stereo;
	VOIPdouble sampleoffset;
	unsigned long numsamp;
	unsigned long numbytes;
	int bytespersample;
	unsigned long samplerate;
	bool issilence;

	int majorcompression;
	int minorcompression;

	unsigned char *info3d;
	unsigned long info3dlen;

	VoiceBlockDeleteHandler sampdelhandler;
	void *samphandlerparam;
	VoiceBlockDeleteHandler inf3ddelhandler;
	void *inf3dhandlerparam;

	VoiceBlockParams *vbparams[VOICEBLOCK_NUMCOMPONENTS];
};

inline unsigned char *VoiceBlock::GetSamples(bool extract)
{
	unsigned char *s;
	s = block;
	if (extract)
	{
		sampdelhandler = NULL;
		samphandlerparam = NULL;
		block = NULL;
	}
	return s;
}

inline void VoiceBlock::SetSamples(unsigned char *samples,unsigned long nbytes)
{
	if (sampdelhandler)
		sampdelhandler(&block,samphandlerparam);
	if (block != NULL)
		delete [] block;
	block = samples;
	numbytes = nbytes;
	sampdelhandler = NULL;
	samphandlerparam = NULL;
}

inline unsigned char *VoiceBlock::GetInfo3D(bool extract)
{
	unsigned char *s;
	s = info3d;
	if (extract)
	{
		inf3ddelhandler = NULL;
		inf3dhandlerparam = NULL;
		info3d = NULL;
	}
	return s;
}

inline void VoiceBlock::SetInfo3D(unsigned char *data,unsigned long len)
{
	if (inf3ddelhandler)
		inf3ddelhandler(&info3d,inf3dhandlerparam);
	if (info3d != NULL)
		delete [] info3d;
	info3d = data;
	info3dlen = len;
	inf3ddelhandler = NULL;
	inf3dhandlerparam = NULL;
}

// end namespace
};
#endif // VOICEBLOCK_H
