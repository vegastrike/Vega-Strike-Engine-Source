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

#include "rtptimeutil.h"

#ifdef WIN32

int gettimeofday(struct timeval *tp, void *t)
{
	struct _timeb tv;

	_ftime(&tv);
	tp->tv_sec = tv.time;
	tp->tv_usec = tv.millitm*1000;

	return 1;
}

#endif // WIN32

#ifdef RTP_VXWORKS

int gettimeofday(struct timeval *tp, void *t)
{
        struct timespec time_stamp;

        clock_gettime(CLOCK_REALTIME,&time_stamp);
        tp->tv_sec = time_stamp.tv_sec;
        tp->tv_usec = time_stamp.tv_nsec/1000; // we need microseconds, not nanoseconds

        return 1;
}

#endif // RTP_VXWORKS

void getntptime(struct timeval *tv,unsigned long *ntplsw,unsigned long *ntpmsw)
{
	double x;

	*ntpmsw = htonl(((unsigned long)tv->tv_sec+(unsigned long)RTP_NTPTIMEOFFSET));
	x = (double)tv->tv_usec;
	x *= (65536.0*65536.0)/1000000.0;
	*ntplsw = htonl((unsigned long)x);
}

void getrtptimestamp(struct timeval *tv,struct timeval *tsoffsettime,unsigned long tsoffset,double tsunit,unsigned long *rtptimestamp)
{
	long diffsec,diffusec;
	double x,diff;

	diffsec = tv->tv_sec-tsoffsettime->tv_sec;
	diffusec = tv->tv_usec-tsoffsettime->tv_usec;
	while (diffusec < 0)
	{
		diffsec--;
		diffusec += 1000000;
	}
	diff = (double)diffsec+(((double)diffusec)/1000000.0);
	x = diff/tsunit;
	*rtptimestamp = htonl(((unsigned long)x)+tsoffset);
}
