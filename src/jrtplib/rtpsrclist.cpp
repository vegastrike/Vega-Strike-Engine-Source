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

#include "rtpsrclist.h"
#include "rtperror.h"

RTPSrcList::RTPSrcList()
{
	first = NULL;
	current = NULL;
}

RTPSrcList::~RTPSrcList()
{
	Clear();
}

int RTPSrcList::Add(RTPuint32 src)
{
	RTPSrcListMember *tmp;
	
	tmp = RTP_NEW(RTPSrcListMember);
	if (tmp == NULL)
		return ERR_RTP_OUTOFMEM;
		
	tmp->next = first;
	first = tmp;
	current = tmp;
	
	return 0;
}

int RTPSrcList::Delete(RTPuint32 src)
{
	RTPSrcListMember *tmp,*tmpprev;
	bool found;
	
	found = false;
	tmp = first;
	tmpprev = NULL;
	while (!found && tmp != NULL)
	{
		if (tmp->src == src)
			found = true;
		else
		{
			tmpprev = tmp;
			tmp = tmp->next;
		}
	}
	if (!found)
		return ERR_RTP_SSRCNOTFOUND;
		
	if (tmpprev == NULL)
		first = first->next;
	else
		tmpprev->next = tmp->next;
	RTP_DELETE(tmp);
	current = first;

	return 0;
}

void RTPSrcList::Clear()
{
	while (first != NULL)
	{
		current = first->next;
		RTP_DELETE(first);
		first = current;
	}
	current = NULL;
}


