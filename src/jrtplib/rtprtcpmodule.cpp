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

#include "rtprtcpmodule.h"
#include "rtptimeutil.h"
#include "rtpdefines.h"
#include "rtpconnection.h"
#include "rtpsources.h"
#include "rtpcontributingsources.h"
#include "rtplocalinfo.h"
#include "rtprandom.h"
#include "rtpsourcedescription.h"
#include "rtpsourcedata.h"
#include "rtphandlers.h"
#include <string.h>

/* Class implementation */

RTPRTCPModule::RTPRTCPModule()
{
	initialized = false;
	rtpconn = NULL;
	sources = NULL;
	contribsrcs = NULL;
	localinf = NULL;
	handlers = NULL;
}

RTPRTCPModule::~RTPRTCPModule()
{
}

void RTPRTCPModule::Initialize()
{
	sessbandwidth = RTP_DEFAULT_SESSIONBANDWIDTH;
	rtcpfrag = RTP_DEFAULT_RTCPFRAGMENT;
	rtcpbandwidth = sessbandwidth*rtcpfrag;
	prevrtcptime = time(NULL);
	CalcNextRTCPTime();
	sentdatasincelastSR = false;
	
	// assume that source is a sender, that there's at least 1 receiver and
	// that the average CNAME length is 30
	avgrtcpsize = (double)(IP_UDP_HEADERSIZE+sizeof(RTCPHeader)+sizeof(SSRCPrefix)
						   +sizeof(RTCPSenderInfo)+sizeof(RTCPReportBlock)
						   +sizeof(RTCPHeader)+sizeof(SSRCPrefix)+sizeof(SDESPrefix)
						   +30);
	numrtcpsent = 0;
	rtcpcount = 0;
}

void RTPRTCPModule::CalcNextRTCPTime()
{
	double localrtcpbandwidth;
	double delay,factor;
	unsigned long delay2;
	
	if (sources == NULL)
		localrtcpbandwidth = rtcpbandwidth;
	else
		localrtcpbandwidth = rtcpbandwidth/((double)sources->GetNumberOfParticipants()+1.0); // +1 for this session
	delay = (avgrtcpsize*8)/localrtcpbandwidth;
	if (delay < RTP_MINIMUM_RTCP_DELAY) // minimum delay of 5 seconds
		delay = RTP_MINIMUM_RTCP_DELAY;
	factor = (((double)RTPRandom::RandomByte())/256.0)+0.5; // factor from interval [0.5,1.5[
	delay *= factor;
	delay2 = (unsigned long)(delay+0.5);
	nextrtcptime = prevrtcptime+delay2;
}

int RTPRTCPModule::RTCPRoutine(unsigned long curtime)
{
	int bytessent;
	bool allsdesinfo;

	if (!initialized)
		return ERR_RTP_RTCPMODULENOTINITIALIZED;

	/* Recalc delay requirements */

	prevrtcptime = curtime;
	CalcNextRTCPTime();

	/* Check if we have to send just the CNAME or all required SDES fields */
	
	if (rtcpcount >= RTP_SDESINFO_INTERVAL)
	{
		allsdesinfo = true;
		rtcpcount = 0;
	}
	else
	{
		rtcpcount++;
		allsdesinfo = false;
	}

	/* Build and send the packets */

	bytessent = BuildAndSendPackets(false,allsdesinfo);
	sources->UpdateAllSources();
	if (bytessent < 0)
		return bytessent;

	/* Recalculate average rtcpsize */

	if (numrtcpsent < 1000000) // this should be enough loops to make a good average
	{
		numrtcpsent++;
		avgrtcpsize = (((double)(numrtcpsent-1))*avgrtcpsize+((double)bytessent))/((double)numrtcpsent);
	}

	sentdatasincelastSR = false;
	return 0;
}

int RTPRTCPModule::SendBYE()
{
	int bytessent;

	if (!initialized)
		return ERR_RTP_RTCPMODULENOTINITIALIZED;

	bytessent = BuildAndSendPackets(true,false);
	sources->UpdateAllSources();
	if (bytessent < 0)
		return bytessent;

	return 0;	
}

