#include <config.h>
#include <fstream>

#include "vsfilesystem.h"
#include "vsnet_dloadmgr.h"
#include "vsnet_notify.h"
#include "vsnet_debug.h"

using namespace std;

namespace VsnetDownload
{

namespace Client
{

/*------------------------------------------------------------*
 * definition VsnetDownload::Client::NotifyMe
 *------------------------------------------------------------*/

NotifyMe::NotifyMe( )
    : _state( Idle )
    , _error( Ok )
    , _total( 0 )
    , _offset( 0 )
{ }

void NotifyMe::notify( State s, VSError e )
{
    _state = s;
    _error = e;
}

void NotifyMe::setTotalBytes( int sz )
{
    _total = sz;
}

void NotifyMe::addBytes( int sz )
{
    _offset += sz;
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

VSError Item::error( ) const
{
    _mx.lock( );
    VSError ret = _error;
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
    VSError e = _error;
    _mx.unlock( );
    if( _notify ) _notify->notify( s, e );
}

void Item::changeState( State s, VSError e )
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

void Item::protected_replace_notifier( NotifyPtr ptr )
{
   _notify = ptr;
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
 * definition VsnetDownload::Client::NoteFile
 *------------------------------------------------------------*/

NoteFile::NoteFile( SOCKETALT          sock,
                    const std::string& filename,
                    std::string        localbasepath )
    : File( sock, filename, localbasepath, NotifyPtr() )
    , _me( new NotifyMe )
{
    protected_replace_notifier( _me );
}

NoteFile::NoteFile( SOCKETALT          sock,
                    const std::string& filename )
    : File( sock, filename, VSFileSystem::homedir, NotifyPtr() )
    , _me( new NotifyMe )
{
    protected_replace_notifier( _me );
}

NoteFile::~NoteFile( )
{
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
{
    COUT << "Created TestItem for downloading " << filename << endl;
}

/*------------------------------------------------------------*
 * definition VsnetDownload::Client::FileSet::NotifyConclusion
 *------------------------------------------------------------*/

class FileSet::NotifyConclusion : public Notify
{
public:
    NotifyConclusion( FileSet* f, std::string s );
    virtual ~NotifyConclusion( );

    virtual void notify( State s, VSError e );

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

void FileSet::NotifyConclusion::notify( State s, VSError e )
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

/*------------------------------------------------------------*
 * definition VsnetDownload::Client::Notify_f
 *------------------------------------------------------------*/

Notify_f::Notify_f( std::string filename, NotifyFunction fun )
    : _filename( filename )
    , _fun( fun )
    , _total( 0 )
    , _offset( 0 )
{ }

Notify_f::~Notify_f()
{ }

void Notify_f::notify( State s, VSError e )
{
    (*_fun)( _filename, s, e, _total, _offset );
}

void Notify_f::setTotalBytes( int sz )
{
    _total = sz;
}

void Notify_f::addBytes( int sz )
{
    _offset += sz;
}

/*------------------------------------------------------------*
 * definition VsnetDownload::Client::VSNotify
 *------------------------------------------------------------*/

void	VSNotify( VsnetDownload::Client::State s, VSFileSystem::VSError e)
{
	cerr << "!!! DOWNLOAD ERROR : State="<< s <<" - Error="<< e <<endl;
}

void	VsnetDownload::Client::VSNotify::setTotalBytes( int sz )
{
}

void	VsnetDownload::Client::VSNotify::addBytes( int sz )
{
}

}; // namespace Client

}; // namespace VsnetDownload
