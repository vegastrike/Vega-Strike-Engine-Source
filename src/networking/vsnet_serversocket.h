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

struct ServerSocket
{
protected:
    int       _fd;
    AddressIP _srv_ip; // own IP address structure of this server

    unsigned _noblock : 1;

public:
    ServerSocket( )
        : _fd(0)
        , _noblock(0)
    { }

    ServerSocket( int fd, const AddressIP& adr )
        : _fd(fd)
    {
        _srv_ip = adr;
        set_block( );
    }

    ServerSocket( const ServerSocket& orig )
    {
        _fd      = orig._fd;
        _srv_ip  = orig._srv_ip;
        _noblock = orig._noblock;
    }

    ServerSocket& operator=( const ServerSocket& orig )
    {
        _fd      = orig._fd;
        _srv_ip  = orig._srv_ip;
        _noblock = orig._noblock;
       	return *this;
    }

    bool set_block( );
    bool set_nonblock( );
    bool get_nonblock( ) const;

    inline bool valid() const { return (_fd>0); }
    inline int  get_udp_sock( ) const { return _fd; }
    inline int  get_tcp_sock( ) const { return _fd; }
    inline const AddressIP& get_adr( ) const { return _srv_ip; }

    inline void watch( SocketSet& set ) { set.setRead(_fd); }
    inline bool isActive( SocketSet& set ) const { return set.is_set(_fd); }

    virtual void      watchForNewConn( SocketSet& set, int ) = 0;
    virtual SOCKETALT acceptNewConn( SocketSet& set ) = 0;

    void      disconnect( const char *s, bool fexit = true );

    friend std::ostream& operator<<( std::ostream& ostr, const ServerSocket& s );
    friend bool operator==( const ServerSocket& l, const ServerSocket& r );
};

class ServerSocketTCP : public ServerSocket
{
public:
    ServerSocketTCP( ) : ServerSocket() { }
    ServerSocketTCP( int fd, const AddressIP& adr ) : ServerSocket( fd, adr ) { }
    ServerSocketTCP( const ServerSocketTCP& orig ) : ServerSocket(orig) { }

    ServerSocketTCP& operator=( const ServerSocketTCP& orig )
    {
        ServerSocket::operator=( orig );
        return *this;
    }

	// Accept a new connection
	virtual void            watchForNewConn( SocketSet& set, int );
	virtual SOCKETALT		acceptNewConn( SocketSet& set );
};

class ServerSocketUDP : public ServerSocket
{
public:
	ServerSocketUDP( ) : ServerSocket() { }
	ServerSocketUDP( int fd, const AddressIP& adr ) : ServerSocket( fd, adr ) { }
	ServerSocketUDP( const ServerSocketUDP& orig ) : ServerSocket(orig) { }

	ServerSocketUDP& operator=( const ServerSocketUDP& orig )
	{
		ServerSocket::operator=( orig );
		return *this;
	}

	// Accept a new connection
	virtual void            watchForNewConn( SocketSet& set, int );
	virtual SOCKETALT		acceptNewConn( SocketSet& set );
};

#endif /* VSNET_SERVERSOCKET_H */

