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

#include "rtpsources.h"
#include "rtpsourcedata.h"
#include "rtppacket.h"
#include "rtpcontributingsources.h"
#include "rtptimeutil.h"
#include "rtphandlers.h"
#include "rtpconnection.h"
#include <string.h>

RTPSources::RTPSources()
{
	int i;

	for (i = 0 ; i < RTP_SOURCETABLE_HASHSIZE ; i++)
		sourcetable[i] = NULL;
	numsources = 0;
	numsenders = 0;
	handlers = NULL;
	contribsources = NULL;
	initialized = false;
}

RTPSources::~RTPSources()
{
	Clear();
}


void RTPSources::Clear()
{
	int i;
	RTPSourceData *tmp,*tmp2;

	for (i = 0 ; i < RTP_SOURCETABLE_HASHSIZE ; i++)
	{
		tmp = sourcetable[i];
		while (tmp != NULL)
		{
			tmp2 = tmp->next;
			RTP_DELETE(tmp);
			tmp = tmp2;
		}
		sourcetable[i] = NULL;
	}
}

RTPSourceData *RTPSources::Retrieve(unsigned long src)
{
	int index;
	bool found;
	RTPSourceData *tmp;

	index = (int)(src%(unsigned long)RTP_SOURCETABLE_HASHSIZE);
	if ((tmp = sourcetable[index]) == NULL)
		return NULL;

	found = false;
	while (!found && tmp != NULL)
	{
		if (tmp->ssrc < src)
			tmp = tmp->next;
		else
			found = true;
	}
	if (!found)
		return NULL;
	if (tmp->ssrc != src)
		return NULL;
	return tmp;
}

int RTPSources::ProcessPacket(RTPPacket *packet,unsigned long ip,int port,double localtsunit)
{
	RTPSourceData *tmp;
	bool created,collis;
	int status;

	if (!initialized)
		return ERR_RTP_SOURCESNOTINITALIZED;

	cursource = NULL;
	curtablepos = RTP_SOURCETABLE_HASHSIZE;
	
	tmp = RetrieveOrCreate(packet->syncsource,localtsunit,&created);
	if (tmp == NULL)
		return ERR_RTP_OUTOFMEM;
	if (created)
	{
		tmp->ip = ip;
		tmp->rtpport = port;
		if (contribsources->DoesCSRCExist(tmp->ssrc))
			tmp->isaCSRC = true;

		if (handlers->handlers[RTP_EXCEPTION_NEWSOURCE].handler != NULL)
			CallNewSourceHandler(tmp->ssrc);
	}
	else // src entry already existed
	{
		collis	= false;
		if (tmp->ip != ip)
			collis = true;
		else
		{
			if (tmp->rtpport < 0) // first rtp packet from source
				tmp->rtpport = port;
			else if (tmp->rtpport != port)
				collis = true;
		}

		if (collis)
		{
			// ssrc collision, ignore this packet
			if (handlers->handlers[RTP_EXCEPTION_SSRCCOLLISION].handler != NULL)
				CallSSRCCollisionHandler(tmp->ssrc,ip,true,port);
			return ERR_RTP_COLLISIONBETWEENSSRCS;
		}
	}
	
	status = tmp->AddPacket(packet);
	if (status < 0)
		return status;

	if (!tmp->hassentnewdata)
	{
		tmp->hassentnewdata = true;
		if (!tmp->isaCSRC)
			numsenders++;
	}
	return 0;
}

int RTPSources::ProcessSRInfo(RTPuint32 src,RTPuint32 ntplsw,RTPuint32 ntpmsw,RTPuint32 rtptimestamp,RTPuint32 packetcount,RTPuint32 bytecount,unsigned long ip,int port,double localtsunit)
{
	RTPSourceData *tmp;
	bool created,collis;

	if (!initialized)
		return ERR_RTP_SOURCESNOTINITALIZED;
	
	cursource = NULL;
	curtablepos = RTP_SOURCETABLE_HASHSIZE;

	tmp = RetrieveOrCreate(src,localtsunit,&created);
	if (tmp == NULL)
		return ERR_RTP_OUTOFMEM;
	if (created)
	{
		tmp->ip = ip;
		tmp->rtcpport = port;
		if (contribsources->DoesCSRCExist(tmp->ssrc))
			tmp->isaCSRC = true;

		if (handlers->handlers[RTP_EXCEPTION_NEWSOURCE].handler != NULL)
			CallNewSourceHandler(tmp->ssrc);
	}
	else // src entry already existed
	{
		collis = false;
		if (tmp->ip != ip)
			collis = true;
		else
		{
			if (tmp->rtcpport < 0)	// first rtcppacket from source
				tmp->rtcpport = port;
			else  if (tmp->rtcpport != port)
				collis = true;
		}

		if (collis)
		{
			// ssrc collision, ignore this packet
			if (handlers->handlers[RTP_EXCEPTION_SSRCCOLLISION].handler != NULL)
				CallSSRCCollisionHandler(tmp->ssrc,ip,false,port);
			return 0;
		}
	}

	tmp->sr.bytecount = bytecount;
	tmp->sr.ntplsw = ntplsw;
	tmp->sr.ntpmsw = ntpmsw;
	tmp->sr.packetcount = packetcount;
	tmp->sr.rtptimestamp = rtptimestamp;
	tmp->sr.srreceived = true;
	tmp->sr.srtime = rtpconn->GetRTCPReceiveTime();
	tmp->stats.lastmsgtime = tmp->sr.srtime.tv_sec;

	return 0;
}

