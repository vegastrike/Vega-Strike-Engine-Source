#ifndef VSNET_SOCKETSET_H
#define VSNET_SOCKETSET_H

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
#if defined(_WIN32) && !defined(__CYGWIN__)
	//#warning "Win32 platform"
	#include <winsock.h>
#else
	#include <sys/socket.h>
	#include <sys/time.h>
#endif
#ifdef __APPLE__
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#endif
#include "const.h"

class SocketSet
{
    fd_set _read_set_select;
    int    _max_sock_select;
    fd_set _read_set_always_true;
    int    _max_sock_always_true;
public:
    SocketSet( );

    void setRead( int fd );
    void setReadAlwaysTrue( int fd );
    bool is_set( int fd ) const;
    bool is_setRead( int fd ) const;
    void clear( );

    int select( timeval* timeout );

private:
    SocketSet( const SocketSet& ); // forbidden copy constructor
    SocketSet& operator=( const SocketSet& ); // forbidden assignment operator
};

#endif /* VSNET_SOCKETSET_H */

