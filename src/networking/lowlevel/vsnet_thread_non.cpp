#include <config.h>

#if (!defined(USE_NET_THREAD_POSIX))&&(!defined(USE_NET_THREAD_SDL))
#include "vsnet_thread.h"

/*------------------------------------------------------------*
 * definition of VSThread                                     *
 *------------------------------------------------------------*/

void VSThread::init( )
{
}

VSThread::VSThread( bool )
{
}

VSThread::~VSThread( )
{
}

void VSThread::start( )
{
}

void VSThread::join( )
{
}

/*------------------------------------------------------------*
 * definition of VSMutex                                      *
 *------------------------------------------------------------*/

VSMutex::VSMutex( )
{
}

VSMutex::~VSMutex( )
{
}

void VSMutex::lock( )
{
}

void VSMutex::unlock( )
{
}

/*------------------------------------------------------------*
 * definition of VSCond                                       *
 *------------------------------------------------------------*/

VSCond::VSCond( )
{
}

VSCond::~VSCond( )
{
}

void VSCond::wait( VSMutex& mx )
{
}

void VSCond::signal( )
{
}

void VSCond::broadcast( )
{
}

#endif /* defined(USE_NET_THREAD_NONE) */

