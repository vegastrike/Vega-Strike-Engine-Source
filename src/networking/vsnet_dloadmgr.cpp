#include <config.h>
#include "vsnet_dloadmgr.h"
#include "vsnet_cmd.h"
#include "netbuffer.h"
#include "packet.h"

using namespace std;

namespace VsnetDownload
{
namespace Client
{

/*------------------------------------------------------------*
 * definition VsnetDownload::Client::Item
 *------------------------------------------------------------*/

Item::Item( const string& filename, Notify* notify )
    : _filename( filename )
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

/*------------------------------------------------------------*
 * definition VsnetDownload::Client::File
 *------------------------------------------------------------*/

File::File( const string& filename,
            string        localbasepath,
            Notify*       notify )
    : Item( filename, notify )
    , _localbasepath( localbasepath )
{
}

/*------------------------------------------------------------*
 * definition VsnetDownload::Client::Buffer
 *------------------------------------------------------------*/

Buffer::Buffer( const string& filename, Notify* notify )
    : Item( filename, notify )
{
}

/*------------------------------------------------------------*
 * definition VsnetDownload::Client::Manager
 *------------------------------------------------------------*/

Manager::Manager( SocketSet& set, SOCKETALT sock, const char** local_search_paths )
    : _set( set )
    , _sock( sock )
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
    _pending_mx.lock( );
    bool e = _pending.empty();
    _pending.push( item );
    _pending_mx.unlock( );
    item->changeState( Queued );
    if( e ) _set.wakeup( );
}

void Manager::addItems( list<Item*> items )
{
    _pending_mx.lock( );
    if( _pending.empty() )
    {
        _pending_mx.unlock( );
        return;
    }

    for( list<Item*>::iterator it=items.begin(); it!=items.end(); it++ )
    {
        _pending.push( *it );
    }
    _pending_mx.unlock( );

    for( list<Item*>::iterator it=items.begin(); it!=items.end(); it++ )
    {
        (*it)->changeState( Queued );
    }

    _set.wakeup( );
}

void Manager::lower_poll( )
{
    _pending_mx.lock( );
    if( _pending.empty() )
    {
        _pending_mx.unlock( );
        return;
    }

    list<Item*> collect;
    while( _pending.empty() == false )
    {
        Item* i = _pending.front( );
        _pending.pop( );

        _pending_mx.unlock( );
        if( lower_private_test_access( i ) )
        {
            i->changeState( Completed, Ok );
        }
        else
        {
            i->changeState( Resolving );
            collect.push_back( i );
            _asked.push( i );
        }
        _pending_mx.lock( );
    }
    _pending_mx.unlock( );

    if( collect.empty() == false )
    {
        NetBuffer netbuf;

        assert( collect.size() < 0xffff );
        netbuf.addShort( collect.size() );
        for( list<Item*>::iterator it=collect.begin(); it!=collect.end(); it++ )
        {
            netbuf.addString( (*it)->getFilename() );
        }

        Packet packet;
        packet.send( CMD_DOWNLOAD, 0,
                     netbuf.getData(), netbuf.getDataLength(),
                     SENDRELIABLE | LOPRI,
                     NULL, _sock,
                     __FILE__, PSEUDO__LINE__(258) );
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
}; // namespace VsnetDownload

