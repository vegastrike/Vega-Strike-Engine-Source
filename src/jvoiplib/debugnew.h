#ifndef DEBUGNEW_H

#define DEBUGNEW_H

#ifdef USE_DEBUG_NEW
	#include <sys/types.h>

	void *operator new(size_t s,char filename[],int line);
	void *operator new[](size_t s,char filename[],int line);

	#define new new (__FILE__,__LINE__)

#endif // USE_DEBUG_NEW

#endif // DEBUGNEW_H
