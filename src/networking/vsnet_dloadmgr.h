#ifndef VSNET_DLOADMGR_H
#define VSNET_DLOADMGR_H

#include <config.h>
#include <string>
#include <vector>
#include <list>
#include <queue>

#include "boost/shared_ptr.hpp"

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

namespace Client
{

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
    Acknowledged,
    Completed
};

/*------------------------------------------------------------*
 * declaration VsnetDownload::Client::Error
 *------------------------------------------------------------*/

enum Error
{
    Ok,
    SocketError,
    FileNotFound,
    LocalPermissionDenied,
    RemotePermissionDenied
};

/*------------------------------------------------------------*
 * declaration VsnetDownload::Client::Notify
 *------------------------------------------------------------*/

class Notify
{
public:
    virtual void notify( State s, Error e ) = 0;
};

/*------------------------------------------------------------*
 * declaration VsnetDownload::Client::Item
 *------------------------------------------------------------*/

class Item
{
public:
    Item( SOCKETALT sock, const std::string& filename, Notify* notify = NULL);

    State state( ) const;
    Error error( ) const;

    void changeState( State s );
    void changeState( State s, Error e );

    const std::string& getFilename( ) const;
    SOCKETALT          getSock() const;
    int                get_fd() const;

private:
    SOCKETALT         _sock;
    const std::string _filename;

    mutable VSMutex _mx;
    State           _state;
    Error           _error;
    Notify*         _notify;
};

/* Introduce typedef boost::shared_ptr<Item> ItemPtr when it is
 * better understood.
 */

/*------------------------------------------------------------*
 * declaration VsnetDownload::Client::File
 *------------------------------------------------------------*/

class File : public Item
{
public:
    File( SOCKETALT          sock,
          const std::string& filename,
          std::string        localbasepath,
          Notify*            notify = NULL);

private:
    std::string _localbasepath;
};

/*------------------------------------------------------------*
 * declaration VsnetDownload::Client::Buffer
 *------------------------------------------------------------*/

class Buffer : public Item
{
public:
    Buffer( SOCKETALT          sock,
            const std::string& filename,
            Notify*            notify = NULL );
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
    /** lower - called only in the network thread
     */
    void private_lower_poll( );

    /** lower   - called only in the network thread
     *  private - called only internally.
     */
    bool lower_private_test_access( Item* i );

private:
    typedef std::list<Item*>                                  ItemList;
    typedef ItemList::iterator                                ItemList_I;
    typedef std::map<SOCKETALT,ItemList,SOCKETALT::CompareLt> ItemMap;
    typedef ItemMap::iterator                                 ItemMap_I;

private:
    SocketSet&               _set;
    std::vector<std::string> _local_search_paths;

    VSMutex           _pending_mx;
    std::queue<Item*> _pending;
    ItemMap           _asked;
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
    void           private_lower_try_push_queue( SOCKETALT sock, ItemQueuePtr q );

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

