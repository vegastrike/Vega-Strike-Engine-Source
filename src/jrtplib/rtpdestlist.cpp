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

#include "rtpdestlist.h"
#include "rtpdefines.h"
#include "rtperror.h"

/* Class implementation */

RTPDestList::RTPDestList()
{
	int i;

	first = NULL;
	current = NULL;
	for (i = 0 ; i < RTP_DESTLIST_HASHSIZE ; i++)
		iphash[i] = NULL;
}

RTPDestList::~RTPDestList()
{
	Clear();
}

int RTPDestList::Add(unsigned long ip,int portbase)
{
	RTPDestListMember *tmp;
	int index;

	tmp = RTP_NEW(RTPDestListMember);
	if (tmp == NULL)
		return ERR_RTP_OUTOFMEM;

	tmp->ip = htonl(ip);
	tmp->rtpport = htons(portbase); 
	tmp->rtcpport = htons(portbase+1);
	tmp->next = first;
	tmp->prev = NULL;
	if (first)
		first->prev = tmp;
	first = tmp;
	current = tmp;	

	index = (int)(ip%((unsigned long)RTP_DESTLIST_HASHSIZE));
	tmp->hashnext = iphash[index];
	iphash[index] = tmp;

	return 0;
}

int RTPDestList::Delete(unsigned long ip,int portbase)
{
	RTPDestListMember *tmp,*tmpprev,*tmpnext,*hashprev;
	bool found;
	unsigned long ip2;
	int rtpport,index;
	
	ip2 = htonl(ip);
	rtpport = htons(portbase);
	found = false;
	index = (int)(ip%((unsigned long)RTP_DESTLIST_HASHSIZE));
	tmp = iphash[index];
	hashprev = NULL;
	while (!found && tmp != NULL)
	{
		if (tmp->ip == ip2 && tmp->rtpport == rtpport)
			found = true;
		else
		{
			hashprev = tmp;
			tmp = tmp->hashnext;
		}
	}
	
	if (!found)
		return ERR_RTP_ENTRYNOTINDESTLIST;
	
	// remove from hashtable
	if (hashprev == NULL)
		iphash[index] = tmp->hashnext;
	else
		hashprev->hashnext = tmp->hashnext;

	// remove from linked list

	tmpprev = tmp->prev;
	tmpnext = tmp->next;

	if (tmpprev == NULL)
	{
		first = tmpnext;
		if (first)
			first->prev = NULL;
	}
	else
	{
		tmpprev->next = tmpnext;
		if (tmpnext)
			tmpnext->prev = tmpprev;
	}
	
	RTP_DELETE(tmp);
	current = first;	
	
	return 0;
}

void RTPDestList::Clear()
{
	int i;

	for (i = 0 ; i < RTP_DESTLIST_HASHSIZE ; i++)
		iphash[i] = NULL;

	while (first != NULL)
	{
		current = first;
		first = first->next;
		RTP_DELETE(current);
	}
	current = NULL;
}

