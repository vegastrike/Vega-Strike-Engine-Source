#include "jcond.h"

JCond::JCond()
{
    initialized = false;
}

JCond::~JCond()
{
    if( initialized )
        pthread_cond_destroy( &cond );
}

int  JCond::Init()
{
    if( initialized )
        return ERR_JCOND_ALREADYINIT;

    if( pthread_cond_init( &cond, NULL ) )
        return ERR_JCOND_CANTCREATECOND;

    initialized = true;
    return 0;
}

int JCond::Signal()
{
    if( !initialized )
        return ERR_JCOND_NOTINIT;

    if( pthread_cond_signal( &cond ) < 0 )
        return ERR_JCOND_SIGNALFAILED;

    return 0;
}

int JCond::Broadcast()
{
    if( !initialized )
        return ERR_JCOND_NOTINIT;

    if( pthread_cond_broadcast( &cond ) < 0 )
        return ERR_JCOND_SIGNALFAILED;

    return 0;
}

int JCond::Wait( JMutex& mx )
{
    if( !initialized )
        return ERR_JCOND_NOTINIT;

    if( pthread_cond_wait( &cond, &mx.mutex ) < 0 )
        return ERR_JCOND_WAITFAILED;

    return 0;
}

