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

#ifndef RTPDESTLIST_H

#define RTPDESTLIST_H

#include "rtpconfig.h"
#include "rtpdefines.h"
#include "rtpdebug.h"
#include <stdlib.h>

#define RTP_DESTLIST_HASHSIZE									256

struct RTPDestListMember
{
	unsigned long ip;
	int rtpport,rtcpport;
	struct RTPDestListMember *next,*prev;
	struct RTPDestListMember *hashnext;
};

class RTPDestList								RTPDEBUGBASE
{
public:
	RTPDestList();
	~RTPDestList();
	void InitRetrieval() { current = first; }
	inline bool GetNext(unsigned long *ip,int *rtpport,int *rtcpport);
	int Add(unsigned long ip,int portbase);
	int Delete(unsigned long ip,int portbase);
	void Clear();
private:
	RTPDestListMember *first,*current;
	RTPDestListMember *iphash[RTP_DESTLIST_HASHSIZE];
};

inline bool RTPDestList::GetNext(unsigned long *ip,int *rtpport,int *rtcpport)
{
	if (current == NULL)
		return false;

	*ip = current->ip;
	*rtpport = current->rtpport;
	*rtcpport = current->rtcpport;
	current = current->next;
	return true;
}

#endif // RTPDESTLIST_H
