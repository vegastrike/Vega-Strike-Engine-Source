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

#ifndef JVOIPSESSIONPARAMS_H

#define JVOIPSESSIONPARAMS_H

#include "jvoiptypes.h"
#include "jvoiperrors.h"
#include "jvoipcomponentparams.h"
#include "jvoipexception.h"
#include <stdio.h>

class JVOIPSessionParams
{
public:
	enum VoiceInputType { NoInput, UserDefinedInput, SoundcardInput };
	enum VoiceOutputType { NoOutput, UserDefinedOutput, SoundcardOutput };
	enum LocalisationType { NoLocalisation, UserDefinedLocalisation, SimpleLocalisation, HRTFLocalisation };
	enum CompressionType { NoCompression, UserDefinedCompression, ULawEncoding, SilenceSuppression, DPCM, GSM, LPC };
	enum MixerType { UserDefinedMixer, NormalMixer };
	enum TransmissionType { UserDefinedTransmission, RTP };
	enum ReceiveType { AcceptAll, AcceptSome, IgnoreSome };
        enum SampleEncodingType { EightBit, SixteenBit };
	
	JVOIPSessionParams();
	~JVOIPSessionParams();
	JVOIPSessionParams(const JVOIPSessionParams &src) throw (JVOIPException);
	JVOIPSessionParams &operator=(const JVOIPSessionParams &src) throw (JVOIPException);

	// basic operations
	void SetInputSamplingRate(JVOIPuint16 rate)					{ inputsamprate = rate; }
	void SetOutputSamplingRate(JVOIPuint16 rate)					{ outputsamprate = rate; }
	void SetSampleInterval(JVOIPuint16 ival)					{ sampinterval = ival; }
	void SetInputSampleEncodingType(SampleEncodingType ie)				{ inputsampenc = ie; }
	void SetOutputSampleEncodingType(SampleEncodingType oe)				{ outputsampenc = oe; }
	void SetVoiceInputType(VoiceInputType i)					{ inputtype = i; }
	void SetVoiceOutputType(VoiceOutputType o)					{ outputtype = o; }
	void SetLocalisationType(LocalisationType l)					{ loctype = l; }
	void SetCompressionType(CompressionType c)					{ comptype = c; }
	void SetMixerType(MixerType m)							{ mixertype = m; }
	void SetTransmissionType(TransmissionType t)					{ transtype = t; }
	void SetReceiveType(ReceiveType r)						{ receivetype = r; }
	
	JVOIPuint16 GetInputSamplingRate() const					{ return inputsamprate; }
	JVOIPuint16 GetOutputSamplingRate() const					{ return outputsamprate; }
	JVOIPuint16 GetSampleInterval() const						{ return sampinterval; }
	SampleEncodingType GetInputSampleEncodingType() const 				{ return inputsampenc; }
	SampleEncodingType GetOutputSampleEncodingType() const				{ return outputsampenc; }
	VoiceInputType GetVoiceInputType() const					{ return inputtype; }
	VoiceOutputType GetVoiceOutputType() const					{ return outputtype; }
	LocalisationType GetLocalisationType() const					{ return loctype; }
	CompressionType GetCompressionType() const					{ return comptype; }
	MixerType GetMixerType() const							{ return mixertype; }
	TransmissionType GetTransmissionType() const					{ return transtype; }
	ReceiveType GetReceiveType() const						{ return receivetype; }
	
	// functions to be able to dynamically pass parameters to any component	
	inline int SetVoiceInputParams(JVOIPComponentParams *i);
	inline int SetVoiceOutputParams(JVOIPComponentParams *o);
	inline int SetLocalisationParams(JVOIPComponentParams *l);
	inline int SetCompressionParams(JVOIPComponentParams *c);
	inline int SetMixerParams(JVOIPComponentParams *m);
	inline int SetTransmissionParams(JVOIPComponentParams *t);
	
