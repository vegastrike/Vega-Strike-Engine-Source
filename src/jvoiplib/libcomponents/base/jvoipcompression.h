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

#ifndef JVOIPCOMPRESSION_H

#define JVOIPCOMPRESSION_H

#include "voicecompressor.h"
#include "voicedecompressor.h"
#include "jvoipcomponent.h"
#include "jvoipsessionparams.h"

#define JVOIPCOMPRESSIONTYPE_MAJOR_NOCOMPRESSION							0
#define JVOIPCOMPRESSIONTYPE_MAJOR_USERDEFINEDCOMPRESSION						1
#define JVOIPCOMPRESSIONTYPE_MAJOR_DPCMCOMPRESSION							2
#define JVOIPCOMPRESSIONTYPE_MAJOR_GSMCOMPRESSION							3
#define JVOIPCOMPRESSIONTYPE_MAJOR_LPCCOMPRESSION							4
#define JVOIPCOMPRESSIONTYPE_MAJOR_ULAWCOMPRESSION							5

class JVOIPCompressionModule;

class JVOIPCompression : public VoIPFramework::VoiceCompressor,public VoIPFramework::VoiceDecompressor,public JVOIPComponent
{
public:
	JVOIPCompression(JVOIPSession *sess,JVOIPCompressionModule *userdefcompmod);
	~JVOIPCompression();
	int Init(int sampinterval,int inputsamprate,int inputbytespersample,JVOIPSessionParams::CompressionType comptype,const JVOIPComponentParams *componentparams);
	int Cleanup();
	int Compress(VoIPFramework::VoiceBlock *vb);
	int Decompress(VoIPFramework::VoiceBlock *vb,VoIPFramework::VOIPuint64 sourceid);
	
	int SetCompressionType(JVOIPSessionParams::CompressionType comptype,const JVOIPComponentParams *componentparams,bool *fatalerror);
	
	bool SupportsSampleInterval(int ival);
	int SetSampleInterval(int ival);
	bool SupportsInputSamplingRate(int irate);
	int SetInputSamplingRate(int irate);
	bool SupportsInputBytesPerSample(int inputbytespersample);
	int SetInputBytesPerSample(int inputbytespersample);
	
	int GetComponentState(JVOIPComponentState **compstate);
	int SetComponentState(JVOIPComponentState *compstate);
	
	std::string GetComponentName();
	std::string GetComponentDescription();
	std::vector<JVOIPCompParamInfo> *GetComponentParameters() throw (JVOIPException);
private:
	int InitModules();
	int DeleteModules();
	int ProcessCompressionType(JVOIPSessionParams::CompressionType comptype,JVOIPCompressionModule **compmod,int *majorcomptype);

	JVOIPCompressionModule *udef_compmod;
	JVOIPCompressionModule *cur_compmod;
	JVOIPCompressionModule *silsup_compmod;
	JVOIPCompressionModule *dpcm_compmod;
	JVOIPCompressionModule *gsm_compmod;
	JVOIPCompressionModule *lpc_compmod;
	JVOIPCompressionModule *ulaw_compmod;
	
	bool init;
	int curmajorcomptype;
};

#endif // JVOIPCOMPRESSION_H
