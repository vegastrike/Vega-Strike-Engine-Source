#include <config.h>

#include "vsnet_headers.h"
#if !defined(_WIN32) || defined(__CYGWIN__)
#include <sys/ioctl.h>
#endif

#include "vsnet_oss.h"
#include "vsnet_socketbase.h"
#include "vsnet_socketset.h"

using namespace std;

/***********************************************************************
 * VsnetSocket - definition
 ***********************************************************************/

VsnetSocket::VsnetSocket( int sock, const AddressIP& remote_ip, SocketSet& sets )
    : VsnetSocketBase( sock, sets )
    , _remote_ip( remote_ip )
{
}

VsnetSocket::~VsnetSocket( )
{
}

// VsnetSocket& VsnetSocket::operator=( const VsnetSocket& orig )
// {
    // VsnetSocketBase::operator=( orig );
    // _remote_ip = orig._remote_ip;
    // return *this;
// }

bool VsnetSocket::eq( const VsnetSocket& r ) const
{
    return ( (isTcp() == r.isTcp()) && (get_fd() == r.get_fd()) && (_remote_ip==r._remote_ip) );
}

bool VsnetSocket::lt( const VsnetSocket& r ) const
{
    if( !isTcp() && r.isTcp() ) return true;
    if( isTcp() == r.isTcp() )
    {
    	if( get_fd() < r.get_fd() ) return true;
	    if( get_fd() == r.get_fd() )
	    {
            if( _remote_ip < r._remote_ip ) return true;
	    }
    }
    return false;
}

bool VsnetSocket::sameAddress( const VsnetSocket& r) const
{
    const VsnetSocket* r2 = (const VsnetSocket*)&r;
    return ( (isTcp() == r2->isTcp()) && (_remote_ip==r2->_remote_ip) );
}

/***********************************************************************
 * VsnetSocketBase - definition
 ***********************************************************************/

VsnetSocketBase::VsnetSocketBase( int fd, SocketSet& sets )
    : _fd( fd )
    , _set( sets )
{
    MAKE_VALID
    set_block( );
    sets.set( this );
}

VsnetSocketBase::~VsnetSocketBase( )
{
    CHECK_VALID
    _set.unset( this );
    MAKE_INVALID
}

bool VsnetSocketBase::valid() const
{
    CHECK_VALID
    return (_fd>=0);
}

int VsnetSocketBase::get_fd() const
{
    CHECK_VALID
    return _fd;
}

int VsnetSocketBase::close_fd( )
{
    CHECK_VALID
    int ret = 0;
    if( _fd >= 0 )
    {
        ret = VsnetOSS::close_socket( _fd );
        _fd = -1;
    }
    return ret;
}

bool VsnetSocketBase::set_nonblock( )
{
    CHECK_VALID
    assert( valid() );
#if !defined(_WIN32) || defined(__CYGWIN__)
    int datato = 1;
    if( ::ioctl( _fd, FIONBIO, &datato ) == -1)
    {
        ::perror( "Error fcntl : ");
        return false;
    }
#else
    unsigned long datato = 1;
    if( ::ioctlsocket( _fd, FIONBIO, &datato ) !=0 )
    {
        ::perror( "Error fcntl : ");
        return false;
    }
#endif
    _noblock = 1;
    return true;
}

bool VsnetSocketBase::set_block( )
{
    CHECK_VALID
    assert( valid() );
#if !defined(_WIN32) || defined(__CYGWIN__)
    int datato = 0;
    if( ::ioctl( _fd, FIONBIO, &datato ) == -1)
    {
        ::perror( "Error fcntl : ");
        return false;
    }
#else
    unsigned long datato = 0;
    if( ::ioctlsocket( _fd, FIONBIO, &datato ) !=0 )
    {
        ::perror( "Error fcntl : ");
        return false;
    }
#endif
    _noblock = 0;
    return true;
}

bool VsnetSocketBase::get_nonblock( ) const
{
    CHECK_VALID
    return _noblock;
}

void VsnetSocketBase::disconnect( const char *s, bool fexit )
{
    CHECK_VALID
    if( _fd > 0 )
    {
        _set.unset( this );
    }

    child_disconnect( s );

    if( fexit )
    {
        exit(1);
    }
}

