#include "config.h"

#include "vsfilesystem.h"
#include "vs_globals.h"
#include "networking/lowlevel/vsnet_oss.h"
#include "networking/lowlevel/vsnet_dloadmgr.h"
#include "networking/lowlevel/vsnet_notify.h"
#include "networking/lowlevel/vsnet_cmd.h"
#include "networking/lowlevel/vsnet_parsecmd.h"
#include "networking/lowlevel/netbuffer.h"
#include "networking/lowlevel/packet.h"

using namespace std;

extern void	getZoneInfoBuffer( unsigned short zoneid, NetBuffer & netbuf);

#ifndef HAVE_ACCESS
    #ifdef R_OK
    #undef R_OK
    #endif
    #define R_OK 1

    #ifdef W_OK
    #undef W_OK
    #endif
    #define W_OK 2

    extern "C" int vs_access( const char* name, int mode );
#else

// Make it use the real access().
#define vs_access access

#endif

#include <sys/stat.h>
#ifdef _WIN32
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

namespace Client
{

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

Manager::Manager( SocketSet& sets )
    : _set( sets )
{
    COUT << "Enter " << __PRETTY_FUNCTION__ << endl;

    _local_search_paths.push_back( VSFileSystem::datadir );
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
        Item* item = (*it);
        if( item->getFilename() != "" )
        {
            COUT << "adding item: " << item->getFilename() << endl;
            _pending.push( item );;
        }
    }
    _pending_mx.unlock( );

