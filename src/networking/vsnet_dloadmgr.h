#ifndef VSNET_DLOADMGR_H
#define VSNET_DLOADMGR_H

#include <config.h>
#include <string>
#include <vector>
#include <list>
#include <queue>

#include "vsnet_headers.h"
#include "vsnet_debug.h"
#include "vsnet_thread.h"
#include "vsnet_socket.h"
#include "vsnet_socketset.h"

namespace VsnetDownload
{
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
    Item( const std::string& filename, Notify* notify = NULL);

    State state( ) const;
    Error error( ) const;

    void changeState( State s );
    void changeState( State s, Error e );

    const std::string& getFilename( ) const;

private:
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
    File( const std::string& filename,
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
    Buffer( const std::string& filename, Notify* notify = NULL );
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
    Manager( SocketSet& set, SOCKETALT sock, const char** local_search_paths );

    void addItem( Item* item );
    void addItems( std::list<Item*> items );

    /** lower - called only in the network thread
     */
    void lower_poll( );

private:
    /** lower   - called only in the network thread
     *  private - called only internally.
     */
    bool lower_private_test_access( Item* i );

private:
    SocketSet&     _set;
    SOCKETALT      _sock;
    std::vector<std::string> _local_search_paths;

    VSMutex           _pending_mx;
    std::queue<Item*> _pending;
    std::queue<Item*> _asked;
};

}; // namespace Client
}; // namespace VsnetDownload

#endif /* VSNET_DLOADMGR_H */

