#ifndef VSNET_DLOADMGR_H
#define VSNET_DLOADMGR_H

#include <config.h>

#include <string>
#include <vector>
#include <list>
#include <queue>
#include <iostream>

#include "boost/shared_ptr.hpp"
#include "boost/shared_array.hpp"

#include "vsnet_headers.h"
#include "vsnet_debug.h"
#include "vsnet_thread.h"
#include "vsnet_socket.h"
#include "vsnet_socketset.h"
#include "netbuffer.h"

namespace VsnetDownload
{
/*------------------------------------------------------------*
 * declaration VsnetDownload::Subcommand
 *------------------------------------------------------------*/

enum Subcommand
{
    ResolveRequest,
    ResolveResponse,
    DownloadRequest,
    DownloadError,
    Download,
    DownloadFirstFragment,
    DownloadFragment,
    DownloadLastFragment,
    UnexpectedSubcommand
};

std::ostream& operator<<( std::ostream& ostr, Subcommand e );

namespace Client
{

class Manager;

/*------------------------------------------------------------*
 * declaration VsnetDownload::Client::State
 *------------------------------------------------------------*/

enum State
{
    Idle,
    Queued,
    Resolving,
    Resolved,
    Requested,
    FragmentReceived,
    Completed
};

std::ostream& operator<<( std::ostream& ostr, State s );

/*------------------------------------------------------------*
 * declaration VsnetDownload::Client::Error
 *------------------------------------------------------------*/

enum Error
{
    Ok,
    SocketError,
    FileNotFound,
    LocalPermissionDenied,
    RemotePermissionDenied,
    DownloadInterrupted
};

std::ostream& operator<<( std::ostream& ostr, Error e );

/*------------------------------------------------------------*
 * declaration VsnetDownload::Client::Notify
 *------------------------------------------------------------*/

class Notify
{
public:
    virtual void notify( State s, Error e ) = 0;
    virtual void setTotalBytes( int sz ) { }
    virtual void addBytes( int sz ) { }
};

typedef boost::shared_ptr<Notify> NotifyPtr;

/*------------------------------------------------------------*
 * declaration VsnetDownload::Client::Item
 *------------------------------------------------------------*/

class Item
{
public:
    Item( SOCKETALT sock, const std::string& filename, NotifyPtr notify = NotifyPtr() );
    virtual ~Item( );

    State state( ) const;
    Error error( ) const;

    void changeState( State s );
    void changeState( State s, Error e );

    void setSize( int len );
    void append( unsigned char* buffer, int bufsize );

    const std::string& getFilename( ) const;
    SOCKETALT          getSock() const;
    int                get_fd() const;

protected:
    virtual void childSetSize( int len ) = 0;
    virtual void childAppend( unsigned char* buffer, int bufsize ) = 0;

private:
    SOCKETALT         _sock;
    const std::string _filename;

    mutable VSMutex _mx;
    State           _state;
    Error           _error;
    NotifyPtr       _notify;
};

/*------------------------------------------------------------*
 * declaration VsnetDownload::Client::File
 *------------------------------------------------------------*/

class File : public Item
{
public:
    File( SOCKETALT          sock,
          const std::string& filename,
          std::string        localbasepath,
          NotifyPtr          notify = NotifyPtr() );

    virtual ~File( );

protected:
    virtual void childSetSize( int len );
    virtual void childAppend( unsigned char* buffer, int bufsize );

private:
    std::string    _localbasepath;
    std::ofstream* _of;
    int            _len;
    int            _offset;
};

/*------------------------------------------------------------*
 * declaration VsnetDownload::Client::Buffer
 *------------------------------------------------------------*/

class Buffer : public Item
{
    typedef unsigned char uchar;

public:
    Buffer( SOCKETALT          sock,
            const std::string& filename,
            NotifyPtr          notify = NotifyPtr() );

    virtual ~Buffer( );

    boost::shared_array<uchar> getBuffer( ) const;

protected:
    virtual void childSetSize( int len );
    virtual void childAppend( unsigned char* buffer, int bufsize );

private:
    boost::shared_array<uchar> _buf;
    int                        _len;
    int                        _offset;
};

/*------------------------------------------------------------*
 * declaration VsnetDownload::Client::TestItem
 *------------------------------------------------------------*/

class TestItem : public Item, public Notify
{
public:
    TestItem( SOCKETALT sock,
              const std::string& filename );

    virtual ~TestItem( ) { }

