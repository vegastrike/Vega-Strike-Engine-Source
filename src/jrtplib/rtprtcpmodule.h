/*

  This file is a part of JRTPLIB
  Copyright (c) 1999-2002 Jori Liesenborgs

  Contact: jori@lumumba.luc.ac.be

  This library (JRTPLIB) was partially developed for my thesis at the
  School for Knowledge Technology (Belgium/The Netherlands)

  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.

*/

#ifndef RTPRTCPMODULE_H

#define RTPRTCPMODULE_H

#include "rtpconfig.h"
#include "rtpdefines.h"
#include "rtpdebug.h"
#include "rtptimeutil.h"
#include "rtpstructs.h"
#include "rtpexceptionstructs.h"
#include "rtperror.h"

class RTPConnection;
class RTPSources;
class RTPContributingSources;
class RTPLocalInfo;
class RTPSourceData;
class RTPSourceDescription;
class RTPHandlers;

class RTPRTCPModule								RTPDEBUGBASE
{
public:
	RTPRTCPModule();
	~RTPRTCPModule();
	inline int SetConnection(RTPConnection *conn);
	inline int SetSources(RTPSources *srcs);
	inline int SetContributingSources(RTPContributingSources *contrib);
	inline int SetLocalInfo(RTPLocalInfo *locinf);
	inline int SetHandlers(RTPHandlers *handl);
	void Initialize();
	void SetSessionBandWidth(double bw)				{ sessbandwidth = bw; rtcpbandwidth = sessbandwidth*rtcpfrag; CalcNextRTCPTime(); }
	void SetControlTrafficFragment(double frag)			{ rtcpfrag = frag; rtcpbandwidth = sessbandwidth*rtcpfrag; CalcNextRTCPTime(); }
	bool RTCPCheck(unsigned long curtime)				{ if (curtime > nextrtcptime) return true; return false; }
	int RTCPRoutine(unsigned long curtime);
	void SentData()							{ sentdatasincelastSR = true; }
	int SendBYE();
	void CalcNextRTCPTime();
private:
	int BuildAndSendPackets(bool bye,bool allsdesinfo);
	int ProcessReports();
	int ProcessSDESInfo(bool allsdesinfo);
	int ProcessAPPData();
	int ProcessBYEMessage();
	inline int SendPacketData();
	void GetRRParams(RTPSourceData *src,RTCPReportBlock *rr);

	bool sentdatasincelastSR;
	double sessbandwidth,rtcpbandwidth;
	double rtcpfrag;
	double avgrtcpsize;
	unsigned long prevrtcptime,nextrtcptime;
	unsigned long numrtcpsent,rtcpcount;
	
	bool initialized;
	RTPConnection *rtpconn;
	RTPSources *sources;
	RTPContributingSources *contribsrcs;
	RTPLocalInfo *localinf;
	RTPHandlers *handlers;

	// some values to be used when constructing RTCP packets
	int maxpacksize;	
	int sendcount;
	int packetoffset;
	
	// APP struct
	RTPExcepAppData ex_appdata;
	
	// packet buffer	
	unsigned char packetbuffer[RTP_MAXIMUMPACKETSIZE];
};

inline int RTPRTCPModule::SetConnection(RTPConnection *conn)
{
	if (conn == NULL)
		return ERR_RTP_ARGUMENTCANTBENULL;
	rtpconn = conn;
	if (sources != NULL && contribsrcs != NULL && localinf != NULL && handlers != NULL)
		initialized = true;
	return 0;
}

inline int RTPRTCPModule::SetSources(RTPSources *srcs)
{
	if (srcs == NULL)
		return ERR_RTP_ARGUMENTCANTBENULL;
	sources = srcs;
	if (rtpconn != NULL && contribsrcs != NULL && localinf != NULL && handlers != NULL)
		initialized = true;
	return 0;
}

inline int RTPRTCPModule::SetContributingSources(RTPContributingSources *contrib)
{
	if (contrib == NULL)
		return ERR_RTP_ARGUMENTCANTBENULL;
	contribsrcs = contrib;
	if (rtpconn != NULL && sources != NULL && localinf != NULL && handlers != NULL)
		initialized = true;
	return 0;
}

inline int RTPRTCPModule::SetLocalInfo(RTPLocalInfo *locinf)
{
	if (locinf == NULL)
		return ERR_RTP_ARGUMENTCANTBENULL;
	localinf = locinf;
	if (rtpconn != NULL && sources != NULL && contribsrcs != NULL && handlers != NULL)
		initialized = true;
	return 0;
}

inline int RTPRTCPModule::SetHandlers(RTPHandlers *hand)
{
	if (hand == NULL)
		return ERR_RTP_ARGUMENTCANTBENULL;
	handlers = hand;
	if (rtpconn != NULL && sources != NULL && contribsrcs != NULL && localinf != NULL)
		initialized = true;
	return 0;
}

#endif // RTPRTCPMODULE_H
