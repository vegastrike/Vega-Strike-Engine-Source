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

#include "rtpmcasttable.h"
#include "rtperror.h"

RTPMCastTable::RTPMCastTable()
{
	int i;
	
	for (i = 0 ; i < RTP_MCASTTABLE_HASHSIZE; i++)
		hashtable[i] = NULL;
}

RTPMCastTable::~RTPMCastTable()
{
	Clear();
}

void RTPMCastTable::Clear()
{
	RTPMCastTableEntry *tmp,*tmpnext;
	int i;
	
	for (i = 0 ; i < RTP_MCASTTABLE_HASHSIZE; i++)
	{
		tmp = hashtable[i];
		while (tmp != NULL)
		{
			tmpnext = tmp->next;
			RTP_DELETE(tmp);
			tmp = tmpnext;
		}
		hashtable[i] = NULL;
	}
}

int RTPMCastTable::AddMCastAddress(unsigned long mcastIP)
{
	int index;
	RTPMCastTableEntry *tmp,*tmpprev,*tmpnew;
	unsigned long mcastIP2;
	bool found;
	
	index = (int)(mcastIP%((unsigned long)RTP_MCASTTABLE_HASHSIZE));
	found = false;
	tmpprev = NULL;
	tmp = hashtable[index];
	
	while (!found && tmp != NULL)
	{
		mcastIP2 = tmp->GetMCastIP();
		if (mcastIP < mcastIP2)
			found = true;
		else
		{
			if (mcastIP == mcastIP2)
				return ERR_RTP_ALREADYINMCASTGROUP;
			
			tmpprev = tmp;
			tmp = tmp->next;
		}
	}
	
	tmpnew = RTP_NEW(RTPMCastTableEntry(mcastIP));
	if (tmpnew == NULL)
		return ERR_RTP_OUTOFMEM;
	
	tmpnew->next = tmp;
	if (tmpprev == NULL)
		hashtable[index] = tmpnew;
	else
		tmpprev->next = tmpnew;
	
	curpos = -1;
	curentry = NULL;	
	return 0;
}

int RTPMCastTable::DeleteMCastAddress(unsigned long mcastIP)
{
	int index;
	RTPMCastTableEntry *tmp,*tmpprev,*tmpnext;
	unsigned long mcastIP2;
	bool found;
	
	index = (int)(mcastIP%((unsigned long)RTP_MCASTTABLE_HASHSIZE));
	found = false;
	tmpprev = NULL;
	tmp = hashtable[index];
	
	while (!found && tmp != NULL)
	{
		mcastIP2 = tmp->GetMCastIP();
		if (mcastIP < mcastIP2)
			return ERR_RTP_NOTINMCASTGROUP;
		else
		{
			if (mcastIP == mcastIP2)
				found = true;
			else
			{
				tmpprev = tmp;
				tmp = tmp->next;
			}
		}
	}
	
	tmpnext = tmp->next;
	RTP_DELETE(tmp);
	
	if (tmpprev == NULL)
		hashtable[index] = tmpnext;
	else
		tmpprev->next = tmpnext;

	curpos = -1;
	curentry = NULL;	
	return 0;
}

void RTPMCastTable::GotoFirstMCastAddress()
{
	curpos = 0;
	while (curpos < RTP_MCASTTABLE_HASHSIZE && hashtable[curpos] == NULL)
		curpos++;
	if (curpos < RTP_MCASTTABLE_HASHSIZE)
		curentry = hashtable[curpos];
}

bool RTPMCastTable::GetNextMCastAddress(unsigned long *mcastIP)
{
	RTPMCastTableEntry *tmp;
	
	if (curpos >= RTP_MCASTTABLE_HASHSIZE)
		return false;
	
	if (curentry != NULL)
	{
		tmp = curentry;
		curentry = curentry->next;
	}
	else
	{
		curpos++;
		while (curpos < RTP_MCASTTABLE_HASHSIZE && hashtable[curpos] == NULL)
			curpos++;
			
		if (curpos >= RTP_MCASTTABLE_HASHSIZE)
			return false;
		tmp = hashtable[curpos];
		curentry = tmp->next;
	}
	*mcastIP = tmp->GetMCastIP();
	return true;
}
