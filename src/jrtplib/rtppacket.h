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

#ifndef RTPPACKET_H

#define RTPPACKET_H

#include "rtpconfig.h"
#include "rtpdefines.h"
#include "rtpdebug.h"
#include "rtpstructs.h"

class RTPPacket									RTPDEBUGBASE
{
public:
	RTPPacket(RTPHeader *rtphdr,unsigned char *rawdat,unsigned char *payloadptr,int paylden);
	~RTPPacket();
	unsigned long GetSSRC() 			{ return syncsource; }
	unsigned long GetExtendedSequenceNumber() 	{ return extendedseqnum; }
	unsigned long GetTimeStamp() 			{ return timestamp; }
	int GetPayloadLength() 				{ return payloadlen; }
	unsigned char *GetRawPacket() 			{ return rawdata; }
	unsigned char *GetPayload() 			{ return payloaddata; }
	unsigned char GetPayloadType()			{ return payloadtype; }
	bool IsMarked()					{ return marked; }
	int GetNumberOfCSRCs()				{ return numcontribsources; }
	inline unsigned long GetCSRC(int pos);
	bool HasHeaderExtension()			{ return hashdrext; }
	unsigned short GetHeaderExtensionID()		{ return hdrextensionID; }
	int GetHeaderExtensionLength()			{ return hdrextlen; }
	unsigned char *GetHeaderExtensionData()		{ return hdrextdata; }
private:
	RTPuint16 seqnum;
	RTPuint32 extendedseqnum;
	RTPuint32 timestamp;
	RTPuint32 syncsource;
	RTPuint32 contribsources[15];
	int numcontribsources;
	bool marked;
	bool hashdrext;
	RTPuint16 hdrextensionID;
	int hdrextlen;
	unsigned char *hdrextdata;
	int payloadlen;
	unsigned char *rawdata;
	unsigned char *payloaddata; // payload data is a pointer into the rawdata block
	unsigned char payloadtype;
	RTPPacket *prev,*next;

friend class RTPSourceData;
friend class RTPSourceStats;
friend class RTPSources;
};

inline unsigned long RTPPacket::GetCSRC(int pos)
{
	if (pos < 0 || pos >= numcontribsources)
		return 0;
	return contribsources[pos];
}

#endif // RTPPACKET_H