int RTPRTCPModule::BuildAndSendPackets(bool bye,bool allsdesinfo)
{
	int status;
	
	maxpacksize = localinf->maxpacksize;
	sendcount = 0;
	packetoffset = 0;
	
	if ((status = ProcessReports()) < 0)
		return status;
	if ((status = ProcessSDESInfo(allsdesinfo)) < 0)
		return status;
	if ((status = ProcessAPPData()) < 0)
		return status;
	if (bye)
	{
		if ((status = ProcessBYEMessage()) < 0)
			return status;
	}
	if (packetoffset > 0) // some info still has to be sent
	{
		if ((status = SendPacketData()) < 0)
			return status;
	}
	return sendcount;
}

int RTPRTCPModule::ProcessReports()
{
	int status;
	bool firsttime;
	bool done,neednewheader,mustsetfields;
	struct timeval tv;
	int RRsourcesprocessed;
	int RRnumsources;
	int blockcount;
	int length;
	RTCPHeader *hdr;
	SSRCPrefix *ssrcpref;
	RTCPSenderInfo *sendinf;
	RTPSourceData *srcdat;
	
	hdr = NULL;
	RRsourcesprocessed = 0;
	RRnumsources = sources->GetNumberOfSenders();
	sources->GotoFirstSender();
	firsttime = true;
	done = false;
	neednewheader = true;
	length = 0;
	blockcount = 0;
	mustsetfields = false;
	while (!done)
	{
		if (firsttime && sentdatasincelastSR) // SR info
		{
			// here, we can be sure that we've got enough space to
			// put the header in, since we're at the start of the packet
			firsttime = false;
			
			hdr = (RTCPHeader *)packetbuffer;
			hdr->version = RTP_VERSION;
			hdr->padding = 0;
			hdr->packettype = TYPE_RTCP_SR;
			blockcount = 0;
			length = ((sizeof(RTCPHeader)+sizeof(SSRCPrefix)+sizeof(RTCPSenderInfo))/sizeof(RTPuint32))-1;
			
			ssrcpref = (SSRCPrefix *)(packetbuffer+sizeof(RTCPHeader));
			ssrcpref->ssrc = contribsrcs->localinfo.src; // already in network byte order
			
			sendinf = (RTCPSenderInfo *)(packetbuffer+sizeof(RTCPHeader)+sizeof(SSRCPrefix));
			gettimeofday(&tv,NULL);
			getntptime(&tv,&(sendinf->NTPlsw),&(sendinf->NTPmsw));
			getrtptimestamp(&tv,&(localinf->tsoffsettime),localinf->timestampoffset,localinf->tsunit,&(sendinf->rtptimestamp));
			sendinf->senderoctetcount = htonl(localinf->octetcount);
			sendinf->senderpacketcount = htonl(localinf->packetcount);
			
			packetoffset = sizeof(RTCPHeader)+sizeof(SSRCPrefix)+sizeof(RTCPSenderInfo);
			neednewheader = false;
			mustsetfields = true;
		}
		else if (RRsourcesprocessed >= RRnumsources)
			done = true;
		else // RR info
		{
			if (neednewheader)
			{
				// check if the header and ssrc identifier will fit
				if ((int)(packetoffset + sizeof(RTCPHeader) + sizeof(SSRCPrefix)
				     + sizeof(RTCPReportBlock)) > maxpacksize)
				{
					hdr->length = htons(length);
					hdr->blockcount = blockcount;
					if ((status = SendPacketData()) < 0)
						return status;
					mustsetfields = false;
				}
				else
				{
					hdr = (RTCPHeader *)(packetbuffer+packetoffset);
					hdr->version = RTP_VERSION;
					hdr->padding = 0;
					hdr->packettype = TYPE_RTCP_RR;
					blockcount = 0;
					length = ((sizeof(RTCPHeader)+sizeof(SSRCPrefix))/sizeof(RTPuint32))-1;
					
					ssrcpref = (SSRCPrefix *)(packetbuffer+packetoffset+sizeof(RTCPHeader));
					ssrcpref->ssrc = contribsrcs->localinfo.src; // already in network byte order
					
					neednewheader = false;
					packetoffset += sizeof(RTCPHeader)+sizeof(SSRCPrefix);
					mustsetfields = true;
				}
			}
			else // add a reception report block, if possible
			{
				if ((int)(packetoffset + sizeof(RTCPReportBlock)) > maxpacksize)
				{
					hdr->length = htons(length);
					hdr->blockcount = blockcount;
					if ((status = SendPacketData()) < 0)
						return status;
					neednewheader = true;
					mustsetfields = false;
				}
				else
				{
					srcdat = sources->GetSourceInfo();
					GetRRParams(srcdat,(RTCPReportBlock *)(packetbuffer+packetoffset));
					blockcount++;
					length += sizeof(RTCPReportBlock)/sizeof(RTPuint32);
					packetoffset += sizeof(RTCPReportBlock);
					RRsourcesprocessed++;
					sources->GotoNextSender();
					if (blockcount == 31) // max number of reports in one RTCP block
					{
						hdr->blockcount = 31;
						hdr->length = htons(length);
						neednewheader = true;
						mustsetfields = false;
					}
					else
						mustsetfields = true;
				}
			}
		}
	}
	if (mustsetfields)
	{
		hdr->length = htons(length);
		hdr->blockcount = blockcount;
	}
	
	return 0;
}

