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

#include "rtpsession.h"
#include "rtpstructs.h"

RTPSession::RTPSession()
{
	initialized = false;
	rtpconn.SetMaximumPacketSize(localinf.maxpacksize);
	contribsources.SetSources(&sources);
	packetprocessor.SetContributingSources(&contribsources);
	packetprocessor.SetHandlers(&handlers);
	packetprocessor.SetSources(&sources);
	packetprocessor.SetConnection(&rtpconn);
	sources.SetContributingSources(&contribsources);
	sources.SetHandlers(&handlers);
	sources.SetConnection(&rtpconn);
	rtcpmodule.SetConnection(&rtpconn);
	rtcpmodule.SetContributingSources(&contribsources);
	rtcpmodule.SetLocalInfo(&localinf);
	rtcpmodule.SetSources(&sources);
	rtcpmodule.SetHandlers(&handlers);
	acceptownpackets = false;
}

RTPSession::~RTPSession()
{
	Destroy();
}

int RTPSession::Create(int localportbase,unsigned long localip)
{	
	int status;

	if (initialized)
		return ERR_RTP_SESSIONALREADYINITIALIZED;
	if ((status = rtpconn.Create(localportbase,localip)) < 0)
		return status;
	if ((status = contribsources.CreateLocalCNAME()) < 0)
	{
		rtpconn.Destroy();
		return status;
	}
	CreateNewSSRC();
	localinf.CreateSessionParameters();
	rtcpmodule.Initialize();
	initialized = true;
	return 0;
}

int RTPSession::Destroy()
{
	if (!initialized)
		return ERR_RTP_SESSIONNOTCREATED;
	initialized = false;
	rtcpmodule.SendBYE();
	rtpconn.Destroy();
	contribsources.Clear();
	localinf.Clear();
	sources.Clear();
	handlers.Clear();
	return 0;
}

int RTPSession::SendPacket(void *data,int len)
{
	int status;
	unsigned long curtime;

	if (!initialized)
		return ERR_RTP_SESSIONNOTCREATED;
	if (!localinf.defvaluesset)
		return ERR_RTP_NODEFAULTVALUESSET;
	if (len > 0)
	{
		status = rtpconn.SendRTPData(data,len,&contribsources,localinf.defpayloadtype,localinf.defmark,localinf.seqnum,localinf.timestamp);
		if (status < 0)
			return status;
		localinf.timestamp += localinf.deftsinc;
		localinf.seqnum++;
		localinf.octetcount += (RTPuint32)len;
		localinf.packetcount++;
		rtcpmodule.SentData();
	}

	/* Check if we have to send RTCP packets */

	curtime = time(NULL);
	if (rtcpmodule.RTCPCheck(curtime))
	{
		status = rtcpmodule.RTCPRoutine(curtime);
		if (status < 0)
			return status;
	}

	return 0;
}

int RTPSession::SendPacket(void *data,int len,unsigned char pt,bool mark,unsigned long timestampinc)
{
	int status;
	unsigned long curtime;

	if (!initialized)
		return ERR_RTP_SESSIONNOTCREATED;
	if (len > 0)
	{
		status = rtpconn.SendRTPData(data,len,&contribsources,pt,mark,localinf.seqnum,localinf.timestamp);
		if (status < 0)
			return status;
		localinf.timestamp += timestampinc;
		localinf.seqnum++;
		localinf.octetcount += (RTPuint32)len;
		localinf.packetcount++;
		rtcpmodule.SentData();
	}

	/* Check if we have to send RTCP packets */

	curtime = time(NULL);
	if (rtcpmodule.RTCPCheck(curtime))
	{
		status = rtcpmodule.RTCPRoutine(curtime);
		if (status < 0)
			return status;
	}

	return 0;
}

