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

#ifndef JVOIPVOICEINPUT_H

#define JVOIPVOICEINPUT_H

#include "sampleinput.h"
#include "jvoipcomponent.h"

class JVOIPSamplingTimer;

// NOTE: the destruction of JVOIPVoiceInput must also cause the destruction of the
//       associated JVOIPSamplingTimer

class JVOIPVoiceInput : public VoIPFramework::SampleInput,public JVOIPComponent
{
public:
	JVOIPVoiceInput(JVOIPSession *sess):JVOIPComponent(sess) { }
	virtual ~JVOIPVoiceInput() { }
	virtual int Init(int sampinterval,int inputsamprate,int inputbytespersample,const JVOIPComponentParams *componentparams) = 0;
	virtual int Cleanup() = 0;
	virtual void Reset()= 0;
	virtual JVOIPSamplingTimer *GetSamplingTimer() = 0; // Note: component must delete this timer itself (if necessary)
	virtual bool SupportsSampleInterval(int ival) = 0;
	virtual int SetSampleInterval(int ival) = 0;
	virtual bool SupportsInputSamplingRate(int irate) = 0;
	virtual int SetInputSamplingRate(int irate) = 0;
	virtual bool SupportsInputBytesPerSample(int inputbytespersample) = 0;
	virtual int SetInputBytesPerSample(int inputbytespersample) = 0;
};

#endif // JVOIPVOICEINPUT_H
