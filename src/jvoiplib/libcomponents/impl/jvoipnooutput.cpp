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

#include "jvoipnooutput.h"

#include "debugnew.h"

// NOTE: as you can see, this is basically a dummy component

JVOIPNoOutput::JVOIPNoOutput(JVOIPSession *sess) : JVOIPVoiceOutput(sess)
{
}

JVOIPNoOutput::~JVOIPNoOutput()
{
}

int JVOIPNoOutput::Init(int sampinterval,int outputsamprate,int outputbytespersample,bool stereo,const JVOIPComponentParams *componentparams)
{
	return 0;
}

int JVOIPNoOutput::Cleanup()
{
	return 0;
}

int JVOIPNoOutput::Play(VoIPFramework::VoiceBlock *vb)
{
	return 0;
}

void JVOIPNoOutput::Reset()
{
}

bool JVOIPNoOutput::SupportsSampleInterval(int ival)
{
	return true;
}

int JVOIPNoOutput::SetSampleInterval(int ival)
{
	return 0;
}

bool JVOIPNoOutput::SupportsOutputSamplingRate(int orate)
{
	return true;
}

int JVOIPNoOutput::SetOutputSamplingRate(int orate)
{
	return 0;
}

bool JVOIPNoOutput::SupportsOutputBytesPerSample(int outputbytespersample)
{
	return true;
}

int JVOIPNoOutput::SetOutputBytesPerSample(int outputbytespersample)
{
	return 0;
}

int JVOIPNoOutput::SetStereo(bool s)
{
	return 0;
}

int JVOIPNoOutput::GetComponentState(JVOIPComponentState **compstate)
{
	*compstate = NULL;
	return 0;
}

int JVOIPNoOutput::SetComponentState(JVOIPComponentState *compstate)
{
	return 0;
}

std::string JVOIPNoOutput::GetComponentName()
{
	return std::string("JVOIPNoOutput");
}

std::string JVOIPNoOutput::GetComponentDescription()
{
	return std::string("JVOIPLIB Internal 'no output' module");
}

std::vector<JVOIPCompParamInfo> *JVOIPNoOutput::GetComponentParameters() throw (JVOIPException)
{
	return NULL;
}

