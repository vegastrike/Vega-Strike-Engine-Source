#ifndef JCOND_H

#define JCOND_H

#ifdef WIN32
    #include <windows.h>
    #include "jsemaphore.h"
#else // using pthread
    #include <pthread.h>
#endif // WIN32

#include "jmutex.h"

#define ERR_JCOND_ALREADYINIT     -1
#define ERR_JCOND_NOTINIT         -2
#define ERR_JCOND_CANTCREATECOND  -3
#define ERR_JCOND_SIGNALFAILED    -4
#define ERR_JCOND_WAITFAILED      -5

class JCond
{
public:
    JCond();
    ~JCond();
    int  Init();
    bool IsInitialized() const { return initialized; }

    int Signal();
    int Broadcast();
    int Wait( JMutex& mx );

private:
#ifdef WIN32
    JMutex      lock;
    int         waiting;
    int         signals;
    JSemaphore  wait_sem;
    JSemaphore  wait_done;
#else
    pthread_cond_t     cond;
#endif
    bool initialized;
};

#endif // JCOND_H
