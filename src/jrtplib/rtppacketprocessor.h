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

#ifndef RTPPACKETPROCESSOR_H

#define RTPPACKETPROCESSOR_H

#include "rtpconfig.h"
#include "rtpdefines.h"
#include "rtpdebug.h"
#include "rtpstructs.h"
#include "rtpexceptionstructs.h"
#include "rtphandlers.h"
#include "rtperror.h"

class RTPSources;
class RTPHandlers;
class RTPContributingSources;
class RTPPacket;
class RTPLocalInfo;
class RTPConnection;

class RTPPacketProcessor							RTPDEBUGBASE
{
public:
	RTPPacketProcessor();
	~RTPPacketProcessor();
	inline int SetSources(RTPSources *s);
	inline int SetHandlers(RTPHandlers *hand);
	inline int SetContributingSources(RTPContributingSources *srcs);
	inline int SetConnection(RTPConnection *c);
	int ProcessRTPBlock(unsigned char *data,int len,unsigned long ip,int port,bool *collis,bool acceptlocalpackets,double localtsunit); // can't use 'data' after calling this function
	int ProcessRTCPBlock(unsigned char *data,int len,unsigned long ip,int port,bool *collis,double localtsunit); // can't use 'data' after calling this function
private:
	static int GetRTPData(unsigned char *data,int len,RTPPacket **packet);
	int ProcessSenderReport(unsigned char *data,int len,int reportcount,bool *collis,double localtsunit);
	int ProcessReceiverReport(unsigned char *data,int len,int reportcount,bool *collis,double localtsunit);
	int ProcessSDES(unsigned char *data,int len,int sourcecount,bool *colli,double localtsunits);
	int ProcessBYE(unsigned char *data,int len,int sourcecount);
	int ProcessReportBlocks(RTPuint32 senderssrc,unsigned char *data,int len,int reportcount,double localtsunit);
	inline void CallLocalSSRCCollHandler(RTPuint32 ssrc,unsigned long ip,bool rtpdata,int port);
	void CallAppDataHandler(unsigned char *data,int len,unsigned char subtype);
	inline void CheckRTCPHandler(unsigned char *data,int len,unsigned long ip, int port);

	RTPSources *sources;
	RTPConnection *conn;
	RTPHandlers *handlers;
	RTPContributingSources *contribsrcs;
	bool initialized;
	unsigned long curip;
	unsigned long curport;

	// exception structs
	RTPExcepSSRCCollision ex_ssrccol;
	RTPExcepAppData ex_appdata;
};

inline int RTPPacketProcessor::SetSources(RTPSources *s)
{
	if (s == NULL)
		return ERR_RTP_ARGUMENTCANTBENULL;
	sources = s;
	if (handlers != NULL && contribsrcs != NULL && conn != NULL)
		initialized = true;
	return 0;
}

inline int RTPPacketProcessor::SetHandlers(RTPHandlers *hand)
{
	if (hand == NULL)
		return ERR_RTP_ARGUMENTCANTBENULL;
	handlers = hand;
	if (sources != NULL && contribsrcs != NULL && conn != NULL)
		initialized = true;
	return 0;
}

inline int RTPPacketProcessor::SetContributingSources(RTPContributingSources *srcs)
{
	if (srcs == NULL)
		return ERR_RTP_ARGUMENTCANTBENULL;
	contribsrcs = srcs;
	if (sources != NULL && handlers != NULL && conn != NULL)
		initialized = true;
	return 0;
}

inline int RTPPacketProcessor::SetConnection(RTPConnection *c)
{
	if (c == NULL)
		return ERR_RTP_ARGUMENTCANTBENULL;
	conn = c;
	if (sources != NULL && handlers != NULL && contribsrcs != NULL)
		initialized = true;
	return 0;
}

inline void RTPPacketProcessor::CallLocalSSRCCollHandler(RTPuint32 ssrc,unsigned long ip,bool rtpdata,int port)
{
	RTPExceptionHandler handler;
	void *usrdata;

	handler = handlers->handlers[RTP_EXCEPTION_LOCALSSRCCOLLISION].handler;
	usrdata = handlers->handlers[RTP_EXCEPTION_LOCALSSRCCOLLISION].usrdata;
	ex_ssrccol.ip = ip;
	ex_ssrccol.port = port;
	ex_ssrccol.rtpdata = rtpdata;
	ex_ssrccol.ssrc = ssrc;
	handler(RTP_EXCEPTION_LOCALSSRCCOLLISION,&ex_ssrccol,usrdata);
}

#endif // RTPPACKETPROCESSOR_H
