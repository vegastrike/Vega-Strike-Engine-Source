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

#ifndef RTPSESSION_H

#define RTPSESSION_H

#include "rtpconfig.h"
#include "rtpdefines.h"
#include "rtpconnection.h"
#include "rtpcontributingsources.h"
#include "rtppacketprocessor.h"
#include "rtplocalinfo.h"
#include "rtprtcpmodule.h"
#include "rtpsources.h"
#include "rtperror.h"
#include "rtphandlers.h"
#include "rtpsourcedata.h"

class RTPPacket;

class RTPSession
{
public:
	RTPSession();
	~RTPSession();
	int Create(int localportbase,unsigned long localip = 0);
	int GetPortBase()									{ if (!initialized) return ERR_RTP_SESSIONNOTCREATED; return rtpconn.GetPortBase(); }
	int GetLocalSSRC(unsigned long *ssrc)							{ if (!initialized) return ERR_RTP_SESSIONNOTCREATED; *ssrc = ntohl(contribsources.GetLocalSSRC()); return 0; }
	int Destroy();

	int GetRTPSocket(RTPSOCKET *sock)							{ if (!initialized) return ERR_RTP_SESSIONNOTCREATED; *sock = rtpconn.GetRTPSocket(); return 0; }
	int GetRTCPSocket(RTPSOCKET *sock)							{ if (!initialized) return ERR_RTP_SESSIONNOTCREATED; *sock = rtpconn.GetRTCPSocket(); return 0; }
	int GetSendSocket(RTPSOCKET *sock)							{ if (!initialized) return ERR_RTP_SESSIONNOTCREATED; *sock = rtpconn.GetSendSocket(); return 0; }
	int GetSendPort()									{ if (!initialized) return ERR_RTP_SESSIONNOTCREATED; return rtpconn.GetSendPort(); }
	int GetLocalIP(unsigned long *ip)							{ if (!initialized) return ERR_RTP_SESSIONNOTCREATED; *ip = rtpconn.GetLocalIP(); return 0; }
	int SetToS(int tos)									{ if (!initialized) return ERR_RTP_SESSIONNOTCREATED; return rtpconn.SetToS(tos); }

	int AddDestination(unsigned long remoteIP,int remoteportbase)				{ return rtpconn.AddDestination(remoteIP,remoteportbase); }
	int DeleteDestination(unsigned long remoteIP,int remoteportbase)			{ return rtpconn.DeleteDestination(remoteIP,remoteportbase); }
	void ClearDestinations()								{ rtpconn.ClearDestinations(); }
	
	bool SupportsMulticasting()								{ return rtpconn.SupportsMulticasting(); }
	int JoinMulticastGroup(unsigned long mcastIP)						{ if (!initialized) return ERR_RTP_SESSIONNOTCREATED; return rtpconn.JoinMulticastGroup(mcastIP); }
	int LeaveMulticastGroup(unsigned long mcastIP)						{ if (!initialized) return ERR_RTP_SESSIONNOTCREATED; return rtpconn.LeaveMulticastGroup(mcastIP); }
	void LeaveAllMulticastGroups()								{ rtpconn.LeaveAllMulticastGroups(); }
	int SetMulticastTTL(unsigned char ttl)							{ if (!initialized) return ERR_RTP_SESSIONNOTCREATED; return rtpconn.SetMulticastTTL(ttl); }
	
	int SendPacket(void *data,int len);
	int SendPacket(void *data,int len,unsigned char pt,bool mark,unsigned long timestampinc);
	int SendPacket(void *data,int len,unsigned short hdrextID,void *hdrextdata,int numhdrextwords);
	int SendPacket(void *data,int len,unsigned char pt,bool mark,unsigned long timestampinc,unsigned short hdrextID,void *hdrextdata,int numhdrextwords);
	void SetDefaultPayloadType(unsigned char pt)						{ localinf.SetDefaultPayloadType(pt); }
	void SetDefaultMark(bool mark)								{ localinf.SetDefaultMark(mark); }
	void SetDefaultTimeStampIncrement(unsigned long inc)					{ localinf.SetDefaultTimestampInc(inc); }
	int IncrementTimeStamp(unsigned long inc)						{ if (!initialized) return ERR_RTP_SESSIONNOTCREATED; localinf.timestamp += inc; return 0; }
	int IncrementTimeStampDefault()								{ if (!initialized) return ERR_RTP_SESSIONNOTCREATED; if (!localinf.deftsincset) return ERR_RTP_NODEFAULTINCREMENTSET; localinf.timestamp += localinf.deftsinc; return 0; }
		
