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

#ifndef JVOIPGSMCOMPRESSION_H

#define JVOIPGSMCOMPRESSION_H

#include "jvoipcompressionmodule.h"
#include "gsm.h"
#include <time.h>
#include <map>
#include <jthread.h>

class JVOIPGSMCompression : public JVOIPCompressionModule,private JThread
{
public:
	JVOIPGSMCompression(JVOIPSession *sess);
	~JVOIPGSMCompression();
	int InitCompressor(int sampinterval,int inputsamprate,int inputbytespersample,const JVOIPComponentParams *componentparams);
	int CleanupCompressor();
	int InitDecompressor();
	int CleanupDecompressor();

	int Compress(VoIPFramework::VoiceBlock *vb);
	int Decompress(VoIPFramework::VoiceBlock *vb,VoIPFramework::VOIPuint64 sourceid);

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
	bool compinit;

	class GSMStateInfo
	{
	public:
		GSMStateInfo() { gsmstate = NULL; lasttime = time(NULL); }
		~GSMStateInfo() { if (gsmstate != NULL) gsm_destroy(gsmstate); }

		time_t lasttime;
		struct gsm_state *gsmstate;
	};

	std::map<VoIPFramework::VOIPuint64,GSMStateInfo *> gsmstates;
	struct gsm_state *localgsmstate;
	JMutex statemutex;
		
	void *Thread();
	void StopThread();
	bool stopthread;
	JMutex stopthreadmutex;
};

#endif // JVOIPGSMCOMPRESSION_H