int RTPSources::ProcessSDESInfo(RTPuint32 src,int sdestype,unsigned char *sdesdata,int len,unsigned long ip,int port,double localtsunit)
{
	RTPSourceData *tmp;
	bool created,matching,collis;
	int status,index,oldlen;

	if (!initialized)
		return ERR_RTP_SOURCESNOTINITALIZED;
	
	cursource = NULL;
	curtablepos = RTP_SOURCETABLE_HASHSIZE;

	tmp = RetrieveOrCreate(src,localtsunit,&created);
	if (tmp == NULL)
		return ERR_RTP_OUTOFMEM;
	if (created)
	{
		tmp->ip = ip;
		tmp->rtcpport = port;
		if (contribsources->DoesCSRCExist(tmp->ssrc))
			tmp->isaCSRC = true;

		if (handlers->handlers[RTP_EXCEPTION_NEWSOURCE].handler != NULL)
			CallNewSourceHandler(tmp->ssrc);
	}
	else // src entry already existed
	{
		collis = false;
		if (tmp->ip != ip)
			collis = true;
		else
		{
			if (tmp->rtcpport < 0)	// first rtcppacket from source
				tmp->rtcpport = port;
			else  if (tmp->rtcpport != port)
				collis = true;
		}

		if (collis)
		{
			// ssrc collision, ignore this packet
			if (handlers->handlers[RTP_EXCEPTION_SSRCCOLLISION].handler != NULL)
				CallSSRCCollisionHandler(tmp->ssrc,ip,false,port);
			return 0;
		}
	}

	index = sdestype-1;
	if (index >= 0 && index < RTP_NUM_SDES_INDICES)
	{
		if (index == TYPE_SDES_CNAME-1 && tmp->sdes.cnameset)
		{
			// already a cname set, check if the old one and the new one are the same
			matching = false;
			oldlen = tmp->sdes.sdesinfolen[index];
			if (len == oldlen)
			{
				if (oldlen == 0)
					matching = true;
				else
				{
					if (memcmp(tmp->sdes.sdesinfo[index],sdesdata,len) == 0)
						matching = true;
				}
			}

			if (!matching) // this means that there is a ssrc collision
			{
				if (handlers->handlers[RTP_EXCEPTION_SSRCCOLLISION].handler != NULL)
					CallSSRCCollisionHandler(tmp->ssrc,ip,false,port);
				return 0;
			}
		}
		else
		{
			status = tmp->sdes.SetSDES(index,(char *)sdesdata,len);
			if (status < 0)
				return status;
		}
	}
	else // invalid sdestype
	{
		if (handlers->handlers[RTP_EXCEPTION_INVALIDSDESTYPE].handler != NULL)
			CallInvalidSDESTypeHandler(tmp->ssrc,sdestype,sdesdata,len);
	}
	tmp->stats.lastmsgtime = time(NULL);
	return 0;
}

int RTPSources::ProcessBYEMessage(RTPuint32 src,unsigned long ip,int port)
{
	int index;
	bool found,collis;
	RTPSourceData *tmp,*tmpprev;

	cursource = NULL;
	curtablepos = RTP_SOURCETABLE_HASHSIZE;
	tmpprev = NULL;

	index = (int)(src%(unsigned long)RTP_SOURCETABLE_HASHSIZE);
	if ((tmp = sourcetable[index]) == NULL) // not found
		found = false;
	else
	{
		found = false;
		while (!found && tmp != NULL)
		{
			if (tmp->ssrc < src)
			{
				tmpprev = tmp;
				tmp = tmp->next;
			}
			else
				found = true;
		}
		if (found)
		{
			if (tmp->ssrc != src)
				found = false;
		}
	}
	
	if (found)
	{
		collis = false;
		if (tmp->ip != ip)
			collis = true;
		else if (tmp->rtcpport >= 0 && tmp->rtcpport != port)
			collis = true;

		if (!collis)
		{
			if (handlers->handlers[RTP_EXCEPTION_SSRCDEPARTURE].handler != NULL)
				CallSSRCDepartureHandler(tmp->ssrc);

			if (tmpprev == NULL)
				sourcetable[index] = tmp->next;
			else
				tmpprev->next = tmp->next;
			if (tmp->hassentnewdata && !tmp->isaCSRC)
				numsenders--;
			numsources--;

			RTP_DELETE(tmp);
		}
	}
	return 0;
}