	int PollData();
	bool GotoFirstSource()									{ if (!initialized) return false; return sources.GotoFirstSource(); }
	bool GotoNextSource()									{ if (!initialized) return false; return sources.GotoNextSource(); }
	bool GotoFirstSourceWithData()								{ if (!initialized) return false; return sources.GotoFirstSourceWithData(); }
	bool GotoNextSourceWithData()								{ if (!initialized) return false; return sources.GotoNextSourceWithData(); }

	inline RTPPacket *GetNextPacket();
	inline RTPSourceData *GetCurrentSourceInfo();
	RTPSourceData *GetSourceInfo(unsigned long ssrc)					{ if (!initialized) return NULL; return sources.Retrieve(ssrc); }
	
	int AddInvalidLocalSSRC(unsigned long ssrc)						{ return contribsources.AddInvalidLocalSSRC(ssrc); }
	int RemoveInvalidLocalSSRC(unsigned long ssrc)						{ return contribsources.RemoveInvalidLocalSSRC(ssrc); }
	void ClearInvalidLocalSSRCList()							{ contribsources.ClearInvalidLocalSSRCList(); }
	                                                                	
	int AddCSRC(unsigned long csrc)								{ return contribsources.AddCSRC(csrc); }
	int DeleteCSRC(unsigned long csrc)							{ return contribsources.DeleteCSRC(csrc); }
	void ClearCSRCList()									{ contribsources.ClearCSRCList(); }
	
	int SetReceiveMode(int mode)								{ return rtpconn.SetReceiveMode(mode); }
	int AddToIgnoreList(unsigned long remoteIP,bool allports,int portbase)			{ return rtpconn.AddToIgnoreList(remoteIP,allports,portbase); }
	int DeleteFromIgnoreList(unsigned long remoteIP,bool allports,int portbase)	 	{ return rtpconn.DeleteFromIgnoreList(remoteIP,allports,portbase); }
	void ClearIgnoreList()														{ rtpconn.ClearIgnoreList(); }
	int AddToAcceptList(unsigned long remoteIP,bool allports,int portbase)			{ return rtpconn.AddToAcceptList(remoteIP,allports,portbase); }
	int DeleteFromAcceptList(unsigned long remoteIP,bool allports,int portbase)		{ return rtpconn.DeleteFromAcceptList(remoteIP,allports,portbase); }
	void ClearAcceptList()									{ rtpconn.ClearAcceptList(); }

	inline int SetMaxPacketSize(int m);
	void SetSessionBandwidth(double bw)							{ rtcpmodule.SetSessionBandWidth(bw); }
	void SetTimestampUnit(double t)								{ localinf.tsunit = t; }
	void SetControlTrafficFragment(double frag)						{ rtcpmodule.SetControlTrafficFragment(frag); }
	
