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

// SDL UDP Mode is buggy -> packet are received twice !! I don't have a clue why...

class NETCLASS
{
		AddressIP				srv_ip;				// IP Address of server
		AddressIP				clt_ip;				// IP Address of client
		SocketSet				client_set;			// Set of clients' sockets
		SocketSet				conn_set;			// Set of sockets used to listen for connections
		SOCKETALT				max_sock;			// Maximum client socket descriptor
		unsigned short			max_clients;		// Maximum number of clients authorized on server
		unsigned short			nb_clients;			// Number of clients
		char					netbuf[MAXBUFFER];	// Network message buffer
		char					server;				// 1=server, 0=client
		struct timeval			srvtimeout;

	public:
		SOCKET					sock;			// SOCKETALT

		// Constructor
		NETCLASS()
		{
			this->max_clients = MAXCLIENTS;
			srvtimeout.tv_sec = 0;
			srvtimeout.tv_usec = 0;
	#if defined(_WIN32) && !defined(__CYGWIN__)
	  cout<<"Initializing Winsock"<<endl;
	  WORD wVersionRequested = MAKEWORD( 1, 1 );
	  WSADATA wsaData; 
	  int res = WSAStartup(wVersionRequested,&wsaData);
	  if( res != 0)
		  cout<<"Error initializing Winsock"<<endl;
	#else
		cout<<"Not win32 VC++"<<endl;
	#endif
		}

		~NETCLASS()
		{
			//this->disconnect( "");
		}
		// Create a socket
		SOCKETALT		createSocket();
		// Create socket for either server or client (for server, server must be set to 1 otherwise 0)
		SOCKETALT	createSocket( char * host, unsigned short port, int srvmode);
		// Accept a new connection
		SOCKETALT	acceptNewConn( AddressIP * ipadr);
		// Receive data from network
		int		recvbuf( SOCKETALT sock, char *buffer, unsigned int &len, AddressIP *from);
		// Send data over network
		int		sendbuf( SOCKETALT sock, void *buffer, unsigned int len, AddressIP *to);

		// Return the receiving socket (used for UDP mode)
		SOCKET	getRecvSocket() { return sock;}

		// Reset a socket set
		void	resetSets()
		{
			FD_ZERO( &client_set);
		}
		// Add a socket to a set to be watched
		void	watchSocket( SOCKET bsock);
		// Checks for client activity and returns #of active clients
		int		activeSockets();
		// Check if socket is active (need to be added in a set before)
		int		isActive( SOCKET bsock);

		// Disconnect
		void	disconnect( char *s, int fexit=1);
		// Close a socket
		void	closeSocket( SOCKETALT bsock)
		{
				#if defined(_WIN32) && !defined(__CYGWIN__)
					closesocket(bsock);
				#else
					close(bsock);
				#endif
		}

		void	showIP( SOCKET socket);

		// returns the number of clients
		int		getNumClients() { return this->nb_clients;}
		// returns dot notation of IP address
		char *	getIPof( AddressIP ipadr) {
			cout<<inet_ntoa( ipadr.sin_addr)<<":"<<ntohs(ipadr.sin_port)<<endl;
			return inet_ntoa( ipadr.sin_addr);
		}

		int		isSameAddress( AddressIP * ip1, AddressIP * ip2)
		{
			int ipsize;
			ipsize = sizeof( unsigned long);
			return (!memcmp( &(ip1->sin_addr.s_addr), &(ip2->sin_addr.s_addr), ipsize));
		}
};

/**************************************************************/
/**** Create (and bind) a socket on host                   ****/
/**************************************************************/

// Creates and bind the socket designed to receive coms
// host == NULL -> localhost

