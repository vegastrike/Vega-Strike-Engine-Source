/*
 * Addition to JThread
 * Adapted from SDL.
 */

#include "jsemaphore.h"

JSemaphore::JSemaphore( )
{
    initialized = false;
}

JSemaphore::~JSemaphore( )
{
    if( initialized )
    	sem_destroy( &semaphore );
}

int JSemaphore::Init( unsigned long initial_value )
{
    if( initialized )
        return ERR_JSEMAPHORE_ALREADYINIT;

    if( sem_init( &semaphore, 0, initial_value ) < 0 )
        return ERR_JSEMAPHORE_CANTCREATESEMAPHORE;

    initialized = true;
    return 0;
}

int JSemaphore::TryWait( )
{
    if( !initialized )
        return ERR_JSEMAPHORE_NOTINIT;

    if( sem_trywait( &semaphore ) < 0 )
        return JSEMAPHORE_TIMEOUT;

    return 0;
}

int JSemaphore::Wait( )
{
    if( !initialized )
        return ERR_JSEMAPHORE_NOTINIT;

    if( sem_wait( &semaphore ) < 0 )
        return ERR_JSEMAPHORE_WAITFAILED;

    return 0;
}

int JSemaphore::Post( )
{
    if( !initialized )
        return ERR_JSEMAPHORE_NOTINIT;

    if( sem_post( &semaphore ) < 0 )
        return ERR_JSEMAPHORE_POSTFAILED;

    return 0;
}

