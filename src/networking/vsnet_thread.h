#ifndef VSNET_THREAD_H
#define VSNET_THREAD_H

#define USE_PTHREAD

/*------------------------------------------------------------*
 * declaration of VSThread                                    *
 *------------------------------------------------------------*/

class VSThread
{
public:
    VSThread( bool detached );
    ~VSThread( );

    void start( );
    void exit( );
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
