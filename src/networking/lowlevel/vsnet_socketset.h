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

#include <config.h>
#include <iostream>
#include <errno.h>
#include <set>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "vsnet_headers.h"
#include "networking/const.h"

#include "vsnet_thread.h"
#include "vsnet_pipe.h"
#include "vsnet_debug.h"

class ServerSocket;
class VsnetSocketBase;

namespace VsnetDownload {
  namespace Client {
    class Manager;
  };
  namespace Server {
    class Manager;
  };
};

class SocketSet : public VSThread
{
    typedef std::set<VsnetSocketBase*>  Set;

    Set    _autoset;

#ifndef USE_NO_THREAD
    VSPipe  _thread_wakeup;
#endif
    VSMutex _thread_mx;
    VSCond  _thread_cond;
#ifndef USE_NO_THREAD
    bool    _thread_end;
#endif

    bool    _blockmain;
    int     _blockmain_pending;
    fd_set  _blockmain_set;
    VSMutex _blockmain_mx;
    VSCond  _blockmain_cond;

    boost::weak_ptr<VsnetDownload::Client::Manager> _client_mgr;
    boost::weak_ptr<VsnetDownload::Server::Manager> _server_mgr;

public:
    SocketSet( bool blockmainthread = false );
    ~SocketSet( );

    bool addDownloadManager( boost::shared_ptr<VsnetDownload::Client::Manager> mgr );
    bool addDownloadManager( boost::shared_ptr<VsnetDownload::Server::Manager> mgr );

    /** Once a socket is registered using this function, setRead is
     *  automatically called for it before each select */
    void set( VsnetSocketBase* s );

    /// The upper thread takes a socket out of the _autoset
    void unset( VsnetSocketBase* s );

    /// The upper thread waits for something to arrive on the socket
    int wait( timeval *tv = NULL );
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
  
  ///only call predestroy if you know the whole set will be destructed soonish--clears the autoset
    void predestroy();
private:
    int  private_select( timeval* timeout );
    void private_addset( int fd, fd_set& fds, int& maxfd );
    void private_wakeup( );

#if defined(VSNET_DEBUG) || defined(__APPLE__)
    void private_test_dump_active_sets( int           maxfd,
                                        fd_set& read_before,
                                        fd_set& read_after,
                                        fd_set& write_before,
                                        fd_set& write_after );
#endif
#ifdef VSNET_DEBUG
    void private_test_dump_request_sets( timeval* timeout );
#endif

private:
    SocketSet( const SocketSet& ); // forbidden copy constructor
    SocketSet& operator=( const SocketSet& ); // forbidden assignment operator
};

std::ostream& operator<<( std::ostream& ostr, const timeval& tv );

#endif /* VSNET_SOCKETSET_H */

