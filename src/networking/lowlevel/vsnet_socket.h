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

#ifndef VSNET_SOCKET_H
#define VSNET_SOCKET_H

#include <config.h>

#include <boost/shared_ptr.hpp>

#include <iostream>

#include "vsnet_address.h"
#include "vsnet_socketset.h"
#include "packetmem.h"

class Packet;
class VsnetSocket;
class SocketSet;

class SOCKETALT
{
    typedef boost::shared_ptr<VsnetSocket> ptr;

    ptr _sock;

public:
    LOCALCONST_DECL(bool,TCP,1)
    LOCALCONST_DECL(bool,UDP,0)

public:
    SOCKETALT( );
    SOCKETALT( const SOCKETALT& orig );
    SOCKETALT( VsnetSocket* sock );
    SOCKETALT( int sock, bool mode, const AddressIP& remote_ip, SocketSet& set );
  // not actually a socketalt    SOCKETALT( std::string url, SocketSet& sets );
    SOCKETALT& operator=( const SOCKETALT& orig );

    int  get_fd() const;
    bool valid() const;
    bool isTcp() const;
    int  queueLen( int pri );
    int  optPayloadSize() const;
    bool isActive();
    // int  sendbuf( PacketMem& packet, const AddressIP* to, int pcktflags );
    int  sendbuf( Packet* packet, const AddressIP* to, int pcktflags );
    bool set_nonblock();
    bool set_block();

    int  recvbuf( Packet* p, AddressIP* ipadr );
    void disconnect( const char *s );

    bool sameAddress(  const SOCKETALT& l) const;
    bool lowerAddress( const SOCKETALT& l) const;

    friend std::ostream& operator<<( std::ostream& ostr, const SOCKETALT& s );
    friend bool          operator==( const SOCKETALT& l, const SOCKETALT& r );
	
	void addToSet( SocketSet &set );
	
	const AddressIP &getRemoteAddress() const;
  //returns false if impossible (TCP)
        bool setRemoteAddress(const AddressIP&);
	
	const AddressIP &getLocalAddress() const;
	bool setLocalAddress(const AddressIP&);
	
    /** Required by the download manager achieve a sorting of sockets.
     */
    friend class CompareLt;
    class CompareLt
    {
    public:
        bool operator()( const SOCKETALT& l, const SOCKETALT& r ) const;
    };
};

std::ostream& operator<<( std::ostream& ostr, const SOCKETALT& s );
bool          operator==( const SOCKETALT& l, const SOCKETALT& r );

#endif /* VSNET_SOCKET_H */