int RTPRTCPModule::ProcessSDESInfo(bool allsdesinfo)
{
	bool needsrchdr,needrtcphdr;
	bool increment,haveheader;
	int status,sdespos;
	int sdeslen,add,mod;
	int blockcount,length;
	RTCPHeader *hdr;
	SSRCPrefix *ssrcpref;
	RTPSourceDescription *cursrc;
	SDESPrefix *sdespref;

	hdr = NULL;
	cursrc = &(contribsrcs->localinfo);	
	sdespos = 0;
	needsrchdr = true;
	needrtcphdr = true;
	length = 0;
	blockcount = 0;
	haveheader = false;
	while (cursrc != NULL)
	{
		increment = true;
		if ((sdespos == (TYPE_SDES_CNAME-1)) || (allsdesinfo && contribsrcs->enabledinfo[sdespos]))
		{
			add = 0;
			if (packetoffset == 0)
				add = sizeof(RTCPHeader)+sizeof(SSRCPrefix);
			if (needrtcphdr)
				add += sizeof(RTCPHeader);
			if (needsrchdr)
				add += sizeof(SSRCPrefix);
			add += sizeof(SDESPrefix);
			sdeslen = cursrc->sdesinfolen[sdespos];

			if ((packetoffset + add + sdeslen) > (maxpacksize-4))
			{
				increment = false;
				if (haveheader)
				{
					packetbuffer[packetoffset++] = 0; // end of chunk
					length++;
					// fill till 32 bit boundary
					if ((mod = (length%sizeof(RTPuint32))) != 0)
					{
						while (mod != sizeof(RTPuint32))
						{
							packetbuffer[packetoffset++] = 0;
							length++;
							mod++;
						}
					}
					hdr->length = htons(length/sizeof(RTPuint32)-1);
					hdr->blockcount = blockcount;
				}
				if ((status = SendPacketData()) < 0)
					return status;
				needrtcphdr = true;
				needsrchdr = true;
				haveheader = false;
			}
			else // info fits
			{
				if (packetoffset == 0)
				{
					hdr = (RTCPHeader *)packetbuffer;
					hdr->version = RTP_VERSION;
					hdr->packettype = TYPE_RTCP_RR;
					hdr->padding = 0;
					hdr->blockcount = 0;
					hdr->length = htons((sizeof(RTCPHeader)+sizeof(SSRCPrefix))/sizeof(RTPuint32)-1);
					
					ssrcpref = (SSRCPrefix *)(packetbuffer+sizeof(RTCPHeader));
					ssrcpref->ssrc = contribsrcs->localinfo.src; // already in network byte order
					
					packetoffset = sizeof(RTCPHeader)+sizeof(SSRCPrefix);
				}
				if (needrtcphdr)
				{
					needrtcphdr = false;
					hdr = (RTCPHeader *)(packetbuffer + packetoffset);
					hdr->version = RTP_VERSION;
					hdr->packettype = TYPE_RTCP_SDES;
					hdr->padding = 0;
					blockcount = 0;
					length = sizeof(RTCPHeader);
					packetoffset += sizeof(RTCPHeader);
					haveheader = true;
				}
				if (needsrchdr)
				{
					needsrchdr = false;
					ssrcpref = (SSRCPrefix *)(packetbuffer + packetoffset);
					ssrcpref->ssrc = cursrc->src; // already in network byte order
					
					packetoffset += sizeof(SSRCPrefix);
					length += sizeof(SSRCPrefix);
					blockcount++;
				}
 				sdespref = (SDESPrefix *)(packetbuffer + packetoffset);
				sdespref->sdestype = sdespos+1;
				sdespref->length = sdeslen;
				length += sizeof(SDESPrefix);
				packetoffset += sizeof(SDESPrefix);
				
				if (sdeslen > 0)
				{
					memcpy(packetbuffer+packetoffset,cursrc->sdesinfo[sdespos],sdeslen);
					length += sdeslen;
					packetoffset += sdeslen;
				}
			}
		}
		
		if (increment)
		{
			sdespos++;
			if (sdespos == RTP_NUM_SDES_INDICES)
			{
				sdespos = 0;
				cursrc = cursrc->next;
				needsrchdr = true;
					
				packetbuffer[packetoffset++] = 0; // end of chunk
				length++;
				// fill till 32 bit boundary
				if ((mod = (length%sizeof(RTPuint32))) != 0)
				{
					while (mod != sizeof(RTPuint32))
					{
						packetbuffer[packetoffset++] = 0;
						length++;
						mod++;
					}
				}
				
				if (blockcount == 31) // max number of chunks in a block
				{
					hdr->length = htons(length/sizeof(RTPuint32)-1);
					hdr->blockcount = blockcount;
					needrtcphdr = true;
				}
			}
		}
	}
	if (haveheader)
	{
		// fill till 32 bit boundary
		if ((mod = (length%sizeof(RTPuint32))) != 0)
		{
			while (mod != sizeof(RTPuint32))
			{
				packetbuffer[packetoffset++] = 0;
				length++;
				mod++;
			}
		}
		hdr->length = htons(length/sizeof(RTPuint32)-1);
		hdr->blockcount = blockcount;
	}
	return 0;
}

