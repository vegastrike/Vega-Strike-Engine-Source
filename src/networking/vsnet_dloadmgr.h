#ifndef VSNET_DLOADMGR_H
#define VSNET_DLOADMGR_H

#include <config.h>

#include <string>
#include <map>
#include <vector>
#include <list>
#include <queue>
#include <iostream>

#include "boost/shared_ptr.hpp"
#include "boost/shared_array.hpp"

#include "vsnet_dloadenum.h"
#include "vsnet_headers.h"
#include "vsnet_debug.h"
#include "vsnet_thread.h"
#include "vsnet_socket.h"
#include "vsnet_socketset.h"
//#include "netbuffer.h"

using std::map;
using std::string;

class NetBuffer;

namespace VsnetDownload
{

namespace Client
{

class Manager;

/*------------------------------------------------------------*
 * forward declaration VsnetDownload::Client::Item
 *------------------------------------------------------------*/

class Item;

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
    Manager( SocketSet& set );

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
    Manager( SocketSet& set );

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

