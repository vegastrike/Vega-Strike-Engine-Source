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

#ifndef RTPERROR_H

#define RTPERROR_H

#include "rtpconfig.h"

#define ERR_RTP_OUTOFMEM								-1
#define ERR_RTP_CONNALREADYCREATED							-2
#define ERR_RTP_PORTBASENOTEVEN								-3
#define ERR_RTP_CANTCREATESOCKET							-4
#define ERR_RTP_CANTBINDSOCKET								-5
#define ERR_RTP_INVALIDRECEIVEMODE							-6
#define ERR_RTP_ILLEGALPORT								-7
#define ERR_RTP_IPTABLEENTRYEXISTS							-8
#define ERR_RTP_IPTABLEENTRYDOESNTEXIST							-9
#define ERR_RTP_ENTRYNOTINDESTLIST							-10
#define ERR_RTP_PACKETTOOLARGE								-11
#define ERR_RTP_SOCKETSNOTOPEN								-12
#define ERR_RTP_MAXPACKSIZETOOSMALL							-13
#define ERR_RTP_NORTPDATAAVAILABLE							-14
#define	ERR_RTP_NORTCPDATAAVAILABLE							-15
#define ERR_RTP_SSRCNOTFOUND								-16
#define ERR_RTP_SDESINDEXOUTOFRANGE							-17
#define ERR_RTP_CSRCNOTINLIST								-18
#define ERR_RTP_ARGUMENTCANTBENULL							-19
#define ERR_RTP_PACKETPROCESSORNOTINITIALIZED						-20
#define	ERR_RTP_PACKETISNOTFORTHISSOURCE						-21
#define ERR_RTP_SOURCESNOTINITALIZED							-22
#define ERR_RTP_COLLISIONBETWEENSSRCS							-23
#define	ERR_RTP_RTCPMODULENOTINITIALIZED						-24
#define ERR_RTP_SESSIONNOTCREATED							-25
#define ERR_RTP_SESSIONALREADYINITIALIZED						-26
#define ERR_RTP_NODEFAULTVALUESSET							-27
#define ERR_RTP_CONTRIBSOURCESNOTINIT							-28
#define ERR_RTP_CANTSETSOCKETBUFFER							-29
#define ERR_RTP_NODEFAULTINCREMENTSET							-30
#define ERR_RTP_MAXPACKSIZETOOBIG							-31
#define ERR_RTP_CANTGETSOCKETPORT							-32
#define ERR_RTP_CANTGETLOCALIP								-33
#define ERR_RTP_MULTICASTINGNOTSUPPORTED						-34
#define ERR_RTP_CANTSETMULTICASTTTL							-35
#define ERR_RTP_NOTAMULTICASTADDRESS							-36
#define ERR_RTP_ALREADYINMCASTGROUP							-37
#define ERR_RTP_NOTINMCASTGROUP								-38
#define ERR_RTP_MULTICASTJOINFAILED							-39
#define ERR_RTP_SUBTYPETOOLARGE								-40
#define ERR_RTP_CANTSETTOS								-41

char *RTPGetErrorString(int rtperrnr);

#endif // RTPERROR_H