inline SOCKETALT	NETCLASS::createSocket( char * host, unsigned short port, int srvmode)
{
	SOCKETALT	ret;
	unsigned short srv_port=0, clt_port=0;
	this->server = srvmode;
	// If port is not given, use the defaults ones --> do not work with specified ones yet... well, didn't try
	if( server)
	{
	#ifdef _TCP_PROTO
		srv_port = port;
	#else
		srv_port = CLIENT_PORT;
	#endif
		if( port!=0)
			clt_port = port;
		else
			clt_port = SERVER_PORT;
	}
	else
	{
		if( port!=0)
			srv_port = port;
		else
			srv_port = SERVER_PORT;
	#ifdef _TCP_PROTO
		clt_port = port;
	#else
		clt_port = CLIENT_PORT;
	#endif
	}

		struct hostent	*he = NULL;
#if defined(_WIN32) && !defined(__CYGWIN__)
		int sockerr= INVALID_SOCKET;
#else
		int sockerr= -1;
#endif
		#ifdef _TCP_PROTO
		if( (this->sock = socket( PF_INET, SOCK_STREAM, 0))==sockerr)
			f_error( "Could not create socket");
		#else
		if( (this->sock = socket( PF_INET, SOCK_DGRAM, 0))==sockerr)
			f_error( "Could not create socket");
		#endif

#ifndef _TCP_PROTO
#if !defined(_WIN32) || defined(__CYGWIN__)
		if( fcntl( this->sock, F_SETFL, O_NONBLOCK) == -1)
		{
			perror( "Error fcntl : ");
			exit( 1);
		}
#else
		unsigned long datato = 1;
		if( ioctlsocket(this->sock,FIONBIO,&datato)!=0) {
				perror( "Error fcntl : ");
				exit( 1);
		}
#endif
#endif
		// Gets the host info for host
		if( host[0]<48 || host[0]>57)
		{
			cout<<"Resolving host name... ";
			if( (he = gethostbyname( host)) == NULL)
				f_error( "\nCould not resolve hostname ");
			memcpy( &this->srv_ip.sin_addr.s_addr, he->h_addr_list[0], he->h_length);
			// srvip = ntohl( srvip);
			cout<<"found : "<<inet_ntoa( this->srv_ip.sin_addr)<<endl;
		}
		else
		{
#if defined(_WIN32) && !defined(__CYGWIN__)
			if( (this->srv_ip.sin_addr.s_addr=inet_addr( host)) == 0)
#else			
			if( inet_aton( host, &this->srv_ip.sin_addr) == 0)
#endif
				f_error( "Error inet_aton ");
		}
		// Store it in srv_ip struct
		this->srv_ip.sin_port= htons( srv_port);
		this->srv_ip.sin_family= AF_INET;

		//getIPof( this->srv_ip);

		this->clt_ip.sin_addr.s_addr = htonl(INADDR_ANY);
#ifdef _UDP_PROTO
		if( !server)
			this->clt_ip.sin_port = htons( 0);
		else
			this->clt_ip.sin_port = htons( clt_port);
#else
		this->clt_ip.sin_port = htons( clt_port);
#endif
		this->clt_ip.sin_family = AF_INET;
#ifdef _UDP_PROTO
		// binds socket
		if( bind( this->sock, (sockaddr *) &clt_ip, sizeof( struct sockaddr))==SOCKET_ERROR)
			this->disconnect( "Cannot bind socket");
		cout<<"Bind on localhost, port = "<<clt_port<<endl;
		this->max_sock = this->sock;
#else
		if( server)
		{
			// binds socket
			cout<<"Bind on "<<ntohl(clt_ip.sin_addr.s_addr)<<", port "<<ntohs( clt_ip.sin_port)<<endl;
			if( bind( this->sock, (sockaddr *) &clt_ip, sizeof( struct sockaddr))==SOCKET_ERROR)
				this->disconnect( "Cannot bind socket");
			cout<<"Accepting max : "<<MAXQUEUE<<endl;
			if( listen( this->sock, MAXQUEUE)==SOCKET_ERROR)
				this->disconnect( "Cannot start to listen");
			cout<<"Listening on socket "<<this->sock<<endl;
		}
		else
		{
			cout<<"Connecting to "<<inet_ntoa( this->srv_ip.sin_addr)<<" on port "<<srv_port<<endl;
			if( connect( this->sock, (sockaddr *) &srv_ip, sizeof( struct sockaddr))==SOCKET_ERROR)
			{
				perror( "Can't connect to server ");
				this->disconnect( "");
			}
			cout<<"Connected to "<<inet_ntoa( this->srv_ip.sin_addr)<<" on port "<<srv_port<<endl;
		}
		this->max_sock = this->sock;
#endif
		ret=this->sock;
		cout<<"Socket n° : "<<ret<<endl;
	return ret;
}

/**************************************************************/
/**** Accept a new connection                              ****/
/**************************************************************/
// Returns the connection socket or 0 if failed or if no activity
// Not used in standard UDP mode
// Returns channel number in SDL UDP mode

