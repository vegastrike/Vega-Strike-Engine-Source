#ifndef VSNET_THREAD_H
#define VSNET_THREAD_H

#include <config.h>

/* Choose an implementation that you like via configure. Only that
 * one will be compiled.
 */
#undef USE_PTHREAD
#undef USE_JTHREAD
#undef USE_NO_THREAD
#if defined(USE_NET_THREAD_POSIX)
#  define USE_PTHREAD
#elif defined(USE_NET_THREAD_J)
#  define USE_JTHREAD
#else
#  define USE_NO_THREAD
#endif

/*------------------------------------------------------------*
 * declaration of VSThread                                    *
 *------------------------------------------------------------*/

class VSThread
{
public:
    VSThread( bool detached );
    virtual ~VSThread( );

    void start( );
    void join( );

    virtual void run( ) = 0;

public:
    /// global initialization function for the thread subsystem
    static void init( );

private:
    struct Private;
    Private* _internal;
};

/*------------------------------------------------------------*
 * declaration of VSMutex                                     *
 *------------------------------------------------------------*/

class VSMutex
{
public:
    VSMutex( );
    ~VSMutex( );

    void lock( );
    void unlock( );

protected:
    struct Private;
    Private* _internal;

    friend class VSCond;
};

/*------------------------------------------------------------*
 * declaration of VSCond                                      *
 *------------------------------------------------------------*/

class VSCond
{
public:
    VSCond( );
    ~VSCond( );

    void wait( VSMutex& mx );
    void signal( );
    void broadcast( );

private:
    struct Private;
    Private* _internal;
};

#endif /* VSNET_THREAD_H */
