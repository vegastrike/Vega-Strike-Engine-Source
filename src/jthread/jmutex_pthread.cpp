/*

    This file is a part of the JThread package, which contains some object-
    oriented thread wrappers for different thread implementations.

    Copyright (c) 2000-2001  Jori Liesenborgs (jori@lumumba.luc.ac.be)

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.

*/

#include "jmutex.h"

JMutex::JMutex()
{
	initialized = false;
}

JMutex::~JMutex()
{
	if (initialized)
    {
		::pthread_mutex_destroy(&mutex);
#if defined(linux) || defined(_AIX)
		::pthread_mutexattr_destroy(&attr);
#endif
    }
}

int JMutex::Init()
{
	if (initialized)
		return ERR_JMUTEX_ALREADYINIT;
/*
#ifndef PTHREAD_MUTEX_NORMAL
	const int PTHREAD_MUTEX_NORMAL = PTHREAD_MUTEX_TIMED_NP;
#endif
#ifndef PTHREAD_MUTEX_ERRORCHECK
	const int PTHREAD_MUTEX_ERRORCHECK = PTHREAD_MUTEX_ERRORCHECK_NP;
#endif
*/
#if defined(linux) || defined(_AIX)
  #if defined(JMUTEX_DEBUG)
	::pthread_mutexattr_init(&attr);
#ifdef __USE_UNIX98
    ::pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_ERRORCHECK );
#endif
    ::pthread_mutexattr_setpshared( &attr, PTHREAD_PROCESS_PRIVATE );
	::pthread_mutex_init(&mutex,&attr);
  #else
	::pthread_mutexattr_init(&attr);
#ifdef __USE_UNIX98	
    ::pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_NORMAL );
#endif
    ::pthread_mutexattr_setpshared( &attr, PTHREAD_PROCESS_PRIVATE );
	::pthread_mutex_init(&mutex,&attr);
  #endif
#else
	::pthread_mutex_init(&mutex,NULL);
#endif
	initialized = true;
	return 0;	
}

int JMutex::Lock()
{
	if (!initialized)
		return ERR_JMUTEX_NOTINIT;
		
	::pthread_mutex_lock(&mutex);
	return 0;
}

int JMutex::Unlock()
{
	if (!initialized)
		return ERR_JMUTEX_NOTINIT;
	
	::pthread_mutex_unlock(&mutex);
	return 0;
}

