#ifndef __NETLAYER_H
#define __NETLAYER_H

#include <config.h>

#define UDP_TIMEOUT 0.4

void f_error( char *s);

#ifdef _TCP_PROTO
	#define DefaultNetUI NetUITCP
	#define TCPNetUI     NetUITCP
#else
	#define DefaultNetUI NetUIUDP
	#define TCPNetUI     NetUITCP
#endif

#endif

