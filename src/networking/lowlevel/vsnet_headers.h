#if defined(_WIN32) && !defined(__CYGWIN__)
	//#warning "Win32 platform"
	#define in_addr_t unsigned long
	//#ifdef USE_WINSOCK2
	//#define _WIN32_WINNT 0x0400
	//#endif
	#include <windows.h>
#else
	#include <netdb.h>
 	#include <string.h>
	#include <sys/socket.h>
	#include <sys/time.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <unistd.h>
	#define SOCKET_ERROR -1
#endif
#ifdef __APPLE__
#include <sys/types.h>
#endif
#include <stdlib.h>

