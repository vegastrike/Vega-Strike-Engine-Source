//#include <sys/time.h>
#include <unistd.h>
#include <math.h>
#include "packet.h"
#include "lin_time.h"

#ifdef _UDP_PROTO
    char    nbpackets = 0;
    int     char_size = sizeof( char)*8;
#endif

static  unsigned int microtime;
LOCALCONST_DEF(Packet,unsigned short,header_length,sizeof( struct Header))

ostream& operator<<( ostream& ostr, PCKTFLAGS f )
{
    int flags = f;
    if( flags == 0 ) ostr << "NONE"; return ostr;
    if( flags & SENDANDFORGET ) ostr << "SENDANDFORGET ";
    if( flags & SENT ) ostr << "SENT ";
    if( flags & RESENT ) ostr << "RESENT ";
    if( flags & ACKED ) ostr << "ACKED ";
    if( flags & SENDRELIABLE ) ostr << "SENDRELIABLE ";
    flags &= ~0x1f;
    if( flags != 0 ) ostr << hex << flags;
    return ostr;
}

Packet::Packet()
{
    h.command     = 0;
    h.serial      = 0;
    h.timestamp   = 0;
    h.data_length = 0;

    flags = NONE;
    nbsent = 0;
    destaddr = NULL;
}

Packet::Packet( const void* buffer, size_t sz )
{
    if( sz >= header_length )
    {
#if 0
        _packet = PacketMem( buffer, sz );
#else
        PacketMem mem( buffer, sz );
        _packet = mem;
#endif

	assert( ((const char*)buffer)[0] == _packet.getConstBuf()[0] );
	h.ntoh( _packet.getConstBuf() );
	assert( ((const char*)buffer)[0] == _packet.getConstBuf()[0] );
        sz -= header_length;
        if( h.data_length > sz )
        {
            COUT << "Packet not correctly received, not enough data for buffer" << endl
	         << "    should be still " << h.data_length << " but buffer has only " << sz << endl;
	    display( __FILE__, __LINE__ );
        }
	else
	{
	    COUT << "Parsed a packet with"
	         << " cmd=" << Cmd(h.command) << "(" << (int)h.command << ")"
	         << " ser=" << h.serial
	         << " ts=" << h.timestamp
	         << " len=" << h.data_length
	         << endl;
	    assert( ((const char*)buffer)[0] == _packet.getConstBuf()[0] );
	    assert( ((unsigned char*)buffer)[0] == h.command );
	}
    }
    else
    {
        COUT << "Packet not correctly received, not enough data for header" << endl;
    }
    flags = NONE;
    nbsent = 0;
    destaddr = NULL;
}

Packet::Packet( const Packet &a )
{
    destaddr = 0;
    copyfrom( a );
}

Packet::~Packet()
{
    if( destaddr ) delete destaddr;
}

void Packet::copyfrom( const Packet& a )
{
    h.command     = a.h.command;
    h.serial      = a.h.serial;
    h.timestamp   = a.h.timestamp;
    h.data_length = a.h.data_length;
    _packet       = a._packet;
    flags         = a.flags;
    nbsent        = a.nbsent;

    if( a.destaddr ) {
        if( destaddr==NULL ) {
            destaddr = new AddressIP;
        }
        memcpy( destaddr, a.destaddr, sizeof( AddressIP));
    } else if( destaddr ) {
        delete destaddr;
        destaddr = 0;
    }

    socket = a.socket;
}

void    Packet::setNetwork( const AddressIP * dst, SOCKETALT sock)
{
    if( destaddr == NULL )
    {
        if( dst != NULL )
        {
            destaddr = new AddressIP;
            memcpy( destaddr, dst, sizeof( AddressIP));
        }
    }
    else
    {
        if( dst != NULL )
        {
            memcpy( destaddr, dst, sizeof( AddressIP));
        }
        else
        {
            delete destaddr;
            destaddr = NULL;
        }
    }
    this->socket = sock;
}

int Packet::send( Cmd cmd, ObjSerial nserial, char * buf, unsigned int length, enum PCKTFLAGS prio, const AddressIP* dst, const SOCKETALT& sock, const char* caller_file, int caller_line )
{
    create( cmd, nserial, buf, length, prio, dst, sock, caller_file, caller_line );
    return send( );
}