int RTPSources::ProcessRRInfo(RTPuint32 src,unsigned char fraclost,long packetslost,RTPuint32 exthighseqnum,RTPuint32 jitter,RTPuint32 lsr,RTPuint32 dlsr,unsigned long ip,int port,double localtsunit)
{
	RTPSourceData *tmp;
	bool created,collis;
	unsigned long msw,lsw,ntp32;
	unsigned long delay;
		
	if (!initialized)
		return ERR_RTP_SOURCESNOTINITALIZED;
	
	cursource = NULL;
	curtablepos = RTP_SOURCETABLE_HASHSIZE;

	tmp = RetrieveOrCreate(src,localtsunit,&created);
	if (tmp == NULL)
		return ERR_RTP_OUTOFMEM;
	if (created)
	{
		tmp->ip = ip;
		tmp->rtcpport = port;
		if (contribsources->DoesCSRCExist(tmp->ssrc))
			tmp->isaCSRC = true;

		if (handlers->handlers[RTP_EXCEPTION_NEWSOURCE].handler != NULL)
			CallNewSourceHandler(tmp->ssrc);
	}
	else // src entry already existed
	{
		collis = false;
		if (tmp->ip != ip)
			collis = true;
		else
		{
			if (tmp->rtcpport < 0)	// first rtcppacket from source
				tmp->rtcpport = port;
			else  if (tmp->rtcpport != port)
				collis = true;
		}

		if (collis)
		{
			// ssrc collision, ignore this packet
			if (handlers->handlers[RTP_EXCEPTION_SSRCCOLLISION].handler != NULL)
				CallSSRCCollisionHandler(tmp->ssrc,ip,false,port);
			return 0;
		}
	}
	tmp->rr.dlsr = dlsr;
	tmp->rr.exthighseqnum = exthighseqnum;
	tmp->rr.fractionlost = fraclost;
	tmp->rr.jitter = jitter;
	tmp->rr.lsr = lsr;
	tmp->rr.packetslost = packetslost;
	tmp->rr.rrreceived = true;
	tmp->rr.rrtime = rtpconn->GetRTCPReceiveTime();
	tmp->stats.lastmsgtime = tmp->rr.rrtime.tv_sec;

	/* Calculate round trip time if possible */

	if (lsr == 0 && dlsr == 0)
		return 0;
	
	getntptime(&(tmp->rr.rrtime),&lsw,&msw);
	lsw = ntohl(lsw);
	msw = ntohl(msw);
	ntp32 = ((msw&0xFFFF)<<16)|((lsw>>16)&0xFFFF);
		
	delay = ntp32 - tmp->rr.lsr - tmp->rr.dlsr;
	if (delay & (1<<31)) // if the high bit is set, the number cycled
		delay = 0;
	tmp->stats.rtt.tv_sec = (int)(((double)delay)/65536.0);
	tmp->stats.rtt.tv_usec = (int)(((double)(delay&0xFFFF))/(65536.0)*1000000.0);
	
	return 0;
}

void RTPSources::CSRCAdded(RTPuint32 csrc)
{
	RTPSourceData *tmp;

	tmp = Retrieve(csrc);
	if (tmp != NULL)
	{
		if (!tmp->isaCSRC)
		{
			tmp->isaCSRC = true;
			if (tmp->hassentnewdata)
				numsenders--; // numsenders is to count the number of RR packets
							  // that this source should transmit. According to
							  // rfc1889, RRs do not have to be sent to CSRCs
		}
	}
}

void RTPSources::CSRCDeleted(RTPuint32 csrc)
{
	RTPSourceData *tmp;

	tmp = Retrieve(csrc);
	if (tmp != NULL)
	{
		if (tmp->isaCSRC)
		{
			tmp->isaCSRC = false;
			if (tmp->hassentnewdata)
				numsenders++; // see remark above
		}
	}
}

