#include <config.h>

#if defined(_WIN32) && !defined(__CYGWIN__)
    //#warning "Win32 platform"
    #include <winsock.h>
#else
    #include <sys/ioctl.h>
#endif

#include "const.h"
#include "vsnet_socket.h"
#include "vsnet_sockettcp.h"
#include "vsnet_socketudp.h"
#include "vsnet_socketset.h"

int close_socket( int fd )
{
#if defined(_WIN32) && !defined(__CYGWIN__)
	return closesocket( fd );
#else
	return close( fd );
#endif
}

LOCALCONST_DEF(SOCKETALT,bool,TCP,1)
LOCALCONST_DEF(SOCKETALT,bool,UDP,0)

/***********************************************************************
 * SOCKETALT
 ***********************************************************************/
 
SOCKETALT::SOCKETALT( )
{
}

SOCKETALT::SOCKETALT( int sock, bool mode, const AddressIP& remote_ip )
{
    if( mode == TCP )
        _sock = new VsnetTCPSocket( sock, remote_ip );
    else
        _sock = new VsnetUDPSocket( sock, remote_ip );
}

SOCKETALT::SOCKETALT( int sock, bool mode, const AddressIP& remote_ip, SocketSet* set )
{
    if( mode == TCP )
        _sock = new VsnetTCPSocket( sock, remote_ip, set );
    else
        _sock = new VsnetUDPSocket( sock, remote_ip, set );
}

SOCKETALT::SOCKETALT( const SOCKETALT& orig )
{
    _sock = orig._sock;
}

SOCKETALT& SOCKETALT::operator=( const SOCKETALT& orig )
{
    _sock = orig._sock;
    return *this;
}

std::ostream& operator<<( std::ostream& ostr, const SOCKETALT& s )
{
    if( !s._sock.isNull() ) s._sock->dump( ostr );
    else ostr << "NULL";
    return ostr;
}

bool operator==( const SOCKETALT& l, const SOCKETALT& r )
{
    if( l._sock.isNull() )
    {
        return ( r._sock.isNull() ? true : false );
    }
    else if( r._sock.isNull() )
    {
        return false;
    }
    else
    {
        return l._sock->eq(*r._sock);
    }
}

bool SOCKETALT::sameAddress( const SOCKETALT& l )
{
    if( l._sock.isNull() )
    {
        return ( this->_sock.isNull() ? true : false );
    }
    else if( this->_sock.isNull() )
    {
        return false;
    }
    else
    {
        return this->_sock->eq(*l._sock);
    }
}

/***********************************************************************
 * VsnetSocket - definition
 ***********************************************************************/

VsnetSocket::VsnetSocket( )
    : VsnetSocketBase( )
{
}

VsnetSocket::VsnetSocket( int sock, const AddressIP& remote_ip, SocketSet* set )
    : VsnetSocketBase( sock, set )
    , _remote_ip( remote_ip )
{
}

VsnetSocket::VsnetSocket( int sock, const AddressIP& remote_ip )
    : VsnetSocketBase( sock )
    , _remote_ip( remote_ip )
{
}

VsnetSocket::VsnetSocket( const VsnetSocket& orig )
    : VsnetSocketBase( orig )
    , _remote_ip( orig._remote_ip )
{
}

VsnetSocket& VsnetSocket::operator=( const VsnetSocket& orig )
{
    VsnetSocketBase::operator=( orig );
    _remote_ip = orig._remote_ip;
    return *this;
}

bool VsnetSocket::eq( const VsnetSocket& r )
{
    const VsnetSocket* r2 = (const VsnetSocket*)&r;
    return ( (isTcp() == r2->isTcp()) && (_fd == r2->_fd) && (_remote_ip==r2->_remote_ip) );
}

bool VsnetSocket::sameAddress( const VsnetSocket& r)
{
    const VsnetSocket* r2 = (const VsnetSocket*)&r;
    return ( (isTcp() == r2->isTcp()) && (_remote_ip==r2->_remote_ip) );
}

/***********************************************************************
 * VsnetSocketBase - definition
 ***********************************************************************/

VsnetSocketBase::VsnetSocketBase( )
    : _fd( -1 )
    , _noblock( 0 )
    , _set( NULL )
{ }

VsnetSocketBase::VsnetSocketBase( int fd )
    : _fd( fd )
    , _set( NULL )
{
    set_block( );
}

VsnetSocketBase::VsnetSocketBase( int fd, SocketSet* set )
    : _fd( fd )
    , _set( set )
{
    set_block( );
    if( set ) set->autosetRead( this );
}

VsnetSocketBase::VsnetSocketBase( const VsnetSocketBase& orig )
    : _fd( orig._fd )
    , _noblock( orig._noblock )
    , _set( orig._set )
{
}

VsnetSocketBase::~VsnetSocketBase( )
{
    if( _set ) _set->autounsetRead( this );
}

VsnetSocketBase& VsnetSocketBase::operator=( const VsnetSocketBase& orig )
{
    _fd      = orig._fd;
    _noblock = orig._noblock;
    _set     = orig._set;
    return *this;
}

bool VsnetSocketBase::valid() const
{
    return (_fd>=0);
}

int VsnetSocketBase::get_fd() const
{
    return _fd;
}

bool VsnetSocketBase::set_nonblock( )
{
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
    return _noblock;
}

void VsnetSocketBase::disconnect( const char *s, bool fexit )
{
    if( _fd > 0 )
    {
        if( _set )
        {
            _set->autounsetRead( this );
            _set = NULL;
        }
    }

    child_disconnect( s );

    if( fexit )
    {
        exit(1);
    }
}

void VsnetSocketBase::watch( SocketSet& set )
{
    set.setRead(_fd);
    child_watch( set );
}

