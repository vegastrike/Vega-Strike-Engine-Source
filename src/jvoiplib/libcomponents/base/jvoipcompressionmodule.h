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

#ifndef JVOIPCOMPRESSIONMODULE_H

#define JVOIPCOMPRESSIONMODULE_H

#include "jvoipcomponent.h"
#include "voiceblock.h"
#include <string>

class JVOIPCompressionModule : public JVOIPComponent
{
public:
	JVOIPCompressionModule(JVOIPSession *sess):JVOIPComponent(sess) { }
	virtual ~JVOIPCompressionModule() { }
	virtual int InitCompressor(int sampinterval,int inputsamprate,int inputbytespersample,const JVOIPComponentParams *componentparams) = 0;
	virtual int CleanupCompressor() = 0;
	virtual int InitDecompressor() = 0;
	virtual int CleanupDecompressor() = 0;
	
	virtual int Compress(VoIPFramework::VoiceBlock *vb) = 0;
	virtual int Decompress(VoIPFramework::VoiceBlock *vb,VoIPFramework::VOIPuint64 sourceid) = 0;
	
	virtual bool SupportsSampleInterval(int ival) = 0;
	virtual int SetSampleInterval(int ival) = 0;
	virtual bool SupportsInputSamplingRate(int irate) = 0;
	virtual int SetInputSamplingRate(int irate) = 0;
	virtual bool SupportsInputBytesPerSample(int inputbytespersample) = 0;
	virtual int SetInputBytesPerSample(int inputbytespersample) = 0;
};

#endif // JVOIPCOMPRESSIONMODULE_H
