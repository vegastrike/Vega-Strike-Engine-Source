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

#ifndef RTPSOURCES_H

#define RTPSOURCES_H

#include "rtpconfig.h"
#include "rtpdefines.h"
#include "rtpdebug.h"
#include "rtpstructs.h"
#include "rtpexceptionstructs.h"
#include "rtperror.h"
#include <stdlib.h>

#define RTP_SOURCETABLE_HASHSIZE	1024

class RTPPacket;
class RTPSourceData;
class RTPHandlers;
class RTPContributingSources;
class RTPConnection;

class RTPSources								RTPDEBUGBASE
{
public:
	RTPSources();
	~RTPSources();
	inline int SetHandlers(RTPHandlers *hand);
	inline int SetContributingSources(RTPContributingSources *contrib);
	inline int SetConnection(RTPConnection *conn);
	void Clear();
	RTPSourceData *Retrieve(unsigned long src);
	int ProcessPacket(RTPPacket *packet,unsigned long ip,int port,double localtsunit);
	int ProcessSRInfo(RTPuint32 src,RTPuint32 ntplsw,RTPuint32 ntpmsw,RTPuint32 rtptimestamp,RTPuint32 packetcount,RTPuint32 bytecount,unsigned long ip,int port,double localtsunit);
	int ProcessSDESInfo(RTPuint32 src,int sdestype,unsigned char *sdesdata,int len,unsigned long ip,int port,double localtsunit);
	int ProcessBYEMessage(RTPuint32 src,unsigned long ip,int port);
	int ProcessRRInfo(RTPuint32 src,unsigned char fraclost,long packetslost,RTPuint32 exthighseqnum,RTPuint32 jitter,RTPuint32 lsr,RTPuint32 dlsr,unsigned long ip,int port,double localtsunit);
	void CSRCAdded(RTPuint32 csrc);
	void CSRCDeleted(RTPuint32 csrc);
	void UpdateAllSources();
	int GetNumberOfParticipants() { return numsources; }
	int GetNumberOfSenders() { return numsenders; }
	bool GotoFirstSender();
	bool GotoNextSender();
	bool GotoFirstSource();
	bool GotoNextSource();
	bool GotoFirstSourceWithData();
	bool GotoNextSourceWithData();
	RTPSourceData *GetSourceInfo() { return cursource; }
private:
	RTPSourceData *RetrieveOrCreate(unsigned long src,double localtsunit,bool *created);
	inline void CallNewSourceHandler(RTPuint32 ssrc);
	inline void CallSSRCCollisionHandler(RTPuint32 ssrc,unsigned long ip,bool rtpdata,int port);
	inline void CallInvalidSDESTypeHandler(RTPuint32 ssrc,int type,unsigned char *data,int datalen);
	inline void CallSSRCDepartureHandler(RTPuint32 ssrc);
	inline void CallSSRCTimeoutHandler(RTPuint32 ssrc);

	RTPSourceData *sourcetable[RTP_SOURCETABLE_HASHSIZE];
	int numsources;
	int numsenders;
	bool initialized;
	RTPHandlers *handlers;
	RTPContributingSources *contribsources;
	RTPConnection *rtpconn;

	// retrieval info
	RTPSourceData *cursource;
	int curtablepos;

	// variables to hold exception info
	RTPExcepSSRCCollision ex_ssrccol;
	RTPExcepSSRC ex_ssrc;
	RTPExcepInvalSDESType ex_invalsdes;
};

inline int RTPSources::SetHandlers(RTPHandlers *hand)
{
	if (hand == NULL)
		return ERR_RTP_ARGUMENTCANTBENULL;
	handlers = hand;
	if (contribsources != NULL && rtpconn != NULL)
		initialized = true;
	return 0;
}

inline int RTPSources::SetContributingSources(RTPContributingSources *contrib)
{
	if (contrib == NULL)
		return ERR_RTP_ARGUMENTCANTBENULL;
	contribsources = contrib;
	if (handlers != NULL && rtpconn != NULL)
		initialized = true;
	return 0;
}

inline int RTPSources::SetConnection(RTPConnection *conn)
{
	if (conn == NULL)
		return ERR_RTP_ARGUMENTCANTBENULL;
	rtpconn = conn;
	if (handlers != NULL && contribsources != NULL)
		initialized = true;
	return 0;
}

#endif // RTPSOURCES_H
