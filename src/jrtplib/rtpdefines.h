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

#ifndef RTPDEFINES_H

#define RTPDEFINES_H

#include "rtpconfig.h"

#ifdef WIN32
	#ifndef RTP_MULTICAST
		#include <windows.h>
	#else
		#include <windows.h>
		#include <ws2tcpip.h>
	#endif // RTP_MULTICAST

	#define RTPCLOSESOCKET(s)			closesocket(s)
	#define RTPIOCTL(a,b,c)				ioctlsocket(a,b,c)
	#define RTPSOCKET				SOCKET // socket type
	#define RTPSOCKERR				INVALID_SOCKET
	#define RTPSOCKLENTYPE				int
#else  // UNIX type sockets
	#include <stdlib.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <sys/ioctl.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <unistd.h>
	#include <netdb.h>
	
#ifndef RTP_VXWORKS
	#include <sys/time.h>
#endif // RTP_VXWORKS
	
	#define RTPCLOSESOCKET(s) 			close(s)
	#define RTPIOCTL(a,b,c) 			ioctl(a,b,c)
	#define RTPSOCKET				int // socket type
	#define RTPSOCKERR				-1

	#ifdef RTP_SOCKLENTYPE_UINT
		#define RTPSOCKLENTYPE			unsigned int
	#else
		#define RTPSOCKLENTYPE			int
	#endif // RTP_SOCKLENTYPE_UINT

#endif // WIN32

#define RTP_NUM_EXCEPTIONS				9
#define RTP_EXCEPTION_LOCALSSRCCOLLISION		0
#define RTP_EXCEPTION_SSRCCOLLISION			1
#define RTP_EXCEPTION_RECEIVERTCPAPPDATA		2
#define RTP_EXCEPTION_NEWSOURCE				3
#define RTP_EXCEPTION_INVALIDSDESTYPE			4
#define RTP_EXCEPTION_SSRCDEPARTURE			5
#define RTP_EXCEPTION_SSRCTIMEOUT			6
#define RTP_EXCEPTION_TRANSMITRTCPAPPDATA		7
#define RTP_EXCEPTION_RTCPPACKET			8

#define RTP_VERSION					2
#define RTP_NUM_SDES_INDICES				8
#define RTP_MINIMUMPACKETSIZE				300
#define RTP_MAXIMUMPACKETSIZE				65535
#define RTP_MINIMUM_RTCP_DELAY				5
#define RTP_TIMEOUTSEC					1800 // 30 minutes
#define RTP_SDESINFO_INTERVAL				10 // 10 packet interval
#define RTP_NTPTIMEOFFSET				2208992400UL

#define RTP_DEFAULT_MAXPACKSIZE				1100
#define RTP_DEFAULT_SESSIONBANDWIDTH			64000.0
#define RTP_DEFAULT_RTCPFRAGMENT			0.05
#define RTP_DEFAULT_TIMESTAMPUNIT			(1.0/8000.0)

#define RTP_SENDBUFFERSIZE				32738
#define RTP_RECEIVEBUFFERSIZE				32738

#define RTP_DEFAULTMULTICAST_TTL			1

#define IP_UDP_HEADERSIZE				(20+8)

#endif // RTPDEFINES_H
