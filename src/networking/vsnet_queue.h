#ifndef VSNET_SOCKET_H
#define VSNET_SOCKET_H

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

/*
  netUI - Network Interface - written by Stephane Vaxelaire <svax@free.fr>
*/

#include <iostream>
#include <errno.h>
#include <assert.h>
#include "const.h"

#if defined(_WIN32) && !defined(__CYGWIN__)
	//#warning "Win32 platform"
	#define in_addr_t unsigned long
	#include <winsock.h>
#else
	#include <sys/socket.h>
	#include <unistd.h>
#endif

#include "vsnet_address.h"
#include "vsnet_socketset.h"

class SOCKETALT
{
    int       fd;
    bool      _tcp;
    AddressIP _remote_ip; // IP address structure of remote server

public:
    static const bool TCP = true;
    static const bool UDP = false;

public:
    SOCKETALT( ) : fd(0)
    {
        _tcp = false;
    }

    SOCKETALT( int sock, bool mode, const AddressIP& remote_ip )
        : fd( sock )
        , _tcp( mode )
    {
        _remote_ip = remote_ip;
    }

    SOCKETALT( const SOCKETALT& orig )
        : fd( orig.fd )
        , _tcp( orig._tcp )
        , _remote_ip( orig._remote_ip )
    {
    }

    SOCKETALT& operator=( const SOCKETALT& orig )
    {
        _tcp   = orig._tcp;
        fd     = orig.fd;
        _remote_ip = orig._remote_ip;
        return *this;
    }

    inline int get_fd() const { return fd; }

    inline void set_fd( int sock, bool mode )
    {
        fd   = sock;
        _tcp = mode;
    }

    inline bool valid() const { return (fd>0); }

    inline void watch( SocketSet& set ) { set.set(fd); }
    inline bool isActive( SocketSet& set ) const { return set.is_set(fd); }

    int  sendbuf( void* buffer, unsigned int len, const AddressIP* to);
    void ack( );

    int  recvbuf( void *buffer, unsigned int &len, AddressIP *from);

    void disconnect( const char *s, bool fexit = true );

    friend std::ostream& operator<<( std::ostream& ostr, const SOCKETALT& s );
    friend bool operator==( const SOCKETALT& l, const SOCKETALT& r );

private:
    int  sendbuf_tcp( void* buffer, unsigned int len, const AddressIP* to );
    int  sendbuf_udp( void* buffer, unsigned int len, const AddressIP* to );

    int  recvbuf_tcp( void* buffer, unsigned int &len, AddressIP *from );
    int  recvbuf_udp( void* buffer, unsigned int &len, AddressIP *from );
};

#endif /* VSNET_SOCKET_H */

