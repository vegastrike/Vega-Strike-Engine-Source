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

#include "rtpconnection.h"
#include "rtpcontributingsources.h"
#include "rtptimeutil.h"
#include <stdlib.h>

#ifdef RTP_HAVE_SYS_FILIO
#include <sys/filio.h>
#endif // RTP_HAVE_SYS_FILIO

#ifdef RTP_VXWORKS
#include <resolvLib.h>
#include <ioLib.h>
#include <hostLib.h>
#else
#include <memory.h>
#endif // RTP_VXWORKS

#define RTP_IS_MCASTADDR(x)				(((x)&0xF0000000) == 0xE0000000)

#define RTP_MCASTMEMBERSHIP(socket,type,mcastip,status)	{\
								struct ip_mreq mreq;\
								\
								mreq.imr_multiaddr.s_addr = htonl(mcastip);\
								mreq.imr_interface.s_addr = htonl(localip);\
								status = setsockopt(socket,IPPROTO_IP,type,(const char *)&mreq,sizeof(struct ip_mreq));\
							}
							
struct RawDataBlock
{
	unsigned char *data;
	int datalen;
	unsigned long ip;
	int port;
	RawDataBlock *next;
};

RTPConnection::RTPConnection()
{
	rtpsock = 0;
	rtcpsock = 0;
	sendsock = 0;
	socketsopened = false;
	portbase = 0;
	receivemode = RECEIVEMODE_ALL;
	maxpacksize = RTP_DEFAULT_MAXPACKSIZE;
	rtp_first = NULL;
	rtp_last = NULL;
	rtcp_first = NULL;
	rtcp_last = NULL;
	sendport = 0;
	localip = 0;
	rtcprecvtime.tv_sec = 0;
	rtcprecvtime.tv_usec = 0;
}

RTPConnection::~RTPConnection()
{
	Destroy();
}

int RTPConnection::Create(int pbase,unsigned long localipaddress)
{
	int size;
	RTPSOCKLENTYPE socklen;

	if (socketsopened)
		return ERR_RTP_CONNALREADYCREATED;
		
	/* Check if the localportbase is even, conforming to RFC 1889 */
	
	if (pbase%2 != 0)
		return ERR_RTP_PORTBASENOTEVEN;
	
	/* Create the sockets */

	rtpsock = socket(AF_INET,SOCK_DGRAM,0);
	if (rtpsock == RTPSOCKERR)
		return ERR_RTP_CANTCREATESOCKET;
	rtcpsock = socket(AF_INET,SOCK_DGRAM,0);
	if (rtcpsock == RTPSOCKERR)
		return ERR_RTP_CANTCREATESOCKET;
	sendsock = socket(AF_INET,SOCK_DGRAM,0);
	if (sendsock == RTPSOCKERR)
		return ERR_RTP_CANTCREATESOCKET;

	/* Set socket receive and send buffers */

	size = RTP_RECEIVEBUFFERSIZE;
	if (setsockopt(rtpsock,SOL_SOCKET,SO_RCVBUF,(const char *)&size,sizeof(int)) != 0)
		return ERR_RTP_CANTSETSOCKETBUFFER;
	size = RTP_RECEIVEBUFFERSIZE;
	if (setsockopt(rtcpsock,SOL_SOCKET,SO_RCVBUF,(const char *)&size,sizeof(int)) != 0)
		return ERR_RTP_CANTSETSOCKETBUFFER;
	size = RTP_SENDBUFFERSIZE;
	if (setsockopt(sendsock,SOL_SOCKET,SO_SNDBUF,(const char *)&size,sizeof(int)) != 0)
		return ERR_RTP_CANTSETSOCKETBUFFER;

	/* Bind the sockets */
	
	struct sockaddr_in addr;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(0);
	addr.sin_addr.s_addr = htonl(0);
	if (bind(sendsock,(struct sockaddr *)&addr,sizeof(struct sockaddr)) != 0)
	{
		RTPCLOSESOCKET(rtpsock);
		RTPCLOSESOCKET(rtcpsock);
		RTPCLOSESOCKET(sendsock);
		return ERR_RTP_CANTBINDSOCKET;
	}
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(pbase);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(rtpsock,(struct sockaddr *)&addr,sizeof(struct sockaddr)) != 0)
	{
		RTPCLOSESOCKET(rtpsock);
		RTPCLOSESOCKET(rtcpsock);
		RTPCLOSESOCKET(sendsock);
		return ERR_RTP_CANTBINDSOCKET;
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(pbase+1);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(rtcpsock,(struct sockaddr *)&addr,sizeof(struct sockaddr)) != 0)
	{
		RTPCLOSESOCKET(rtpsock);
		RTPCLOSESOCKET(rtcpsock);
		RTPCLOSESOCKET(sendsock);
		return ERR_RTP_CANTBINDSOCKET;
	}

	/* Get the port number of the send socket */

	socklen = sizeof(struct sockaddr_in);
	if (getsockname(sendsock,(struct sockaddr *)&addr,&socklen) != 0)
	{
		RTPCLOSESOCKET(rtpsock);
		RTPCLOSESOCKET(rtcpsock);
		RTPCLOSESOCKET(sendsock);
		return ERR_RTP_CANTGETSOCKETPORT;
	}

	sendport = ntohs(addr.sin_port);

	/* Get the local IP address */

	if (localipaddress == 0) // user didn't supply an IP address
	{
		localip = CalcLocalIP();
		if (localip == 0)
		{
			RTPCLOSESOCKET(rtpsock);
			RTPCLOSESOCKET(rtcpsock);
			RTPCLOSESOCKET(sendsock);
			return ERR_RTP_CANTGETLOCALIP;
		}
	}
	else // user specified the local IP address
		localip = localipaddress;

	socketsopened = true;