int RTPRTCPModule::ProcessAPPData()
{
	RTPExceptionHandler apphandler;
	RTCPAPPPrefix *apppref;
	RTCPHeader *hdr;
	SSRCPrefix *ssrcpref;
	void *usrdata;
	bool done,gotdata;
	int prevreturn,size,status;
	
	if ((apphandler = handlers->handlers[RTP_EXCEPTION_TRANSMITRTCPAPPDATA].handler) == NULL)
		return 0;
	usrdata = handlers->handlers[RTP_EXCEPTION_TRANSMITRTCPAPPDATA].usrdata;
	
	done = false;
	gotdata = false;
	prevreturn = 0;
	size = 0;
	while (!done)
	{
		if (!gotdata)
		{
			ex_appdata.subtype = 0;
			// by default: use local ssrc
			ex_appdata.src = ntohl(contribsrcs->localinfo.src);
			ex_appdata.name[0] = ' ';
			ex_appdata.name[1] = ' ';
			ex_appdata.name[2] = ' ';
			ex_appdata.name[3] = ' ';
			ex_appdata.data = NULL;
			ex_appdata.numwords = 0;
			ex_appdata.validdata = false;
			ex_appdata.prevreturnval = prevreturn;
			
			apphandler(RTP_EXCEPTION_TRANSMITRTCPAPPDATA,&ex_appdata,usrdata);
			if (!ex_appdata.validdata)
				done = true;
			else if (ex_appdata.subtype > 31)
				prevreturn = ERR_RTP_SUBTYPETOOLARGE;
			else  // check if the data can possibly fit
			{
				if (ex_appdata.numwords < 0)
					ex_appdata.numwords = 0;
				// first take an empty RR packet
				size = sizeof(RTCPHeader)+sizeof(SSRCPrefix);
				// add the APP packet
				size += sizeof(RTCPHeader)+sizeof(RTCPAPPPrefix);
				size += ex_appdata.numwords*sizeof(RTPuint32);
				if (size > maxpacksize)
					prevreturn = ERR_RTP_PACKETTOOLARGE;
				else
				{
					gotdata = true;
					prevreturn = 0;
				}
			}
		}
		else // try to put the data in a packet
		{	
			if ((packetoffset + size) > maxpacksize)
			{
				if ((status = SendPacketData()) < 0)
					return status;
			}
			else
			{
				if (packetoffset == 0)
				{
					hdr = (RTCPHeader *)packetbuffer;
					hdr->version = RTP_VERSION;
					hdr->padding = 0;
					hdr->blockcount = 0;
					hdr->length = htons(((sizeof(RTCPHeader)+sizeof(SSRCPrefix))/sizeof(RTPuint32))-1);
					hdr->packettype = TYPE_RTCP_RR;
					
					ssrcpref = (SSRCPrefix *)(packetbuffer+sizeof(RTCPHeader));
					ssrcpref->ssrc = contribsrcs->localinfo.src; // already in network byte order
					
					packetoffset = sizeof(RTCPHeader)+sizeof(SSRCPrefix);
				}
				hdr = (RTCPHeader *)(packetbuffer+packetoffset);
				hdr->version = RTP_VERSION;
				hdr->padding = 0;
				hdr->blockcount = ex_appdata.subtype;
				hdr->packettype = TYPE_RTCP_APP;
				hdr->length = htons(ex_appdata.numwords+(sizeof(RTCPHeader)+sizeof(RTCPAPPPrefix))/sizeof(RTPuint32)-1);
				packetoffset += sizeof(RTCPHeader);
				
				apppref = (RTCPAPPPrefix *)(packetbuffer+packetoffset);
				apppref->src = htonl(ex_appdata.src);
				apppref->name[0] = ex_appdata.name[0];
				apppref->name[1] = ex_appdata.name[1];
				apppref->name[2] = ex_appdata.name[2];
				apppref->name[3] = ex_appdata.name[3];
				packetoffset += sizeof(RTCPAPPPrefix);
				
				if (ex_appdata.numwords > 0)
				{
					memcpy(packetbuffer+packetoffset,ex_appdata.data,ex_appdata.numwords*sizeof(RTPuint32));
					packetoffset += ex_appdata.numwords*sizeof(RTPuint32);
				}
				gotdata = false;
			}
		}
	}
			
	return 0;
}

