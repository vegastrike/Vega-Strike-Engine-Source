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
#include "vsnet_thread.h"

#include <queue>

struct ServerSocket : public VsnetSocketBase
{
protected:
    AddressIP _srv_ip; // own IP address structure of this server

public:
    ServerSocket( int fd, const AddressIP& adr, SocketSet& set )
        : VsnetSocketBase( fd, set )
    {
        _srv_ip = adr;
    }

    inline const AddressIP& get_adr( ) const { return _srv_ip; }

    virtual SOCKETALT acceptNewConn( ) = 0;

    friend std::ostream& operator<<( std::ostream& ostr,
                                     const ServerSocket& s );
    friend bool operator==( const ServerSocket& l, const ServerSocket& r );

protected:
    virtual void child_disconnect( const char *s );

private:
    ServerSocket( );
    ServerSocket( const ServerSocket& orig );
    ServerSocket& operator=( const ServerSocket& orig );
};

class ServerSocketTCP : public ServerSocket
{
public:
    ServerSocketTCP( int fd, const AddressIP& adr, SocketSet& set );

    virtual bool isActive( );

	// Accept a new connection
	virtual SOCKETALT acceptNewConn( );

    virtual void lower_selected( );

private:
    std::queue<SOCKETALT> _accepted_connections;
    VSMutex               _ac_mx;

private:
    ServerSocketTCP( );
    ServerSocketTCP( const ServerSocketTCP& orig );
    ServerSocketTCP& operator=( const ServerSocketTCP& orig );
};

class ServerSocketUDP : public ServerSocket
{
public:
	ServerSocketUDP( int fd, const AddressIP& adr, SocketSet& set );

    virtual bool isActive( );

	// Accept a new connection
	virtual SOCKETALT acceptNewConn( );

private:
	ServerSocketUDP( );
	ServerSocketUDP( const ServerSocketUDP& orig );
	ServerSocketUDP& operator=( const ServerSocketUDP& orig );
};

#endif /* VSNET_SERVERSOCKET_H */