	JVOIPComponentParams *GetVoiceInputParams() const				{ return params_input; }
	JVOIPComponentParams *GetVoiceOutputParams() const				{ return params_output; }
	JVOIPComponentParams *GetLocalisationParams() const				{ return params_loc; }
	JVOIPComponentParams *GetCompressionParams() const				{ return params_comp; }
	JVOIPComponentParams *GetMixerParams() const					{ return params_mixer; }
	JVOIPComponentParams *GetTransmissionParams() const				{ return params_trans; }
private:
	bool CopyMembers(const JVOIPSessionParams &src);
	
	JVOIPuint16 inputsamprate,outputsamprate;
	JVOIPuint16 sampinterval;
	SampleEncodingType inputsampenc,outputsampenc;
	VoiceInputType inputtype;
	VoiceOutputType outputtype;
	LocalisationType loctype;
	CompressionType comptype;
	TransmissionType transtype;
	MixerType mixertype;
	ReceiveType receivetype;
	
	// component parameters
	JVOIPComponentParams *params_input;
	JVOIPComponentParams *params_output;
	JVOIPComponentParams *params_loc;
	JVOIPComponentParams *params_comp;
	JVOIPComponentParams *params_mixer;
	JVOIPComponentParams *params_trans;
};

inline int JVOIPSessionParams::SetVoiceInputParams(JVOIPComponentParams *i)
{
	JVOIPComponentParams *n;
	
	if (i == NULL)
		n = NULL;
	else
	{		
		n = i->CreateCopy();
		if (n == NULL)
			return ERR_JVOIPLIB_GENERAL_OUTOFMEM;
	}
	if (params_input)
		delete params_input;
	params_input = n;
	return 0;
}

inline int JVOIPSessionParams::SetVoiceOutputParams(JVOIPComponentParams *o)
{
	JVOIPComponentParams *n;
	
	if (o == NULL)
		n = NULL;
	else
	{		
		n = o->CreateCopy();
		if (n == NULL)
			return ERR_JVOIPLIB_GENERAL_OUTOFMEM;
	}
	if (params_output)
		delete params_output;
	params_output = n;
	return 0;
}

inline int JVOIPSessionParams::SetLocalisationParams(JVOIPComponentParams *l)
{
	JVOIPComponentParams *n;
	
	if (l == NULL)
		n = NULL;
	else
	{		
		n = l->CreateCopy();
		if (n == NULL)
			return ERR_JVOIPLIB_GENERAL_OUTOFMEM;
	}
	if (params_loc)
		delete params_loc;
	params_loc = n;
	return 0;
}

inline int JVOIPSessionParams::SetCompressionParams(JVOIPComponentParams *c)
{
	JVOIPComponentParams *n;
	
	if (c == NULL)
		n = NULL;
	else
	{		
		n = c->CreateCopy();
		if (n == NULL)
			return ERR_JVOIPLIB_GENERAL_OUTOFMEM;
	}
	if (params_comp)
		delete params_comp;
	params_comp = n;
	return 0;
}

inline int JVOIPSessionParams::SetMixerParams(JVOIPComponentParams *m)
{
	JVOIPComponentParams *n;
	
	if (m == NULL)
		n = NULL;
	else
	{		
		n = m->CreateCopy();
		if (n == NULL)
			return ERR_JVOIPLIB_GENERAL_OUTOFMEM;
	}
	if (params_mixer)
		delete params_mixer;
	params_mixer = n;
	return 0;
}

inline int JVOIPSessionParams::SetTransmissionParams(JVOIPComponentParams *t)
{
	JVOIPComponentParams *n;
	
	if (t == NULL)
		n = NULL;
	else
	{		
		n = t->CreateCopy();
		if (n == NULL)
			return ERR_JVOIPLIB_GENERAL_OUTOFMEM;
	}
	if (params_trans)
		delete params_trans;
	params_trans = n;
	return 0;
}


#endif // JVOIPSESSIONPARAMS_H
