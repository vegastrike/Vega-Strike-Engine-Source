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

#ifdef _WIN32
#include <sys/stat.h>
#define HAVE_LSTAT
#define lstat _stat
#define stat _stat
#endif

#ifndef HAVE_LSTAT
    struct stat
    {
        size_t st_size;
    };

    extern "C" int lstat( const char* name, struct stat* buf );
#endif


namespace VsnetDownload
{

/*------------------------------------------------------------*
 * definition VsnetDownload::Subcommand
 *------------------------------------------------------------*/

/** Protocol:
 *
 *  resolve request
 *    CMD_DOWNLOAD, object serial 0, COMPRESS|SENDRELIABLE|LOPRI
 *    char                       : subcommand ResolveRequest
 *    int16                      : number of entries
 *    ( int16 stringlen, char* )*: list of filenames
 *
 *  resolve response
 *    CMD_DOWNLOAD, object serial 0, COMPRESS|SENDRELIABLE|LOPRI
 *    char                      : subcommand ResolveResponse
 *    int16                     : number of entries
 *    ( int16 stringlen, char*  : name
 *      char )*                 : ok(1), not ok(0)
 *
 *  download request
 *    CMD_DOWNLOAD, object serial 0, COMPRESS|SENDRELIABLE|LOPRI
 *    char                       : subcommand DownloadRequest
 *    int16                      : number of entries
 *    ( int16 stringlen, char* )*: list of filenames
 *
 *  download error
 *    CMD_DOWNLOAD, object serial 0, COMPRESS|SENDRELIABLE|LOPRI
 *    char                       : subcommand DownloadError
 *    ( int16 stringlen, char* ) : failed filename
 *    
 *  download entire file in one packet
 *    CMD_DOWNLOAD, object serial 0, COMPRESS|SENDRELIABLE|LOPRI
 *    char                       : subcommand Download
 *    ( int16 stringlen, char* ) : filename
 *    int16                      : payload length
 *    char*                      : payload
 *
 *  download first fragment of a file
 *    CMD_DOWNLOAD, object serial 0, COMPRESS|SENDRELIABLE|LOPRI
 *    char                       : subcommand DownloadFirstFragment
 *    ( int16 stringlen, char* ) : filename
 *    int32                      : file length
 *    int16                      : payload length
 *    char*                      : payload
 *
 *  download a middle fragment of a file
 *    CMD_DOWNLOAD, object serial 0, COMPRESS|SENDRELIABLE|LOPRI
 *    char                       : subcommand DownloadFragment
 *    int16                      : payload length
 *    char*                      : payload
 *
 *  download last fragment of a file
 *    CMD_DOWNLOAD, object serial 0, COMPRESS|SENDRELIABLE|LOPRI
 *    char                       : subcommand DownloadLastFragment
 *    int16                      : payload length
 *    char*                      : payload
 */

std::ostream& operator<<( std::ostream& ostr, Subcommand e )
{
    switch( e )
    {
    case ResolveRequest :
        ostr << "ResolveRequest";
        break;
    case ResolveResponse :
        ostr << "ResolveResponse";
        break;
    case DownloadRequest :
        ostr << "DownloadRequest";
        break;
    case DownloadError :
        ostr << "DownloadError";
        break;
    case Download :
        ostr << "Download";
        break;
    case DownloadFirstFragment :
        ostr << "DownloadFirstFragment";
        break;
    case DownloadFragment :
        ostr << "DownloadFragment";
        break;
    case DownloadLastFragment :
        ostr << "DownloadLastFragment";
        break;
    case UnexpectedSubcommand :
        ostr << "UnexpectedSubcommand";
        break;
    default :
        ostr << "missing case";
        break;
    }
    return ostr;
}

namespace Client
{

/*------------------------------------------------------------*
 * definition VsnetDownload::Client::State
 *------------------------------------------------------------*/

std::ostream& operator<<( std::ostream& ostr, State s )
{
    switch( s )
    {
    case Idle :
        ostr << "Idle";
        break;
    case Queued :
        ostr << "Queued";
        break;
    case Resolving :
        ostr << "Resolving";
        break;
    case Resolved :
        ostr << "Resolved";
        break;
    case Requested :
        ostr << "Requested";
        break;
    case FragmentReceived :
        ostr << "FragmentReceived";
        break;
    case Completed :
        ostr << "Completed";
        break;
    default :
        ostr << "unknown";
        break;
    };
    return ostr;
}
/*------------------------------------------------------------*
 * definition VsnetDownload::Client::Error
 *------------------------------------------------------------*/

std::ostream& operator<<( std::ostream& ostr, Error e )
{
    switch( e )
    {
    case Ok :
        ostr << "Ok";
        break;
    case SocketError :
        ostr << "SocketError";
        break;
    case FileNotFound :
        ostr << "FileNotFound";
        break;
    case LocalPermissionDenied :
        ostr << "LocalPermissionDenied";
        break;
    case RemotePermissionDenied :
        ostr << "RemotePermissionDenied";
        break;
    case DownloadInterrupted :
        ostr << "DownloadInterrupted";
        break;
    default :
        ostr << "unknown";
        break;
    };
    return ostr;
}

/*------------------------------------------------------------*
 * definition VsnetDownload::Client::Item
 *------------------------------------------------------------*/

Item::Item( SOCKETALT sock, const string& filename, NotifyPtr notify )
    : _sock( sock )
    , _filename( filename )
    , _state( Idle )
    , _error( Ok )
    , _notify( notify )
{
}

Item::~Item( )
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

void Item::setSize( int len )
{
    childSetSize( len );
    if( _notify ) _notify->setTotalBytes( len );
}

void Item::append( unsigned char* buffer, int bufsize )
{
    childAppend( buffer, bufsize );
    if( _notify ) _notify->addBytes( bufsize );
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
            NotifyPtr     notify )
    : Item( sock, filename, notify )
    , _localbasepath( localbasepath )
    , _of( NULL )
    , _len( 0 )
    , _offset( 0 )
{
}

File::~File( )
{
    if( _of )
    {
        _of->close( );
        delete _of;
    }
}

void File::childSetSize( int len )
{
    string filename = _localbasepath + "/" + getFilename();

    _of = new ofstream( filename.c_str(), ios::out );
    if( !_of->is_open() )
    {
        delete _of;
        _of = NULL;
    }
    else
    {
        _len = len;
    }
}

void File::childAppend( unsigned char* buffer, int bufsize )
{
    if( _of )
    {
        _of->write( (const char*)buffer, bufsize );
        _offset += bufsize;
        if( _offset >= _len )
        {
            _of->close( );
            delete _of;
            _of = NULL;
        }
    }
}

/*------------------------------------------------------------*
 * definition VsnetDownload::Client::Buffer
 *------------------------------------------------------------*/

Buffer::Buffer( SOCKETALT sock, const string& filename, NotifyPtr notify )
    : Item( sock, filename, notify )
    , _len( 0 )
    , _offset( 0 )
{
}

Buffer::~Buffer( )
{
}

boost::shared_array<Buffer::uchar> Buffer::getBuffer( ) const
{
    return _buf;
}

void Buffer::childSetSize( int len )
{
    _len    = len;
    _offset = 0;
    _buf.reset( new uchar[len] );
}

void Buffer::childAppend( unsigned char* buffer, int bufsize )
{
    if( _offset + bufsize <= _len )
    {
        memcpy( _buf.get()+_offset, buffer, bufsize );
        _offset += bufsize;
    }
}

/*------------------------------------------------------------*
 * definition VsnetDownload::Client::TestItem
 *------------------------------------------------------------*/

TestItem::TestItem( SOCKETALT sock, const string& filename )
    : Item( sock, filename, NotifyPtr(this) )
    , _len( 0 )
    , _offset( 0 )
{
    COUT << "Created TestItem for downloading " << filename << endl;
}

void TestItem::childSetSize( int len )
{
    _len = len;
    COUT << "Expecting " << _len << " bytes from server" << endl;
}

void TestItem::childAppend( unsigned char* buffer, int bufsize )
{
    COUT << "Received buffer with " << bufsize << " bytes" << endl;
    _offset += bufsize;
    if( _offset > _len )
        COUT << "Received too many bytes" << endl;
    else if( _offset == _len )
        COUT << "Download seems to be complete" << endl;
}

void TestItem::notify( State s, Error e )
{
    COUT << "State updated for download of " << getFilename()
         << ", changed to " << s << " (" << e << ")" << endl;
}

/*------------------------------------------------------------*
 * definition VsnetDownload::Client::Manager
 *------------------------------------------------------------*/

Manager::Manager( SocketSet& sets, const char** local_search_paths )
    : _set( sets )
{
    COUT << "Enter " << __PRETTY_FUNCTION__ << endl;

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
    COUT << "Enter " << __PRETTY_FUNCTION__ << endl;

    list<Item*>::iterator it;

    _pending_mx.lock( );
    for( it=items.begin(); it!=items.end(); it++ )
    {
        COUT << "adding an item" << endl;
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
    COUT << "Enter " << __PRETTY_FUNCTION__ << endl;

    char c = buffer.getChar( );
    Subcommand sc = (Subcommand)c;
    switch( sc )
    {
    case ResolveResponse :
        private_eval_resolve_response( sock, buffer );
        break;
    case DownloadError :
        private_eval_download_error( sock, buffer );
        break;
    case Download :
    case DownloadFirstFragment :
    case DownloadFragment :
    case DownloadLastFragment :
        private_eval_download( sock, buffer, sc );
        break;
    case DownloadRequest :
    case UnexpectedSubcommand :
    case ResolveRequest :
    default :
        COUT << "unexpected subcommand " << sc << ", ignoring" << endl;
        break;
    }
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

    COUT << "Enter " << __PRETTY_FUNCTION__ << endl;

    ItemListMap collect;
    while( _pending.empty() == false )
    {
        Item* i = _pending.front( );
        _pending.pop( );

        _pending_mx.unlock( );
        if( private_lower_test_access( i ) )
        {
            i->changeState( Completed, Ok );
        }
        else if( i->getSock().valid() == false )
        {
            i->changeState( Completed, SocketError );
        }
        else
        {
            i->changeState( Resolving );
            collect[i->getSock()].push_back( i );
            _asked[i->getSock()].insert( ItemPair(i->getFilename(),i) );
        }
        _pending_mx.lock( );
    }
    _pending_mx.unlock( );

    ItemListMap_I it;
    for( it = collect.begin(); it != collect.end(); it++ )
    {
        ItemList* cl = &it->second;
        if( cl->empty() == false )
        {
            NetBuffer netbuf;
            netbuf.addChar( (char)ResolveRequest );

            assert( cl->size() < 0xffff );
            netbuf.addShort( cl->size() );
            for( ItemList_I strit=cl->begin(); strit!=cl->end(); strit++ )
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

void Manager::private_eval_resolve_response( SOCKETALT sock, NetBuffer& respbuffer )
{
    ItemMapMap_I it = _asked.find( sock );
    if( it == _asked.end() )
    {
        COUT << "received resolve response for unasked socket " << sock << endl;
    }
    else
    {
        ItemList   requestlist;
        ItemList_I rli;
        ItemMap*   cl = &it->second;
        short      num;
        for( num = respbuffer.getShort(); num > 0 ; num-- )
        {
            string filename = respbuffer.getString( );
            char   ok       = respbuffer.getChar( );

            ItemMap_I mapi = cl->find( filename );
            if( mapi != cl->end() )
            {
                if( ok == 1 )
                {
                    COUT << "file " << filename << " found on server" << endl;
                    mapi->second->changeState( Resolved, Ok );
                    requestlist.push_back( mapi->second );
                }
                else
                {
                    COUT << "file " << filename << " not found on server" << endl;
                    mapi->second->changeState( Completed, FileNotFound );
                    cl->erase( mapi );
                }
            }
            else
            {
                COUT << "file " << filename << " not in request list any more" << endl;
            }
        }

        if( requestlist.empty() == false )
        {
            short num = requestlist.size();

            NetBuffer reqbuffer;
            reqbuffer.addChar( DownloadRequest );
            reqbuffer.addShort( num );
            for( rli=requestlist.begin(); rli!=requestlist.end(); rli++ )
            {
                reqbuffer.addString( (*rli)->getFilename() );
                (*rli)->changeState( Requested, Ok );
            }

            Packet packet;
            packet.send( CMD_DOWNLOAD, 0,
                        reqbuffer.getData(), reqbuffer.getDataLength(),
                        COMPRESSED | SENDRELIABLE | LOPRI,
                        NULL, sock,
                        __FILE__, PSEUDO__LINE__(462) );
        }
    }
}

void Manager::private_eval_download_error( SOCKETALT sock, NetBuffer& respbuffer )
{
    ItemMapMap_I it = _asked.find( sock );
    if( it == _asked.end() )
    {
        COUT << "received resolve response for unasked socket " << sock << endl;
    }
    else
    {
        string    filename = respbuffer.getString( );
        ItemMap*  cl = &it->second;
        ItemMap_I mapi = cl->find( filename );
        if( mapi != cl->end() )
        {
            COUT << "file " << filename << " not found on server" << endl;
            mapi->second->changeState( Completed, FileNotFound );
            cl->erase( mapi );
        }
        else
        {
            COUT << "file " << filename << " not in request list any more" << endl;
        }
    }
}

void Manager::private_eval_download( SOCKETALT sock, NetBuffer& buffer, Subcommand sc )
{
    ItemMapMap_I it = _asked.find( sock );
    if( it == _asked.end() )
    {
        COUT << "received resolve response for unasked socket " << sock << endl;
    }
    else
    {
        COUT << "got data in a " << sc << endl;

        switch( sc )
        {
        case Download :
            {
                string filename = buffer.getString( );
                ItemMap*  cl    = &it->second;
                ItemMap_I mapi = cl->find( filename );
                if( mapi != cl->end() )
                {
                    ItemSockMap_I smi = _currentItems.find( sock );
                    if( smi != _currentItems.end() )
                    {
                        smi->second->changeState( Completed, DownloadInterrupted );
                        _currentItems.erase( smi );
                    }
                    cl->erase( mapi );

                    short sz = buffer.getShort( );
                    mapi->second->setSize( sz );
                    mapi->second->append( buffer.getBuffer(sz), sz );
                    mapi->second->changeState( Completed, Ok );
                }
                else
                {
                    COUT << "file " << filename << " not in request list any more" << endl;
                    return;
                }
            }
            break;
        case DownloadFirstFragment :
            {
                string    filename = buffer.getString( );
                ItemMap*  cl = &it->second;
                ItemMap_I mapi = cl->find( filename );
                if( mapi != cl->end() )
                {
                    ItemSockMap_I smi = _currentItems.find( sock );
                    if( smi != _currentItems.end() )
                    {
                        smi->second->changeState( Completed, DownloadInterrupted );
                        _currentItems.erase( smi );
                    }
                    _currentItems.insert( ItemSockPair(sock,mapi->second) );
                    cl->erase( mapi );

                    int   len = buffer.getInt32( );
                    short sz  = buffer.getShort( );
                    mapi->second->setSize( len );
                    mapi->second->append( buffer.getBuffer(sz), sz );
                    mapi->second->changeState( FragmentReceived, Ok );
                }
                else
                {
                    COUT << "file " << filename << " not in request list any more" << endl;
                    return;
                }
            }
            break;
        case DownloadFragment :
        case DownloadLastFragment :
            {
                ItemSockMap_I smi = _currentItems.find( sock );
                if( smi != _currentItems.end() )
                {
                    short sz = buffer.getShort( );
                    smi->second->append( buffer.getBuffer(sz), sz );
                    if( sc == DownloadFragment )
                    {
                        smi->second->changeState( FragmentReceived, Ok );
                    }
                    else
                    {
                        smi->second->changeState( Completed, Ok );
                        _currentItems.erase( smi );
                    }
                }
                else
                {
                    COUT << "no current item for this socket" << endl;
                    return;
                }
            }
            break;
        default :
            COUT << "programming error" << endl;
            break;
        }
    }
}

bool Manager::private_lower_test_access( Item* i )
{
    COUT << "Enter " << __PRETTY_FUNCTION__ << endl;

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

/*------------------------------------------------------------*
 * definition VsnetDownload::Client::FileSet::NotifyConclusion
 *------------------------------------------------------------*/

class FileSet::NotifyConclusion : public Notify
{
public:
    NotifyConclusion( FileSet* f, std::string s );
    virtual ~NotifyConclusion( );

    virtual void notify( State s, Error e );

private:
    FileSet*    _fileset;
    std::string _file;
};

FileSet::NotifyConclusion::NotifyConclusion( FileSet* f, std::string s )
    : _fileset( f )
    , _file( s )
{
}

FileSet::NotifyConclusion::~NotifyConclusion( )
{
}

void FileSet::NotifyConclusion::notify( State s, Error e )
{
    if( s==Completed ) _fileset->update( _file, (e==Ok) );
}

/*------------------------------------------------------------*
 * definition VsnetDownload::Client::FileSet
 *------------------------------------------------------------*/

FileSet::FileSet( boost::shared_ptr<Manager> mgr,
                  SOCKETALT                  sock,
                  std::list<std::string>     filenames,
                  std::string                path )
    : _to_go( 0 )
{
    std::list<std::string>::const_iterator cit;
    std::list<Item*>                       items;

    for( cit=filenames.begin(); cit!=filenames.end(); cit++ )
    {
        _files.insert( std::pair<std::string,int>( *cit, -1 ) );
        NotifyPtr ptr( new NotifyConclusion( this, *cit ) );
        items.push_back( new File( sock, *cit, path, ptr ) );
        _to_go++;
    }

    mgr->addItems( items );
}

bool FileSet::isDone( ) const
{
    return ( _to_go == 0 );
}

void FileSet::update( std::string s, bool v )
{
    std::map<std::string,int>::iterator it;
    it = _files.find( s );
    if( it != _files.end() && it->second == -1 )
    {
        it->second = (v ? 1 : 0 );
        _to_go--;
    }
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
    COUT << "Enter " << __PRETTY_FUNCTION__ << endl;

    const char** c = local_search_paths;
    while( *c != NULL )
    {
        _local_search_paths.push_back( *c );
        c++;
    }
}

void Manager::addCmdDownload( SOCKETALT sock, NetBuffer& buffer )
{
    COUT << "Enter " << __PRETTY_FUNCTION__ << endl;

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
                respbuffer.addChar( ok ? 1 : 0 );
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
    COUT << "Enter " << __PRETTY_FUNCTION__ << endl;

    _lower_download.erase( s );
}

void Manager::lower_check_queues( )
{
    _download_mx.lock( );
    while( _download.empty() == false )
    {
        COUT << "Enter " << __PRETTY_FUNCTION__ << endl;

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

    list<SOCKETALT>                 tbd; // to be deleted
    list<SOCKETALT>::const_iterator tbdi;

    ItemMap_I it;
    for( it=_lower_download.begin(); it != _lower_download.end(); it++ )
    {
        COUT << "Enter " << __PRETTY_FUNCTION__ << endl;

        bool done = private_lower_try_push_queue( it->first, it->second );
        if( done )
        {
            tbd.push_back( it->first );

            /* This is expensive but I don't dare to call erase() here
             * because I don't know whether the STL standard allows iterator
             * invalidation for map<>. And anyway, MSVC doesn't care about
             * standards, so better safe than sorry.
             */
        }
    }

    for( tbdi=tbd.begin(); tbdi!=tbd.end(); tbdi++ )
    {
        _lower_download.erase( *tbdi );
    }
}

/** Try to push packets onto the socket's queue, if it's not too full.
 *  In case that the queue for this socket runs empty, return true,
 *  otherwise return false.
 */
bool Manager::private_lower_try_push_queue( SOCKETALT sock, ItemQueuePtr q )
{
    COUT << "Enter " << __PRETTY_FUNCTION__ << endl;

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
                 * 4 bytes total file size
                 * 2 bytes payload length.
                 */
                l = l - Packet::getHeaderLength() - 1 - 2 - f.length() - 4 - 2;
            }
            else
            {
                firstfrag = false;
                l = l - Packet::getHeaderLength() - 1 - 2;
            }

            if( item->remainingSize() <= l )
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
                    respbuffer.addInt32( item->remainingSize() );
                }
                else
                {
                    respbuffer.addChar( DownloadFragment );
                }
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
    return q->empty();
}

bool Manager::private_test_access( const string& file )
{
    COUT << "Enter " << __PRETTY_FUNCTION__ << endl;

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
    COUT << "Enter " << __PRETTY_FUNCTION__ << endl;

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
    COUT << "Enter " << __PRETTY_FUNCTION__ << endl;

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
    FILE* f = fopen( name , "r" );
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

using namespace VsnetDownload::Client;
#define CASE(a) case a: return #a; break;

const char * getState( State s)
{
	switch( s)
	{
		CASE( Idle);
		CASE( Queued);
		CASE( Resolving);
		CASE( Resolved);
		CASE( Requested);
		CASE( FragmentReceived);
		CASE( Completed);
		default:
			return "Unknown state";
	}
}

const char * getError( Error e)
{
	switch( e)
	{
		CASE( Ok);
		CASE( SocketError);
		CASE( FileNotFound);
		CASE( LocalPermissionDenied);
		CASE( RemotePermissionDenied);
		CASE( DownloadInterrupted);
		default:
			return "Unknown error";
	}
}

