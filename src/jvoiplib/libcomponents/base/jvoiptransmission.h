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

#ifndef JVOIPTRANSMISSION_H

#define JVOIPTRANSMISSION_H

#include "voicetransmitter.h"
#include "jvoipcomponent.h"
#include "jvoipsessionparams.h"
#include "jvoiptypes.h"
#include "jvoipsession.h"

class JVOIPTransmission : public VoIPFramework::VoiceTransmitter,public JVOIPComponent
{
public:
	JVOIPTransmission(JVOIPSession *sess):JVOIPComponent(sess) { }
	virtual ~JVOIPTransmission() { }
	virtual int Init(int sampinterval,int inputsamprate,int inputbytespersample,const JVOIPComponentParams *componentparams) = 0;
	virtual int Cleanup() = 0;
	virtual void Reset() = 0;
	
	virtual bool AddDestination(JVOIPuint32 destip,JVOIPuint16 destportbase) = 0;
	virtual bool DeleteDestination(JVOIPuint32 destip,JVOIPuint16 destportbase) = 0;
	virtual void ClearDestinations() = 0;

	virtual bool SupportsMulticasting() = 0;	
	virtual bool JoinMulticastGroup(JVOIPuint32 mcastip) = 0;
	virtual bool LeaveMulticastGroup(JVOIPuint32 mcastip) = 0;
	virtual void LeaveAllMulticastGroups() = 0;

	virtual bool SupportsReceiveType(JVOIPSessionParams::ReceiveType rt) = 0;
	virtual bool SetReceiveType(JVOIPSessionParams::ReceiveType rt) = 0;
	virtual bool AddToAcceptList(JVOIPuint32 ip,JVOIPuint16 port = JVOIP_ALLPORTS) = 0;
	virtual bool DeleteFromAcceptList(JVOIPuint32 ip,JVOIPuint16 port = JVOIP_ALLPORTS) = 0;
	virtual void ClearAcceptList() = 0;
	virtual bool AddToIgnoreList(JVOIPuint32 ip,JVOIPuint16 port = JVOIP_ALLPORTS) = 0;
	virtual bool DeleteFromIgnoreList(JVOIPuint32 ip,JVOIPuint16 port = JVOIP_ALLPORTS) = 0;
	virtual void ClearIgnoreList() = 0;

	virtual bool SupportsSampleInterval(int ival) = 0;
	virtual int SetSampleInterval(int ival) = 0;
	virtual bool SupportsInputSamplingRate(int irate) = 0;
	virtual int SetInputSamplingRate(int irate) = 0;
	virtual bool SupportsInputBytesPerSample(int inputbytespersample) = 0;
	virtual int SetInputBytesPerSample(int inputbytespersample) = 0;
};

#endif // JVOIPTRANSMISSION_H
