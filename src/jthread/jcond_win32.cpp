#include "jcond.h"

JCond::JCond()
{
    initialized = false;
}

JCond::~JCond()
{
}

int  JCond::Init()
{
    if( initialized )
        return ERR_JCOND_ALREADYINIT;

    waiting = 0;
    signals = 0;
    if( lock.Init( )!=0 || wait_sem.Init( 0)!=0 || wait_done.Init( 0)!=0 )
        return ERR_JCOND_CANTCREATECOND;

    initialized = true;
    return 0;
}

int JCond::Signal()
{
    if( !initialized )
        return ERR_JCOND_NOTINIT;

    lock.Lock( );
    if( waiting > signals )
    {
        signals++;
        wait_sem.Post( );
        lock.Unlock( );
        wait_done.Wait( );
    }
    else
    {
        lock.Unlock( );
    }

    return 0;
}

int JCond::Broadcast()
{
	int i=0;
    if( !initialized )
        return ERR_JCOND_NOTINIT;

    lock.Lock( );
    if( waiting > signals )
    {
        int num_waiting = waiting - signals;
        signals = waiting;
        for( i=0; i<num_waiting; i++ )
            wait_sem.Post( );
        lock.Unlock( );
        for( i=0; i<num_waiting; i++ )
            wait_done.Wait( );
    }
    else
    {
        lock.Unlock( );
    }

    return 0;
}

int JCond::Wait( JMutex& mx )
{
    if( !initialized )
        return ERR_JCOND_NOTINIT;

    lock.Lock( );
    waiting++;
    lock.Unlock( );

    mx.Unlock( );
    int ret = wait_sem.Wait( );
    lock.Lock( );
    if( signals > 0 )
    {
        if( ret != JSEMAPHORE_TIMEOUT )
            wait_sem.Wait( );
        wait_done.Post( );
        signals--;
    }
    waiting--;
    lock.Unlock( );
    mx.Lock( );

    return ret;
}

