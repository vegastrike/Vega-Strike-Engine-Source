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

#ifndef RTPCONTRIBUTINGSOURCES_H

#define RTPCONTRIBUTINGSOURCES_H

#include "rtpconfig.h"
#include "rtpdefines.h"
#include "rtpdebug.h"
#include "rtpsourcedescription.h"
#include "rtpsrclist.h"
#include "rtprandom.h"
#include "rtperror.h"

class RTPSources;

class RTPContributingSources							RTPDEBUGBASE
{
public:
	RTPContributingSources();
	~RTPContributingSources();
	
	inline void Clear();
	inline int SetSources(RTPSources *s);

	void CreateLocalSSRC();
	int AddInvalidLocalSSRC(unsigned long ssrc)			{ return invalidssrcs.Add(ssrc); }
	int RemoveInvalidLocalSSRC(unsigned long ssrc)			{ return invalidssrcs.Delete(ssrc); }
	void ClearInvalidLocalSSRCList()				{ invalidssrcs.Clear(); }
	unsigned long GetLocalSSRC()					{ return localinfo.src; } // in network byte order
	
	int AddCSRC(unsigned long csrc);
	int DeleteCSRC(unsigned long csrc);
	void ClearCSRCList();
	int GetNumberOfCSRCs()						{ return numcsrcs; }
	bool DoesCSRCExist(unsigned long csrc);
	void FillCSRCs(unsigned char *data); // fills in CSRC info, in network byte order, with a maximum of 15 CSRCs

	int CreateLocalCNAME();
	int SetLocalName(char *s,int len)				{ return localinfo.SetSDES(TYPE_SDES_NAME-1,s,len); }
	int SetLocalEMail(char *s,int len)				{ return localinfo.SetSDES(TYPE_SDES_EMAIL-1,s,len); }
	int SetLocalLocation(char *s,int len)				{ return localinfo.SetSDES(TYPE_SDES_LOC-1,s,len); }
	int SetLocalPhone(char *s,int len)				{ return localinfo.SetSDES(TYPE_SDES_PHONE-1,s,len); }
	int SetLocalTool(char *s,int len)				{ return localinfo.SetSDES(TYPE_SDES_TOOL-1,s,len); }
	int SetLocalNote(char *s,int len)				{ return localinfo.SetSDES(TYPE_SDES_NOTE-1,s,len); }
	int SetCSRC_CNAME(unsigned long csrc,char *s,int len)		{ return SetCSRCInfo(csrc,TYPE_SDES_CNAME-1,s,len); }
	int SetCSRCName(unsigned long csrc,char *s,int len)		{ return SetCSRCInfo(csrc,TYPE_SDES_NAME-1,s,len); }
	int SetCSRCEMail(unsigned long csrc,char *s,int len)		{ return SetCSRCInfo(csrc,TYPE_SDES_EMAIL-1,s,len); }
	int SetCSRCLocation(unsigned long csrc,char *s,int len)		{ return SetCSRCInfo(csrc,TYPE_SDES_LOC-1,s,len); }
	int SetCSRCPhone(unsigned long csrc,char *s,int len)		{ return SetCSRCInfo(csrc,TYPE_SDES_PHONE-1,s,len); }
	int SetCSRCTool(unsigned long csrc,char *s,int len)		{ return SetCSRCInfo(csrc,TYPE_SDES_TOOL-1,s,len); }
	int SetCSRCNote(unsigned long csrc,char *s,int len)		{ return SetCSRCInfo(csrc,TYPE_SDES_NOTE-1,s,len); }
	void EnableSendName(bool val)							{ ProcessEnableRequest(TYPE_SDES_NAME-1,val); }
	void EnableSendEMail(bool val)							{ ProcessEnableRequest(TYPE_SDES_EMAIL-1,val); }
	void EnableSendLocation(bool val)						{ ProcessEnableRequest(TYPE_SDES_LOC-1,val); }
	void EnableSendPhone(bool val)							{ ProcessEnableRequest(TYPE_SDES_PHONE-1,val); }
	void EnableSendTool(bool val)							{ ProcessEnableRequest(TYPE_SDES_TOOL-1,val); }
	void EnableSendNote(bool val)							{ ProcessEnableRequest(TYPE_SDES_NOTE-1,val); }
private:
	RTPSourceDescription *GetCSRC(RTPuint32 csrc);
	inline int SetCSRCInfo(RTPuint32 csrc,int index,char *s,int len);
	void ResetEnabledInfo();
	inline void ProcessEnableRequest(int index,bool val);
	void GetLoginName(char *buf,int buflen);

	RTPSourceDescription localinfo;
	RTPSourceDescription *firstcsrc;
	int numcsrcs;
	
	RTPuint32 fillcsrcs[15];
	bool csrcschanged;
	int numfillcsrcs;

	bool enabledinfo[RTP_NUM_SDES_INDICES];
	int enabledinfocount;

	RTPSources *srcs;
	RTPSrcList invalidssrcs;
	RTPRandom rand;

friend class RTPRTCPModule;
};

inline void RTPContributingSources::Clear()
{
	ClearCSRCList();
	localinfo.Clear();
	invalidssrcs.Clear();
	ResetEnabledInfo();
}

inline int RTPContributingSources::SetSources(RTPSources *s)
{
	if (s == NULL)
		return ERR_RTP_ARGUMENTCANTBENULL;
	srcs = s;
	return 0;
}

inline int RTPContributingSources::SetCSRCInfo(RTPuint32 csrc,int index,char *s,int len)
{
	RTPSourceDescription *tmp;

	if ((tmp = GetCSRC(csrc)) == NULL)
		return ERR_RTP_CSRCNOTINLIST;
	return tmp->SetSDES(index,s,len);
}

inline void RTPContributingSources::ProcessEnableRequest(int index,bool val)
{
	if (enabledinfo[index] != val)
	{
		enabledinfo[index] = val;
		if (val)
			enabledinfocount++;
		else
			enabledinfocount--;
	}
}

#endif // RTPCONTRIBUTINGSOURCES_H

