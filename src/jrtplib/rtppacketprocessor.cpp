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

#include "rtppacketprocessor.h"
#include "rtpsources.h"
#include "rtpcontributingsources.h"
#include "rtppacket.h"
#include "rtpconnection.h"
#include <stdlib.h>
#include <string.h>

RTPPacketProcessor::RTPPacketProcessor()
{
	sources = NULL;
	handlers = NULL;
	contribsrcs = NULL;
	conn = NULL;
	initialized = false;
}

RTPPacketProcessor::~RTPPacketProcessor()
{
}

int RTPPacketProcessor::ProcessRTPBlock(unsigned char *data,int len,unsigned long ip,int port,bool *collis,bool acceptlocalpackets,double localtsunit)
{
	int status;
	RTPPacket *packet;

	*collis = false;
	if (!initialized)
	{
		RTP_DELETE_ARRAY(data);
		return ERR_RTP_PACKETPROCESSORNOTINITIALIZED;
	}

	if ((status = GetRTPData(data,len,&packet)) < 0)
	{
		RTP_DELETE_ARRAY(data);
		return status;
	}
	if (packet != NULL) // valid packet
	{
		// check for collision with local ssrc and process further
		if (contribsrcs->GetLocalSSRC() == htonl(packet->GetSSRC()))
		{
			// collision with local ssrc

			if (ip == conn->GetLocalIP() && port == conn->GetSendPort())
			{
				if (!acceptlocalpackets)
				{
					RTP_DELETE(packet);
					return 0;
				}
			}
			else
			{
				if (handlers->handlers[RTP_EXCEPTION_LOCALSSRCCOLLISION].handler != NULL)
					CallLocalSSRCCollHandler(packet->GetSSRC(),ip,true,port);
			
				RTP_DELETE(packet);
				*collis = true;
				return 0;
			}
		}
		
		status = sources->ProcessPacket(packet,ip,port,localtsunit);
		if (status < 0)
		{
			RTP_DELETE(packet);

			/* On a collision we still want other data to be processed,
			   so we will not count this as an error */
			if (status == ERR_RTP_COLLISIONBETWEENSSRCS)
				return 0;

			return status;
		}
	}
	return 0;
}

int RTPPacketProcessor::ProcessRTCPBlock(unsigned char *data,int len,unsigned long ip,int port,bool *collis,double localtsunit)
{
	RTCPHeader *rtcphdr;
	unsigned char *data2;
	int type,restlen,calclen;
	int status,count;
	bool firstblock;

	*collis = false;
	if (!initialized)
	{
		RTP_DELETE_ARRAY(data);
		return ERR_RTP_PACKETPROCESSORNOTINITIALIZED;
	}

	if (ip == conn->GetLocalIP() && port == conn->GetSendPort())
	{
		CheckRTCPHandler(data,len,ip,port); // let user process packet
		
		/* processing own control information is not useful, so we won't
		   do it ... */
		RTP_DELETE_ARRAY(data);
		return 0;
	}

	curip = ip;
	curport = port;

	data2 = data;
	restlen = len;
	firstblock = true;
	while (restlen >= (int)sizeof(RTCPHeader))
	{
		rtcphdr = (RTCPHeader *)data2;
		data2 += sizeof(RTCPHeader);
		restlen -= sizeof(RTCPHeader);

		if (rtcphdr->version == RTP_VERSION)
		{
			type = rtcphdr->packettype;
			calclen = ntohs(rtcphdr->length);
			calclen *= sizeof(RTPuint32);
			count = rtcphdr->blockcount;
			if (calclen <= restlen) // otherwise not all the info is included
			{
				/* Check if the compound packet starts with a SR or RR */

				if (firstblock)
				{
					firstblock = false;
					if (type != TYPE_RTCP_SR && type != TYPE_RTCP_RR) // not a rtcp compound packet
					{
						CheckRTCPHandler(data,len,ip,port); // let user process packet
						RTP_DELETE_ARRAY(data);
						return 0;
					}
				}

				/* Just handle the types */

				status = 0;
				switch(type)
				{
				case TYPE_RTCP_SR:
					status = ProcessSenderReport(data2,calclen,count,collis,localtsunit);
					break;
				case TYPE_RTCP_RR:
					status = ProcessReceiverReport(data2,calclen,count,collis,localtsunit);
					break;
				case TYPE_RTCP_SDES:
					if (count > 0)
						status = ProcessSDES(data2,calclen,count,collis,localtsunit);
					break;
				case TYPE_RTCP_BYE:
					if (count > 0)
						status = ProcessBYE(data2,calclen,count);
					break;
				case TYPE_RTCP_APP:
					if (handlers->handlers[RTP_EXCEPTION_RECEIVERTCPAPPDATA].handler != NULL)
						CallAppDataHandler(data2,calclen,count);
					break;
				}
				if (status < 0)
				{
					CheckRTCPHandler(data,len,ip,port); // let user process packet
					RTP_DELETE_ARRAY(data);
					return status;
				}
			}
		}
		else // Not a rtcp compound packet
		{
			CheckRTCPHandler(data,len,ip,port); // let user process packet
			RTP_DELETE_ARRAY(data);
			return 0;
		}

		restlen -= calclen;
		data2 += calclen;
	}
	
	if (*collis) // call handler
	{
		if (handlers->handlers[RTP_EXCEPTION_LOCALSSRCCOLLISION].handler != NULL)
			CallLocalSSRCCollHandler(ntohl(contribsrcs->GetLocalSSRC()),ip,false,port);
	}
	CheckRTCPHandler(data,len,ip,port); // let user process packet
	RTP_DELETE_ARRAY(data);
	return 0;
}