inline SOCKETALT	NETCLASS::acceptNewConn( AddressIP * ipadr)
{
	SOCKETALT	ret;

	#ifdef HAVE_SDLnet
		conn_set=SDLNet_AllocSocketSet(1);
		if(!conn_set) {
			cout<<"SDLNet_AllocSOCKETALTSet: "<<SDLNet_GetError()<<endl;
			exit(1);
		}

		#ifdef _TCP_PROTO
			int numused;
			int numready;
			numused=SDLNet_TCP_AddSocket(conn_set,sock);
			if(numused==-1) {
				cout<<"SDLNet_AddSocket: "<<SDLNet_GetError()<<endl;
				// perhaps you need to restart the set and make it bigger...
			}
			if( this->nb_clients<this->max_clients)
			{
				numready=SDLNet_CheckSockets(conn_set, 0);
				if(numready==-1) {
					cout<<"SDLNet_CheckSockets: "<<SDLNet_GetError()<<endl;
					//most of the time this is a system error, where perror might help you.
					perror("SDLNet_CheckSockets");
				}
				else if(numready)
				{
					// Do an accept
					ret = SDLNet_TCP_Accept(sock);
					if(!ret)
						cout<<"SDLNet_TCP_Accept: "<<SDLNet_GetError()<<endl;
					// Adds the new client's socket to the client_set
					this->nb_clients++;
					// Add the new socket to the client socket set
					numused=SDLNet_TCP_AddSocket( this->client_set, ret);
				}
				else ret = NULL;
			}
			else ret = NULL;
		#endif
		#ifdef _UDP_PROTO
			// We have to bind to the new connection received
			if((ret = SDLNet_UDP_Bind(sock, -1, ipadr))==-1)
			{
				printf("SDLNet_UDP_Bind: %s\n",SDLNet_GetError());
				exit(2);
			}
			// Shouldn't return 0 as channel number because considered as error in other modes
			if( ret==0)
				ret = 1;
		#endif
		SDLNet_FreeSocketSet( conn_set);
	#else
		// SOCKETALT part, only in TCP mode, returns 0 when no activity, socket descriptor if activity detected
		#ifdef _TCP_PROTO
			SOCKETALT bsock;
			struct sockaddr_in clt_addr;
			int s;
#if defined (_WIN32) || defined (__APPLE__)
			int len;
#else
			socklen_t len;
#endif
			len = sizeof( struct sockaddr);
			FD_ZERO( &conn_set);
			FD_SET( this->sock, &conn_set);
			if( (s = select( (this->sock)+1, &conn_set, NULL, NULL, &this->srvtimeout)) == SOCKET_ERROR )
			{
#if defined(_WIN32) && !defined(__CYGWIN__)
				if( WSAGetLastError()!=WSAEINVAL)
					cout<<"WIN32 error : "<<WSAGetLastError()<<endl;
#else
				cout<<"Select accept new connection failed, descriptor = "<<(this->sock)<<endl;
				perror( "Select failed : ");
#endif
				ret = 0;
			}
			if( s > 0)
			{
				if( (bsock = accept( this->sock, (sockaddr *) &clt_addr, &len))>0)
					ret = bsock;
				else
				{
#if defined(_WIN32) && !defined(__CYGWIN__)
					cout<<"WIN32 error : "<<WSAGetLastError()<<endl;
#else
					printf("Error accepting new conn\n");
#endif
					ret = 0;
				}
			}
			else
				ret = 0;
		#else
				// In classic UDP mode return 1 because there is nothing to do but we still have to create new client
				ret = 1;
		#endif
	#endif
	
	return ret;
}

/**************************************************************/
/**** Add a socket to a set to be watched                  ****/
/**************************************************************/
inline void	NETCLASS::watchSocket( SOCKET bsock)
{
#ifdef HAVE_SDLnet
	#ifdef _UDP_PROTO
			int numused=SDLNet_UDP_AddSocket(client_set, bsock);
			if(numused==-1) {
				cout<<"SDLNet_AddSocket: "<<SDLNet_GetError()<<endl;
				exit( 1);
				// perhaps you need to restart the set and make it bigger...
			}
	#endif
	#ifdef _TCP_PROTO
			int numused=SDLNet_TCP_AddSocket(client_set, bsock);
			if(numused==-1) {
				cout<<"SDLNet_AddSocket: "<<SDLNet_GetError()<<endl;
				exit( 1);
				// perhaps you need to restart the set and make it bigger...
			}
	#endif
#else
	// Now bsock is used to check client's descriptors (contains the max desc)
	#ifdef _TCP_PROTO
		if( bsock > max_sock)
			max_sock = bsock;
	#else
		if( bsock < this->sock)
			bsock = this->sock;
		//cerr<<"NETUI : watching descriptor "<<bsock<<endl;
	#endif
	FD_SET( bsock, &client_set);
#endif
}