#ifdef RTP_MULTICAST
	if (SetMulticastTTL(RTP_DEFAULTMULTICAST_TTL) < 0)
	{
		socketsopened = false;
		RTPCLOSESOCKET(rtpsock);
		RTPCLOSESOCKET(rtcpsock);
		RTPCLOSESOCKET(sendsock);
		return ERR_RTP_CANTSETMULTICASTTTL;
	}
#endif // RTP_MULTICAST
	
	portbase = pbase;
	return 0;
}

void RTPConnection::Destroy()
{
	if (socketsopened)
	{
		LeaveAllMulticastGroups();
		socketsopened = false;
		RTPCLOSESOCKET(rtpsock);
		RTPCLOSESOCKET(rtcpsock);
		RTPCLOSESOCKET(sendsock);
	}
	destinations.Clear();
	ignoreIPs.Clear();
	acceptIPs.Clear();
	mcasttable.Clear();
	receivemode = RECEIVEMODE_ALL;
	maxpacksize = RTP_DEFAULT_MAXPACKSIZE;
	FlushPackets();
}

int RTPConnection::GetPortBase()
{
	if (!socketsopened)
		return ERR_RTP_SOCKETSNOTOPEN;
	return portbase;
}

// version without header extension
int RTPConnection::SetToS(int tos)
{
        int status;

        if (!socketsopened)
                return ERR_RTP_SOCKETSNOTOPEN;

        status = setsockopt(sendsock,IPPROTO_IP,IP_TOS,(const char *)&tos,sizeof(int));
        if (status != 0)
                return ERR_RTP_CANTSETTOS;

        return 0;
}