int RTPRTCPModule::ProcessBYEMessage()
{
	bool needbyehdr,mustsetfields;
	int length,status,blockcount,add;
	RTPSourceDescription *cursrc;
	RTCPHeader *hdr;
	SSRCPrefix *ssrcpref;
	
	length = 0;
	blockcount = 0;
	hdr = NULL;
	cursrc = &(contribsrcs->localinfo);
	mustsetfields = false;
	needbyehdr = true;
	while (cursrc != NULL)
	{
		if (packetoffset == 0)
		{
			hdr = (RTCPHeader *)packetbuffer;
			hdr->blockcount = 0;
			hdr->padding = 0;
			hdr->version = RTP_VERSION;
			hdr->packettype = TYPE_RTCP_RR;
			hdr->length = htons(((sizeof(RTCPHeader)+sizeof(SSRCPrefix))/sizeof(RTPuint32))-1);
			
			ssrcpref = (SSRCPrefix *)(packetbuffer+sizeof(RTCPHeader));
			ssrcpref->ssrc = contribsrcs->localinfo.src; // already in network byte order
			
			packetoffset = sizeof(RTCPHeader)+sizeof(SSRCPrefix);
			needbyehdr = true;
			mustsetfields = false;
		}
		else
		{
			if (needbyehdr)
				add = sizeof(RTCPHeader);
			else
				add = 0;
			
			if ((int)(packetoffset+add+sizeof(SSRCPrefix)) > maxpacksize)
			{
				if (mustsetfields)
				{
					hdr->length = htons(length);
					hdr->blockcount = blockcount;
				}
				if ((status = SendPacketData()) < 0)
					return status;
				needbyehdr = true;
				mustsetfields = false;
			}
			else
			{
				if (needbyehdr)
				{
					hdr = (RTCPHeader *)(packetbuffer+packetoffset);
					hdr->version = RTP_VERSION;
					hdr->padding = 0;
					hdr->packettype = TYPE_RTCP_BYE;
					mustsetfields = true;
					length = sizeof(RTCPHeader)/sizeof(RTPuint32)-1;
					blockcount = 0;
					packetoffset += sizeof(RTCPHeader);
					needbyehdr = false;
				}
				ssrcpref = (SSRCPrefix *)(packetbuffer + packetoffset);
				ssrcpref->ssrc = cursrc->src; // already in network byte order
				packetoffset += sizeof(SSRCPrefix);
				length += sizeof(SSRCPrefix)/sizeof(RTPuint32);
				blockcount++;
				if (blockcount == 31)
				{
					hdr->length = htons(length);
					hdr->blockcount = blockcount;
					needbyehdr = true;
					mustsetfields = false;
				}
				cursrc = cursrc->next;
			}
		}
	}
	if (mustsetfields)
	{
		hdr->blockcount = blockcount;
		hdr->length = htons(length);
	}
	
	return 0;
}

