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

#include "rtpsourcedescription.h"
#include "rtperror.h"
#include <stdlib.h>
#include <string.h>

RTPSourceDescription::RTPSourceDescription()
{
	int i;

	src = 0;
	next = NULL;
	for (i = 0 ; i < RTP_NUM_SDES_INDICES ; i++)
	{
		sdesinfo[i] = NULL;
		sdesinfolen[i] = 0;
	}
	cnameset = false;
}

RTPSourceDescription::RTPSourceDescription(RTPuint32 s)
{
	int i;

	src = htonl(s);
	next = NULL;
	for (i = 0 ; i < RTP_NUM_SDES_INDICES ; i++)
	{
		sdesinfo[i] = NULL;
		sdesinfolen[i] = 0;
	}
}

RTPSourceDescription::~RTPSourceDescription()
{
	Clear();
}

int RTPSourceDescription::SetSDES(int index,char *s,int len)
{
	int len2;
	unsigned char *tmp;

	if (index < 0 || index >= RTP_NUM_SDES_INDICES)
		return ERR_RTP_SDESINDEXOUTOFRANGE;

	len2 = (len<255)?len:255;
	if (len2 < 0)
		len2 = 0;
	if (len2 == 0 || s == NULL)
	{
		if (sdesinfo[index] != NULL)
			RTP_DELETE_ARRAY(sdesinfo[index]);
		sdesinfolen[index] = 0;
		sdesinfo[index] = NULL;
	}
	else
	{
		tmp = RTP_NEW(unsigned char[len2]);
		if (tmp == NULL)
			return ERR_RTP_OUTOFMEM;
		memcpy(tmp,s,len2);
		if (sdesinfo[index] != NULL)
			RTP_DELETE_ARRAY(sdesinfo[index]);
		sdesinfo[index] = tmp;
		sdesinfolen[index] = len2;
	}
	if (index == TYPE_SDES_CNAME-1)
		cnameset = true;
	return 0;
}

void RTPSourceDescription::Clear()
{
	int i;

	for (i = 0 ; i < RTP_NUM_SDES_INDICES ; i++)
	{
		if (sdesinfo[i] != NULL)
			RTP_DELETE_ARRAY(sdesinfo[i]);
		sdesinfo[i] = NULL;
		sdesinfolen[i] = 0;
	}
	src = 0;
	cnameset = false;
}
