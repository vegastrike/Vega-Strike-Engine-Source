#include <config.h>
#include "vsnet_dloadmgr.h"
#include "vsnet_cmd.h"
#include "netbuffer.h"
#include "packet.h"

using namespace std;

#ifndef HAVE_ACCESS
    #ifdef R_OK
    #undef R_OK
    #endif
    #define R_OK 1

    #ifdef W_OK
    #undef W_OK
    #endif
    #define W_OK 2

    extern "C" int access( const char* name, int mode );
#endif

#ifndef HAVE_LSTAT
    struct stat
    {
        size_t st_size;
    };

    extern "C" int lstat( const char* name, struct stat* buf );
#endif

/** Protocol:
 *
 *  resolve request
 *    CMD_DOWNLOAD
 *    object serial 0
 *    SENDRELIABLE, LOPRI
 *    char:  subcommand ResolveRequest
 *    short: number of entries
 *    ( short stringlen, char* )*: list of filenames
 */
namespace VsnetDownload
{

namespace Client
{

/*------------------------------------------------------------*
 * definition VsnetDownload::Client::Item
 *------------------------------------------------------------*/

Item::Item( SOCKETALT sock, const string& filename, Notify* notify )
    : _sock( sock )
    , _filename( filename )
    , _state( Idle )
    , _error( Ok )
    , _notify( notify )
{
}

State Item::state( ) const
{
    _mx.lock( );
    State ret = _state;
    _mx.unlock( );
    return ret;
}

Error Item::error( ) const
{
    _mx.lock( );
    Error ret = _error;
    _mx.unlock( );
    return ret;
}

void Item::changeState( State s )
{
    _mx.lock( );
    _state = s;
    Error e = _error;
    _mx.unlock( );
    if( _notify ) _notify->notify( s, e );
}

void Item::changeState( State s, Error e )
{
    _mx.lock( );
    _state = s;
    _error = e;
    _mx.unlock( );
    if( _notify ) _notify->notify( s, e );
}

const string& Item::getFilename( ) const
{
    return _filename;
}

SOCKETALT Item::getSock() const
{
    return _sock;
}

int Item::get_fd() const
{
    return _sock.get_fd( );
}

/*------------------------------------------------------------*
 * definition VsnetDownload::Client::File
 *------------------------------------------------------------*/

File::File( SOCKETALT     sock,
            const string& filename,
            string        localbasepath,
            Notify*       notify )
    : Item( sock, filename, notify )
    , _localbasepath( localbasepath )
{
}

/*------------------------------------------------------------*
 * definition VsnetDownload::Client::Buffer
 *------------------------------------------------------------*/

Buffer::Buffer( SOCKETALT sock, const string& filename, Notify* notify )
    : Item( sock, filename, notify )
{
}

/*------------------------------------------------------------*
 * definition VsnetDownload::Client::Manager
 *------------------------------------------------------------*/

Manager::Manager( SocketSet& sets, const char** local_search_paths )
    : _set( sets )
{
    const char** c = local_search_paths;
    while( *c != NULL )
    {
        _local_search_paths.push_back( *c );
        c++;
    }
}

void Manager::addItem( Item* item )
{
    list<Item*> l;
    l.push_back( item );
    addItems( l );
}

void Manager::addItems( list<Item*>& items )
{
    _pending_mx.lock( );
    if( _pending.empty() )
    {
        _pending_mx.unlock( );
        return;
    }

    list<Item*>::iterator it;
    for( it=items.begin(); it!=items.end(); it++ )
    {
        _pending.push( *it );
    }
    _pending_mx.unlock( );

    for( it=items.begin(); it!=items.end(); it++ )
    {
        (*it)->changeState( Queued );
    }

    _set.wakeup( );
}

void Manager::processCmdDownload( SOCKETALT sock, NetBuffer& buffer )
{
}

void Manager::lower_check_queues( )
{
    private_lower_poll( );
}

void Manager::private_lower_poll( )
{
    _pending_mx.lock( );
    if( _pending.empty() )
    {
        _pending_mx.unlock( );
        return;
    }

    ItemMap collect;
    while( _pending.empty() == false )
    {
        Item* i = _pending.front( );
        _pending.pop( );

        _pending_mx.unlock( );
        if( lower_private_test_access( i ) )
        {
            i->changeState( Completed, Ok );
        }
        else if( i->getSock().isActive() == false )
        {
            i->changeState( Completed, SocketError );
        }
        else
        {
            i->changeState( Resolving );
            collect[i->getSock()].push_back( i );
            _asked[i->getSock()].push_back( i );
        }
        _pending_mx.lock( );
    }
    _pending_mx.unlock( );

    ItemMap_I it;
    for( it = collect.begin(); it != collect.end(); it++ )
    {
        ItemList& cl( it->second );
        if( cl.empty() == false )
        {
            NetBuffer netbuf;
            netbuf.addChar( (char)ResolveRequest );

            assert( cl.size() < 0xffff );
            netbuf.addShort( cl.size() );
            for( ItemList_I strit=cl.begin(); strit!=cl.end(); strit++ )
            {
                netbuf.addString( (*strit)->getFilename() );
            }

            Packet packet;
            packet.send( CMD_DOWNLOAD, 0,
                        netbuf.getData(), netbuf.getDataLength(),
                        COMPRESSED | SENDRELIABLE | LOPRI,
                        NULL, it->first,
                        __FILE__, PSEUDO__LINE__(258) );
        }
    }
}

bool Manager::lower_private_test_access( Item* i )
{
    string file( i->getFilename() );
    for( vector<string>::const_iterator it=_local_search_paths.begin();
         it!=_local_search_paths.end();
         it++ )
    {
        string path = *it + "/" + file;
        if( ::access( path.c_str(), R_OK ) == 0 )
        {
            COUT << "Found local file " << path.c_str() << endl;
            return true;
        }
    }
    return false;
}

}; // namespace Client

namespace Server
{

/*------------------------------------------------------------*
 * definition VsnetDownload::Server::DownloadItem
 *------------------------------------------------------------*/

DownloadItem::DownloadItem( SOCKETALT sock, string file )
    : _sock( sock )
    , _error( true )
    , _file( file )
    , _handle( NULL )
{
}

DownloadItem::DownloadItem( SOCKETALT sock, string file, ifstream* f, size_t sz )
    : _sock( sock )
    , _error( false )
    , _file( file )
    , _handle( f )
    , _size( sz )
    , _offset( 0 )
{
}

DownloadItem::~DownloadItem( )
{
    if( _handle ) delete _handle;
}

SOCKETALT DownloadItem::getSock( ) const
{
    return _sock;
}

bool DownloadItem::error( ) const
{
    return _error;
}

string DownloadItem::file( ) const
{
    return _file;
}

size_t DownloadItem::offset( ) const
{
    return _offset;
}

size_t DownloadItem::remainingSize( ) const
{
    return ( _size - _offset );
}

void DownloadItem::copyFromFile( unsigned char* buf, size_t sz )
{
    _handle->read( (char*)buf, sz );
    _offset += sz;
}

/*------------------------------------------------------------*
 * definition VsnetDownload::Server::Manager
 *------------------------------------------------------------*/

LOCALCONST_DEF(Manager,int,_packetWorkahead,5)

Manager::Manager( SocketSet& sets, const char** local_search_paths )
    : _set( sets )
{
    const char** c = local_search_paths;
    while( *c != NULL )
    {
        _local_search_paths.push_back( *c );
        c++;
    }
}

void Manager::addCmdDownload( SOCKETALT sock, NetBuffer& buffer )
{
    char c = buffer.getChar( );

    switch( c )
    {
    case ResolveRequest :
        {
            short num = buffer.getShort( );

            NetBuffer respbuffer;
            respbuffer.addChar( ResolveResponse );
            respbuffer.addShort( num );

            while( num > 0 )
            {
                string file;
                bool   ok;
                file = buffer.getString( );
                ok   = private_test_access( file );
                respbuffer.addString( file );
                respbuffer.addChar( ok );
                num--;
            }

            Packet packet;
            packet.send( CMD_DOWNLOAD, 0,
                         respbuffer.getData(), respbuffer.getDataLength(),
                         COMPRESSED | SENDRELIABLE | LOPRI,
                         NULL, sock,
                         __FILE__, PSEUDO__LINE__(334) );
        }
        break;
    case DownloadRequest :
        {
            short num = buffer.getShort( );
            if( num > 0 )
            {
                while( num > 0 )
                {
                    DownloadItemPtr di;
                    string          file = buffer.getString( );
                    string          path = file;
                    ifstream*       f    = private_access( path );
                    if( f )
                    {
                        size_t bytes = private_file_size( path );
                        di.reset( new DownloadItem( sock, file, f, bytes ) );
                    }
                    else
                    {
                        // Couldn't open for reading, maybe removed since resolve?
                        di.reset( new DownloadItem( sock, file ) );
                    }
                    _download_mx.lock( );
                    _download.push( di );
                    _download_mx.unlock( );
                    num--;
                }
                _set.wakeup( );
            }
        }
        break;
    default :
        {
            NetBuffer respbuffer;

            respbuffer.addChar( UnexpectedSubcommand );
            respbuffer.addChar( c );
            Packet packet;
            packet.send( CMD_DOWNLOAD, 0,
                         respbuffer.getData(), respbuffer.getDataLength(),
                         COMPRESSED | SENDRELIABLE | LOPRI,
                         NULL, sock,
                         __FILE__, PSEUDO__LINE__(334) );
        }
        break;
    }
}

void Manager::cleanDownload( SOCKETALT s )
{
    _lower_download.erase( s );
}

void Manager::lower_check_queues( )
{
    _download_mx.lock( );
    while( _download.empty() == false )
    {
        DownloadItemPtr item = _download.front( );
        _download.pop( );
        ItemMap_I it = _lower_download.find( item->getSock() );
        if( it == _lower_download.end() )
        {
            ItemQueuePtr q( new ItemQueue );
            _lower_download.insert( ItemMapPair( item->getSock(), q ) );
            it = _lower_download.find( item->getSock() );
        }
        it->second->push( item );
    }
    _download_mx.unlock( );

    ItemMap_I it;
    for( it=_lower_download.begin(); it != _lower_download.end(); it++ )
    {
        private_lower_try_push_queue( it->first, it->second );
    }
}

void Manager::private_lower_try_push_queue( SOCKETALT sock, ItemQueuePtr q )
{
    if( !q->empty() && sock.queueLen( LOPRI ) <= _packetWorkahead )
    {
        NetBuffer respbuffer;

        DownloadItemPtr item = q->front( );
        if( item->error() )
        {
            respbuffer.addChar( DownloadError );
            respbuffer.addString( item->file() );
            Packet packet;
            packet.send( CMD_DOWNLOAD, 0,
                         respbuffer.getData(), respbuffer.getDataLength(),
                         COMPRESSED | SENDRELIABLE | LOPRI,
                         NULL, sock,
                         __FILE__, PSEUDO__LINE__(425) );
            q->pop( );
        }
        else
        {
            size_t l = sock.optPayloadSize();
            if( !sock.isTcp() )
            {
                // UDP fragments itself because it must. But if we allow clients
                // to load large chunks into these intermediate buffers all at
                // once, that's an invitation for a DDOS attack. So we seek a
                // middle path.
                l *= 5;
            }
            bool   firstfrag;
            string f( item->file() );
            if( item->offset() == 0 )
            {
                firstfrag = true;

                /* Subtract packet header length, 1 byte download subcommand,
                 * 2 bytes string length, length of the string itself,
                 * 2 bytes payload length.
                 */
                l = l - Packet::getHeaderLength() - 1 - 2 - f.length() - 2;
            }
            else
            {
                firstfrag = false;
                l = l - Packet::getHeaderLength() - 1 - 2;
            }

            if( item->remainingSize() < l )
            {
                q->pop( );

                short sz = item->remainingSize();
                unsigned char* buf;
                if( firstfrag )
                {
                    string f( item->file() );
                    respbuffer.addChar( Download );
                    respbuffer.addString( f );
                }
                else
                {
                    respbuffer.addChar( DownloadLastFragment );
                }
                respbuffer.addShort( sz );
                buf = respbuffer.extAddBuffer( sz );
                item->copyFromFile( buf, sz );
            }
            else
            {
                unsigned char* buf;
                if( firstfrag )
                {
                    respbuffer.addChar( DownloadFirstFragment );
                    respbuffer.addString( f );
                }
                else
                {
                    respbuffer.addChar( DownloadFragment );
                }
                respbuffer.addString( f );
                respbuffer.addShort( l );
                buf = respbuffer.extAddBuffer( l );
                item->copyFromFile( buf, l );
            }
            Packet packet;
            packet.send( CMD_DOWNLOAD, 0,
                         respbuffer.getData(), respbuffer.getDataLength(),
                         COMPRESSED | SENDRELIABLE | LOPRI,
                         NULL, sock,
                         __FILE__, PSEUDO__LINE__(525) );
        }
    }
}

bool Manager::private_test_access( const string& file )
{
    for( vector<string>::const_iterator it=_local_search_paths.begin();
         it!=_local_search_paths.end();
         it++ )
    {
        string path = *it + "/" + file;
        if( ::access( path.c_str(), R_OK ) == 0 )
        {
            COUT << "Found local file " << path.c_str() << endl;
            return true;
        }
    }
    COUT << "Didn't find local file for " << file.c_str() << endl;
    return false;
}

std::ifstream* Manager::private_access( string& file )
{
    for( vector<string>::const_iterator it=_local_search_paths.begin();
         it!=_local_search_paths.end();
         it++ )
    {
        string path = *it + "/" + file;
        ifstream* f = new ifstream( path.c_str(), ios::in );
        if( f && f->good() )
        {
            COUT << "Opened local file " << path.c_str() << endl;
            file = path;
            return f;
        }
    }
    COUT << "Couldn't open local file for " << file.c_str() << endl;
    return NULL;
}

size_t Manager::private_file_size( const string& file )
{
    struct stat buf;
    if( ::lstat( file.c_str(), &buf ) == 0 )
    {
        size_t ret = buf.st_size;
        return ret;
    }
    else
    {
        return 0;
    }
}

}; // namespace Server

}; // namespace VsnetDownload

#ifndef HAVE_ACCESS
int access( const char* name, int mode )
{
    if( mode == R_OK )
    {
        std::ifstream f( name, std::ios::in );
        if( f.good() ) return 0;
        return -1;
    }
    else
    {
        std::ofstream f( name, std::ios::out );
        if( f.good() ) return 0;
        return -1;
    }
}
#endif /* HAVE_ACCESS */

#ifndef HAVE_LSTAT
int lstat( const char* name, struct stat* buf )
{
    FILE* f = fopen( name );
    if( f == NULL ) return -1;

    int retval = -1;
    if( fseek( f, 0, SEEK_END ) == 0 )
    {
        buf->st_size = ftell( f );
        if( buf->st_size >= 0 ) retval = 0;
    }
    fclose( f );
    return retval;
}
#endif /* HAVE_LSTAT */