inline int RTPRTCPModule::SendPacketData()
{
	int status;
	
	status = rtpconn->SendRTCPCompoundData(packetbuffer,packetoffset);
	if (status < 0)
		return status;
	sendcount += packetoffset;
	packetoffset = 0;
	return 0;
}

void RTPRTCPModule::GetRRParams(RTPSourceData *src,RTCPReportBlock *rr)
{
	struct timeval tv;
	long diffsec,diffusec;
	double diff,f,f2;
	RTPuint32 ext,prevext;
	long packlost;
	RTPuint32 lsr;

	// source
	rr->ssrc = htonl(src->ssrc);

	// dlsr & lsr

	if (src->sr.srreceived)
	{
		// dlsr
	
		gettimeofday(&tv,NULL);
		diffsec = tv.tv_sec-src->sr.srtime.tv_sec;
		diffusec = tv.tv_usec-src->sr.srtime.tv_usec;
		while (diffusec < 0)
		{
			diffsec--;
			diffusec += 1000000;
		}
		diff = (double)diffsec+(((double)diffusec)/1000000.0);
		diff *= 65536;
		rr->dlsr = htonl((RTPuint32)diff);
		
		// lsr
		
		lsr = ((src->sr.ntpmsw&65535)<<16)|((src->sr.ntplsw>>16)&65535);
		rr->lsr = htonl(lsr);
	}
	else
	{
		rr->dlsr = htonl(0);
		rr->lsr = htonl(0);
	}
	
	// ext highest seqnum
		
	ext = src->stats.maxseq;
	rr->exthsnr = htonl(ext);
	
	// fraction lost

	prevext = src->stats.prevmaxseq;
	f = (double)(ext-prevext);
	f2 = f-(double)src->stats.numnewpackets;
	f = f2/f;
	if (f < 0)
		f = 0;
	rr->fractionlost = (unsigned char)(f*256.0);

	// jitter

	rr->jitter = htonl(src->stats.jitter);
	
	// packets lost
		
	packlost = (long)(ext-src->stats.seqbase);
	packlost -= src->stats.numpacketsreceived;
	rr->packetslost[2] = packlost&255;
	rr->packetslost[1] = (packlost>>8)&255;
	rr->packetslost[0] = (packlost>>16)&255;
}
