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

#ifndef RTPMCASTTABLE_H

#define RTPMCASTTABLE_H

#include "rtpconfig.h"
#include "rtpdefines.h"
#include "rtpdebug.h"
#include <stdlib.h>

#define RTP_MCASTTABLE_HASHSIZE 						1024

class RTPMCastTableEntry
{
public:
	RTPMCastTableEntry(unsigned long mcastIP) { ip = mcastIP; next = NULL; }
	unsigned long GetMCastIP() { return ip; }
private:
	unsigned long ip;
public:
	RTPMCastTableEntry *next;
};

class RTPMCastTable								RTPDEBUGBASE
{
public:
	RTPMCastTable();
	~RTPMCastTable();
	int AddMCastAddress(unsigned long mcastIP);
	int DeleteMCastAddress(unsigned long mcastIP);
	void GotoFirstMCastAddress();
	bool GetNextMCastAddress(unsigned long *mcastIP);
	void Clear();
private:
	RTPMCastTableEntry *hashtable[RTP_MCASTTABLE_HASHSIZE];
	RTPMCastTableEntry *curentry;
	int curpos;
};

#endif // RTPMCASTTABLE_H