int RTPConnection::SendRTPData(void *data,int len,RTPContributingSources *srcs,unsigned char pt,bool mark,RTPuint16 seqnr,RTPuint32 timestamp)
{
	RTPHeader *hdr;
	int blocklen,numcsrcs;
		
	if (!socketsopened)
		return ERR_RTP_SOCKETSNOTOPEN;

	if (len <= 0) // nothing to send
		return 0;

	numcsrcs = srcs->GetNumberOfCSRCs();
	if (numcsrcs > 15)
		numcsrcs = 15;

	blocklen = sizeof(RTPHeader)+sizeof(RTPuint32)*numcsrcs+len;
	if (blocklen > maxpacksize)
		return ERR_RTP_PACKETTOOLARGE;

	/* Init header fields */
		
	hdr = (RTPHeader *)packetbuffer;
	hdr->version = RTP_VERSION;
	hdr->padding = 0;
	hdr->extension = 0;
	hdr->cc = numcsrcs;
	hdr->marker = (mark)?1:0;
	hdr->payloadtype = pt;
	hdr->seqnum = htons(seqnr);
	hdr->timestamp = htonl(timestamp);
	hdr->ssrc = srcs->GetLocalSSRC();
	
	// copy CSRC members (CSRC numbers are already in network byte order)
	if (numcsrcs > 0)
		srcs->FillCSRCs(packetbuffer+sizeof(RTPHeader));
	// copy data
	memcpy((packetbuffer+sizeof(RTPHeader)+sizeof(RTPuint32)*numcsrcs),data,len);
	
	// send the data to all destinations
	struct sockaddr_in addr;	
	unsigned long ip;
	int port,rtcpport;

	destinations.InitRetrieval();
	addr.sin_family = AF_INET;
	while (destinations.GetNext(&ip,&port,&rtcpport))
	{
		addr.sin_port = port; // already in network byte order
		addr.sin_addr.s_addr = ip; // already in network byte order
		sendto(sendsock,(const char *)packetbuffer,blocklen,0,(struct sockaddr *)&addr,sizeof(struct sockaddr));
	}

	return 0;
}

// The version with header extension
int RTPConnection::SendRTPData(void *data,int len,RTPContributingSources *srcs,unsigned char pt,bool mark,RTPuint16 seqnr,RTPuint32 timestamp,unsigned short hdrextID,void *hdrextdata,int numhdrextwords)
{
	RTPHeader *hdr;
	RTPHeaderExtension *hdrext;
	int blocklen,numcsrcs,numextwords;
	unsigned char *datapos;
		
	if (!socketsopened)
		return ERR_RTP_SOCKETSNOTOPEN;

	if (len <= 0) // nothing to send
		return 0;

	numcsrcs = srcs->GetNumberOfCSRCs();
	if (numcsrcs > 15)
		numcsrcs = 15;
		
	numextwords = numhdrextwords;
	if (numextwords < 0)
		numextwords = 0;

	blocklen = sizeof(RTPHeader)+sizeof(RTPuint32)*numcsrcs+len+numextwords*sizeof(RTPuint32)+sizeof(RTPHeaderExtension);
	if (blocklen > maxpacksize)
		return ERR_RTP_PACKETTOOLARGE;

	/* Init header fields */
		
	hdr = (RTPHeader *)packetbuffer;
	hdr->version = RTP_VERSION;
	hdr->padding = 0;
	hdr->extension = 1;
	hdr->cc = numcsrcs;
	hdr->marker = (mark)?1:0;
	hdr->payloadtype = pt;
	hdr->seqnum = htons(seqnr);
	hdr->timestamp = htonl(timestamp);
	hdr->ssrc = srcs->GetLocalSSRC();
	
	// copy CSRC members (CSRC numbers are already in network byte order)
	if (numcsrcs > 0)
		srcs->FillCSRCs(packetbuffer+sizeof(RTPHeader));
	
	// set the header extension
	datapos = packetbuffer+sizeof(RTPHeader)+sizeof(RTPuint32)*numcsrcs;
	hdrext = (RTPHeaderExtension *)datapos;
	hdrext->userdefined = htons(hdrextID);
	hdrext->length = htons(numextwords);
	datapos += sizeof(RTPHeaderExtension);
	if (numextwords > 0)
		memcpy(datapos,hdrextdata,numextwords*sizeof(RTPuint32));
		
	// copy data
	datapos += numextwords*sizeof(RTPuint32);
	memcpy(datapos,data,len);
	
	// send the data to all destinations
	struct sockaddr_in addr;	
	unsigned long ip;
	int port,rtcpport;

	destinations.InitRetrieval();
	addr.sin_family = AF_INET;
	while (destinations.GetNext(&ip,&port,&rtcpport))
	{
		addr.sin_port = port; // already in network byte order
		addr.sin_addr.s_addr = ip; // already in network byte order
		sendto(sendsock,(const char *)packetbuffer,blocklen,0,(struct sockaddr *)&addr,sizeof(struct sockaddr));
	}

	return 0;
}