    virtual void notify( State s, Error e );

protected:
    virtual void childSetSize( int len );
    virtual void childAppend( unsigned char* buffer, int bufsize );

private:
    int _len;
    int _offset;
};

/*------------------------------------------------------------*
 * declaration VsnetDownload::Client::Manager
 *------------------------------------------------------------*/

/** The download manager works in the networking thread.
 *  New requests are added from all threads, and notifications may
 *  interfere with the Items that are owned by other threads,
 *  but the management of files and buffers is handled entirely in
 *  the networking threads.
 */
class Manager
{
public:
    Manager( SocketSet& set, const char** local_search_paths );

    void addItem( Item* item );
    void addItems( std::list<Item*>& items );

    /** NetClient::recvMsg calls this for answers from a server mgr
     *  (currently only from the server because no other socket is
     *   checked).
     */
    void processCmdDownload( SOCKETALT sock, NetBuffer& buffer );

    /** Called by SocketSet after each round.
     */
    void lower_check_queues( );

private:
    /** process a ResolveResponse */
    void private_eval_resolve_response( SOCKETALT sock, NetBuffer& buffer );
    void private_eval_download_error( SOCKETALT sock, NetBuffer& respbuffer );
    void private_eval_download( SOCKETALT sock, NetBuffer& buffer, Subcommand sc );

    /** lower - called only in the network thread
     */
    void private_lower_poll( );

    /** lower   - called only in the network thread
     *  private - called only internally.
     */
    bool private_lower_test_access( Item* i );

private:
    typedef std::list<Item*>                                  ItemList;
    typedef std::map<SOCKETALT,ItemList,SOCKETALT::CompareLt> ItemListMap;
    typedef std::map<string,Item*>                            ItemMap;
    typedef std::map<SOCKETALT,ItemMap,SOCKETALT::CompareLt>  ItemMapMap;
    typedef std::map<SOCKETALT,Item*,SOCKETALT::CompareLt>    ItemSockMap;

    typedef std::pair<string,Item*>                           ItemPair;
    typedef std::pair<SOCKETALT,Item*>                        ItemSockPair;

    typedef ItemList::iterator                                ItemList_I;
    typedef ItemListMap::iterator                             ItemListMap_I;
    typedef ItemMap::iterator                                 ItemMap_I;
    typedef ItemMapMap::iterator                              ItemMapMap_I;
    typedef ItemSockMap::iterator                             ItemSockMap_I;

private:
    SocketSet&               _set;
    std::vector<std::string> _local_search_paths;

    VSMutex           _pending_mx;
    std::queue<Item*> _pending;
    ItemMapMap        _asked;
    ItemSockMap       _currentItems;
};

/*------------------------------------------------------------*
 * declaration VsnetDownload::Client::FileSet
 *------------------------------------------------------------*/

/** We can't really support blocking until a set of files has been
 *  downloaded inside this class. The problem is that the variables
 *  that we must wait for exist in netserver, netclient and
 *  accountserver, and they are called in several places and potentially
 *  several threads.
 *  Therefore the waiting itself should be handled in those classes.
 *  This helper class allows a simple check.
 *
 *  Create a FileSet with the appropriate Manager, the socket to the
 *  remote side that you want to use, the list of filenames as string,
 *  and the existing(!) directory where you want to store the files.
 *  Then call isDone() a couple of times. If it returns true, the downloads
 *  are all completed (but they may have failed - if you need to check that,
 *  add a function for reading the int values from the member variable
 *  _files, 1 success, 0 failure).
 */
class FileSet
{
public:
    FileSet( boost::shared_ptr<Manager> mgr,
             SOCKETALT                  sock,
             std::list<std::string>     filesnames,
             std::string                localbasepath );

    bool isDone( ) const;
    void update( std::string s, bool v );

private:
    std::map<std::string,int> _files;
    int                       _to_go;

private:
    class NotifyConclusion;
};

/*------------------------------------------------------------*
 * declaration VsnetDownload::Client::Notify_f
 *------------------------------------------------------------*/

class Notify_f : public Notify
{
public:
    typedef void (*NotifyFunction)( std::string str, State s, Error e, int total, int offset );

public:
    Notify_f( std::string filename, NotifyFunction fun )
        : _filename( filename )
        , _fun( fun )
        , _total( 0 )
        , _offset( 0 )
    { }

    virtual ~Notify_f() { }

    virtual void notify( State s, Error e ) {
        (*_fun)( _filename, s, e, _total, _offset );
    }

    virtual void setTotalBytes( int sz ) {
        _total = sz;
    }