int RTPPacketProcessor::GetRTPData(unsigned char *data,int len,RTPPacket **packet)
{
	RTPPacket *pack;
	RTPHeader *hdr;
	RTPHeaderExtension *hdrext;
	int plen,ccsize,padnum;
	unsigned char *pos;
	RTPuint16 extlen;

	*packet = NULL;
	hdr = (RTPHeader *)data;
	if (hdr->version != RTP_VERSION) // check version
		return 0;
	
	plen = len;
	plen -= sizeof(RTPHeader);
	ccsize = ((int)(hdr->cc))*sizeof(RTPuint32);
	plen -= ccsize;
	pos = data+sizeof(RTPHeader)+ccsize;
	if (hdr->extension == 1)
	{
		hdrext = (RTPHeaderExtension *)pos;
		extlen = ntohs(hdrext->length);
		pos += sizeof(RTPHeaderExtension);
		plen -= sizeof(RTPHeaderExtension);
		pos += ((int)(extlen))*4;
		plen -= ((int)(extlen))*4;
	}
	if (hdr->padding == 1)
	{
		if (len == 0)
			return 0;
		padnum = (int)data[len-1];
		plen -= padnum;
	}
	if (plen < 0)
		return 0;
	
	pack = RTP_NEW(RTPPacket(hdr,data,pos,plen));
	if (pack == NULL)
		return ERR_RTP_OUTOFMEM;
	*packet = pack;
	return 0;
}

int RTPPacketProcessor::ProcessSenderReport(unsigned char *data,int len,int reportcount,bool *collis,double localtsunit)
{
	int len2,status;
	unsigned char *data2;
	SSRCPrefix *ssrcpref;
	RTCPSenderInfo *sendinf;
	RTPuint32 senderssrc,ntpmsw,ntplsw,rtpts,packcnt,bytecnt;

	data2 = data;
	len2 = len;

	if (len2 < (int)sizeof(SSRCPrefix)) // not a full report, just skip it
		return 0;
	ssrcpref = (SSRCPrefix *)data2;
	senderssrc = ntohl(ssrcpref->ssrc);

	/* Check for a collision with the local SSRC */
	
	if (senderssrc == contribsrcs->GetLocalSSRC()) // both in network byte order
	{
		*collis = true;
		return 0;
	}

	data2 += sizeof(SSRCPrefix);
	len2 -= sizeof(SSRCPrefix);

	if (len2 < (int)sizeof(RTCPSenderInfo)) // not a full report, just skip it
		return 0;
	sendinf = (RTCPSenderInfo *)data2;
	ntpmsw = ntohl(sendinf->NTPmsw);
	ntplsw = ntohl(sendinf->NTPlsw);
	rtpts = ntohl(sendinf->rtptimestamp);
	packcnt = ntohl(sendinf->senderpacketcount);
	bytecnt = ntohl(sendinf->senderoctetcount);
	data2 += sizeof(RTCPSenderInfo);
	len2 -= sizeof(RTCPSenderInfo);

	status = sources->ProcessSRInfo(senderssrc,ntplsw,ntpmsw,rtpts,packcnt,bytecnt,curip,curport,localtsunit);
	if (status < 0)
		return status;
	
	if (reportcount > 0)
	{
		status = ProcessReportBlocks(senderssrc,data2,len2,reportcount,localtsunit);
		if (status < 0)
			return status;
	}
		
	return 0;
}