void RTPSources::UpdateAllSources()
{
	int i;
	RTPSourceData *tmp,*tmpprev,*tmpnext;
	unsigned long curtime;

	cursource = NULL;
	curtablepos = RTP_SOURCETABLE_HASHSIZE;

	curtime = time(NULL);
	for (i = 0 ; i < RTP_SOURCETABLE_HASHSIZE ; i++)
	{
		tmpprev = NULL;
		tmp = sourcetable[i];
		while (tmp != NULL)
		{
			if ((curtime - tmp->stats.lastmsgtime) >= RTP_TIMEOUTSEC) // timeout
			{
				if (handlers->handlers[RTP_EXCEPTION_SSRCTIMEOUT].handler != NULL)
					CallSSRCTimeoutHandler(tmp->ssrc);
		
				tmpnext = tmp->next;
				if (tmpprev == NULL)
					sourcetable[i] = tmpnext;
				else
					tmpprev->next = tmpnext;
				numsources--;
				RTP_DELETE(tmp);
				tmp = tmpnext;
			}
			else // no timeout
			{
				tmp->hassentnewdata = false;
				tmp->stats.prevmaxseq = tmp->stats.maxseq;
				tmp->stats.numnewpackets = 0;
				
				tmpprev = tmp;
				tmp = tmp->next;
			}
		}
	}
	numsenders = 0;
}

RTPSourceData *RTPSources::RetrieveOrCreate(unsigned long src,double localtsunit,bool *created)
{
	int index;
	bool done;
	RTPSourceData *tmp,*prevtmp,*nexttmp;

	*created = false;
	index = (int)(src%(unsigned long)RTP_SOURCETABLE_HASHSIZE);
	if ((tmp = sourcetable[index]) == NULL)
	{
		tmp = RTP_NEW(RTPSourceData(src,localtsunit));
		if (tmp == NULL)
			return NULL;
		*created = true;
		sourcetable[index] = tmp;
		numsources++;
		return tmp;
	}

	prevtmp = NULL;
	done = false;
	while (!done && tmp != NULL)
	{
		if (tmp->ssrc < src)
		{
			prevtmp = tmp;
			tmp = tmp->next;
		}
		else
			done = true;
	}

	if (tmp == NULL)
	{
		tmp = RTP_NEW(RTPSourceData(src,localtsunit));
		if (tmp == NULL)
			return NULL;
		*created = true;
		prevtmp->next = tmp;
		numsources++;
	}
	else // tmp found
	{
		if (tmp->ssrc != src)
		{
			if (prevtmp == NULL)
			{
				tmp = RTP_NEW(RTPSourceData(src,localtsunit));
				if (tmp == NULL)
					return NULL;
				*created = true;
				tmp->next = sourcetable[index];
				sourcetable[index] = tmp;
				numsources++;
			}
			else
			{
				nexttmp = tmp;
				tmp = RTP_NEW(RTPSourceData(src,localtsunit));
				if (tmp == NULL)
					return NULL;
				*created = true;
				tmp->next = nexttmp;
				prevtmp->next = tmp;
				numsources++;
			}
		}
	}

	return tmp;
}

bool RTPSources::GotoFirstSender()
{
	bool found;

	curtablepos = 0;
	found = false;
	while (!found && curtablepos < RTP_SOURCETABLE_HASHSIZE)
	{
		cursource = sourcetable[curtablepos];
		while (!found && cursource != NULL)
		{
			if (!cursource->isaCSRC && cursource->hassentnewdata)
				found = true;
			else
				cursource = cursource->next;
		}
		
		if (!found)
			curtablepos++;
	}
	if (!found)
		return false;
	return true;
}	

bool RTPSources::GotoNextSender()
{
	bool found;

	found = false;
	if (cursource != NULL)
		cursource = cursource->next;
	while (!found && curtablepos < RTP_SOURCETABLE_HASHSIZE)
	{
		while (!found && cursource != NULL)
		{
			if (!cursource->isaCSRC && cursource->hassentnewdata)
				found = true;
			else
				cursource = cursource->next;
		}
		
		if (!found)
		{
			curtablepos++;
			if (curtablepos < RTP_SOURCETABLE_HASHSIZE)
				cursource = sourcetable[curtablepos];
		}
	}
	if (!found)
		return false;
	return true;
}

bool RTPSources::GotoFirstSource()
{
	bool found;

	curtablepos = 0;
	found = false;
	while (!found && curtablepos < RTP_SOURCETABLE_HASHSIZE)
	{
		cursource = sourcetable[curtablepos];
		if (cursource != NULL)
			found = true;
		else
			curtablepos++;
	}
	if (!found)
		return false;
	return true;
}

