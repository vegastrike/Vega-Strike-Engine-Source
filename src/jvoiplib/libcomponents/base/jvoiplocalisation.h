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

#ifndef JVOIPLOCALISATION_H

#define JVOIPLOCALISATION_H

#include "location3d.h"
#include "transform3d.h"
#include "jvoipcomponent.h"

class JVOIPLocalisation : public VoIPFramework::Location3D,public VoIPFramework::Transform3D,public JVOIPComponent
{
public:
	JVOIPLocalisation(JVOIPSession *sess):JVOIPComponent(sess) { }
	virtual ~JVOIPLocalisation() { }
	virtual int Init(int outputsamprate,int outputbytespersample,const JVOIPComponentParams *componentparams) = 0;
	virtual int Cleanup() = 0;
	virtual bool SupportsOutputSamplingRate(int orate) = 0;
	virtual int SetOutputSamplingRate(int orate) = 0;
	virtual bool SupportsOutputBytesPerSample(int outputbytespersample) = 0;
	virtual int SetOutputBytesPerSample(int outputbytespersample) = 0;
};

#endif // JVOIPLOCALISATION_H
