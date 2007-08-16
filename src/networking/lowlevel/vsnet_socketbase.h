#ifndef VSNET_SOCKETBASE_H
#define VSNET_SOCKETBASE_H

#include <config.h>

// #include "boost/shared_ptr.hpp"

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

#include "vsnet_debug.h"
#include "vsnet_address.h"
#include "vsnet_socketset.h"

class SocketSet;
class Packet;
class SOCKETALT;
class PacketMem;

// Number of times we resend a "reliable" packet in UDP mode
// #define NUM_RESEND 3

class VsnetSocketBase
{
    DECLARE_VALID
public:
    VsnetSocketBase( int fd, const char* socktype, SocketSet& set );

    virtual ~VsnetSocketBase( );

    bool valid() const;

    int  get_fd() const;
    int  close_fd( );

    const char* get_socktype() const;

    bool set_block( );
    bool set_nonblock( );
    bool get_nonblock( ) const;

    virtual bool isActive( ) = 0;

    void disconnect( const char *s );
    
    virtual bool lower_selected( int datalen=-1 ) = 0;

    virtual bool need_test_writable( ) { return false; }
    virtual bool write_on_negative( ) {return false;}
    virtual int  get_write_fd( ) const { return _fd; }
    virtual int  lower_sendbuf( ) { return 0; }
    /** Called when we noticed that the primary file descriptor is closed
     *  but data remains in the send queue.
     */
    virtual void lower_clean_sendbuf( ) { }
    virtual bool isReadyToSend(fd_set*);//i.e. can call send_lower based on fd_set

protected:
    virtual void child_disconnect( const char* s ) { }

protected:
	// _fd may need to change for HTTP if server closes connection.
    int        _fd;

private:
    /// variable meant to figure out what type of socket triggered select
    char*      _socktype;
protected:
    SocketSet& _set;

private:
    // Indicates whether a socket is in blocking or non-blocking mode.
    // Necessary since WIN32 does not allow testing.
	int _noblock;
private:
    VsnetSocketBase( );
    VsnetSocketBase( const VsnetSocketBase& orig );
    VsnetSocketBase& operator=( const VsnetSocketBase& orig );
};

class VsnetSocket : public VsnetSocketBase
{
public:
    VsnetSocket( int              sock,
                 const AddressIP& remote_ip,
                 const char*      socktype,
                 SocketSet&       set );
    virtual ~VsnetSocket( );

    virtual bool isTcp() const = 0;

    virtual int  optPayloadSize( ) const = 0;
    virtual int  queueLen( int pri ) = 0;

    bool eq( const VsnetSocket& r ) const;
    bool lt( const VsnetSocket& r ) const;
    bool sameAddress( const VsnetSocket& r ) const;

    // virtual int  sendbuf( PacketMem& packet, const AddressIP* to, int pcktflags ) = 0;
    virtual int  sendbuf( Packet* packet, const AddressIP* to, int pcktflags ) = 0;

    /** This function copies or moves data into the given PacketMem variable.
     *  It is preferred over the other recvbuf function because it may reduce
     *  the number of copy operations by at least one.
     */
    virtual int  recvbuf( Packet* p, AddressIP* from ) = 0;

    friend std::ostream& operator<<( std::ostream& ostr, const VsnetSocket& s );

    virtual void dump( std::ostream& ostr ) const = 0;
	
	const AddressIP &getRemoteAddress() const { return _remote_ip; }
  virtual  bool setRemoteAddress(const AddressIP&){return false;}
    // virtual void ack( ) = 0;

protected:
    AddressIP  _remote_ip; // IP address structure of remote server

private:
    VsnetSocket( );
    VsnetSocket( const VsnetSocket& orig );
    VsnetSocket& operator=( const VsnetSocket& orig );
};


#endif /* VSNET_SOCKETBASE_H */

