#ifndef VSNET_SERVERSOCKET_H
#define VSNET_SERVERSOCKET_H

#include <config.h>

/* 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "vsnet_address.h"
#include "vsnet_socketset.h"
#include "vsnet_socket.h"

struct ServerSocket : public VsnetSocketBase
{
protected:
    AddressIP _srv_ip; // own IP address structure of this server

public:
    ServerSocket( )
        : VsnetSocketBase(0)
    { }

    ServerSocket( int fd, const AddressIP& adr, SocketSet* set )
        : VsnetSocketBase( fd, set )
    {
        _srv_ip = adr;
    }

    ServerSocket( int fd, const AddressIP& adr )
        : VsnetSocketBase( fd )
    {
        _srv_ip = adr;
    }

    ServerSocket( const ServerSocket& orig )
        : VsnetSocketBase( orig )
    {
        _srv_ip  = orig._srv_ip;
    }

    ServerSocket& operator=( const ServerSocket& orig )
    {
        VsnetSocketBase::operator=( orig );
        _srv_ip  = orig._srv_ip;
       	return *this;
    }

    inline int  get_udp_sock( ) const { return _fd; }
    inline int  get_tcp_sock( ) const { return _fd; }
    inline const AddressIP& get_adr( ) const { return _srv_ip; }

    inline bool isActive( SocketSet& set ) const { return set.is_set(_fd); }

    virtual SOCKETALT acceptNewConn( SocketSet& set, bool addToSet ) = 0;

    friend std::ostream& operator<<( std::ostream& ostr, const ServerSocket& s );
    friend bool operator==( const ServerSocket& l, const ServerSocket& r );

protected:
    virtual void child_disconnect( const char *s );
};

class ServerSocketTCP : public ServerSocket
{
public:
    ServerSocketTCP( );
    ServerSocketTCP( int fd, const AddressIP& adr, SocketSet* set );
    ServerSocketTCP( int fd, const AddressIP& adr );
    ServerSocketTCP( const ServerSocketTCP& orig );

    ServerSocketTCP& operator=( const ServerSocketTCP& orig )
    {
        ServerSocket::operator=( orig );
        return *this;
    }

	// Accept a new connection
	virtual SOCKETALT		acceptNewConn( SocketSet& set, bool addToSet );
};

class ServerSocketUDP : public ServerSocket
{
public:
	ServerSocketUDP( );
	ServerSocketUDP( int fd, const AddressIP& adr, SocketSet* set );
	ServerSocketUDP( int fd, const AddressIP& adr );
	ServerSocketUDP( const ServerSocketUDP& orig );

	ServerSocketUDP& operator=( const ServerSocketUDP& orig )
	{
		ServerSocket::operator=( orig );
		return *this;
	}

	// Accept a new connection
	virtual SOCKETALT		acceptNewConn( SocketSet& set, bool addToSet );
};

#endif /* VSNET_SERVERSOCKET_H */

