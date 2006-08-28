#ifndef VSNET_SOCKET_TCP_H
#define VSNET_SOCKET_TCP_H

#include <config.h>

#include "vsnet_headers.h"

#include <assert.h>
#include <map>
#include <list>
#include <queue>
#include <vector>
#include <sys/types.h>

#include "vsnet_socketbase.h"
#include "vsnet_thread.h"
#include "packetmem.h"

typedef unsigned char u_int8_t;
typedef unsigned int u_int32_t;

/***********************************************************************
 * VsnetTCPSocket - declaration
 ***********************************************************************/
 
class VsnetTCPSocket : public VsnetSocket
{
public:
	// Needs to be public to get around a VC++ 6 bug that doesn't allow you to define functions that return private types in that class.
    typedef boost::shared_ptr<Packet> PacketPtr;

    VsnetTCPSocket( int sock, const AddressIP& remote_ip, SocketSet& set );

    virtual ~VsnetTCPSocket( );

    virtual bool isTcp() const { return true; }

    virtual int  optPayloadSize( ) const;
    virtual int  queueLen( int pri );

    // virtual int  sendbuf( PacketMem& packet, const AddressIP* to, int pcktflags );
    virtual int  sendbuf( Packet* packet, const AddressIP* to, int pcktflags );
    virtual int  recvbuf( Packet* p, AddressIP* ipadr );

    virtual void dump( std::ostream& ostr ) const;

    virtual bool isActive( );

    virtual bool lower_selected( int datalen = -1 );

    virtual bool need_test_writable( );
    virtual int  get_write_fd( ) const;
    virtual int  lower_sendbuf( );
    virtual void lower_clean_sendbuf( );

protected:
    virtual void child_disconnect( const char *s );

protected:
	// HTTP needs to be able to add raw data to send and receive.
    /* --- BEGIN section for nonblocking receive support --- */

    struct Blob;

    /** This structure is sent as a prefix for each TCP "packet".
     *  It's contents are always in network byte order.
     */
    struct Header
    {
        u_int32_t _len;
        u_int8_t  _pri;
        u_int8_t  _flags;

        Header( ) {
            _len   = 0;
            _pri   = 1;
            _flags = 0;
	}
		
        Header( int l ) {
            _len   = htonl( l );
            _pri   = 1;
            _flags = 0;
        }

        inline u_int32_t h_len() const {
            return ntohl(_len);
        }
    };

    /** if we have received part of a TCP packet but not the complete packet,
     *  the expected length and received number of bytes are stored in
     *  _incomplete_packet. If several packets have been received at once, but
     *  the application processes them one at a time, the received, unprocessed
     *  packets are stored in the list.
     */
    Blob*                 _incomplete_packet;

    /// cpq = completed packet queue
    std::queue<PacketPtr> _cpq;
    VSMutex               _cpq_mx;

    /** We send sizeof(Header) bytes as a packet length indicator. Unfortunately,
     *  even these sizeof(Header) bytes may be split by TCP. These two variables are
     *  needed for collecting the sizeof(Header) bytes.
     *  Note: for the obvious reason that this happens rarely, the collection
     *        code can not be considered tested.
     */
    int         _incomplete_header;
    Header      _header;

    /** Closed connections are noticed in isActive but evaluated by the
     *  application after recvbuf. So, we remember the situation here until
     *  the application notices it.
     */
    bool        _connection_closed;

    /** The number of pending packets reported to the set.
     */
    void inc_pending( );
    void dec_pending( );

    /** TCP may refuse to send all bytes at once, hence we have to queue packets
     *  for sending. In addition, we use the same thread for sending and receiving,
     *  thus it would be advantageous to use asynchronous sending. For both, we
     *  need a send queue. _sq is the a of priority queues.
     *  _sq_current is the queue (with up to two entries) for sending the current
     *  packet, which consists of the packet length and the packet payload.
     *  _sq.off is the number of bytes that have already been sent from
     *  PacketMem _sq_current.front(). _sq_mx protects the queues.
     */
    typedef std::queue<PacketPtr>     SqQueue;
    typedef std::queue<PacketMem>     SqQueueP;

    class SqQueues
    {
    public:
        SqQueues( );

        bool      empty( ) const;
        int       getLength( int idx );
        void      push( int idx, PacketPtr m );
        PacketPtr pop( );

    private:
        // int                   _debug_array[4];

        std::map<int,SqQueue> _q;
        int                   _ct;

        SqQueues( const SqQueues& );
        SqQueues& operator=( const SqQueues& );
    };

    SqQueues              _sq;
    SqQueueP              _sq_current;
    size_t                _sq_off;
    VSMutex               _sq_mx;
    int                   _sq_fd;

    /* --- END section for nonblocking receive support --- */

    void inner_complete_a_packet( Blob* b );

    void private_nothread_conditional_write( );

private:
    /** This will eventually contains the MTU size estimation in bytes.
     *  Dummy for now.
     */
    int _mtu_size_estimation;

private:
    VsnetTCPSocket( );
    VsnetTCPSocket( const VsnetTCPSocket& orig );
    VsnetTCPSocket& operator=( const VsnetTCPSocket& orig );
};

#endif /* VSNET_SOCKET_TCP_H */