void Packet::create( Cmd cmd, ObjSerial nserial, char * buf, unsigned int length, enum PCKTFLAGS prio, const AddressIP* dst, const SOCKETALT& sock, const char* caller_file, int caller_line )
{
    COUT << "enter " << __PRETTY_FUNCTION__ << endl
	 << "*** from " << caller_file << ":" << caller_line << endl
         << "*** send " << cmd << " ser=" << nserial << ", "
         << length << " bytes to socket " << sock << endl;

    this->flags = prio;
    // Get a timestamp for packet (used for interpolation on client side)
    double curtime = getNewTime();
    microtime = (unsigned int) (floor(curtime*1000));
    h.timestamp = microtime;
    h.command = cmd;

    // buf is an allocated char * containing message
    if( length > MAXBUFFER)
    {
        cout<<"Error : initializing network packet with data length > MAX (length="<<length<<")"<<endl;
        cout<<"Command was : " << Cmd(h.command) << endl;
        exit(1);
    }
    h.serial = nserial;
    h.data_length = length;
    
    _packet = PacketMem( length + header_length );
    char* c = _packet.getVarBuf( );
    h.hton( c );
    memcpy( &c[header_length], buf, length );
    COUT << "Created a packet of length " << length+header_length << " for sending" << endl;
    _packet.dump( cout, 0 );

    if( destaddr == NULL )
    {
        if( dst != NULL )
        {
            destaddr = new AddressIP;
            memcpy( destaddr, dst, sizeof( AddressIP));
        }
    }
    else
    {
        if( dst != NULL )
        {
            memcpy( destaddr, dst, sizeof( AddressIP));
        }
        else
        {
            delete destaddr;
            destaddr = NULL;
        }
    }

    socket = sock;
}

void    Packet::display( const char* file, int line )
{
    cout << "*** " <<  file << ":" << line << " " << endl;
    cout << "*** Packet display -- Command : " << Cmd(h.command)
         << " - Serial : " << h.serial << " - Flags : " << this->flags << endl;
    cout<<"***                   Size   : " << getDataLength() + header_length << endl;
    cout<<"***                   Buffer : " << endl;
    _packet.dump( cout, 4 );
}

void    Packet::displayHex()
{
    cout<<"Packet : "<<h.command<<" | "<<h.serial<<" | ";
    const char* c = _packet.getConstBuf();
    for( size_t i=0; i<_packet.len(); i++)
        cout<<c[i]<<" ";
    cout<<endl;
}

void Packet::Header::ntoh( const char* buf )
{
    // TO CHANGE IF ObjSerial IS NOT A SHORT ANYMORE
    const Header* h = (const Header*)buf;
    command         = h->command;
    serial          = ntohs( h->serial);
    timestamp       = ntohl( h->timestamp);
    data_length     = ntohs( h->data_length);
}

void Packet::Header::hton( char* buf )
{
    // TO CHANGE IF ObjSerial IS NOT A SHORT ANYMORE
    Header* h      = (Header*)buf;
    h->command     = command;
    h->serial      = htons( serial);
    h->timestamp   = htonl( timestamp);
    h->data_length = htons( data_length);
}

int Packet::send( )
{
    COUT << "sending " << Cmd(h.command) << " through " << socket << " to ";
    if( destaddr==0 )
        cout << "NULL" << endl;
    else
        cout << *destaddr << endl;

    int ret;
    if( (ret = socket.sendbuf( _packet, destaddr )) == -1)
    {
        h.ntoh( _packet.getConstBuf() );
        perror( "Error sending packet ");
	cout << Cmd(h.command) << endl;
    }
    else
    {
	COUT << "packet after sendbuf: " << endl;
        h.ntoh( _packet.getConstBuf() );

	PacketMem m( _packet.getVarBuf(), _packet.len(), PacketMem::LeaveOwnership );
	m.dump( cout, 3 );
    }
    return ret;
}

void Packet::ack( )
{
    socket.ack( );
}

char* Packet::getData()
{
    char* c = _packet.getVarBuf();
    c += header_length;
    return c;
}

const char* Packet::getData() const
{
    const char* c = _packet.getConstBuf();
    c += header_length;
    return c;
}

