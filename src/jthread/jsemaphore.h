/*
 * Addition to JThread
 * Adapted from SDL.
 */

#ifndef JSEMAPHORE_H
#define JSEMAPHORE_H

#ifdef WIN32
    #include <windows.h>
#else // using pthread
    #include <semaphore.h>
#endif // WIN32

#define JSEMAPHORE_TIMEOUT                  1
#define ERR_JSEMAPHORE_ALREADYINIT         -1
#define ERR_JSEMAPHORE_NOTINIT             -2
#define ERR_JSEMAPHORE_CANTCREATESEMAPHORE -3
#define ERR_JSEMAPHORE_WAITFAILED          -4
#define ERR_JSEMAPHORE_POSTFAILED          -5

#define JMUTEX_DEBUG

class JSemaphore
{
public:
    JSemaphore();
    ~JSemaphore();
    int Init( unsigned long initial_value );
    bool IsInitialized() const { return initialized; }

    int TryWait( );
    int Wait( );
    int Post( );

private:
#ifdef WIN32
    HANDLE                 semaphore;
#else
    sem_t                  semaphore;
#endif
    bool initialized;
};

#endif // JSEMAPHORE_H