/**************************************************************/
/**** Check number of sockets that are ready               ****/
/**************************************************************/
// Checks for client activity and returns #of active clients
inline int		NETCLASS::activeSockets()
{
			int ret = 0;
#ifndef HAVE_SDLnet
			int s;
			if( (s = select( (this->max_sock)+1, &client_set, 0, 0, &this->srvtimeout)) == SOCKET_ERROR )
			{
#if defined(_WIN32) && !defined(__CYGWIN__)
				if( WSAGetLastError()!=WSAEINVAL)
					cout<<"WIN32 error : "<<WSAGetLastError()<<endl;
#else
				cout<<"Select conn failed, descriptor = "<<(this->max_sock+1)<<endl;
#endif
				return -1;
			}
			if( s > 0)
				ret = s;
#else
			ret = SDLNet_CheckSockets( client_set, 0);
			if( ret<0)
			{
				perror( "Check socket failed");
				//cout<<"Check socket failed : "<<SDLNet_GetError()<<endl;
				exit( 1);
			}
#endif
			return ret;
}

/**************************************************************/
/**** Check if socket is ready to receive (active)         ****/
/**************************************************************/
// Returns 1 if socket active, -1 on error

inline int		NETCLASS::isActive( SOCKET bsock)
{
	int	ret;

	#ifdef HAVE_SDLnet
		#ifdef _TCP_PROTO
				ret = SDLNet_SocketReady( bsock);
		#endif
		#ifdef _UDP_PROTO
				ret = SDLNet_SocketReady( bsock);
		#endif
	#else
			ret = FD_ISSET( bsock, &client_set);
	#endif
	return ret;
}

/**************************************************************/
/**** recvbuf data from network                            ****/
/**************************************************************/

inline int		NETCLASS::recvbuf( SOCKETALT bsock, char *buffer, unsigned int &len, AddressIP * from)
{
	int ret = 0;
	int tmplen;
	tmplen = MAXBUFFER;

	#ifdef HAVE_SDLnet
		#ifdef _TCP_PROTO
			// Not to use ! Because with SDL_net lib, MAXBUFFER bytes have to be read each time, not more, not less :(
			// Seems to be working besides what the documentation says
			ret=SDLNet_TCP_Recv(bsock,buffer,MAXBUFFER);
			if(ret==-1) {
				this->closeSocket( bsock);
			}
		#endif
		#ifdef _UDP_PROTO
			UDPpacket * packet;
			packet=SDLNet_AllocPacket(MAXBUFFER+16);
			// In UDP mode, always receive data on sock
			ret=SDLNet_UDP_Recv( this->sock, packet);
			if(ret<=0)
			{
				ret = -1;
				//this->closeSOCKETALT( sock);
			}
			else
			{
				len = packet->len;
				ret = len;
				memcpy( buffer, packet->data, len);
				memcpy( from, &packet->address, sizeof(AddressIP));
				//*(buffer+len) = '\0';
			}
			SDLNet_FreePacket(packet);
		#endif
	#else
		#ifdef _TCP_PROTO
		if( (ret = recv( bsock, buffer, MAXBUFFER, 0)) < 0)
			ret = -1;
		len = ret;
		#endif
		#ifdef _UDP_PROTO
#if defined (_WIN32) || defined (__APPLE__)
			int len1;
#else
			socklen_t len1;
#endif
		// In UDP mode, always receive data on sock
		len1 = sizeof( struct sockaddr);
		if( (ret = recvfrom( this->sock, buffer, MAXBUFFER, 0, (sockaddr *) from, &len1)) <= 0)
		{
			cout<<"Received "<<ret<<" bytes : "<<buffer<<endl;
#if defined(_WIN32) && !defined(__CYGWIN__)
			cout<<"WIN32 error : "<<WSAGetLastError()<<endl;
#else
			//getIPof( *from);
			perror( "Error receiving ");
#endif
			ret = -1;
		}
		len = len1;
		cerr<<"NETUI : Recvd "<<len<<" bytes"<<" <- "<<inet_ntoa( from->sin_addr)<<":"<<ntohs(from->sin_port)<<endl;
		#endif
	#endif
	//cout<<"Received "<<ret<<" bytes"<<endl;
	return ret;
}

/**************************************************************/
/**** sendbuf data over the network                        ****/
/**************************************************************/