int RTPPacketProcessor::ProcessReceiverReport(unsigned char *data,int len,int reportcount,bool *collis,double localtsunit)
{
	int len2,status;
	unsigned char *data2;
	SSRCPrefix *ssrcpref;
	RTPuint32 senderssrc;

	len2 = len;
	data2 = data;

	if (len2 < (int)sizeof(SSRCPrefix)) // not a full report, just skip it
		return 0;
	ssrcpref = (SSRCPrefix *)data;
	senderssrc = ntohl(ssrcpref->ssrc);

	/* Check for a collision with the local SSRC */
	
	if (senderssrc == contribsrcs->GetLocalSSRC()) // both in network byte order
	{
		*collis = true;
		return 0;
	}

	data2 += sizeof(SSRCPrefix);
	len2 -= sizeof(SSRCPrefix);

	status = ProcessReportBlocks(senderssrc,data2,len2,reportcount,localtsunit);
	if (status < 0)
		return status;
	
	return 0;
}

int RTPPacketProcessor::ProcessSDES(unsigned char *data,int len,int sourcecount,bool *collis,double localtsunit)
{
	unsigned char *data2;
	int len2,i;
	SSRCPrefix *ssrcpref;
	SDESPrefix *sdespref;
	RTPuint32 ssrc,ssrc_nbo;
	int bytecount,mod,adjust;
	int sdestype,length,status;
	bool done,ssrccollis,newssrccollis;
	RTPuint32 localssrc_nbo;
	
	i = 0;
	len2 = len;
	data2 = data;
	localssrc_nbo = contribsrcs->GetLocalSSRC();
	ssrccollis = false;
	while (i < sourcecount && len2 >= (int)(sizeof(SDESPrefix)+sizeof(SSRCPrefix)))
	{
		ssrcpref = (struct SSRCPrefix *)data2;
		ssrc_nbo = ssrcpref->ssrc;
		ssrc = ntohl(ssrc_nbo);
		
		/* Check for a collision with the local SSRC */
	
		newssrccollis = false;
		if (ssrc_nbo == localssrc_nbo) // both in network byte order
		{
			ssrccollis = true;
			newssrccollis = true;
		}

		data2 += sizeof(SSRCPrefix);
		len2 -= sizeof(SSRCPrefix);
		done = false;
		bytecount = 0;
		while (!done && len2 >= 1) //  > 1 because at least 1 byte is needed to signal the end of the chunk
		{
			sdespref = (SDESPrefix *)data2;
			sdestype = (int)sdespref->sdestype;
			if (sdestype == TYPE_SDES_END) // end of source block
			{
				done = true;
				data2++;
				len2--;
				bytecount++;

				// align to a 32 bit boundary

				mod = bytecount%(sizeof(RTPuint32));
				if (mod != 0)
				{
					adjust = sizeof(RTPuint32)-mod;
					data2 += adjust;
					len2 -= adjust;
				}
			}
			else // a 'normal' type
			{
				if (len2 < (int)sizeof(SDESPrefix))
					done = true;
				else
				{
					data2 += sizeof(SDESPrefix);
					len2 -= sizeof(SDESPrefix);
					bytecount += sizeof(SDESPrefix);

					length = (int)sdespref->length;
					if (length <= len2)
					{
						if (!newssrccollis) // uses own ssrc, just skip it
						{
							status = sources->ProcessSDESInfo(ssrc,sdestype,data2,length,curip,curport,localtsunit);
							if (status < 0)
								return status;
						}
						len2 -= length;
						data2 += length;
						bytecount += length;
					}
					else
					{
						data2 += len2;
						len2 = 0;
						bytecount +=len2;
					}
				}
			}
		}
		i++;
	}
	if (ssrccollis)
		*collis = true;
	return 0;
}

