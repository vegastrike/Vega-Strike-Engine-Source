#ifndef VSNET_THREAD_H
#define VSNET_THREAD_H

/* Choose an implementation that you like. Only that one will be
 * compiled. Later, the decision should be made by configure.
 */
#undef  USE_PTHREAD
#undef  USE_SDL_THREAD
#define USE_NO_THREAD

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
