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

#include "rtprandom.h"
#include <time.h>

RTPRandom::RTPRandom()
{
	numcalls = 0;
}

RTPRandom::~RTPRandom()
{
}

RTPuint32 RTPRandom::RandomUInt32()
{
	unsigned int seed;
	unsigned char byte;
	RTPuint32 retval;

	retval = 0;
	if (numcalls == 0)
	{
		numcalls++;
		seed = time(NULL);
		seed *= numcalls;
		seed -= clock();
		seed += (unsigned int)this;
		srand(seed);
	}
	byte = RandomByte();
	retval |= ((RTPuint32)byte);
	byte = RandomByte();
	retval |= (((RTPuint32)byte)<<8);
	byte = RandomByte();
	retval |= (((RTPuint32)byte)<<16);
	byte = RandomByte();
	retval |= (((RTPuint32)byte)<<24);

	return retval;
}

RTPuint16 RTPRandom::RandomUInt16()
{
	unsigned int seed;
	unsigned char byte;
	RTPuint16 retval;

	retval = 0;
	numcalls++;
	seed = time(NULL);
	seed += numcalls;
	seed += clock();
	srand(seed);
	byte = RandomByte();
	retval |= ((RTPuint32)byte);
	byte = RandomByte();
	retval |= (((RTPuint32)byte)<<8);

	return retval;
}

