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

#ifndef JMUTEX_H

#define JMUTEX_H

#ifdef WIN32
    #include <windows.h>
#else // using pthread
    #include <pthread.h>
#endif // WIN32

#define ERR_JMUTEX_ALREADYINIT     -1
#define ERR_JMUTEX_NOTINIT         -2
#define ERR_JMUTEX_CANTCREATEMUTEX -3

#define JMUTEX_DEBUG

class JMutex
{
public:
    JMutex();
    ~JMutex();
    int Init();
    int Lock();
    int Unlock();
    bool IsInitialized() const { return initialized; }

private:
#ifdef WIN32
    HANDLE mutex;
#elif defined(linux) || defined(_AIX)
    pthread_mutexattr_t attr;
    pthread_mutex_t     mutex;
    friend class JCond;
#else
    pthread_mutex_t     mutex;
    friend class JCond;
#endif
    bool initialized;
};

#endif // JMUTEX_H
