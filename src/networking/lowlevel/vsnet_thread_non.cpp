#include "vsnet_thread.h"
#include "vsnet_debug.h"

using namespace std;

#ifdef USE_NO_THREAD

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

#endif /* USE_NO_THREAD */