int RTPConnection::SendRTCPCompoundData(void *data,int len)
{
	struct sockaddr_in addr;	
	unsigned long ip;
	int port,rtcpport;

	if (!socketsopened)
		return ERR_RTP_SOCKETSNOTOPEN;

	if (len <= 0) // nothing to send
		return 0;

	if (len > maxpacksize)
		return ERR_RTP_PACKETTOOLARGE;
	destinations.InitRetrieval();
	addr.sin_family = AF_INET;
	while (destinations.GetNext(&ip,&port,&rtcpport))
	{
		addr.sin_port = rtcpport; // already in network byte order
		addr.sin_addr.s_addr = ip; // already in network byte order
		sendto(sendsock,(const char *)data,len,0,(struct sockaddr *)&addr,sizeof(struct sockaddr));
	}
	return 0;
}

int RTPConnection::ReceiveRTPData(unsigned char **data,int *len,unsigned long *ip,int *port)
{
	RawDataBlock *tmp;
	
	if (rtp_first == NULL)
		return ERR_RTP_NORTPDATAAVAILABLE;
	*data = rtp_first->data;
	*len = rtp_first->datalen;
	*ip = rtp_first->ip;
	*port = rtp_first->port;
	tmp = rtp_first;
	rtp_first = rtp_first->next;
	RTP_DELETE(tmp);
	return 0;
}

int RTPConnection::ReceiveRTCPData(unsigned char **data,int *len,unsigned long *ip,int *port)
{
	RawDataBlock *tmp;
	
	if (rtcp_first == NULL)
		return ERR_RTP_NORTCPDATAAVAILABLE;
	*data = rtcp_first->data;
	*len = rtcp_first->datalen;
	*ip = rtcp_first->ip;
	*port = rtcp_first->port;
	tmp = rtcp_first;
	rtcp_first = rtcp_first->next;
	RTP_DELETE(tmp);
	return 0;
}

