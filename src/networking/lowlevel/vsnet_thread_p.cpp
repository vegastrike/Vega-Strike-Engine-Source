#include "vsnet_thread.h"

#ifdef USE_PTHREAD

#include "vsnet_debug.h"

#include <pthread.h>
#include <stdio.h>

/*------------------------------------------------------------*
 * definition of VSThread                                     *
 *------------------------------------------------------------*/

void VSThread::init( )
{
    // nothing to do
}

static void* vs_thread_start( void* info )
{
    ((VSThread*)info)->run();
    return NULL;
}

struct VSThread::Private
{
    bool           detach;
    pthread_attr_t ta;
    pthread_t      t;
};

VSThread::VSThread( bool detached )
{
    _internal = new Private;
    _internal->detach = detached;
#if defined(_AIX)
    ::pthread_init( &_internal->t );
#endif
    ::pthread_attr_init( &_internal->ta );
}

VSThread::~VSThread( )
{
    ::pthread_attr_destroy( &_internal->ta );
    delete _internal;
}

void VSThread::start( )
{
    int ret;
    int err;

    if( _internal->detach )
    {
        ret = ::pthread_attr_setdetachstate( &_internal->ta,
                                               PTHREAD_CREATE_DETACHED );
        if( ret != 0 )
        {
            perror( "pthread_attr_setdetachstate failed" );
        }
    }
#if !defined(_AIX)
    ret = ::pthread_attr_setscope( &_internal->ta, PTHREAD_SCOPE_SYSTEM );
    if( ret != 0 )
    {
        perror( "pthread_attr_setscope failed" );
    }
    // signal( SIGRTMIN, SIG_DFL );
#endif

#if defined(_AIX) && defined(_AIX32_THREADS)
    // obsolete DCE thread API on AIX
    err = ::pthread_create( &_internal->t,
                            _internal->ta,
                            vs_thread_start,
                            (void*)this );
#else
    err = ::pthread_create( &_internal->t,
                            &_internal->ta,
                            vs_thread_start,
                            (void*)this );
#endif
    if ( err != 0 )
    {
        perror( "pthread_create failed" );
        // return false;
    }
    // return true;
}

void VSThread::join( )
{
    if( _internal->detach == false )
    {
        ::pthread_join( _internal->t, NULL );
    }
}

/*------------------------------------------------------------*
 * definition of VSMutex                                      *
 *------------------------------------------------------------*/

struct VSMutex::Private
{
#if defined(linux) || defined(_AIX)
    pthread_mutexattr_t attr;
#endif
    pthread_mutex_t     lck;
};

VSMutex::VSMutex( )
{
#if defined(linux) || defined(_AIX)
    _internal = new Private;
    int ret = ::pthread_mutexattr_init( &_internal->attr );
    if( ret != 0 )
    {
        perror("pthread_mutexattr_init failed (ignored)");
    	ret = ::pthread_mutex_init( &_internal->lck, NULL );
    }
    else
    {
        /* PTHREAD_MUTEX_DEFAULT    - undefined behaviour
         * PTHREAD_MUTEX_NORMAL     - hang on double lock
         * PTHREAD_MUTEX_ERRORCHECK - return an error on double lock
         * PTHREAD_MUTEX_RECURSIVE  - allow recursive locking (Not Windows
         *                            semantics, this is a counter!)
         */
    #ifdef VSNET_DEBUG
        ::pthread_mutexattr_settype( &_internal->attr, PTHREAD_MUTEX_ERRORCHECK );
    #else
        ::pthread_mutexattr_settype( &_internal->attr, PTHREAD_MUTEX_NORMAL );
    #endif

        /* PTHREAD_PROCESS_PRIVATE - the default, mutex is not shared with
         *                           another process
         * PTHREAD_PROCESS_SHARED  - mutex may be shared with another process
         */
        ::pthread_mutexattr_setpshared( &_internal->attr, PTHREAD_PROCESS_PRIVATE );

        ret = ::pthread_mutex_init ( &_internal->lck, &_internal->attr );
    }
    if( ret != 0 )
    {
        perror("pthread_mutex_init failed");
    }
#else /*linux||_AIX*/
    _internal = new Private;
    int ret = ::pthread_mutex_init( &_internal->lck, NULL );
    if( ret != 0 )
    {
        perror("pthread_mutex_init failed");
    }
#endif /*linux||_AIX*/
}

VSMutex::~VSMutex( )
{
    int ret = ::pthread_mutex_destroy( &_internal->lck );
    if( ret != 0 )
    {
        perror("pthread_mutex_destroy failed");
    }
#if defined(linux) || defined(_AIX)
    ret = ::pthread_mutexattr_destroy( &_internal->attr );
    if( ret != 0 )
    {
        perror("pthread_mutexattr_destroy failed");
    }
#endif
    delete _internal;
}

void VSMutex::lock( )
{
    int ret = ::pthread_mutex_lock( &_internal->lck );
    if ( ret != 0 )
    {
        perror("pthread_mutex_lock failed");
    }
}

void VSMutex::unlock( )
{
    int ret = ::pthread_mutex_unlock( &_internal->lck );
    if ( ret != 0 )
    {
        perror("pthread_mutex_unlock failed");
    }
}

/*------------------------------------------------------------*
 * definition of VSCond                                       *
 *------------------------------------------------------------*/

struct VSCond::Private
{
    pthread_condattr_t attr;
    pthread_cond_t     ctr;
};

VSCond::VSCond( )
{
    _internal = new Private;
    int ret = ::pthread_condattr_init( &_internal->attr );
    if( ret != 0 )
    {
        perror( "pthread_condattr_init failed" );
    }
    else
    {
        ret = ::pthread_cond_init( &_internal->ctr, &_internal->attr );
        if( ret != 0 )
        {
            perror( "pthread_cond_init failed" );
        }
    }
}

VSCond::~VSCond( )
{
    int ret = ::pthread_cond_destroy( &_internal->ctr );
    if( ret != 0 )
    {
        perror( "pthread_cond_destroy failed" );
    }
    ret = ::pthread_condattr_destroy( &_internal->attr );
    if( ret != 0 )
    {
        perror( "pthread_condattr_destroy failed" );
    }
    delete _internal;
}

void VSCond::wait( VSMutex& mx )
{
    int ret = ::pthread_cond_wait( &_internal->ctr, &mx._internal->lck );
    if( ret != 0 )
    {
        perror( "pthread_cond_wait failed" );
    }
}

void VSCond::signal( )
{
    int ret = ::pthread_cond_signal( &_internal->ctr );
    if( ret != 0 )
    {
        perror( "pthread_cond_signal failed" );
    }
}

void VSCond::broadcast( )
{
    int ret = ::pthread_cond_broadcast( &_internal->ctr );
    if( ret != 0 )
    {
        perror( "pthread_cond_broadcast failed" );
    }
}

#endif /* USE_PTHREAD */