#if 0
// /*** Receive all the packets in the network buffer and adds them to the list ***/
// /*** Also sends back an ack in UDP mode ***/
// int     PacketQueue::receive( NetUI * net, SOCKETALT & sock, AddressIP & ipaddr, ObjSerial ser)
// {
//     // Read the networkfor packets
//     // Can receive 8 packets at the same time for now ? Seems reasonable
//     int nb=0;
//     unsigned int len = MAXBUFFER*8;
//     char buffer[MAXBUFFER*8];
//     unsigned int recvsize=0, totalsize=0;
// 
//     if( (net->recvbuf( sock, buffer, len, &ipaddr)) <= 0 )
//     {
//         //perror( "Error recv -1 ");
//         nb = -1;
//         //net->closeSocket( this->clt_sock);
//     }
//     else
//     {
//         // Until we haven't read all the buffer
//         while( totalsize<=len)
//         {
//             if( len<MAXBUFFER)
//                 recvsize = len;
//             else
//                 recvsize = Packet::header_length+MAXBUFFER;
//             totalsize += recvsize;
//             // We receive packets
//             Packet p( buffer, recvsize );
//             packets.push_back( p);
// #ifdef _UDP_PROTO
//             // Prepare an ACK for the packet
//             p.create( CMD_ACK, ser, NULL, 0, SENDRELIABLE, p.destaddr, p.socket, __FILE__, __LINE__ );
// 	    p.send( );
// #endif
//             nb++;
//         }
//     }
//     return nb;
// }
// 
// /*** Returns the front element from the list and remove it ***/
// Packet PacketQueue::getNextPacket()
// {
//     Packet ret = packets.front();
//     packets.pop_front();
//     return ret;
// }
// 
// /*** Mark a packet as ACKED for removal from the list ***/
// void PacketQueue::ack( Packet p)
// {
//     for( PaI i=packets.begin(); i!=packets.end(); i++)
//     {
//         if( i->h.timestamp==p.h.timestamp && i->h.serial==p.h.serial)
//             i->flags = ACKED;
//     }
// }
// 
// /*** Sends or resends the packets and removes those that are ACKED or never were ***/
// void PacketQueue::send( NetUI * net)
// {
//     cout << __FILE__ << ":" << __LINE__ << " enter " << __PRETTY_FUNCTION__ << endl
//          << "    packets in queue: " << packets.size() << endl;
// 
//     // In UDP, we should wait for a timeout before sending the packet again.
//     bool timeout_expired = false;
//     double now = getNewTime();
//     int ret;
//     for( PaI i=packets.begin(); i!=packets.end(); i++)
//     {
//         // Here compute if the ack timeout has been reached
//         if( i->nbsent == 0 )
//             timeout_expired = true;
//         else if( i->nbsent > 0 && ((now - i->h.timestamp)*(i->nbsent) > (UDP_TIMEOUT*(i->nbsent)) ) )
//             timeout_expired = true;
//         // See if the packet has to be sent
//         if( i->flags == SENDANDFORGET || (i->flags == SENDRELIABLE && i->nbsent < NUM_RESEND && timeout_expired))
//         {
// 	    	cout << __FILE__ << ":" << __LINE__ << " sending" << endl;
//             if( (ret = i->send()) == -1)
//             {
//                 perror( "Error sending packet ");
//                 i->displayCmd();
//             }
//             i->oldtimestamp = newTimeToLong( now );
//             i->nbsent++;
//         }
// 	else
// 	{
// 	    cout << __FILE__ << ":" << __LINE__ << " don't send packet: ";
// 	    if( i->flags == SENDANDFORGET ) cout << "in spite of SENDANDFORGET";
// 	    else if( i->flags == SENDRELIABLE )
// 	    {
// 	        if( i->nbsent >= NUM_RESEND ) cout << "resent too often";
// 		else if( !timeout_expired ) cout << "timeout not expired yet";
// 		else cout << "for no reason";
// 	    }
// 	    else
// 	    {
// 	        cout << "neither SENDANDFORGET nor SENDRELIABLE";
// 	    }
// 	    cout << endl;
// 	}
//         timeout_expired = false;
//     }
//     for( PaI j=packets.begin(); j!=packets.end(); j++)
//     {
//         // in TCP every packet has to be deleted since it is reliable
//         // In UDP mode we must not delete the packet since it may not have been received
//         //
//         // NOTE - NOTE - NOTE
//         // For now, UDP retransmission mecanism is incomplete and thus is disabled
// 
// #ifdef _UDP_PROTO
//         // Or removed because ACKED or sent too many times or a SENDANDFORGET packet has been sent
//         //if( j->flags==ACKED || j->nbsent>=NUM_RESEND || (j->flags==SENDANDFORGET && j->nbsent>0))
//         //{
// #endif
//             packets.remove( (*j));
// #ifdef _UDP_PROTO
//         //}
// #endif
//     }
// }
// 
// /*** Adds a packet to the list (in order to be sent later) ***/
// void PacketQueue::add( Packet p)
// {
//     p.display( __FILE__, __LINE__ );
//     packets.push_back( p);
// }
#endif