int RTPConnection::Poll(bool rtp)
{
	unsigned long len,ip;
	RawDataBlock *tmpblk;
	bool acceptpacket;
	int port,sock;
	struct sockaddr_in addr;
	unsigned char *data2;
	int len2;
	RTPSOCKLENTYPE fromlen;

	if (!socketsopened)
		return ERR_RTP_SOCKETSNOTOPEN;

	if (rtp)
		sock = rtpsock;
	else
		sock = rtcpsock;

	len = 0;
	RTPIOCTL(sock,FIONREAD,&len);

        if (!rtp && len > 0)
        	gettimeofday(&rtcprecvtime,NULL);
	
	// I assume that we can read faster than the packets arrive
	
	// This version of the routine should also work with buggy ioctl versions:
	// in some ioctl versions, ioctl(sock,FIONREAD,&len) doesn't put the size of
	// the first packet in 'len', but the total size of all packets in the queue.
	
	while (len > 0)
	{
		fromlen = sizeof(struct sockaddr);
		len2 = recvfrom(sock,(char *)packetbuffer,(int)len,0,(struct sockaddr *)&addr,&fromlen);
		if (len2 > 0)
		{
			/* Check if this packet comes from a source from which we want to receive */

			ip = ntohl(addr.sin_addr.s_addr);
			port = ntohs(addr.sin_port);
			if (receivemode != RECEIVEMODE_ALL)
			{
				if (receivemode == RECEIVEMODE_IGNORESOME)
				{
					if (ignoreIPs.Exists(ip,true,0))
						acceptpacket = false;
					else if (ignoreIPs.Exists(ip,false,port))
						acceptpacket = false;
					else
						acceptpacket = true;
				}
				else // RECEIVEMODE_ACCEPTSOME
				{
					if (acceptIPs.Exists(ip,true,0))
						acceptpacket = true;
					else if (acceptIPs.Exists(ip,false,port))
						acceptpacket = true;
					else
						acceptpacket = false;
				}
			}
			else
				acceptpacket = true;

			if (acceptpacket)
			{
				/* Process packet */
				data2 = RTP_NEW(unsigned char[len2]);
				if (data2 == NULL)
					return ERR_RTP_OUTOFMEM;
				memcpy(data2,packetbuffer,len2);

				tmpblk = RTP_NEW(RawDataBlock);
				if (tmpblk == NULL)
				{
					RTP_DELETE_ARRAY(data2);
					return ERR_RTP_OUTOFMEM;
				}

				if (rtp)
				{
					if (rtp_first == NULL)
						rtp_first = tmpblk;
					else
						rtp_last->next = tmpblk;
					rtp_last = tmpblk;
				}
				else	
				{
					if (rtcp_first == NULL)
						rtcp_first = tmpblk;
					else
						rtcp_last->next = tmpblk;
					rtcp_last = tmpblk;
				}
				tmpblk->next = NULL;
				tmpblk->data = data2;
				tmpblk->datalen = len2;
				tmpblk->ip = ip;
				tmpblk->port = port;
			}
		}
		len = 0;
		RTPIOCTL(sock,FIONREAD,&len);
	}

	return 0;
}

void RTPConnection::FlushPackets()
{
	while (rtp_first != NULL)
	{
		rtp_last = rtp_first->next;
		if (rtp_first->data != NULL)
			RTP_DELETE_ARRAY(rtp_first->data);
		RTP_DELETE(rtp_first);
		rtp_first = rtp_last;
	}
	while (rtcp_first != NULL)
	{
		rtcp_last = rtcp_first->next;
		if (rtcp_first->data != NULL)
			RTP_DELETE_ARRAY(rtcp_first->data);
		RTP_DELETE(rtcp_first);
		rtcp_first = rtcp_last;
	}
}

#ifdef RTP_VXWORKS

unsigned long RTPConnection::CalcLocalIP()
{
        unsigned long ip;
        char name[256];

        gethostname(name,255);
        ip = (unsigned long)hostGetByName(name);
        ip = ntohl(ip);
}

#else // other systems

unsigned long RTPConnection::CalcLocalIP()
{
	struct hostent *he;
	char name[256];
	unsigned long ip;
	bool done;
	int i,j;

	gethostname(name,255);
	name[255] = 0;
	he = gethostbyname(name);
	if (he == NULL)
		return 0;
	
	ip = 0;
	i = 0;
	done = false;
	while (!done)
	{
		if (he->h_addr_list[i] == NULL)
			done = true;
		else
		{
			ip = 0;
			for (j = 0 ; j < 4 ; j++)
				ip |= ((unsigned long)((unsigned char)he->h_addr_list[i][j])<<((3-j)*8));
			
			if (he->h_addr_list[i][0] != 127 && he->h_addr_list[i][0] != 0)
				done = true;
			else
				i++;
		}
	}

	return ip;
}

#endif // RTP_VXWORKS

#ifndef RTP_MULTICAST
/*****************************************************************************

		ROUTINES FOR USE WITHOUT MULTICASTING SUPPORT

*****************************************************************************/

bool RTPConnection::SupportsMulticasting()
{
	return false;
}

int RTPConnection::JoinMulticastGroup(unsigned long mcastIP)
{
	return ERR_RTP_MULTICASTINGNOTSUPPORTED;
}

