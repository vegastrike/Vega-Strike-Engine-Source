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

#include "rtpiptable.h"
#include "rtperror.h"
#include <stdlib.h>

/* Definition of table member struct */

struct RTPIPTableMember
{
	unsigned long ip;
	bool allports;
	int port;
	RTPIPTableMember *next;
};

/* Class implementation */

RTPIPTable::RTPIPTable()
{
	int i;

	for (i = 0 ; i < RTP_IPTABLE_HASHSIZE ; i++)
		table[i] = NULL;
}

RTPIPTable::~RTPIPTable()
{
	Clear();
}

int RTPIPTable::Add(unsigned long ip,bool allports,int port)
{
	int index;
	RTPIPTableMember *tmp,*tmpprev,*tmpnext;
	bool done;

	if (allports)
		port = 0;
	else
	{
		if (port <= 0)
			return ERR_RTP_ILLEGALPORT;
	}

	index = (int)(ip%((unsigned long)RTP_IPTABLE_HASHSIZE));
	if (table[index] == NULL)
	{
		tmp = RTP_NEW(RTPIPTableMember);
		if (tmp == NULL)
			return ERR_RTP_OUTOFMEM;
		table[index] = tmp;
		tmp->next = NULL;
	}
	else // already entries in the hashtable
	{
		// sort on ip

		tmpprev = NULL;
		tmp = table[index];
		done = false;
		while (!done)
		{
			if (tmp == NULL)
				done = true;
			else
			{
				if (ip > tmp->ip)
				{
					tmpprev = tmp;
					tmp = tmp->next;
				}
				else
					done = true;
			}
		}
		if (tmp != NULL)
		{
			if (tmp->ip == ip) // same ip address
			{
				// sort on portnum			
	
				done = false;	
				while (!done)
				{
					if (tmp == NULL)
						done = true;
					else
					{
						if (port > tmp->port && ip == tmp->ip)
						{
							tmpprev = tmp;
							tmp = tmp->next;
						}
						else
							done = true;
					}
				}

				if (tmp != NULL && tmp->port == port && tmp->allports == allports && tmp->ip == ip)
					return ERR_RTP_IPTABLEENTRYEXISTS;
			}
		}

		if (tmp == NULL)
		{
			tmp = RTP_NEW(RTPIPTableMember);
			if (tmp == NULL)
				return ERR_RTP_OUTOFMEM;
			tmp->next = NULL;
			tmpprev->next = tmp;
		}
		else
		{
			if (tmpprev == NULL)
			{
				tmp = RTP_NEW(RTPIPTableMember);
				if (tmp == NULL)
					return ERR_RTP_OUTOFMEM;
				tmp->next = table[index];
				table[index] = tmp;
			}
			else
			{
				tmpnext = tmp;
				tmp = RTP_NEW(RTPIPTableMember);
				if (tmp == NULL)
					return ERR_RTP_OUTOFMEM;
				tmp->next = tmpnext;
				tmpprev->next = tmp;
			}
		}
	}
	tmp->allports = allports;
	tmp->ip = ip;
	tmp->port = port;
	return 0;
}

int RTPIPTable::Delete(unsigned long ip,bool allports,int port)
{
	int index;
	RTPIPTableMember *tmp,*tmpprev,*tmpnext;
	bool done;

	if (allports)
		port = 0;
	else
	{
		if (port <= 0)
			return ERR_RTP_ILLEGALPORT;
	}

	index = (int)(ip%((unsigned long)RTP_IPTABLE_HASHSIZE));
	if (table[index] == NULL)
		return ERR_RTP_IPTABLEENTRYDOESNTEXIST;

	/* Search for the right IP */
	
	tmpprev = NULL;
	tmp = table[index];
	done = false;
	while (!done)
	{
		if (tmp == NULL)
			done = true;
		else
		{
			if (ip > tmp->ip)
			{
				tmpprev = tmp;
				tmp = tmp->next;
			}
			else
				done = true;
		}
	}
	if (tmp == NULL)
		return ERR_RTP_IPTABLEENTRYDOESNTEXIST;

	if (tmp->ip != ip)
		return ERR_RTP_IPTABLEENTRYDOESNTEXIST;
	
	/* search for the right port */
	
	done = false;	
	while (!done)
	{
		if (tmp == NULL)
			done = true;
		else
		{
			if (port > tmp->port && ip == tmp->ip)
			{
				tmpprev = tmp;
				tmp = tmp->next;
			}
			else
				done = true;
		}
	}

	if (tmp == NULL)
		return ERR_RTP_IPTABLEENTRYDOESNTEXIST;
	if ( !(tmp->port == port && tmp->allports == allports && tmp->ip == ip))
		return ERR_RTP_IPTABLEENTRYDOESNTEXIST;

	tmpnext = tmp->next;
	if (tmpprev == NULL)
		table[index] = tmpnext;
	else
		tmpprev->next = tmpnext;
	RTP_DELETE(tmp);

	return 0;
}

bool RTPIPTable::Exists(unsigned long ip,bool allports,int port)
{
	int index;
	RTPIPTableMember *tmp;
	bool done;

	if (allports)
		port = 0;
	else
	{
		if (port <= 0)
			return false;
	}

	index = (int)(ip%((unsigned long)RTP_IPTABLE_HASHSIZE));
	if (table[index] == NULL)
		return false;

	/* Search for the right IP */
	
	tmp = table[index];
	done = false;
	while (!done)
	{
		if (tmp == NULL)
			done = true;
		else
		{
			if (ip > tmp->ip)
				tmp = tmp->next;
			else
				done = true;
		}
	}
	if (tmp == NULL)
		return false;

	if (tmp->ip != ip)
		return false;
	
	/* search for the right port */
	
	done = false;	
	while (!done)
	{
		if (tmp == NULL)
			done = true;
		else
		{
			if (port > tmp->port && ip == tmp->ip)
				tmp = tmp->next;
			else
				done = true;
		}
	}

	if (tmp == NULL)
		return false;
	if ( !(tmp->port == port && tmp->allports == allports && tmp->ip == ip))
		return false;
	return true;
}

void RTPIPTable::Clear()
{
	int i;
	RTPIPTableMember *tmp,*tmp2;

	for (i = 0 ; i < RTP_IPTABLE_HASHSIZE ; i++)
	{
		tmp = table[i];
		while (tmp != NULL)
		{
			tmp2 = tmp->next;
			RTP_DELETE(tmp);
			tmp = tmp2;
		}
		table[i] = NULL;
	}
}

