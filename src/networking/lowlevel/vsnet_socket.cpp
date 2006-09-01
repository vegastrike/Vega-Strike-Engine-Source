#include <config.h>

#include "vsnet_headers.h"
#if !defined(_WIN32) || defined(__CYGWIN__)
#include <sys/ioctl.h>
#endif

#include "vsnet_socket.h"
#include "vsnet_sockettcp.h"
#include "vsnet_sockethttp.h"
#include "vsnet_socketudp.h"
#include "vsnet_socketset.h"


using namespace std;

LOCALCONST_DEF(SOCKETALT,bool,TCP,1)
LOCALCONST_DEF(SOCKETALT,bool,UDP,0)

/***********************************************************************
 * SOCKETALT
 ***********************************************************************/
 
SOCKETALT::SOCKETALT( )
{
}

SOCKETALT::SOCKETALT( int sock, bool mode, const AddressIP& remote_ip, SocketSet& sets )
{
    if( mode == TCP )
        _sock = ptr( new VsnetTCPSocket( sock, remote_ip, sets ) );
    else
        _sock = ptr( new VsnetUDPSocket( sock, remote_ip, sets ) );
}

SOCKETALT::SOCKETALT( const SOCKETALT& orig )
{
    _sock = orig._sock;
}

SOCKETALT::SOCKETALT( VsnetSocket* s )
{
    _sock = ptr( s );
}

SOCKETALT& SOCKETALT::operator=( const SOCKETALT& orig )
{
    _sock = orig._sock;
    return *this;
}

int SOCKETALT::get_fd() const
{
    return (!_sock ? -1 : _sock->get_fd());
}

bool SOCKETALT::valid() const
{
    if( !_sock ) return false;
    return _sock->valid();
}

bool SOCKETALT::isTcp( ) const
{
    return (!_sock ? false : _sock->isTcp());
}

int SOCKETALT::queueLen( int pri )
{
    return (!_sock ? -1 : _sock->queueLen(pri));
}

int SOCKETALT::optPayloadSize( ) const
{
    return (!_sock ? -1 : _sock->optPayloadSize());
}

std::ostream& operator<<( std::ostream& ostr, const SOCKETALT& s )
{
    if( s._sock ) s._sock->dump( ostr );
    else ostr << "NULL";
    return ostr;
}

bool operator==( const SOCKETALT& l, const SOCKETALT& r )
{
    if( !l._sock )
    {
        return ( (!r._sock) ? true : false );
    }
    else if( !r._sock )
    {
        return false;
    }
    else
    {
        return l._sock->eq(*r._sock);
    }
}

bool SOCKETALT::CompareLt::operator()( const SOCKETALT& l, const SOCKETALT& r ) const
{
    return l.lowerAddress( r );
}

bool SOCKETALT::setRemoteAddress(const AddressIP &inp){

    if( !_sock )
    {
        return false;
    }
    else
    {
        bool r = _sock->setRemoteAddress(inp);
        return r;
    }
}
bool SOCKETALT::isActive( )
{
    if( !_sock )
    {
        return false;
    }
    else
    {
        bool r = _sock->isActive();
        return r;
    }
}

int SOCKETALT::sendbuf( Packet* packet, const AddressIP* to, int pcktflags )
{
    if( !_sock || !packet )
    {
        return -1;
    }
    else
    {
        return _sock->sendbuf( packet, to, pcktflags );
    }
}
// int SOCKETALT::sendbuf( PacketMem& packet, const AddressIP* to, int pcktflags )
// {
//     return ( !_sock ? -1 : _sock->sendbuf( packet, to, pcktflags ) );
// }

bool SOCKETALT::set_nonblock( )
{
    return ( !_sock ? false : _sock->set_nonblock() );
}

bool SOCKETALT::set_block( )
{
    return ( !_sock ? false : _sock->set_block() );
}

int SOCKETALT::recvbuf( Packet* p, AddressIP* ipadr )
{
    if( !_sock || !p )
    {
        return -1;
    }
    else
    {
        int retval = _sock->recvbuf( p, ipadr );
        return retval;
    }
}

void SOCKETALT::disconnect( const char *s, bool fexit )
{
    if( _sock ) _sock->disconnect( s, fexit );
}

bool SOCKETALT::sameAddress( const SOCKETALT& l ) const
{
    if( !l._sock )
    {
        return ( (!_sock) ? true : false );
    }
    else if( !_sock )
    {
        return false;
    }
    else
    {
        return _sock->eq(*l._sock);
    }
}

bool SOCKETALT::lowerAddress( const SOCKETALT& right ) const
{
    if( !_sock )
    {
        if( right._sock ) return true;
        return false;
    }
    else if( !right._sock )
    {
        return false;
    }
    else
    {
        return this->_sock->lt( *right._sock );
    }
}

void SOCKETALT::addToSet( SocketSet &sockset )
{
	if ( _sock ) sockset.set( &(*this->_sock) );
}

const AddressIP &SOCKETALT::getRemoteAddress() const {
	static AddressIP nullAdr;
	if ( _sock ) {
		return _sock->getRemoteAddress();
	} else {
		return nullAdr; // just in case.
	}
}