int RTPConnection::LeaveMulticastGroup(unsigned long mcastIP)
{
	return ERR_RTP_MULTICASTINGNOTSUPPORTED;
}

void RTPConnection::LeaveAllMulticastGroups()
{
}

int RTPConnection::SetMulticastTTL(unsigned char ttl)
{
	return ERR_RTP_MULTICASTINGNOTSUPPORTED;
}

#else
/*****************************************************************************

	    ROUTINES FOR USE WITH MULTICASTING SUPPORT ENABLED

*****************************************************************************/

bool RTPConnection::SupportsMulticasting()
{
	return true;
}

int RTPConnection::JoinMulticastGroup(unsigned long mcastIP)
{
	int status;
	
	if (!socketsopened)
		return ERR_RTP_SOCKETSNOTOPEN;
	
	if (!RTP_IS_MCASTADDR(mcastIP))
		return ERR_RTP_NOTAMULTICASTADDRESS;
		
	status = mcasttable.AddMCastAddress(mcastIP);
	if (status < 0)
		return status;
	
	RTP_MCASTMEMBERSHIP(rtpsock,IP_ADD_MEMBERSHIP,mcastIP,status);
	if (status != 0)
	{
		mcasttable.DeleteMCastAddress(mcastIP);
		return ERR_RTP_MULTICASTJOINFAILED;
	}
	
	RTP_MCASTMEMBERSHIP(rtcpsock,IP_ADD_MEMBERSHIP,mcastIP,status);
	if (status != 0)
	{
		RTP_MCASTMEMBERSHIP(rtpsock,IP_DROP_MEMBERSHIP,mcastIP,status);
		mcasttable.DeleteMCastAddress(mcastIP);
		return ERR_RTP_MULTICASTJOINFAILED;
	}
	
	return 0;
}

int RTPConnection::LeaveMulticastGroup(unsigned long mcastIP)
{
	int status;
	
	if (!socketsopened)
		return ERR_RTP_SOCKETSNOTOPEN;
	
	if (!RTP_IS_MCASTADDR(mcastIP))
		return ERR_RTP_NOTAMULTICASTADDRESS;
		
	status = mcasttable.DeleteMCastAddress(mcastIP);
	if (status < 0)
		return status;
	
	RTP_MCASTMEMBERSHIP(rtpsock,IP_DROP_MEMBERSHIP,mcastIP,status);
	RTP_MCASTMEMBERSHIP(rtcpsock,IP_DROP_MEMBERSHIP,mcastIP,status);
	
	return 0;
}

void RTPConnection::LeaveAllMulticastGroups()
{
	unsigned long mcastip;
	int status;
	
	if (!socketsopened)
		return;
	
	mcasttable.GotoFirstMCastAddress();
	while (mcasttable.GetNextMCastAddress(&mcastip))
	{
		RTP_MCASTMEMBERSHIP(rtpsock,IP_DROP_MEMBERSHIP,mcastip,status);
		RTP_MCASTMEMBERSHIP(rtcpsock,IP_DROP_MEMBERSHIP,mcastip,status);
	}
	mcasttable.Clear();
}

int RTPConnection::SetMulticastTTL(unsigned char ttl)
{
	int status;
	
	if (!socketsopened)
		return ERR_RTP_SOCKETSNOTOPEN;
	
#ifdef _AIX
	status = setsockopt(sendsock,IPPROTO_IP,IP_MULTICAST_TTL,&ttl,sizeof(unsigned char));
#else
	int ttl2;
	
	ttl2 = (int)ttl;
	status = setsockopt(sendsock,IPPROTO_IP,IP_MULTICAST_TTL,(const char *)&ttl2,sizeof(int));
#endif // _AIX

	if (status != 0)
		return ERR_RTP_CANTSETMULTICASTTTL;
	
	return 0;
}

#endif // RTP_MULTICAST