int RTPPacketProcessor::ProcessBYE(unsigned char *data,int len,int sourcecount)
{
	unsigned char *data2;
	int len2,i;
	SSRCPrefix *ssrcpref;
	RTPuint32 ssrc;

	i = 0;
	len2 = len;
	data2 = data;
	while(i < sourcecount && len2 >= (int)sizeof(SSRCPrefix))
	{
		ssrcpref = (SSRCPrefix *)data2;
		ssrc = ntohl(ssrcpref->ssrc);
		sources->ProcessBYEMessage(ssrc,curip,curport);
		data2 += sizeof(SSRCPrefix);
		len2 -= sizeof(SSRCPrefix);
		i++;
	}
	return 0;
}

int RTPPacketProcessor::ProcessReportBlocks(RTPuint32 senderssrc,unsigned char *data,int len,int reportcount,double localtsunit)
{
	unsigned char *data2;
	int len2,i,status;
	bool found;
	unsigned char fraclost;
	long packetslost,*pl2;
	unsigned long pl;
	RTCPReportBlock *rb;
	RTPuint32 exthighseqnum,jitter,lsr,dlsr;
	RTPuint32 nbo_localssrc;

	len2 = len;
	data2 = data;
	found = false;
	i = 0;
	nbo_localssrc = contribsrcs->GetLocalSSRC();
	while (!found && i < reportcount && len2 >= (int)sizeof(RTCPReportBlock))
	{
		rb = (RTCPReportBlock *)data2;
		if (rb->ssrc == nbo_localssrc) // both in network byte order
		{
			found = true;
			fraclost = rb->fractionlost;

			/* create the number of packets lost */
			pl = 0;
			pl |= (rb->packetslost[2]);
			pl |= ((rb->packetslost[1])<<8);
			pl |= ((rb->packetslost[0])<<16);
			if ((rb->packetslost[0])&128) // test the highest bit (to check if it's a negative number)
				pl |= ((0xFF)<<24);
			pl2 = (long *)&pl;
			packetslost = *pl2;

			exthighseqnum = ntohl(rb->exthsnr);
			jitter = ntohl(rb->jitter);
			lsr = ntohl(rb->lsr);
			dlsr = ntohl(rb->dlsr);

			status = sources->ProcessRRInfo(senderssrc,fraclost,packetslost,exthighseqnum,jitter,lsr,dlsr,curip,curport,localtsunit);
			if (status < 0)
				return status;
		}
		else
		{
			data2 += sizeof(RTCPReportBlock);
			len2 -= sizeof(RTCPReportBlock);
			i++;
		}
	}
	return 0;
}

void RTPPacketProcessor::CallAppDataHandler(unsigned char *data,int len,unsigned char subtype)
{
	RTPExceptionHandler handler;
	RTCPAPPPrefix *apppref;
	void *usrdata;
	int numwords;

	handler = handlers->handlers[RTP_EXCEPTION_RECEIVERTCPAPPDATA].handler;
	usrdata = handlers->handlers[RTP_EXCEPTION_RECEIVERTCPAPPDATA].usrdata;
	
	numwords = len/4;
	if (numwords < 2) // invalid packet
		return;
	apppref = (RTCPAPPPrefix *)data;
	
	ex_appdata.subtype = subtype;
	ex_appdata.src = ntohl(apppref->src);
	ex_appdata.name[0] = apppref->name[0];
	ex_appdata.name[1] = apppref->name[1];
	ex_appdata.name[2] = apppref->name[2];
	ex_appdata.name[3] = apppref->name[3];
	ex_appdata.data = data+sizeof(RTCPAPPPrefix);
	ex_appdata.numwords = numwords-2;
	ex_appdata.validdata = true;
	ex_appdata.prevreturnval = 0;
	handler(RTP_EXCEPTION_RECEIVERTCPAPPDATA,&ex_appdata,usrdata);
}

inline void RTPPacketProcessor::CheckRTCPHandler(unsigned char *data,int len,unsigned long ip, int port)
{
	if (handlers->handlers[RTP_EXCEPTION_RTCPPACKET].handler != NULL)
	{
		RTPExceptionHandler handler;
		void *usrdata;
		RTPExcepRTCPPacket packinfo;
		
		packinfo.packet = data;
		packinfo.packetlen = len;
		packinfo.sourceip = ip;
		packinfo.sourceport = port;
		
		handler = handlers->handlers[RTP_EXCEPTION_RTCPPACKET].handler;
		usrdata = handlers->handlers[RTP_EXCEPTION_RTCPPACKET].usrdata;
		handler(RTP_EXCEPTION_RTCPPACKET,&packinfo,usrdata);
	}
}
