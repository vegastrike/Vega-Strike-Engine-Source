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

#ifndef RTPEXCEPTIONSTRUCTS_H

#define RTPEXCEPTIONSTRUCTS_H

#include "rtpconfig.h"

struct RTPExcepSSRCCollision
{
	unsigned long ssrc;
	unsigned long ip;
	bool rtpdata;
	int port;
};

struct RTPExcepSSRC
{
	unsigned long ssrc;
};

struct RTPExcepAppData
{
	unsigned char subtype;
	unsigned long src;
	unsigned char name[4];
	unsigned char *data;
	int numwords;
	bool validdata;
	int prevreturnval;
};

struct RTPExcepInvalSDESType
{
	unsigned long ssrc;
	int sdestype;
	unsigned char *sdesdata;
	int datalen;
};

struct RTPExcepRTCPPacket
{
	unsigned char *packet;
	int packetlen;
	unsigned long sourceip;
	int sourceport;
};

#endif // RTPEXCEPTIONSTRUCTS_H
