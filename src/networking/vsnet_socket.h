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

#include "GCPtr.h"
#include "vsnet_address.h"
#include "vsnet_socketset.h"
#include "packetmem.h"

class SocketSet;

int close_socket( int fd );

class VsnetSocketBase
{
public:
    VsnetSocketBase( );
    VsnetSocketBase( int fd );
    VsnetSocketBase( int fd, SocketSet* set );
    VsnetSocketBase( const VsnetSocketBase& orig );

    virtual ~VsnetSocketBase( );

    VsnetSocketBase& operator=( const VsnetSocketBase& orig );

    bool valid() const;

    int  get_fd() const;

    bool set_block( );
    bool set_nonblock( );
    bool get_nonblock( ) const;

    void watch( SocketSet& set );
    void disconnect( const char *s, bool fexit );
    
    virtual bool needReadAlwaysTrue( ) const { return false; }

protected:
    virtual void child_watch( SocketSet& set ) { }
    virtual void child_disconnect( const char* s ) { }

protected:
    int        _fd;

private:
	// bits for boolean operations
	unsigned   _noblock : 1;

    SocketSet* _set;
};

class VsnetSocket : public VsnetSocketBase
{
protected:
    AddressIP  _remote_ip; // IP address structure of remote server

public:
    VsnetSocket( );
    VsnetSocket( int sock, const AddressIP& remote_ip );
    VsnetSocket( int sock, const AddressIP& remote_ip, SocketSet* set );
    VsnetSocket( const VsnetSocket& orig );

    VsnetSocket& operator=( const VsnetSocket& orig );

    virtual bool isTcp() const = 0;

    virtual bool isActive( SocketSet& set ) = 0;

    bool eq( const VsnetSocket& r );
    bool sameAddress( const VsnetSocket& r );

    virtual int  sendbuf( PacketMem& packet, const AddressIP* to) = 0;

    /** This function copies or moves data into the given PacketMem variable.
     *  It is preferred over the other recvbuf function because it may reduce
     *  the number of copy operations by at least one.
     */
    virtual int  recvbuf( PacketMem& buffer, AddressIP* from ) = 0;

    virtual int  recvbuf( void *buffer, unsigned int &len, AddressIP *from) = 0;

    virtual void ack( ) = 0;


    friend std::ostream& operator<<( std::ostream& ostr, const VsnetSocket& s );

    virtual void dump( std::ostream& ostr ) const = 0;
};


class SOCKETALT
{
    GCPtr<VsnetSocket> _sock;

public:
    LOCALCONST_DECL(bool,TCP,1)
    LOCALCONST_DECL(bool,UDP,0)

public:
    SOCKETALT( );
    SOCKETALT( int sock, bool mode, const AddressIP& remote_ip );
    SOCKETALT( int sock, bool mode, const AddressIP& remote_ip, SocketSet* set );
    SOCKETALT( const SOCKETALT& orig );

    SOCKETALT& operator=( const SOCKETALT& orig );

    inline int get_fd() const {
        return (_sock.isNull() ? -1 : _sock->get_fd());
    }

    inline bool valid() const {
        return (_sock.isNull() ? false : _sock->valid());
    }

    inline void watch( SocketSet& set ) {
        if(!_sock.isNull()) _sock->watch(set);
    }

    inline bool isTcp( ) {
        return (_sock.isNull() ? false : _sock->isTcp());
    }

    inline bool isActive( SocketSet& set ) {
        return (_sock.isNull() ? false : _sock->isActive(set));
    }

    inline int  sendbuf( PacketMem& packet, const AddressIP* to) {
        return ( _sock.isNull() ? -1 : _sock->sendbuf( packet, to ) );
    }

    inline bool set_nonblock( ) {
        return ( _sock.isNull() ? false : _sock->set_nonblock() );
    }

    inline void ack( ) {
        if( !_sock.isNull() ) _sock->ack( );
    }

    inline int recvbuf( void *buffer, unsigned int &len, AddressIP *from) {
        return ( _sock.isNull() ? -1 : _sock->recvbuf( buffer, len, from ) );
    }

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

