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

#ifndef RTPHANDLERS_H

#define RTPHANDLERS_H

#include "rtpconfig.h"
#include "rtpdefines.h"
#include <stdlib.h>

typedef void (*RTPExceptionHandler)(int exceptiontype,void *exceptiondata,void *usrdata);

class RTPHandlerInfo
{
public:
	RTPHandlerInfo() { handler = NULL; usrdata = NULL; }
	~RTPHandlerInfo() { }

	RTPExceptionHandler handler;
	void *usrdata;
};

class RTPHandlers
{
public:
	RTPHandlers() { }
	~RTPHandlers() { }
	void SetLocalSSRCCollisionHandler(RTPExceptionHandler handler,void *usrdata)			{ SetHandler(RTP_EXCEPTION_LOCALSSRCCOLLISION,handler,usrdata); }
	void SetSSRCCollisionHandler(RTPExceptionHandler handler,void *usrdata)				{ SetHandler(RTP_EXCEPTION_SSRCCOLLISION,handler,usrdata); }
	void SetNewSourceHandler(RTPExceptionHandler handler,void *usrdata)				{ SetHandler(RTP_EXCEPTION_NEWSOURCE,handler,usrdata); }
	void SetInvalidSDESTypeHandler(RTPExceptionHandler handler,void *usrdata)			{ SetHandler(RTP_EXCEPTION_INVALIDSDESTYPE,handler,usrdata); }
	void SetSSRCDepartureHandler(RTPExceptionHandler handler,void *usrdata)				{ SetHandler(RTP_EXCEPTION_SSRCDEPARTURE,handler,usrdata); }
	void SetSSRCTimeoutHandler(RTPExceptionHandler handler,void *usrdata)				{ SetHandler(RTP_EXCEPTION_SSRCTIMEOUT,handler,usrdata); }
	void SetReceiveRTCPAPPHandler(RTPExceptionHandler handler,void *usrdata)			{ SetHandler(RTP_EXCEPTION_RECEIVERTCPAPPDATA,handler,usrdata); }
	void SetTransmitRTCPAPPHandler(RTPExceptionHandler handler,void *usrdata)			{ SetHandler(RTP_EXCEPTION_TRANSMITRTCPAPPDATA,handler,usrdata); }
	void SetRTCPPacketHandler(RTPExceptionHandler handler,void *usrdata)				{ SetHandler(RTP_EXCEPTION_RTCPPACKET,handler,usrdata); }
	void Clear();
private:
	inline void SetHandler(int index,RTPExceptionHandler handler,void *usrdata);

	RTPHandlerInfo handlers[RTP_NUM_EXCEPTIONS];

friend class RTPSources;
friend class RTPPacketProcessor;
friend class RTPRTCPModule;
};

inline void RTPHandlers::SetHandler(int index,RTPExceptionHandler handler,void *usrdata)
{
	handlers[index].handler = handler;
	handlers[index].usrdata = usrdata;
}

#endif // RTPHANDLERS_H
