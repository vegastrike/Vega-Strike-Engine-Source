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

#ifndef RTPLOCALINFO_H

#define RTPLOCALINFO_H

#include "rtpconfig.h"
#include "rtpdefines.h"
#include "rtprandom.h"
#include "rtpstructs.h"
#include "rtptimeutil.h"

class RTPLocalInfo
{
public:
	RTPLocalInfo();
	~RTPLocalInfo();
	void Clear();
	inline void CreateSessionParameters(); // generates random timestamp, seqnum
	inline void SetDefaultPayloadType(unsigned char pt);
	inline void SetDefaultMark(bool m);
	inline void SetDefaultTimestampInc(unsigned long inc);
	void SetTimestampUnit(double tsu) 						{ tsunit = tsu; }

	int portbase;
	int maxpacksize;
	RTPuint16 seqnum;
	RTPuint32 timestamp,timestampoffset;
	RTPuint32 packetcount,octetcount;
	struct timeval tsoffsettime;
	bool defptset,defmarkset,deftsincset,defvaluesset;
	bool defmark;
	unsigned char defpayloadtype;
	unsigned long deftsinc;
	double tsunit;
private:
	RTPRandom rand;	
};

inline void RTPLocalInfo::CreateSessionParameters()
{
	timestamp = rand.RandomUInt32();
	timestampoffset = timestamp;
	seqnum = rand.RandomUInt16();
	gettimeofday(&tsoffsettime,NULL);
}

inline void RTPLocalInfo::SetDefaultPayloadType(unsigned char pt)
{
	defpayloadtype = pt;
	defptset = true;
	if (defmarkset && deftsincset)
		defvaluesset = true;
}

inline void RTPLocalInfo::SetDefaultMark(bool m)
{
	defmark = m;
	defmarkset = true;
	if (defptset && deftsincset)
		defvaluesset = true;
}

inline void RTPLocalInfo::SetDefaultTimestampInc(unsigned long inc)
{
	deftsinc = inc;
	deftsincset = true;
	if (defptset && defmarkset)
		defvaluesset = true;
}

#endif // RTPLOCALINFO_H
