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

#ifndef RTPDEBUG_H

#define RTPDEBUG_H

#include "rtpconfig.h"

#ifdef RTPDEBUG

#define RTPDEBUGBASE	: public RTPDebug

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct RTPDebugMemoryBlock
{
	void *ptr;
	int size;
	char filename[256];
	int linenr;
	RTPDebugMemoryBlock *next;
};

class RTPDebug
{
public:
	RTPDebug();
	RTPDebug(bool log);
	~RTPDebug();
	void AddBlock(void *ptr,int size);
	void DeleteBlock(void *ptr);
	void SetFile(char f[]) { strcpy(fname,f); }
	void SetLine(int l) { line = l; }
	void *operator new(size_t size);
	void *operator new[](size_t size);
	void operator delete(void *p);
	void operator delete[](void *p);
private:
	void DumpLeaks();

	static char fname[256];
	static int line;
	static RTPDebugMemoryBlock *first;
	static FILE *dbgfile;
	bool log;
};

extern RTPDebug rtpdebug;

#define RTP_NEW(type)				(rtpdebug.SetFile(__FILE__),rtpdebug.SetLine(__LINE__),new type)
#define RTP_DELETE(param)			(rtpdebug.SetFile(__FILE__),rtpdebug.SetLine(__LINE__),delete param)
#define RTP_DELETE_ARRAY(param)			(rtpdebug.SetFile(__FILE__),rtpdebug.SetLine(__LINE__),delete [] param)

#else /* Not debug version */

#define RTPDEBUGBASE

#define RTP_NEW(type)				new type
#define RTP_DELETE(param)			delete param
#define RTP_DELETE_ARRAY(param)			delete [] param

#endif /* RTPDEBUG */

#endif /* RTPDEBUG_H */