bool RTPSources::GotoNextSource()
{
	bool found;

	found = false;
	if (cursource != NULL)
		cursource = cursource->next;
	while (!found && curtablepos < RTP_SOURCETABLE_HASHSIZE)
	{
		if (cursource != NULL)
			found = true;
		else
		{
			curtablepos++;
			if (curtablepos < RTP_SOURCETABLE_HASHSIZE)
				cursource = sourcetable[curtablepos];
		}
	}
	if (!found)
		return false;
	return true;
}

bool RTPSources::GotoFirstSourceWithData()
{
	bool found;

	curtablepos = 0;
	found = false;
	while (!found && curtablepos < RTP_SOURCETABLE_HASHSIZE)
	{
		cursource = sourcetable[curtablepos];
		while (!found && cursource != NULL)
		{
			if (cursource->firstpacket != NULL)
				found = true;
			else
				cursource = cursource->next;
		}
		
		if (!found)
			curtablepos++;
	}
	if (!found)
		return false;
	return true;
}	

bool RTPSources::GotoNextSourceWithData()
{
	bool found;

	found = false;
	if (cursource != NULL)
		cursource = cursource->next;
	while (!found && curtablepos < RTP_SOURCETABLE_HASHSIZE)
	{
		while (!found && cursource != NULL)
		{
			if (cursource->firstpacket != NULL)
				found = true;
			else
				cursource = cursource->next;
		}
		
		if (!found)
		{
			curtablepos++;
			if (curtablepos < RTP_SOURCETABLE_HASHSIZE)
				cursource = sourcetable[curtablepos];
		}
	}
	if (!found)
		return false;
	return true;
}

void RTPSources::CallNewSourceHandler(RTPuint32 ssrc)
{
	RTPExceptionHandler handler;
	void *usrdata;

	handler = handlers->handlers[RTP_EXCEPTION_NEWSOURCE].handler;
	usrdata = handlers->handlers[RTP_EXCEPTION_NEWSOURCE].usrdata;
	ex_ssrc.ssrc = ssrc;
	handler(RTP_EXCEPTION_NEWSOURCE,&ex_ssrc,usrdata);
}

void RTPSources::CallSSRCCollisionHandler(RTPuint32 ssrc,unsigned long ip,bool rtpdata,int port)
{
	RTPExceptionHandler handler;
	void *usrdata;

	handler = handlers->handlers[RTP_EXCEPTION_SSRCCOLLISION].handler;
	usrdata = handlers->handlers[RTP_EXCEPTION_SSRCCOLLISION].usrdata;
	ex_ssrccol.ip = ip;
	ex_ssrccol.port = port;
	ex_ssrccol.rtpdata = rtpdata;
	ex_ssrccol.ssrc = ssrc;
	handler(RTP_EXCEPTION_SSRCCOLLISION,&ex_ssrccol,usrdata);
}

void RTPSources::CallInvalidSDESTypeHandler(RTPuint32 ssrc,int type,unsigned char *data,int datalen)
{
	RTPExceptionHandler handler;
	void *usrdata;

	handler = handlers->handlers[RTP_EXCEPTION_INVALIDSDESTYPE].handler;
	usrdata = handlers->handlers[RTP_EXCEPTION_INVALIDSDESTYPE].usrdata;
	ex_invalsdes.datalen = datalen;
	ex_invalsdes.sdesdata = data;
	ex_invalsdes.sdestype = type;
	ex_invalsdes.ssrc = ssrc;
	handler(RTP_EXCEPTION_INVALIDSDESTYPE,&ex_invalsdes,usrdata);
}

void RTPSources::CallSSRCDepartureHandler(RTPuint32 ssrc)
{
	RTPExceptionHandler handler;
	void *usrdata;

	handler = handlers->handlers[RTP_EXCEPTION_SSRCDEPARTURE].handler;
	usrdata = handlers->handlers[RTP_EXCEPTION_SSRCDEPARTURE].usrdata;
	ex_ssrc.ssrc = ssrc;
	handler(RTP_EXCEPTION_SSRCDEPARTURE,&ex_ssrc,usrdata);
}

void RTPSources::CallSSRCTimeoutHandler(RTPuint32 ssrc)
{
	RTPExceptionHandler handler;
	void *usrdata;

	handler = handlers->handlers[RTP_EXCEPTION_SSRCTIMEOUT].handler;
	usrdata = handlers->handlers[RTP_EXCEPTION_SSRCTIMEOUT].usrdata;
	ex_ssrc.ssrc = ssrc;
	handler(RTP_EXCEPTION_SSRCTIMEOUT,&ex_ssrc,usrdata);
}

