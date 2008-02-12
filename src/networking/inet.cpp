#include "config.h"
#if defined(_WIN32)
#define in_addr_t unsigned long
#include <windows.h>
#else
#define SOCKET_ERROR -1
#include <sys/time.h>
#include <sys/types.h>
// #if !defined(__APPLE__) && !defined(__CYGWIN__) && !defined(BSD) && !defined(__FreeBSD__) && !defined(SOLARIS)
// #include <error.h>
// #endif
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#endif
#include <stdio.h>
#include <stdlib.h>

bool INET_BytesToRead (int socket) {
#ifdef _WIN32
	unsigned long datato;
	if (0==ioctlsocket(socket,FIONREAD,&datato)) {
	  
		return (datato>0);
	}
#else
  struct timeval tv;
  tv.tv_sec=0;  tv.tv_usec=00000;
  int selres=0;
  fd_set rfds;
  FD_ZERO (&rfds);
  FD_SET (socket,&rfds);
   if ((selres=select (socket+1,&rfds,NULL,NULL,&tv))){
      if (selres==-1) {
        return false;
      }
      return true;
   }
#endif
	return false;
       
}
void INET_close (int socket) {
#ifdef _WIN32
	closesocket (socket);
#else
	close (socket);
#endif
}
void INET_cleanup () {
#ifdef _WIN32
	WSACleanup();
#endif
}
int INET_Recv (int socket, char * data, int bytestoread) {
  return recv(socket,data,bytestoread,0);
}
bool INET_Read (int socket, char *data, int bytestoread) {
	int bytes_read=0;
	int ret;
	while (bytes_read < bytestoread) {
		ret = recv(socket,data+bytes_read,bytestoread-bytes_read,0);
		if (ret==0||SOCKET_ERROR==ret) {
			return false;
		}
		bytes_read +=ret;
	}
	return true;
}


int INET_Write (int socket,int bytestowrite,const char *data) {
	return send(socket,data,bytestowrite,0);
}
void INET_startup() {
#ifdef _WIN32
  WORD wVersionRequested = MAKEWORD( 1, 1 );
  WSADATA wsaData; 
  WSAStartup(wVersionRequested,&wsaData);
#endif
}
char INET_fgetc (int socket) {
	char myc='\0';
	INET_Read (socket,&myc,sizeof(char));
	return myc;
}

bool INET_getHostByName (const char * hostname, unsigned short port, sockaddr_in & connexto) {
  bool gotaddr=false;
  connexto.sin_port = htons (port);
  connexto.sin_family=AF_INET;
  if (hostname==NULL) {
	connexto.sin_addr.s_addr=INADDR_ANY;
  }
  hostent * addrs  = gethostbyname(hostname);
  if (addrs) {
    if (addrs->h_addr_list[0]) {

      memset (&connexto.sin_addr,0,sizeof (in_addr));
      if (addrs->h_length>(int)sizeof (in_addr))
		addrs->h_length=sizeof(in_addr);
      memcpy (&connexto.sin_addr ,addrs->h_addr_list[0],addrs->h_length);
      gotaddr=true;
    }
  } else {
    in_addr_t tmp=inet_addr(hostname);
    memcpy (&connexto.sin_addr,&tmp,sizeof(in_addr_t)<sizeof(in_addr)?sizeof(in_addr_t):sizeof(in_addr));
    if (*((int*)&tmp) != -1) {
      gotaddr=true;
    }
  }
	return gotaddr;
}
int INET_listen (unsigned short port, const char * hostname) {
  int listenqueue=5;
  int hServerSocket; // so signal can be caught;
  struct sockaddr_in Address; //Internet socket address stuct
#if defined (_WIN32) || defined(__CYGWIN__) || defined(MAC_OS_X_VERSION_10_3) || defined(MAC_OS_X_VERSION_10_2) || defined(MAC_OS_X_VERSION_10_1)
  int
#else
  socklen_t
#endif
  nAddressSize=sizeof(struct sockaddr_in);
  hServerSocket=socket(AF_INET,SOCK_STREAM,0);
  int sockerr = SOCKET_ERROR;
#ifdef _WIN32
  sockerr= INVALID_SOCKET;
#endif
  if(hServerSocket == sockerr) {
#ifdef _WIN32
	  int err = WSAGetLastError();
	  printf("\nCould not make a socket %d\n",err);

#endif
        return -1;
  }
  INET_getHostByName (hostname,port,Address);
  Address.sin_addr.s_addr = INADDR_ANY;
  Address.sin_family=AF_INET;
  if(bind(hServerSocket,(struct sockaddr*)&Address,sizeof(Address)) 
                        == SOCKET_ERROR) {
        printf("\nCould not connect to host\n");
	printf ("%d Error ",errno);
        return -1;
  }
  getsockname( hServerSocket, (struct sockaddr *) &Address,&nAddressSize);
  if(listen(hServerSocket,listenqueue) == SOCKET_ERROR) {
    printf("\nCould not listen\n");
        return -1;
  }
  return hServerSocket;
  // get the connected socket 
}
int INET_Accept (int hServerSocket) {
  sockaddr_in Address;
#if defined (_WIN32) || defined (__CYGWIN__) || defined(MAC_OS_X_VERSION_10_3) || defined(MAC_OS_X_VERSION_10_2) || defined(MAC_OS_X_VERSION_10_1)
  int 
#else
    socklen_t
#endif
    nAddressSize = sizeof (Address);
  return accept(hServerSocket,(struct sockaddr*)&Address,&nAddressSize);
}

int INET_AcceptFrom ( unsigned short port,const char * hostname) {
  int hServerSocket = INET_listen (port,hostname);
  int hSocket = INET_Accept (hServerSocket);
  INET_close (hServerSocket);
  return hSocket;
}
int INET_ConnectTo (const char * hostname, unsigned short port) {

  sockaddr_in connexto;
  int aftype=AF_INET;
#ifdef _WIN32
  aftype = PF_INET;
#endif
  int retval=-1;
  int my_socket=-1;
  if (INET_getHostByName (hostname,port,connexto)) {
      my_socket =  socket (aftype,SOCK_STREAM,0);
      retval=true;
      if (connect(my_socket,(struct sockaddr *) &connexto,sizeof(connexto))!=-1) {
		return my_socket;
      }else {
		retval=-1;
#ifdef _WIN32
		fprintf (stderr,"Socket Error %d",WSAGetLastError());
#endif

      }
  }
  return retval;
}
