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

#ifndef RTPSOURCEDATA_H

#define RTPSOURCEDATA_H

#include "rtpconfig.h"
#include "rtpdefines.h"
#include "rtpdebug.h"
#include "rtpstructs.h"
#include "rtpsourcedescription.h"
#include "rtptimeutil.h"

class RTPPacket;

class RTPSenderReportData
{
public:
	RTPSenderReportData();
	~RTPSenderReportData() { }

	bool srreceived;
	struct timeval srtime;
	RTPuint32 ntplsw,ntpmsw;
	RTPuint32 rtptimestamp;
	RTPuint32 packetcount;
	RTPuint32 bytecount;
};

class RTPReceiverReportData
{
public:
	RTPReceiverReportData();
	~RTPReceiverReportData() { }

	bool rrreceived;
	struct timeval rrtime;
	unsigned char fractionlost;
	long packetslost;
	RTPuint32 exthighseqnum;
	RTPuint32 jitter;
	RTPuint32 lsr;
	RTPuint32 dlsr;
};

class RTPSourceStats
{
public:
	RTPSourceStats();
	~RTPSourceStats() { }
	void ProcessPacketInStats(RTPPacket *p,double tsunit);

	bool hassentdata;
	long numpacketsreceived;
	long numnewpackets;
	RTPuint32 numcycles; // shifted left 16 bits
	RTPuint32 seqbase,maxseq,prevmaxseq;
	RTPuint32 prevts;
	RTPuint32 jitter;
	double djitter;
	struct timeval prevpacktime;
	struct timeval rtt; // round trip time
	
	unsigned long lastmsgtime; // for timeout
};

class RTPSourceData								RTPDEBUGBASE
{
public:
	RTPSourceData(unsigned long src,double tsunit);
	~RTPSourceData();
	RTPPacket *ExtractPacket();
	void FlushPackets();
	unsigned long GetSSRC()										{ return ssrc; }
	bool HasData()											{ if (firstpacket == NULL) return false; return true; }

	void SetTimestampUnit(double ts)								{ tsunit = ts; }
	double GetTimestampUnit()									{ return tsunit; }
	
	bool SR_HasInfo()										{ return sr.srreceived; }
	void SR_GetNTPTimestamp(unsigned long *msw,unsigned long *lsw)					{ *msw = sr.ntpmsw; *lsw = sr.ntplsw; }
	unsigned long SR_GetRTPTimestamp()								{ return sr.rtptimestamp; }
	unsigned long SR_GetPacketCount()								{ return sr.packetcount; }
	unsigned long SR_GetByteCount()									{ return sr.bytecount; }
	struct timeval SR_GetReceiveTime()								{ return sr.srtime; }
	
	bool RR_HasInfo()										{ return rr.rrreceived; }
	float RR_GetFractionLost()									{ return (((float)rr.fractionlost)/(float)256.0); }
	long RR_GetPacketsLost()									{ return rr.packetslost; }
	unsigned long RR_GetExtendedHighestSequenceNumber()						{ return rr.exthighseqnum; }
	unsigned long RR_GetJitter()									{ return rr.jitter; }
	unsigned long RR_GetLastSRTimestamp()								{ return rr.lsr; }
	unsigned long RR_GetDelaySinceLastSR()								{ return rr.dlsr; }
	struct timeval RR_GetReceiveTime()								{ return rr.rrtime; }

	bool INF_HasSentData()										{ return stats.hassentdata; }
	long INF_GetNumPacketsReceived()								{ return stats.numpacketsreceived; }
	unsigned long INF_GetBaseSequenceNumber()							{ return stats.seqbase; }
	unsigned long INF_GetExtendedHighestSequenceNumber()						{ return stats.maxseq; }
	unsigned long INF_GetJitter()									{ return stats.jitter; }
	struct timeval INF_GetRoundTripTime()								{ return stats.rtt; }

	char *SDES_GetCNAME(int *len)									{ return GetSDESInfo(TYPE_SDES_CNAME-1,len); }
	char *SDES_GetName(int *len)									{ return GetSDESInfo(TYPE_SDES_NAME-1,len); }
	char *SDES_GetEMail(int *len)									{ return GetSDESInfo(TYPE_SDES_EMAIL-1,len); }
	char *SDES_GetLocation(int *len)								{ return GetSDESInfo(TYPE_SDES_LOC-1,len); }
	char *SDES_GetPhone(int *len)									{ return GetSDESInfo(TYPE_SDES_PHONE-1,len); }
	char *SDES_GetTool(int *len)									{ return GetSDESInfo(TYPE_SDES_TOOL-1,len); }
	char *SDES_GetNote(int *len)									{ return GetSDESInfo(TYPE_SDES_NOTE-1,len); }
private:
	int AddPacket(RTPPacket *p);
	char *GetSDESInfo(int index,int *len)								{ *len = sdes.sdesinfolen[index]; return (char *)sdes.sdesinfo[index]; }


	RTPuint32 ssrc;
	RTPPacket *firstpacket,*lastpacket;

	RTPSenderReportData sr;
	RTPReceiverReportData rr;
	RTPSourceDescription sdes;
	RTPSourceStats stats;
	bool hassentnewdata;
	bool isaCSRC;
	unsigned long ip;
	int rtpport,rtcpport;
	double tsunit;
		
	RTPSourceData *next;

friend class RTPSources;
friend class RTPRTCPModule;
};

#endif // RTPSOURCEDATA_H
