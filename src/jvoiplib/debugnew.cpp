#ifdef USE_DEBUG_NEW

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct MemoryInfo
{
	void *ptr;
	size_t size;
	int lineno;
	char *filename;
	
	MemoryInfo *next;
};

class MemoryTracker
{
public:
	MemoryTracker() { firstblock = NULL; }
	~MemoryTracker()
	{
		MemoryInfo *tmp;
		int count = 0;
		
		printf("Checking for memory leaks...\n");fflush(stdout);
		while(firstblock)
		{
			count++;
			printf("Unfreed block %p of %d bytes (file '%s', line %d)\n",firstblock->ptr,firstblock->size,firstblock->filename,firstblock->lineno);;
			
			tmp = firstblock->next;
			
			free(firstblock->ptr);
			if (firstblock->filename)
				free(firstblock->filename);
			free(firstblock);
			firstblock = tmp;
		}
		if (count == 0)
			printf("No memory leaks found\n");
		else
			printf("%d leaks found\n",count);
	}
	
	MemoryInfo *firstblock;	
};

static MemoryTracker memtrack;

void *donew(size_t s,char filename[],int line)
{	
	void *p;
	MemoryInfo *meminf;
	
	p = malloc(s);
	meminf = (MemoryInfo *)malloc(sizeof(MemoryInfo));
	
	meminf->ptr = p;
	meminf->size = s;
	meminf->lineno = line;
	meminf->filename = (char *)malloc(strlen(filename)+1);
	strcpy(meminf->filename,filename);
	meminf->next = memtrack.firstblock;
	
	memtrack.firstblock = meminf;
	
	return p;
}

void dodelete(void *p)
{
	MemoryInfo *tmp,*tmpprev;
	bool found;
	
	tmpprev = NULL;
	tmp = memtrack.firstblock;
	found = false;
	while (tmp != NULL && !found)
	{
		if (tmp->ptr == p)
			found = true;
		else
		{
			tmpprev = tmp;
			tmp = tmp->next;
		}
	}
	if (!found)
	{
		printf("Couldn't free block %p!\n",p);
		fflush(stdout);
	}
	else
	{
		MemoryInfo *n;
		
		fflush(stdout);
		n = tmp->next;
		free(tmp->ptr);
		if (tmp->filename)
			free(tmp->filename);
		free(tmp);
		
		if (tmpprev)
			tmpprev->next = n;
		else
			memtrack.firstblock = n;
	}
}

void *operator new(size_t s)
{
	return donew(s,"UNKNOWN FILE",0);
}

void *operator new[](size_t s)
{
	return donew(s,"UNKNOWN FILE",0);
}

void *operator new(size_t s,char filename[],int line)
{
	return donew(s,filename,line);
}

void *operator new[](size_t s,char filename[],int line)
{
	return donew(s,filename,line);
}

void operator delete(void *p)
{
	dodelete(p);
}

void operator delete[](void *p)
{
	dodelete(p);
}

#endif // USE_DEBUG_NEW
