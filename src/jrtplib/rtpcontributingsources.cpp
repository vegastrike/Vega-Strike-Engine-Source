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

#include "rtpcontributingsources.h"
#include "rtpsources.h"
#include <stdlib.h>
#include <string.h>

#ifndef WIN32
#ifndef RTP_VXWORKS
#include <pwd.h>    // for 'getpwuid'
#include <stdio.h>  // for 'sprintf'
#endif // RTP_VXWORKS
#endif // WIN32

RTPContributingSources::RTPContributingSources()
{
	firstcsrc = NULL;
	numcsrcs = 0;
	csrcschanged = true;
	ResetEnabledInfo();
	srcs = NULL;
}

RTPContributingSources::~RTPContributingSources()
{
	Clear();
}

void RTPContributingSources::CreateLocalSSRC()
{
	RTPuint32 src,src2;
	bool found;

	do
	{
		src = rand.RandomUInt32();
		found = false;
		invalidssrcs.InitRetrieval();
		while (!found && invalidssrcs.GetNext(&src2))
		{
			if (src2 == src)
				found = true;
		}
	} while (found);
	localinfo.SetSRC(src);
}

int RTPContributingSources::AddCSRC(unsigned long csrc)
{
	RTPSourceDescription *tmp;

	if (srcs == NULL)
		return ERR_RTP_CONTRIBSOURCESNOTINIT;

	tmp = RTP_NEW(RTPSourceDescription(csrc));
	if (tmp == NULL)
		return ERR_RTP_OUTOFMEM;
	tmp->next = firstcsrc;
	firstcsrc = tmp;
	localinfo.next = tmp;
	srcs->CSRCAdded(csrc);

	numcsrcs++;
	csrcschanged = true;
	return 0;
}

int RTPContributingSources::DeleteCSRC(unsigned long csrc)
{
	bool found;
	RTPSourceDescription *tmp,*tmpprev;
	RTPuint32 nbo_csrc;

	if (srcs == NULL)
		return ERR_RTP_CONTRIBSOURCESNOTINIT;

	tmpprev = NULL;
	tmp = firstcsrc;
	found = false;
	nbo_csrc = htonl(csrc);
	while (!found && tmp != NULL)
	{
		if (tmp->src == nbo_csrc)
			found = true;
		else
		{
			tmpprev = tmp;
			tmp = tmp->next;
		}
	}
	if (!found)
		return ERR_RTP_CSRCNOTINLIST;
	
	if (tmpprev == NULL)
	{
		firstcsrc = firstcsrc->next;
		localinfo.next = firstcsrc;
	}
	else
		tmpprev->next = tmp->next;
	srcs->CSRCDeleted(ntohl(tmp->src));

	RTP_DELETE(tmp);
	numcsrcs--;
	csrcschanged = true;
	return 0;
}

void RTPContributingSources::ClearCSRCList()
{
	RTPSourceDescription *tmp;

	while (firstcsrc != NULL)
	{
		tmp = firstcsrc->next;
		if (srcs != NULL)
			srcs->CSRCDeleted(ntohl(firstcsrc->src));
		RTP_DELETE(firstcsrc);
		firstcsrc = tmp;
	}
	localinfo.next = NULL;
	numcsrcs = 0;
	csrcschanged = true;
}

bool RTPContributingSources::DoesCSRCExist(unsigned long csrc)
{
	bool found;
	RTPSourceDescription *tmp;
	RTPuint32 nbo_csrc;

	tmp = firstcsrc;
	found = false;
	nbo_csrc = htonl(csrc);
	while (!found && tmp != NULL)
	{
		if (tmp->src == nbo_csrc)
			found = true;
		else
			tmp = tmp->next;
	}
	if (!found)
		return false;
	return true;
}

void RTPContributingSources::FillCSRCs(unsigned char *data) // fills in CSRC info, in network byte order, with a maximum of 15 CSRCs
{
	RTPSourceDescription *tmp;
	unsigned char *ptr;
	int num,i;

	if (!csrcschanged)
		memcpy(data,fillcsrcs,numfillcsrcs*sizeof(RTPuint32));
	else
	{
		num = numcsrcs;
		if (num > 15)
			num = 15;
		ptr = data;
		tmp = firstcsrc;
		for (i = 0 ; i < num ; i++)
		{	
			memcpy(ptr,&(tmp->src),sizeof(RTPuint32));
			fillcsrcs[i] = tmp->src;
			ptr += sizeof(RTPuint32);
			tmp = tmp->next;
		}
		numfillcsrcs = num;
		csrcschanged = false;
	}
}

int RTPContributingSources::CreateLocalCNAME()
{
	char buf[1024];
	char bufname[256];
	char bufhost[256];
	int status;

	status = gethostname(bufhost,256);
	if (status != 0)
		strcpy(bufhost,"unknown-host");
	bufhost[255] = 0;

	GetLoginName(bufname,256);
	
	strcpy(buf,bufname);
	strcat(buf,"@");
	strcat(buf,bufhost);
	
	return localinfo.SetSDES(TYPE_SDES_CNAME-1,buf,strlen(buf));
}

RTPSourceDescription *RTPContributingSources::GetCSRC(RTPuint32 csrc)
{
	RTPSourceDescription *tmp;
	RTPuint32 nbo_csrc;

	nbo_csrc = htonl(csrc);
	tmp = firstcsrc;
	while (tmp != NULL)
	{
		if (tmp->src == nbo_csrc)
			return tmp;
		else
			tmp = tmp->next;
	}
	return NULL;
}

void RTPContributingSources::ResetEnabledInfo()
{
	int i;

	for (i = 0 ; i < RTP_NUM_SDES_INDICES ; i++)
	{
		if (i == (TYPE_SDES_CNAME-1))
			enabledinfo[i] = true;
		else
			enabledinfo[i] = false;
	}
	enabledinfocount = 1; // for CNAME
}

#ifdef WIN32

void RTPContributingSources::GetLoginName(char *buf,int buflen)
{
	unsigned long len;
	char buf2[256];
	
	len = 255;
	if (!GetUserName(buf2,&len))
		strcpy(buf2,"curuser");
	strncpy(buf,buf2,buflen-1);
	buf[buflen-1] = 0;
}

#elif RTP_VXWORKS

void RTPContributingSources::GetLoginName(char *buf,int buflen)
{
	strncpy(buf,"vxworks",buflen-1);
	buf[buflen-1] = 0;
}

#else // Unix-version

void RTPContributingSources::GetLoginName(char *buf,int buflen)
{
	uid_t uid;
	struct passwd *pwdent;
	
	uid = geteuid();
	pwdent = getpwuid(uid);
	if (pwdent == NULL)
	{
		sprintf(buf,"uid_%d",uid);
		return;
	}
	strncpy(buf,pwdent->pw_name,buflen-1);
	buf[buflen-1] = 0;
}

#endif

