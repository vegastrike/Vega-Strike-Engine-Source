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
#include "vsnet_headers.h"
#include "const.h"

#include <set>

#include "vsnet_thread.h"
#include "vsnet_pipe.h"

class ServerSocket;
class VsnetSocketBase;

class SocketSet : public VSThread
{
    typedef std::set<VsnetSocketBase*>  Set;

    Set    _autoset;

    VSPipe  _thread_wakeup;
    VSMutex _thread_mx;
    VSCond  _thread_cond;
    bool    _thread_end;

    bool    _blockmain;
    size_t  _blockmain_pending;
    fd_set  _blockmain_set;
    VSMutex _blockmain_mx;
    VSCond  _blockmain_cond;

public:
    SocketSet( bool blockmainthread = false );
    ~SocketSet( );

    /** Once a socket is registered using this function, setRead is
     *  automatically called for it before each select */
    void set( VsnetSocketBase* s );

    /// The upper thread takes a socket out of the _autoset
    void unset( VsnetSocketBase* s );

    /// The upper thread waits for something to arrive on the socket
    void wait( );
    void add_pending( int fd );
    void rem_pending( int fd );

    /** Use this function liberally in you code. If you don't have a
     *  network thread, it will check select and return. If you
     *  have a place in your code where you want to wait anyway,
     *  replace your waiting function with a call to this function.
     */
    void waste_time( long sec, long usec );

    virtual void run( );

    void wakeup( );

private:
    int private_select( timeval* timeout );
    void private_wakeup( );

private:
    SocketSet( const SocketSet& ); // forbidden copy constructor
    SocketSet& operator=( const SocketSet& ); // forbidden assignment operator
};

#endif /* VSNET_SOCKETSET_H */