	int SetLocalName(char *s,int len)							{ return contribsources.SetLocalName(s,len); }
	int SetLocalEMail(char *s,int len)							{ return contribsources.SetLocalEMail(s,len); }
	int SetLocalLocation(char *s,int len)							{ return contribsources.SetLocalLocation(s,len); }
	int SetLocalPhone(char *s,int len)							{ return contribsources.SetLocalPhone(s,len); }
	int SetLocalTool(char *s,int len)							{ return contribsources.SetLocalTool(s,len); }
	int SetLocalNote(char *s,int len)							{ return contribsources.SetLocalNote(s,len); }
	int SetCSRC_CNAME(unsigned long csrc,char *s,int len)					{ return contribsources.SetCSRC_CNAME(csrc,s,len); }
	int SetCSRCName(unsigned long csrc,char *s,int len)					{ return contribsources.SetCSRCName(csrc,s,len); }
	int SetCSRCEMail(unsigned long csrc,char *s,int len)					{ return contribsources.SetCSRCEMail(csrc,s,len); }
	int SetCSRCLocation(unsigned long csrc,char *s,int len)					{ return contribsources.SetCSRCLocation(csrc,s,len); }
	int SetCSRCPhone(unsigned long csrc,char *s,int len)					{ return contribsources.SetCSRCPhone(csrc,s,len); }
	int SetCSRCTool(unsigned long csrc,char *s,int len)					{ return contribsources.SetCSRCTool(csrc,s,len); }
	int SetCSRCNote(unsigned long csrc,char *s,int len)					{ return contribsources.SetCSRCNote(csrc,s,len); }
	void EnableSendName(bool val)								{ contribsources.EnableSendName(val); }
	void EnableSendEMail(bool val)								{ contribsources.EnableSendEMail(val); }
	void EnableSendLocation(bool val)							{ contribsources.EnableSendLocation(val); }
	void EnableSendPhone(bool val)								{ contribsources.EnableSendPhone(val); }
	void EnableSendTool(bool val)								{ contribsources.EnableSendTool(val); }
	void EnableSendNote(bool val)								{ contribsources.EnableSendNote(val); }

	void SetLocalSSRCCollisionHandler(RTPExceptionHandler handler,void *usrdata)		{ handlers.SetLocalSSRCCollisionHandler(handler,usrdata); }
	void SetSSRCCollisionHandler(RTPExceptionHandler handler,void *usrdata)			{ handlers.SetSSRCCollisionHandler(handler,usrdata); }
	void SetNewSourceHandler(RTPExceptionHandler handler,void *usrdata)			{ handlers.SetNewSourceHandler(handler,usrdata); }
	void SetInvalidSDESTypeHandler(RTPExceptionHandler handler,void *usrdata)		{ handlers.SetInvalidSDESTypeHandler(handler,usrdata); }
	void SetSSRCDepartureHandler(RTPExceptionHandler handler,void *usrdata)			{ handlers.SetSSRCDepartureHandler(handler,usrdata); }
	void SetSSRCTimeoutHandler(RTPExceptionHandler handler,void *usrdata)			{ handlers.SetSSRCTimeoutHandler(handler,usrdata); }
	void SetReceiveRTCPAPPHandler(RTPExceptionHandler handler,void *usrdata)		{ handlers.SetReceiveRTCPAPPHandler(handler,usrdata); }
	void SetTransmitRTCPAPPHandler(RTPExceptionHandler handler,void *usrdata)		{ handlers.SetTransmitRTCPAPPHandler(handler,usrdata); }
	void SetRTCPPacketHandler(RTPExceptionHandler handler,void *usrdata)			{ handlers.SetRTCPPacketHandler(handler,usrdata); }
	void ClearHandlers()									{ handlers.Clear(); }

	void SetAcceptOwnPackets(bool accept)							{ acceptownpackets = accept; }
	bool GetAcceptOwnPackets()								{ return acceptownpackets; }
private:
	void CreateNewSSRC();

	RTPConnection rtpconn;
	RTPContributingSources contribsources; // includes local info
	RTPPacketProcessor packetprocessor;
	RTPLocalInfo localinf;
	RTPSources sources;
	RTPRTCPModule rtcpmodule;
	RTPHandlers handlers;
	bool initialized;
	bool acceptownpackets;
};

inline RTPPacket *RTPSession::GetNextPacket()
{
	RTPSourceData *srcdat;

	if (!initialized)
		return NULL;
	if ((srcdat = sources.GetSourceInfo()) == NULL)
		return NULL;
	return srcdat->ExtractPacket();
}

inline RTPSourceData *RTPSession::GetCurrentSourceInfo()
{
	if (!initialized)
		return NULL;
	return sources.GetSourceInfo();
}

inline int RTPSession::SetMaxPacketSize(int m)
{
	if (m < RTP_MINIMUMPACKETSIZE)
		return ERR_RTP_MAXPACKSIZETOOSMALL;
	if (m > RTP_MAXIMUMPACKETSIZE)
		return ERR_RTP_MAXPACKSIZETOOBIG;
	localinf.maxpacksize = m;
	rtpconn.SetMaximumPacketSize(m);
	return 0;
}

#endif // RTPSESSION_H
