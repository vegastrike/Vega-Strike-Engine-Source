#include <config.h>

#include "vsnet_headers.h"
#if !defined(_WIN32) || defined(__CYGWIN__)
#include <sys/ioctl.h>
#endif

#include "const.h"
#include "vsnet_socket.h"
#include "vsnet_sockettcp.h"
#include "vsnet_socketudp.h"
#include "vsnet_socketset.h"

using namespace std;

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
 * PCKTFLAGS
 ***********************************************************************/
 
ostream& operator<<( ostream& ostr, PCKTFLAGS f )
{
    int flags = f;
    if( flags == 0 ) ostr << "NONE"; return ostr;
    if( flags & SENDANDFORGET ) ostr << "SENDANDFORGET ";
    if( flags & SENT          ) ostr << "SENT ";
    if( flags & RESENT        ) ostr << "RESENT ";
    if( flags & ACKED         ) ostr << "ACKED ";
    if( flags & SENDRELIABLE  ) ostr << "SENDRELIABLE ";
    if( flags & LOPRI         ) ostr << "LOPRI ";
    if( flags & HIPRI         ) ostr << "HIPRI ";
    if( flags & COMPRESSED    ) ostr << "COMPRESSED ";
    flags &= ~0x101f;
    if( flags != 0 ) ostr << hex << flags;
    return ostr;
}

/***********************************************************************
 * SOCKETALT
 ***********************************************************************/
 
SOCKETALT::SOCKETALT( )
{
}

SOCKETALT::SOCKETALT( int sock, bool mode, const AddressIP& remote_ip, SocketSet& sets )
{
    if( mode == TCP )
        _sock = new VsnetTCPSocket( sock, remote_ip, sets );
    else
        _sock = new VsnetUDPSocket( sock, remote_ip, sets );
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

bool SOCKETALT::CompareLt::operator()( const SOCKETALT& l, const SOCKETALT& r )
{
    return l.lowerAddress( r );
}

bool SOCKETALT::sameAddress( const SOCKETALT& l ) const
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

bool SOCKETALT::lowerAddress( const SOCKETALT& right ) const
{
    if( _sock.isNull() )
    {
        if( right._sock.isNull() == false ) return true;
        return false;
    }
    else if( right._sock.isNull() )
    {
        return false;
    }
    else
    {
        return this->_sock->lt( *right._sock );
    }
}

/***********************************************************************
 * VsnetSocket - definition
 ***********************************************************************/

VsnetSocket::VsnetSocket( int sock, const AddressIP& remote_ip, SocketSet& sets )
    : VsnetSocketBase( sock, sets )
    , _remote_ip( remote_ip )
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
    return ( (isTcp() == r.isTcp()) && (_fd == r._fd) && (_remote_ip==r._remote_ip) );
}

bool VsnetSocket::lt( const VsnetSocket& r ) const
{
    if( !isTcp() && r.isTcp() ) return true;
    if( isTcp() == r.isTcp() )
    {
    	if( _fd < r._fd ) return true;
	    if( _fd == r._fd )
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
    set_block( );
    sets.set( this );
}

VsnetSocketBase::~VsnetSocketBase( )
{
    _set.unset( this );
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
        _set.unset( this );
    }

    child_disconnect( s );

    if( fexit )
    {
        exit(1);
    }
}

