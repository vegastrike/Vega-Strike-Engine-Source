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

SOCKETALT::SOCKETALT( const SOCKETALT& orig )
{
    _sock = orig._sock;
}

SOCKETALT& SOCKETALT::operator=( const SOCKETALT& orig )
{
    _sock = orig._sock;
    return *this;
}

void SOCKETALT::set_fd( int sock, bool mode )
{
    if( _sock.isNull() )
    {
	    _sock = (mode  ? (VsnetSocket*)new VsnetTCPSocket : (VsnetSocket*)new VsnetUDPSocket );
        _sock->set_fd( sock );
    }
    else
    {
        if( (mode==TCP && _sock->isTcp()) || (mode==UDP && !_sock->isTcp()) )
        {
	        _sock->set_fd( sock );
        }
        else
        {
	        _sock = (mode  ? (VsnetSocket*)new VsnetTCPSocket : (VsnetSocket*)new VsnetUDPSocket );
	        _sock->set_fd( sock );
        }
    }
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
    : _fd(0)
    , _noblock(0)
{
}

VsnetSocket::VsnetSocket( int sock, const AddressIP& remote_ip )
    : _fd( sock )
    , _remote_ip( remote_ip )
{
    set_block( );
}

VsnetSocket::VsnetSocket( const VsnetSocket& orig )
    : _fd( orig._fd )
    , _remote_ip( orig._remote_ip )
{
    set_block( );
}

VsnetSocket::~VsnetSocket( )
{
}

VsnetSocket& VsnetSocket::operator=( const VsnetSocket& orig )
{
    _fd     = orig._fd;
    _remote_ip = orig._remote_ip;
    return *this;
}

int VsnetSocket::get_fd() const
{
    return _fd;
}

void VsnetSocket::set_fd( int sock )
{
    _fd   = sock;
}

bool VsnetSocket::valid() const
{
    return (_fd>=0);
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

bool VsnetSocket::set_nonblock( )
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

bool VsnetSocket::set_block( )
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

bool VsnetSocket::get_nonblock( ) const
{
    return _noblock;
}

