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

#ifdef RTPDEBUG

#include "rtpdebug.h"
#include "rtpdefines.h"

RTPDebug rtpdebug(true);

char RTPDebug::fname[256];
int RTPDebug::line = 0;
RTPDebugMemoryBlock *RTPDebug::first = NULL;
FILE *RTPDebug::dbgfile = NULL;

RTPDebug::RTPDebug()
{
	log = false;
}

RTPDebug::RTPDebug(bool l)
{
	log = l;
	if (log && dbgfile == NULL)
		dbgfile = fopen("rtpdebugfile.txt","wt");
}

RTPDebug::~RTPDebug()
{
	if (log)
	{
		if (first != NULL)
			DumpLeaks();
		else
		{
			if (dbgfile)
				fprintf(dbgfile,"No memory leaks found\n");
		}
		if (dbgfile)
			fclose(dbgfile);
	}
}

void RTPDebug::AddBlock(void *ptr,int size)
{
	RTPDebugMemoryBlock *tmp;

	if (ptr == NULL)
	{
		fprintf(stderr,"OUT OF MEMORY");
		exit(0);
	}
	tmp = (struct RTPDebugMemoryBlock *)malloc(sizeof(RTPDebugMemoryBlock));
	if (tmp == NULL)	
	{
		fprintf(stderr,"OUT OF MEMORY");
		exit(0);
	}
	tmp->ptr = ptr;
	tmp->size = size;
	strcpy(tmp->filename,fname);
	tmp->linenr = line;	
	tmp->next = first;
	first = tmp;
	printf("Added block %p (%d bytes) from file %s, line %d\n",ptr,size,fname,line);
}

void RTPDebug::DeleteBlock(void *ptr)
{
	RTPDebugMemoryBlock *tmp,*tmpprev;
	bool found;
	
	tmpprev = NULL;
	tmp = first;
	found = false;
	while (tmp != NULL && !found)
	{
		if (tmp->ptr == ptr)
			found = true;
		else
		{
			tmpprev = tmp;
			tmp = tmp->next;
		}
	}
	
	if (!found)
	{
		if (dbgfile)
			fprintf(dbgfile,"Deleting unallocated block %p from file %s at line %d\n",dbgfile,fname,line);
	}
	else
	{
		if (tmpprev == NULL)
			first = first->next;
		else
			tmpprev->next = tmp->next;
		free(tmp);
	}
}

void RTPDebug::DumpLeaks()
{
	RTPDebugMemoryBlock *tmp;
	int count;

	count = 0;
	while (first != NULL)
	{
		tmp = first->next;
		if (dbgfile)
			fprintf(dbgfile,"Block %p (%d bytes) not freed. Allocated from file %s at line %d\n",first->ptr,first->size,first->filename,first->linenr);
		free(first);
		first = tmp;
		count++;
	}
	if (dbgfile)
		fprintf(dbgfile,"Found %d memory leaks\n",count);
}

void *RTPDebug::operator new(size_t size)
{
	void *p;
	
	p = malloc(size);
	rtpdebug.AddBlock(p,size);
	return p;
}

void *RTPDebug::operator new[](size_t size)
{
	void *p;
	
	p = malloc(size);
	rtpdebug.AddBlock(p,size);
	return p;
}

void RTPDebug::operator delete(void *p)
{
	rtpdebug.DeleteBlock(p);
	free(p);
}

void RTPDebug::operator delete[](void *p)
{
	rtpdebug.DeleteBlock(p);
	free(p);
}

#endif /* RTPDEBUG */

