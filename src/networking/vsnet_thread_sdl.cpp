#include <config.h>

#ifdef USE_SDL_THREADS

#include <SDL.h>
#include <SDL_thread.h>

/*------------------------------------------------------------*
 * definition of VSThread                                     *
 *------------------------------------------------------------*/

void VSThread::init( )
{
}

static void* vs_thread_start( void* info )
{
    ((VSThread*)info)->run();
    return NULL;
}

struct Private
{
    bool        detach;
    SDL_Thread* t;
};

VSThread::VSThread( bool detached )
{
    _internal         = new Private;
    _internal->detach = detached;
    _internal->t      = NULL;
}

VSThread::~VSThread( )
{
    delete _internal;
}

void VSThread::start( )
{
    _internal->t = SDL_CreateThread( &vs_thread_start, this );
}

void VSThread::join( )
{
    SDL_WaitThread( _internal->t, NULL );
}

/*------------------------------------------------------------*
 * definition of VSMutex                                      *
 *------------------------------------------------------------*/

struct VSMutex::Private
{
    SDL_mutex* mx;
};

VSMutex::VSMutex( )
{
    _internal = new Private;
    _internal->mx = SDL_CreateMutex( );
}

VSMutex::~VSMutex( )
{
    SDL_DestroyMutex( _internal->mx );
    delete _internal;
}

void VSMutex::lock( )
{
    if( SDL_mutexP( _internal->mx ) == -1 )
    {
        cerr << "Can't lock SDL mutex" << endl;
    }
}

void VSMutex::unlock( )
{
    if( SDL_mutexV( _internal->mx ) == -1 )
    {
        cerr << "Can't unlock SDL mutex" << endl;
    }
}

/*------------------------------------------------------------*
 * definition of VSCond                                       *
 *------------------------------------------------------------*/

struct VSCond::Private
{
    SDL_cond* cond;
};

VSCond::VSCond( )
{
    _internal = new Private;
    _internal->cond = SDL_CreateCond( );
}

VSCond::~VSCond( )
{
    SDL_DestroyCond( _internal->cond );
    delete _internal;
}

void VSCond::wait( VSMutex& mx )
{
    if( SDL_CondWait( _internal->cond, mx->_internal.mx ) == -1 )
    {
        cerr << "Can't wait for SDL condition variable" << endl;
    }
}

void VSCond::signal( )
{
    if( SDL_CondSignal( _internal->cond ) == -1 )
    {
        cerr << "Can't signal SDL condition variable" << endl;
    }
}

void VSCond::broadcast( )
{
    if( SDL_CondBroadcast( _internal->cond ) == -1 )
    {
        cerr << "Can't broadcast to SDL condition variable" << endl;
    }
}

#endif /* USE_SDL_THREADS */

