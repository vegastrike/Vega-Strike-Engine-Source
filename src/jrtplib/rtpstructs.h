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

#ifndef RTPSTRUCTS_H

#define RTPSTRUCTS_H

#include "rtpconfig.h"

#define TYPE_RTCP_SR		200
#define TYPE_RTCP_RR		201
#define TYPE_RTCP_SDES		202
#define TYPE_RTCP_BYE		203
#define TYPE_RTCP_APP		204

#define TYPE_SDES_END		0
#define TYPE_SDES_CNAME		1
#define TYPE_SDES_NAME		2
#define TYPE_SDES_EMAIL		3
#define TYPE_SDES_PHONE		4
#define TYPE_SDES_LOC		5
#define TYPE_SDES_TOOL		6
#define TYPE_SDES_NOTE		7
#define TYPE_SDES_PRIV		8

typedef unsigned short RTPuint16;
typedef unsigned long RTPuint32;

struct RTPHeader
{
#ifdef RTP_BIG_ENDIAN
	unsigned char version:2;
	unsigned char padding:1;
	unsigned char extension:1;
	unsigned char cc:4;
	unsigned char marker:1;
	unsigned char payloadtype:7;
#else // little endian
	unsigned char cc:4;
	unsigned char extension:1;
	unsigned char padding:1;
	unsigned char version:2;
	unsigned char payloadtype:7;
	unsigned char marker:1;
#endif
	RTPuint16 seqnum;
	RTPuint32 timestamp;
	RTPuint32 ssrc;
};

struct RTPHeaderExtension
{
	RTPuint16 userdefined;
	RTPuint16 length;
};

struct RTCPHeader
{
#ifdef RTP_BIG_ENDIAN
	unsigned char version:2;
	unsigned char padding:1;
	unsigned char blockcount:5;
#else // little endian
	unsigned char blockcount:5;
	unsigned char padding:1;
	unsigned char version:2;
#endif
	unsigned char packettype;
	RTPuint16 length;
};

struct SSRCPrefix
{
	RTPuint32 ssrc;
};

struct RTCPSenderInfo
{
	RTPuint32 NTPmsw;
	RTPuint32 NTPlsw;
	RTPuint32 rtptimestamp;
	RTPuint32 senderpacketcount;
	RTPuint32 senderoctetcount;
};

struct RTCPReportBlock
{
	RTPuint32 ssrc;
	unsigned char fractionlost;
	unsigned char packetslost[3];
	RTPuint32 exthsnr; // extended highest sequence number received
	RTPuint32 jitter;
	RTPuint32 lsr; // last SR timestamp
	RTPuint32 dlsr; // delay since last SR
};

struct SDESPrefix
{
	unsigned char sdestype;
	unsigned char length;
};

struct RTCPAPPPrefix
{
	RTPuint32 src;
	unsigned char name[4];
};

#endif // RTPSTRUCTS_H