    virtual void addBytes( int sz ) {
        _offset += sz;
    }

private:
    std::string    _filename;
    NotifyFunction _fun;
    int            _total;
    int            _offset;
};

/*------------------------------------------------------------*
 * declaration VsnetDownload::Client::Notify_fp
 *------------------------------------------------------------*/

struct Notify_fp : public NotifyPtr
{
    typedef void (*NotifyFunction)( std::string str, State s, Error e, int total, int offset );

    Notify_fp( std::string filename, NotifyFunction fun )
        : NotifyPtr( new Notify_f(filename,fun) )
    { }
};

/*------------------------------------------------------------*
 * declaration VsnetDownload::Client::Notify_t<T>
 *------------------------------------------------------------*/

template <class T> class Notify_t : public Notify
{
public:
    typedef void (T::*NotifyFunction)( State s, Error e );

public:
    Notify_t( T* object, NotifyFunction fun )
        : _object( object )
        , _fun( fun )
        , _total( 0 )
        , _offset( 0 )
    { }

    virtual void notify( State s, Error e ) {
        (_object->*_fun)( s, e );
    }

    virtual void setTotalBytes( int sz ) {
        _total = sz;
    }

    virtual void addBytes( int sz ) {
        _offset += sz;
    }

    inline int total( ) const {
        return _total;
    }

    inline int offset( ) const {
        return _offset;
    }

private:
    T*             _object;
    NotifyFunction _fun;
    int            _total;
    int            _offset;
};

/*------------------------------------------------------------*
 * declaration VsnetDownload::Client::Notify_tp<T>
 *------------------------------------------------------------*/

template <class T> struct Notify_tp : public NotifyPtr
{
    typedef typename Notify_t<T>::NotifyFunction T_Function;
    typedef Notify_t<T>                          T_Class;

    Notify_tp( T* object, T_Function fun )
        : NotifyPtr( new T_Class(object,fun) )
    { }
};

}; // namespace Client

namespace Server
{

class DownloadItem
{
public:
    DownloadItem( SOCKETALT sock, std::string failed_file );
    DownloadItem( SOCKETALT sock, std::string file, std::ifstream* f, size_t sz );
    ~DownloadItem( );

    SOCKETALT getSock( ) const;
    bool      error( ) const;
    string    file( ) const;

    size_t    offset( ) const;
    size_t    remainingSize( ) const;
    void      copyFromFile( unsigned char* buf, size_t sz );

private:
    SOCKETALT         _sock;
    bool              _error;
    const std::string _file;
    std::ifstream*    _handle;
    size_t            _size;
    size_t            _offset;
};

/** The download manager on the server side works in the networking thread
 *  as well. It does not catch CMD_DOWNLOAD messages from the client, but
 *  it is handed these requests by NetServer::processPacket. It doesn't care
 *  about the client, but only about the socket that connects the server to
 *  the client.
 *  The download manager could run in a separate process, but that would make
 *  it very difficult to manage the connection bandwidth.
 */
class Manager
{
public:
    Manager( SocketSet& set, const char** local_search_paths );

    /** NetServer::processPacket calls this. NetServer doesn't care, but we
     *  process the NetBuffer actually already in the main thread before
     *  triggering wakeup.
     */
    void addCmdDownload( SOCKETALT sock, NetBuffer& buffer );

    /** The NetServer must tell the download manager that a socket has been
     *  closed or is otherwise expired.
     * @Note: function needs cleanup (via mutex or private_lower*) - there is
     *        a bad race right now.
     */
    void cleanDownload( SOCKETALT s );

    /** Called by SocketSet after each round.
     */
    void lower_check_queues( );

private:
    typedef boost::shared_ptr<DownloadItem>                       DownloadItemPtr;
    typedef std::queue<DownloadItemPtr>                           ItemQueue;
    typedef boost::shared_ptr<ItemQueue>                          ItemQueuePtr;
    typedef std::map<SOCKETALT,ItemQueuePtr,SOCKETALT::CompareLt> ItemMap;
    typedef std::pair<SOCKETALT,ItemQueuePtr>                     ItemMapPair;
    typedef ItemMap::iterator                                     ItemMap_I;

private:
    bool           private_test_access( const std::string& s );
    std::ifstream* private_access( std::string& file );
    size_t         private_file_size( const std::string& file );
    bool           private_lower_try_push_queue( SOCKETALT sock, ItemQueuePtr q );

private:
    LOCALCONST_DECL(int,_packetWorkahead,5)

    SocketSet&                _set;
    std::vector<std::string>  _local_search_paths;

    VSMutex                   _download_mx;
    ItemQueue                 _download;

    ItemMap                   _lower_download;
};

}; // namespace Server

}; // namespace VsnetDownload

#endif /* VSNET_DLOADMGR_H */

