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

#ifndef JVOIPSIMPLELOCALISATION_H

#define JVOIPSIMPLELOCALISATION_H

#include "jvoipsessionlocalisation.h"
#include "jvoipsampleconverter.h"

class JVOIPSimpleLocalisation : public JVOIPSessionLocalisation
{
public:
	JVOIPSimpleLocalisation(JVOIPSession *sess);
	~JVOIPSimpleLocalisation();
	int Init(int outputsamprate,int outputbytespersample,const JVOIPComponentParams *componentparams);
	int Cleanup();
	int Create3DEffect(double local_x,double local_y,double local_z,
                           double righteardir_x,double righteardir_y,double righteardir_z,
                           double frontdir_x,double frontdir_y,double frontdir_z,
			   double updir_x,double updir_y,double updir_z,
                           double remote_x,double remote_y,double remote_z,
                           VoIPFramework::VoiceBlock *vb,
                           VoIPFramework::VOIPuint64 sourceid);
	
	bool SupportsOutputSamplingRate(int orate);
	int SetOutputSamplingRate(int orate);
	bool SupportsOutputBytesPerSample(int outputbytespersample);
	int SetOutputBytesPerSample(int outputbytespersample);
	
	int GetComponentState(JVOIPComponentState **compstate);
	int SetComponentState(JVOIPComponentState *compstate);
	
	std::string GetComponentName();
	std::string GetComponentDescription();
	std::vector<JVOIPCompParamInfo> *GetComponentParameters() throw (JVOIPException);
private:
	void CalculateDistances();
	int AllocateNewBuffer();

	bool init;
	int samplerate,bytespersample;
	JVOIPSampleConverter sampconv;

	VoIPFramework::VoiceBlock *vblock;
	unsigned char *data;
	int datalen;
	unsigned char *newdata;
	int newdatalen;
	int newnumsamples;

	double localpos[3];
	double remotepos[3];
	double righteardir[3];

	double lefteardist,righteardist;
	double lefteartimedist,righteartimedist;
	double leftearampfactor,rightearampfactor;
	int leftearsampleoffset,rightearsampleoffset;

};

#endif // JVOIPSIMPLELOCALISATION_H
