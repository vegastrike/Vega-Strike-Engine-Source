#include <config.h>

#include "vsnet_thread.h"
#ifdef USE_JTHREAD

#include <iostream>
using namespace std;

#include "vsnet_debug.h"
#include "jthread.h"
#include "jmutex.h"
#include "jcond.h"

/*------------------------------------------------------------*
 * definition of VSThread                                     *
 *------------------------------------------------------------*/

void VSThread::init( )
{
}

struct VSThread::Private : public JThread
{
    VSThread* _t;
public:
    Private( VSThread* t ) : _t(t) { }

    virtual void* Thread( ) {
        _t->run( );
        return NULL;
    }
};

VSThread::VSThread( bool /*detached*/ )
{
    _internal = new Private( this );
}

VSThread::~VSThread( )
{
    delete _internal;
}

void VSThread::start( )
{
    _internal->Start( );
}

void VSThread::join( )
{
}

/*------------------------------------------------------------*
 * definition of VSMutex                                      *
 *------------------------------------------------------------*/

struct VSMutex::Private : public JMutex
{
};

VSMutex::VSMutex( )
{
    _internal = new Private;
    _internal->Init( );
}

VSMutex::~VSMutex( )
{
    delete _internal;
}

void VSMutex::lock( )
{
    if( _internal->Lock( ) != 0 )
    {
        cerr << "Can't lock JMutex" << endl;
    }
}

void VSMutex::unlock( )
{
    if( _internal->Unlock( ) != 0 )
    {
        cerr << "Can't unlock JMutex" << endl;
    }
}

/*------------------------------------------------------------*
 * definition of VSCond                                       *
 *------------------------------------------------------------*/

struct VSCond::Private : public JCond
{
};

VSCond::VSCond( )
{
    _internal = new Private;
    _internal->Init( );
}

VSCond::~VSCond( )
{
    delete _internal;
}

void VSCond::wait( VSMutex& mx )
{
    if( _internal->Wait( *mx._internal ) != 0 )
    {
        cerr << "Can't wait for JCond condition variable" << endl;
    }
}

void VSCond::signal( )
{
    if( _internal->Signal( ) != 0 )
    {
        cerr << "Can't signal JCond condition variable" << endl;
    }
}

void VSCond::broadcast( )
{
    if( _internal->Broadcast( ) != 0 )
    {
        cerr << "Can't broadcast to JCond condition variable" << endl;
    }
}

#endif /* USE_JTHREAD */