    for( it=items.begin(); it!=items.end(); it++ )
    {
        Item* item = (*it);
        if( item->getFilename() == "" )
        {
            item->changeState( Completed, FileNotFound );
        }
        else
        {
            item->changeState( Queued );
        }
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
				netbuf.addChar( (*strit)->getFileType() );
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
                reqbuffer.addChar( (*rli)->getFileType() );
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

                    short sz = buffer.getShort( );
                    mapi->second->setSize( sz );
                    mapi->second->append( buffer.getBuffer(sz), sz );
                    mapi->second->changeState( Completed, Ok );

                    cl->erase( mapi ); // mapi is now bad!
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

                    int   len = buffer.getInt32( );
                    short sz  = buffer.getShort( );
                    mapi->second->setSize( len );
                    mapi->second->append( buffer.getBuffer(sz), sz );
                    mapi->second->changeState( FragmentReceived, Ok );

                    cl->erase( mapi );
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
        if( ::vs_access( path.c_str(), R_OK ) == 0 )
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

DownloadItem::DownloadItem( SOCKETALT sock, bool error, const string& file )
    : _sock( sock )
    , _error( error )
    , _file( file )
{
}

DownloadItem::~DownloadItem( )
{
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

/*------------------------------------------------------------*
 * definition VsnetDownload::Server::DownloadItemFile
 *------------------------------------------------------------*/

DownloadItemFile::DownloadItemFile( SOCKETALT sock, const string& file )
    : DownloadItem( sock, true, file )
    , _handle( NULL )
    , _size( 0 )
    , _offset( 0 )
{
}

DownloadItemFile::DownloadItemFile( SOCKETALT sock, const string& file, VSFileSystem::VSFile * f, size_t sz )
    : DownloadItem( sock, false, file )
    , _handle( f )
    , _size( sz )
    , _offset( 0 )
{
}

DownloadItemFile::~DownloadItemFile( )
{
    if( _handle ) delete _handle;
}

size_t DownloadItemFile::offset( ) const
{
    return _offset;
}

size_t DownloadItemFile::remainingSize( ) const
{
    return ( _size - _offset );
}

void DownloadItemFile::copyFromFile( unsigned char* buf, size_t sz )
{
    if( _handle )
    {
        _handle->Read( (char*)buf, sz );
        _offset += sz;
    }
}

/*------------------------------------------------------------*
 * definition VsnetDownload::Server::DownloadItemBuf
 *------------------------------------------------------------*/

DownloadItemBuf::DownloadItemBuf( SOCKETALT sock, const string& file, const char* buf, size_t sz )
    : DownloadItem( sock, false, file )
    , _size( sz )
    , _offset( 0 )
{
    _buf = new char[sz];
    VsnetOSS::memcpy( _buf, buf, sz );
}

DownloadItemBuf::~DownloadItemBuf( )
{
    if( _buf ) delete [] _buf;
}

size_t DownloadItemBuf::offset( ) const
{
    return _offset;
}

size_t DownloadItemBuf::remainingSize( ) const
{
    return ( _size - _offset );
}

void DownloadItemBuf::copyFromFile( unsigned char* buf, size_t sz )
{
    if( remainingSize() < sz ) sz = remainingSize();
    if( sz > 0 )
    {
        VsnetOSS::memcpy( buf, &_buf[_offset], sz );
        _offset += sz;
    }
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

Manager::Manager( SocketSet& sets )
    : _set( sets )
{
    COUT << "Enter " << __PRETTY_FUNCTION__ << endl;

    _local_search_paths.push_back( VSFileSystem::datadir );
}

void Manager::addCmdDownload( SOCKETALT sock, NetBuffer& buffer )
{
    COUT << "Enter " << __PRETTY_FUNCTION__ << endl;

    RecvCmdDownload recv( buffer );
    Subcommand c = recv.parse( );

    COUT << "   *** " << " cmd " << c << endl;

    switch( c )
    {
    case ResolveRequest :
        {
            Adapter::ResolveRequest* r = recv.asResolveRequest();

            short num = r->num;

            NetBuffer respbuffer;
            respbuffer.addChar( ResolveResponse );
            respbuffer.addShort( num );

            Adapter::ResolveRequest::iterator iter;
            for( iter = r->files.begin(); iter!=r->files.end(); iter++ )
            {
                bool   ok;
				char   ft   = iter->ft;
                string file = iter->file;
				// If we want to download a memory buffer from server access is considered ok
				if( ft==VSFileSystem::ZoneBuffer)
				{
					// We receive a filename containing a zone id so we test it exists
					int zoneid = atoi( file.c_str());
					ok = true;
				}
				else
                {
                	ok = private_test_access( file , (VSFileSystem::VSFileType) ft );
                }
                respbuffer.addString( file );
                respbuffer.addChar( ok ? 1 : 0 );
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
            Adapter::DownloadRequest* r = recv.asDownloadRequest();
            short num = r->num;

            if( num > 0 )
            {
                Adapter::DownloadRequest::iterator iter;
                for( iter = r->files.begin(); iter!=r->files.end(); iter++ )
                {
					char            ft   = iter->ft;
                    string          file = iter->file;
                    string          path = file;
                    DownloadItemPtr di;
					VSFileSystem::VSFile * f;
					// If a request for ZoneBuffer we create a VSFile based on a memory buffer
					if( ft==VSFileSystem::ZoneBuffer)
					{
						NetBuffer netbuf;
						getZoneInfoBuffer( atoi( file.c_str()), netbuf);
						// f = new VSFile( netbuf.getData(), netbuf.getDataLength());

                        DownloadItemBuf* buf;
                        buf = new DownloadItemBuf( sock, file, netbuf.getData(), netbuf.getDataLength() );
                        di.reset( buf );
					}
					else
                    {
                    	f = private_access( path , (VSFileSystem::VSFileType) ft );
                        if( f )
                        {
                            size_t bytes = f->Size();
                            di.reset( new DownloadItemFile( sock, file, f, bytes ) );
                        }
                        else
                        {
                            // Couldn't open for reading, maybe removed since resolve?
                            di.reset( new DownloadItemFile( sock, file ) );
                        }
                    }
                    _download_mx.lock( );
                    _download.push( di );
                    _download_mx.unlock( );
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

bool Manager::private_test_access( const string& file , VSFileSystem::VSFileType ft )
{
    COUT << "Enter " << __PRETTY_FUNCTION__ << endl;

    for( vector<string>::const_iterator it=_local_search_paths.begin();
         it!=_local_search_paths.end();
         it++ )
    {
        string path = *it + "/" + file;
        //if( ::vs_access( path.c_str(), R_OK ) == 0 )
		string ffile( file);
		if( LookForFile( ffile, ft) <= Ok)
        {
            COUT << "Found local file " << path.c_str() << endl;
            return true;
        }
    }
    COUT << "Didn't find local file for " << file.c_str() << endl;
    return false;
}

VSFileSystem::VSFile * Manager::private_access( string& file , VSFileSystem::VSFileType ft )
{
    COUT << "Enter " << __PRETTY_FUNCTION__ << endl;

	VSFileSystem::VSFile * f = new VSFileSystem::VSFile;
	VSFileSystem::VSError err;
	if( (err = f->OpenReadOnly( file, ft)) <=Ok)
	{
    	COUT << "Opened local file " << file << endl;
		return f;
	}
	/*
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
	*/
    COUT << "Couldn't open local file for " << file << endl;
	delete f;
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
extern "C" {
  int vs_access (const char * name, int mode ) {
    FILE *fp;
    if (mode & R_OK) {
      fp = fopen(name, "rb");
      if (fp) {
        fclose(fp);
      } else {
        return -1;
      }
    }
    if (mode & W_OK) {
      fp = fopen(name, "r+b"); //NOTE: any other mode may destroy existing data
      if (fp) {
        fclose(fp);
      } else {
        return -1;
      }
    }
    return 1;
  }

}
#endif /* HAVE_ACCESS */

#ifndef HAVE_LSTAT
int lstat( const char* name, struct stat* buf )
{
    FILE* f = VSFileSystem::OpenFile( name , "rb" );
    if( f == NULL ) return -1;

    int retval = -1;
    if( VSFileSystem::Fseek( f, 0, SEEK_END ) == 0 )
    {
        buf->st_size = VSFileSystem::Ftell( f );
        if( buf->st_size >= 0 ) retval = 0;
    }
    VSFileSystem::Close( f );
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

