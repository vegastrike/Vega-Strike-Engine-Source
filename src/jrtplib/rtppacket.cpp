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

#include "rtppacket.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

RTPPacket::RTPPacket(RTPHeader *rtphdr,unsigned char *rawdat,unsigned char *payloadptr,int payldlen)
{
	seqnum = ntohs(rtphdr->seqnum);
	extendedseqnum = seqnum; // this will later be modified (in rtpsourcedata)
	timestamp = ntohl(rtphdr->timestamp);
	syncsource = ntohl(rtphdr->ssrc);
	payloadtype = (unsigned char)(rtphdr->payloadtype);
	marked = (rtphdr->marker == 1)?true:false;
	numcontribsources = (int)rtphdr->cc;
	
	// fill the array of contributing sources
	unsigned char *p;
	int i;
	
	for (p = rawdat+sizeof(RTPHeader), i = 0 ; i < numcontribsources ; i++ , p += sizeof(RTPuint32))
	{
		RTPuint32 csrc;
		
		memcpy(&csrc,p,sizeof(RTPuint32));
		contribsources[i] = ntohl(csrc);
	}
	
	hashdrext = (rtphdr->extension == 1)?true:false;
	if (!hashdrext)
	{
		hdrextensionID = 0;
		hdrextlen = 0;
		hdrextdata = NULL;
	}
	else
	{
		RTPHeaderExtension *hdrext;
		
		hdrextdata = rawdat+sizeof(RTPHeader)+numcontribsources*sizeof(RTPuint32);
		hdrext = (RTPHeaderExtension *)hdrextdata;
		hdrextdata += sizeof(RTPHeaderExtension);
		
		hdrextensionID = ntohs(hdrext->userdefined);
		hdrextlen = ntohs(hdrext->length);
		hdrextlen *= sizeof(RTPuint32);
		if (hdrextlen == 0)
			hdrextdata = NULL;
	}
	
	payloadlen = payldlen;
	rawdata = rawdat;
	payloaddata = payloadptr;
	prev = NULL;
	next = NULL;
}

RTPPacket::~RTPPacket()
{
	if (rawdata != NULL)
		RTP_DELETE_ARRAY(rawdata);
}
	
