#ifndef __NETLAYER_H
#define __NETLAYER_H

#include <iostream>
#include <errno.h>
#include <assert.h>
#include "const.h"
using std::cout;
using std::cerr;
using std::endl;

#ifdef HAVE_SDLnet
	#include <SDL/SDL_net.h>
	typedef IPaddress AddressIP;
	typedef SDLNet_SocketSet SocketSet;
	#define TCPSOCKET TCPsocket
	#define UDPSOCKET UDPsocket
#else
	#if defined(_WIN32) && !defined(__CYGWIN__)
	//#warning "Win32 platform"
	#define in_addr_t unsigned long
	#include <winsock.h>
	#else
	//#warning "GCC platform"
	#define SOCKET_ERROR -1

#if !defined(__APPLE__) && !defined(__CYGWIN__)
	#include <error.h>
#endif
	#include <netdb.h>
	#include <string.h>
	#include <sys/socket.h>
	#include <sys/time.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <unistd.h>
	#include <fcntl.h>
	#include <stdio.h>
	#endif

	#include <stdio.h>
	#include <stdlib.h>
	#define SOCKET int
	#define SOCKETALT int
	#define TCPSOCKET int
	#define UDPSOCKET int
	typedef sockaddr_in AddressIP;
	typedef fd_set SocketSet;
	#define MAXQUEUE 10
#endif


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
	#ifdef HAVE_SDLnet
		#ifdef __TCP_PROTO
			#define SOCKET TCPsocket
			#define SOCKETALT TCPsocket
		#else
			#define SOCKET UDPsocket
			// Type SocketAlt is designed to handle channel numbers in SDL UDP mode
			#define SOCKETALT int
		#endif
	#endif
	#define NETCLASS NetUI
	#include "netui.h"
	#undef NETCLASS
#undef DEFAULT

#ifdef HAVE_SDLnet
	#undef SOCKET
	#undef SOCKETALT
#endif

// Include NetUI to create classes for UDP and TCP
#undef _UDP_PROTO
#undef _TCP_PROTO

// Include NetUI UDP version
#define _UDP_PROTO
	#ifdef HAVE_SDLnet
		#define SOCKET UDPsocket
		// Type SocketAlt is designed to handle channel numbers in SDL UDP mode
		#define SOCKETALT int
	#endif
	#define NETCLASS UDPNetUI
	#include "netui.h"
	#undef NETCLASS
#undef _UDP_PROTO

#ifdef HAVE_SDLnet
	#undef SOCKET
	#undef SOCKETALT
#endif

// Include NetUI TCP version
#define _TCP_PROTO
	#ifdef HAVE_SDLnet
		#define SOCKET TCPsocket
		#define SOCKETALT TCPsocket
	#endif
	#define NETCLASS TCPNetUI
	#include "netui.h"
	#undef NETCLASS
#undef _TCP_PROTO

#ifdef HAVE_SDLnet
	#undef SOCKET
	#undef SOCKETALT
#endif

#ifdef TCPMODE
	#define _TCP_PROTO
	#ifdef HAVE_SDLnet
		#define SOCKET TCPsocket
		#define SOCKETALT TCPsocket
	#endif
#else
	#define _UDP_PROTO
	#ifdef HAVE_SDLnet
		#define SOCKET UDPsocket
		// Type SocketAlt is designed to handle channel numbers in SDL UDP mode
		#define SOCKETALT int
	#endif
#endif

#endif