int RTPSession::SendPacket(void *data,int len,unsigned short hdrextID,void *hdrextdata,int numhdrextwords)
{
	int status;
	unsigned long curtime;

	if (!initialized)
		return ERR_RTP_SESSIONNOTCREATED;
	if (!localinf.defvaluesset)
		return ERR_RTP_NODEFAULTVALUESSET;
	if (len > 0)
	{
		status = rtpconn.SendRTPData(data,len,&contribsources,localinf.defpayloadtype,localinf.defmark,localinf.seqnum,localinf.timestamp,hdrextID,hdrextdata,numhdrextwords);
		if (status < 0)
			return status;
		localinf.timestamp += localinf.deftsinc;
		localinf.seqnum++;
		localinf.octetcount += (RTPuint32)len;
		localinf.packetcount++;
		rtcpmodule.SentData();
	}

	/* Check if we have to send RTCP packets */

	curtime = time(NULL);
	if (rtcpmodule.RTCPCheck(curtime))
	{
		status = rtcpmodule.RTCPRoutine(curtime);
		if (status < 0)
			return status;
	}

	return 0;
}

int RTPSession::SendPacket(void *data,int len,unsigned char pt,bool mark,unsigned long timestampinc,unsigned short hdrextID,void *hdrextdata,int numhdrextwords)
{
	int status;
	unsigned long curtime;

	if (!initialized)
		return ERR_RTP_SESSIONNOTCREATED;
	if (len > 0)
	{
		status = rtpconn.SendRTPData(data,len,&contribsources,pt,mark,localinf.seqnum,localinf.timestamp,hdrextID,hdrextdata,numhdrextwords);
		if (status < 0)
			return status;
		localinf.timestamp += timestampinc;
		localinf.seqnum++;
		localinf.octetcount += (RTPuint32)len;
		localinf.packetcount++;
		rtcpmodule.SentData();
	}

	/* Check if we have to send RTCP packets */

	curtime = time(NULL);
	if (rtcpmodule.RTCPCheck(curtime))
	{
		status = rtcpmodule.RTCPRoutine(curtime);
		if (status < 0)
			return status;
	}

	return 0;
}

int RTPSession::PollData()
{
	int status,len,port,prevnumsources;
	unsigned char *data;
	unsigned long ip,curtime;
	bool collis;

	if (!initialized)
		return ERR_RTP_SESSIONNOTCREATED;

	/* Save the number of sources for comparison */

	prevnumsources = sources.GetNumberOfParticipants();
	
	/* Process data on RTP port */
	
	if ((status = rtpconn.PollRTP()) < 0)
		return status;
	while (rtpconn.RTPDataAvailable())
	{
		if ((status = rtpconn.ReceiveRTPData(&data,&len,&ip,&port)) < 0)
			return status;

		// the packet processor deletes 'data' in case of an error
		if ((status = packetprocessor.ProcessRTPBlock(data,len,ip,port,&collis,acceptownpackets,localinf.tsunit)) < 0)
			return status;
		if (collis) // collision with local ssrc
		{
			if ((status = rtcpmodule.SendBYE()) < 0)
				return status;
			CreateNewSSRC();
			localinf.octetcount = 0;
			localinf.packetcount = 0;
		}
	}

	/* Process data on RTCP port */

	if ((status = rtpconn.PollRTCP()) < 0)
		return status;
	while (rtpconn.RTCPDataAvailable())
	{
		if ((status = rtpconn.ReceiveRTCPData(&data,&len,&ip,&port)) < 0)
			return status;

		// the packet processor deletes 'data' in case of an error
		if ((status = packetprocessor.ProcessRTCPBlock(data,len,ip,port,&collis,localinf.tsunit)) < 0)
			return status;
		if (collis) // collision with local ssrc
		{
			if ((status = rtcpmodule.SendBYE()) < 0)
				return status;
			CreateNewSSRC();
			localinf.octetcount = 0;
			localinf.packetcount = 0;
		}
	}

	/* Check if any sources have joined or left */

	if (sources.GetNumberOfParticipants() != prevnumsources)
		rtcpmodule.CalcNextRTCPTime();
	
	/* Check if we have to send RTCP packets */

	curtime = time(NULL);
	if (rtcpmodule.RTCPCheck(curtime))
	{
		status = rtcpmodule.RTCPRoutine(curtime);
		if (status < 0)
			return status;
	}

	return 0;
}

void RTPSession::CreateNewSSRC()
{
	bool found;

	do
	{
		found = false;
		contribsources.CreateLocalSSRC();
		if (sources.Retrieve(ntohl(contribsources.GetLocalSSRC())) != NULL)
			found = true;
		else if (contribsources.DoesCSRCExist(ntohl(contribsources.GetLocalSSRC())))
			found = true;
	} while (found);
}
