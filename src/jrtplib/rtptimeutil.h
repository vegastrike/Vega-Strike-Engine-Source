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

#ifndef RTPTIMEUTIL_H

#define RTPTIMEUTIL_H

#include "rtpconfig.h"
#include "rtpdefines.h"
#include <time.h>

#ifdef WIN32
	#include <sys/timeb.h>

	int gettimeofday(struct timeval *tp, void *t);
#elif RTP_VXWORKS
	int gettimeofday(struct timeval *tp, void *t);
#endif // WIN32,RTP_VXWORKS

void getntptime(struct timeval *tv,unsigned long *ntplsw,unsigned long *ntpmsw);
void getrtptimestamp(struct timeval *tv,struct timeval *tsoffsettime,unsigned long tsoffset,double tsunit,unsigned long *rtptimestamp);

#endif // RTPTIMEUTIL_H
