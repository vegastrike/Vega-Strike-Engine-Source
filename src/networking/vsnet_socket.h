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

#include "GCPtr.h"
#include "vsnet_address.h"
#include "vsnet_socketset.h"
#include "packetmem.h"

class SocketSet;

int close_socket( int fd );

class VsnetSocketBase
{
public:
    VsnetSocketBase( int fd, SocketSet& set );

    virtual ~VsnetSocketBase( );

    bool valid() const;

    int  get_fd() const;

    bool set_block( );
    bool set_nonblock( );
    bool get_nonblock( ) const;

    virtual bool isActive( ) = 0;

    void disconnect( const char *s, bool fexit );
    
    virtual void lower_selected( ) { }

protected:
    virtual void child_disconnect( const char* s ) { }

protected:
    int        _fd;
    SocketSet& _set;

private:
	// bits for boolean operations
	unsigned   _noblock : 1;

private:
    VsnetSocketBase( );
    VsnetSocketBase( const VsnetSocketBase& orig );
    VsnetSocketBase& operator=( const VsnetSocketBase& orig );
};

class VsnetSocket : public VsnetSocketBase
{
public:
    VsnetSocket( int sock, const AddressIP& remote_ip, SocketSet& set );

    virtual bool isTcp() const = 0;

    bool eq( const VsnetSocket& r );
    bool sameAddress( const VsnetSocket& r );

    virtual int  sendbuf( PacketMem& packet, const AddressIP* to) = 0;

    /** This function copies or moves data into the given PacketMem variable.
     *  It is preferred over the other recvbuf function because it may reduce
     *  the number of copy operations by at least one.
     */
    virtual int  recvbuf( PacketMem& buffer, AddressIP* from ) = 0;

    friend std::ostream& operator<<( std::ostream& ostr, const VsnetSocket& s );

    virtual void dump( std::ostream& ostr ) const = 0;

    // virtual void ack( ) = 0;

protected:
    AddressIP  _remote_ip; // IP address structure of remote server

private:
    VsnetSocket( );
    VsnetSocket( const VsnetSocket& orig );
    VsnetSocket& operator=( const VsnetSocket& orig );
};


class SOCKETALT
{
    GCPtr<VsnetSocket> _sock;

public:
    LOCALCONST_DECL(bool,TCP,1)
    LOCALCONST_DECL(bool,UDP,0)

public:
    SOCKETALT( );
    SOCKETALT( const SOCKETALT& orig );
    // SOCKETALT( int sock, bool mode, const AddressIP& remote_ip );
    SOCKETALT( int sock, bool mode, const AddressIP& remote_ip, SocketSet& set );

    SOCKETALT& operator=( const SOCKETALT& orig );

    inline int get_fd() const {
        return (_sock.isNull() ? -1 : _sock->get_fd());
    }

    inline bool valid() const {
        return (_sock.isNull() ? false : _sock->valid());
    }

    inline bool isTcp( ) {
        return (_sock.isNull() ? false : _sock->isTcp());
    }

    inline bool isActive( ) {
        return (_sock.isNull() ? false : _sock->isActive());
    }

    inline int  sendbuf( PacketMem& packet, const AddressIP* to) {
        return ( _sock.isNull() ? -1 : _sock->sendbuf( packet, to ) );
    }

    inline bool set_nonblock( ) {
        return ( _sock.isNull() ? false : _sock->set_nonblock() );
    }

//     inline void ack( ) {
//         if( !_sock.isNull() ) _sock->ack( );
//     }

    inline int recvbuf( PacketMem& buffer, AddressIP *from) {
        return ( _sock.isNull() ? -1 : _sock->recvbuf( buffer, from ) );
    }

    inline void disconnect( const char *s, bool fexit = true ) {
        if( !_sock.isNull() ) _sock->disconnect( s, fexit );
    }

	bool sameAddress( const SOCKETALT& l);

    friend std::ostream& operator<<( std::ostream& ostr, const SOCKETALT& s );
    friend bool operator==( const SOCKETALT& l, const SOCKETALT& r );
};

#endif /* VSNET_SOCKET_H */

