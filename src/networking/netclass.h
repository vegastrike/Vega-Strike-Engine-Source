#ifndef __NETLAYER_H
#define __NETLAYER_H

#include <iostream>
#include <errno.h>
#include "const.h"
using std::cout;
using std::endl;

#ifdef HAVE_SDLnet
	#include <SDL/SDL_net.h>
	#ifdef _UDP_PROTO
		typedef UDPsocket Socket;
		// Type SocketAlt is designed to handle channel numbers in SDL UDP mode
		typedef int SocketAlt;
	#endif
	#ifdef _TCP_PROTO
		typedef TCPsocket Socket;
		typedef TCPsocket SocketAlt;
	#endif
	typedef IPaddress AddressIP;
	typedef SDLNet_SocketSet SocketSet;
#else
	#ifdef _WIN32
	#define in_addr_t unsigned long
	#include <winsock.h>
	#else
	#define SOCKET_ERROR -1
	#include <error.h>
	#include <netdb.h>
	#include <string.h>
	#include <sys/socket.h>
	#include <sys/time.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <unistd.h>
	#endif
	#include <stdio.h>
	#include <stdlib.h>
	typedef int Socket;
	typedef int SocketAlt;
	typedef sockaddr_in AddressIP;
	typedef fd_set SocketSet;
	#define MAXQUEUE 10
#endif

#include <stdio.h>
#include <fcntl.h>

using std::cout;
using std::cin;
using std::endl;
using std::hex;

inline void f_error( char *s)
{
#ifdef HAVE_SDLnet
	cout<<endl<<s<<" - "<<SDLNet_GetError()<<endl;
#else
	perror( s);
#endif
	exit(1);
}


#ifdef _TCP_PROTO
	#define TCPMODE
#else
	#define UDPMODE
#endif

// See what's the default NetUI to use
#define DEFAULT
	#define NETCLASS NetUI
	#include "netui.h"
	#undef NETCLASS
#undef DEFAULT

// Include NetUI to create classes for UDP and TCP
#undef _UDP_PROTO
#undef _TCP_PROTO

// Include NetUI UDP version
#define _UDP_PROTO
	#define NETCLASS UDPNetUI
	#include "netui.h"
	#undef NETCLASS
#undef _UDP_PROTO

// Include NetUI TCP version
#define _TCP_PROTO
	#define NETCLASS TCPNetUI
	#include "netui.h"
	#undef NETCLASS
#undef _TCP_PROTO

#ifdef TCPMODE
#define _TCP_PROTO
#else
#define _UDP_PROTO
#endif

#endif
