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

#include "rtpsourcedata.h"
#include "rtperror.h"
#include "rtptimeutil.h"
#include "rtppacket.h"
#include <stdlib.h>

/* RTPSenderReportData */

RTPSenderReportData::RTPSenderReportData()
{
	srreceived = false;
	srtime.tv_sec = 0;
	srtime.tv_usec = 0;
	ntplsw = 0;
	ntpmsw = 0;
	rtptimestamp = 0;
	packetcount = 0;
	bytecount = 0;
}

/* RTPReceiverReportData */

RTPReceiverReportData::RTPReceiverReportData()
{
	rrreceived = false;
	rrtime.tv_sec = 0;
	rrtime.tv_usec = 0;
	fractionlost = 0;
	packetslost = 0;
	exthighseqnum = 0;
	jitter = 0;
	lsr = 0;
	dlsr = 0;
}

/* RTPSourceStats */

RTPSourceStats::RTPSourceStats()
{
	hassentdata = false;
	numpacketsreceived = 0;
	numnewpackets = 0;
	numcycles = 0;
	seqbase = 0;
	maxseq = 0;
	prevmaxseq = 0;
	jitter = 0;
	djitter = 0;
	prevts = 0;
	prevpacktime.tv_sec = 0;
	prevpacktime.tv_usec = 0;
	lastmsgtime = 0;
	rtt.tv_sec = 0;
	rtt.tv_usec = 0;
}

void RTPSourceStats::ProcessPacketInStats(RTPPacket *p,double tsunit)
{
	numpacketsreceived++;
	numnewpackets++;
	if (!hassentdata)
	{
		hassentdata = true;
		if (p->seqnum == 0)
		{
			seqbase = 0xFFFF;
			numcycles = 0x00010000;
		}
		else
			seqbase = p->seqnum-1;
		maxseq = seqbase+1;
		p->extendedseqnum = maxseq;
		prevmaxseq = seqbase;
		gettimeofday(&prevpacktime,NULL);
	}
	else // already sent some data
	{
		RTPuint16 maxseq16;
		RTPuint32 extseq;
		struct timeval tv;
		long diffsec,diffusec;
		double diff;
		double diffts1,diffts2;

		/* Adjust max extended sequence number and set extende seq nr of packet */
		
		maxseq16 = (RTPuint16)(maxseq&0x0000FFFF);
		if (p->seqnum >= maxseq16)
		{
			extseq = numcycles+(RTPuint32)p->seqnum;
			maxseq = extseq;
		}
		else
		{
			unsigned short dif1,dif2;

			dif1 = (p->seqnum-maxseq16);
			dif2 = (maxseq16-p->seqnum); 
			if (dif1 < dif2) // check on which side it's closest
			{
				numcycles += 0x00010000;
				extseq = numcycles+(RTPuint32)p->seqnum;
				maxseq = extseq;
			}
			else
				extseq = numcycles+(RTPuint32)p->seqnum;
		}
		p->extendedseqnum = extseq;

		/* Calculate jitter */

		gettimeofday(&tv,NULL);
		diffsec = tv.tv_sec-prevpacktime.tv_sec;
		diffusec = tv.tv_usec-prevpacktime.tv_usec;
		while (diffusec < 0)
		{
			diffsec--;
			diffusec += 1000000;
		}
		diffts1 = (((double)diffsec+((double)diffusec)/1000000.0)/tsunit);
		diffts2 = (double)p->timestamp-(double)prevts;
		diff = diffts1-diffts2;
		if (diff < 0)
			diff = -diff;
		diff -= djitter;
		diff /= 16.0;
		djitter += diff;
		if (djitter < 0)
			djitter = 0;
		jitter = (RTPuint32)djitter;
		prevpacktime = tv;
	}
	prevts = p->timestamp;
	lastmsgtime = prevpacktime.tv_sec;
}

/* RTPSourceData */

RTPSourceData::RTPSourceData(unsigned long src,double ts)
{
	ssrc = src;
	sdes.SetSRC(src);
	hassentnewdata = false;
	isaCSRC = false;
	ip = 0;
	rtpport = -1;
	rtcpport = -1;
	tsunit = ts;
	firstpacket = NULL;
	lastpacket = NULL;
	next = NULL;
}

RTPSourceData::~RTPSourceData()
{
	FlushPackets();
}

int RTPSourceData::AddPacket(RTPPacket *p)
{
	RTPuint32 extseqnum;
	RTPPacket *tmp,*tmp2;
	bool found;

	if (p == NULL)
		return 0;
	if (p->syncsource != ssrc)
		return ERR_RTP_PACKETISNOTFORTHISSOURCE;

	stats.ProcessPacketInStats(p,tsunit);
	
	/* look for the right place to insert the packet */
	if (firstpacket == NULL)
	{
		firstpacket = p;
		lastpacket = p;
	}
	else // already some packets in the queue
	{
		/* Search for the right place to insert the packet */

		tmp = lastpacket;
		found = false;
		extseqnum = p->extendedseqnum;
		while (!found && tmp != NULL)
		{
			if (extseqnum < tmp->extendedseqnum)
				tmp = tmp->prev;
			else
				found = true;
		}

		if (found)
		{
			if (tmp->extendedseqnum != extseqnum)
			{
				tmp2 = tmp->next;
				tmp->next = p;
				p->prev = tmp;
				p->next = tmp2;
				if (tmp2 != NULL)
					tmp2->prev = p;
				else
					lastpacket = p;
			}
		}
		else // insert at the front of the list
		{
			p->next = firstpacket;
			firstpacket->prev = p;
			firstpacket = p;
		}
	}
	return 0;
}

RTPPacket *RTPSourceData::ExtractPacket()
{
	RTPPacket *tmp;

	if (firstpacket == NULL)
		return NULL;
	tmp = firstpacket;
	firstpacket = firstpacket->next;
	if (firstpacket != NULL)
		firstpacket->prev = NULL;
	else
		lastpacket = NULL;
	tmp->prev = NULL;
	tmp->next = NULL;
	return tmp;
}

void RTPSourceData::FlushPackets()
{
	while (firstpacket != NULL)
	{
		lastpacket = firstpacket->next;
		RTP_DELETE(firstpacket);
		firstpacket = lastpacket;
	}
}

