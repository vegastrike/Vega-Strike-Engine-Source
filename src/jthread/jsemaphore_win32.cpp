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
        CloseHandle( semaphore );
}

int JSemaphore::Init( unsigned long initial_value )
{
    if( initialized )
        return ERR_JSEMAPHORE_ALREADYINIT;

    semaphore = CreateSemaphore(NULL, initial_value, 32*1024, NULL);
    if( semaphore == 0 )
        return ERR_JSEMAPHORE_CANTCREATESEMAPHORE;
    initialized = true;
    return 0;
}

int JSemaphore::TryWait( )
{
    if( !initialized )
        return ERR_JSEMAPHORE_NOTINIT;

    switch( WaitForSingleObject( semaphore, 0 ) )
    {
    case WAIT_OBJECT_0 :
        return 0;
        break;
    case WAIT_TIMEOUT :
        return JSEMAPHORE_TIMEOUT;
        break;
    default :
        return ERR_JSEMAPHORE_WAITFAILED;
        break;
    }
}

int JSemaphore::Wait( )
{
    if( !initialized )
        return ERR_JSEMAPHORE_NOTINIT;

    switch( WaitForSingleObject( semaphore, INFINITE ) )
    {
    case WAIT_OBJECT_0 :
        return 0;
        break;
    case WAIT_TIMEOUT :
        return JSEMAPHORE_TIMEOUT;
        break;
    default :
        return ERR_JSEMAPHORE_WAITFAILED;
        break;
    }
}

int JSemaphore::Post( )
{
    if( !initialized )
        return ERR_JSEMAPHORE_NOTINIT;

    if( ReleaseSemaphore( semaphore, 1, NULL ) == FALSE )
        return ERR_JSEMAPHORE_POSTFAILED;

    return 0;
}