inline int		NETCLASS::sendbuf( SOCKETALT bsock, void *buffer, unsigned int len, AddressIP * to)
{
	int numsent;
	//assert( len<=MAXBUFFER);

	#ifdef HAVE_SDLnet
		#ifdef _TCP_PROTO
			// TCP
			numsent=SDLNet_TCP_Send( bsock, buffer, len);
			int tmplen = len;
			if(numsent<tmplen) {
				cout<<"SDLNet_TCP_Send: "<<SDLNet_GetError()<<endl;
				this->closeSocket( bsock);
				return -1;
			}
		#endif
		#ifdef _UDP_PROTO
			UDPpacket *packet;

			packet=SDLNet_AllocPacket(len);
			if(!packet)
			{
				cout<<"SDLNet_AllocPacket: "<<SDLNet_GetError()<<endl;
				return -1;
			}
			packet->len = len;
			packet->channel = -1;
			memcpy( &packet->address, to, sizeof( AddressIP));
			memcpy( packet->data, buffer, len);
			// In UDP mode, always send on sock - here sock is the channel number
			//cout<<"Send on channel n° "<<sock<<endl;
			numsent=SDLNet_UDP_Send( this->sock, -1, packet);
			if(!numsent) {
				cout<<"SDLNet_UDP_Send on sock: "<<SDLNet_GetError()<<endl;
				// Here the send command failed
			}
			SDLNet_FreePacket(packet);
		#endif
	#else
		#ifdef _TCP_PROTO
#if defined(_WIN32) && !defined(__CYGWIN__)
		if( (numsent=send( bsock, (char *)buffer, len, 0))<0)
#else
		if( (numsent=send( bsock, buffer, len, 0))<0)
#endif
		{
			perror( "\tsending data : ");
			if( errno == EBADF)
				return -1;
			//keeprun = 0;
		}
		#endif
		#ifdef _UDP_PROTO
		// In UDP mode, always send on this->sock
		sockaddr_in * dest;
		if( server)
			dest = to;
		else
			dest = &this->srv_ip;
		//getIPof( (*dest));
#if defined(_WIN32) && !defined(__CYGWIN__)
		if( (numsent = sendto( this->sock, (char *) buffer, len, 0, (sockaddr *) dest, sizeof( struct sockaddr)))<0)
#else
		if( (numsent = sendto( this->sock, buffer, len, 0, (sockaddr *) dest, sizeof( struct sockaddr)))<0)
#endif
		{
#if defined(_WIN32) && !defined(__CYGWIN__)
				if( WSAGetLastError()!=WSAEINVAL)
					cout<<"WIN32 error : "<<WSAGetLastError()<<endl;
#else
			//getIPof( *from);
			perror( "Error sending ");
#endif
			return -1;
		}
		cerr<<"NETUI : Sent "<<numsent<<" bytes"<<" -> "<<inet_ntoa( dest->sin_addr)<<":"<<ntohs(dest->sin_port)<<endl;
		#endif
	#endif
	return 0;
}

/**************************************************************/
/**** Disconnect a client                                  ****/
/**************************************************************/

inline void	NETCLASS::disconnect( char *s, int fexit)
{
	if( sock!=0)
	{
		#ifdef HAVE_SDLnet
			#ifdef _TCP_PROTO
				SDLNet_TCP_Close(this->sock);
			#endif
			#ifdef _UDP_PROTO
				SDLNet_UDP_Close(this->sock);
			#endif
			SDLNet_Quit();
		#else
			#if defined(_WIN32) && !defined(__CYGWIN__)
				closesocket (this->sock);
			#else
				close(this->sock);
			#endif
		#endif
	}
	cout<<s<<" :\t";
	perror( "Warning: disconnected");
	if( fexit)
		exit(1);
}

inline void	NETCLASS::showIP( SOCKET socket)
{
#ifndef HAVE_SDLnet
	struct sockaddr_in tmp;
#if defined (_WIN32) || defined (__APPLE__) || defined(__CYGWIN__)
	int ulen;
#else
	unsigned int ulen;
#endif
	int ret;

	ulen = sizeof( struct sockaddr_in);

	if( (ret = getsockname( socket, (sockaddr *)&tmp, &ulen))==-1)
		f_error( "Error getting socket name ");

	cout<<endl<<"IP dec: "<<hex<<tmp.sin_addr.s_addr<<endl;
	cout<<"INADDR_ANY: "<<INADDR_ANY<<endl;
	cout<<"IP: "<<inet_ntoa( tmp.sin_addr)<<endl;
	cout<<"Port: "<<( tmp.sin_port)<<"\t"<<ntohs(tmp.sin_port)<<endl;
#endif
}
