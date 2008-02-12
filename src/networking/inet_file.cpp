#if defined(_WIN32)
#define in_addr_t unsigned long
#include <wchar.h>
#include <windows.h>
#include <io.h>
#else
#define SOCKET_ERROR -1
#if defined(__CYGWIN__)
#define in_addr_t unsigned long
#endif
#include <sys/time.h>
#include <sys/types.h>
// #if !defined(__APPLE__) && !defined(__CYGWIN__) && !defined(BSD)
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

#define micro_sleep(n) do { \
	struct timeval tv; \
	tv.tv_usec = n%1000000; \
	tv.tv_sec=n/1000000; \
	select(0, NULL, NULL, NULL, &tv); \
} while (0)

bool fNET_BytesToRead (int socket) {
  /*
  char c;
  if (read (socket,&c,1)==1) {
    printf ("read %c",c);
    printf ("seekin %d",lseek (socket,SEEK_CUR,-1));

    return true;
  }
  return false;
  */
  
    off_t curpos = lseek (socket,SEEK_CUR,0);
    off_t endpos = lseek (socket,SEEK_END,0);
    if (curpos+1 !=endpos) {
        lseek (socket,SEEK_SET,curpos);
        return true;
    }
    return false;
  
}
void fNET_close (int socket) {
	close (socket);
}
int fNET_Recv (int socket, char * data, int bytestoread) {
    int retval=0;
    while (retval==0) {
        retval = read(socket,data,bytestoread);
        if (!retval)micro_sleep(50000);
    }
    return retval;
}
bool fNET_Read (int socket, char *data, int bytestoread) {
	int bytes_read=0;
	int ret;
	while (bytes_read < bytestoread) {
		ret = read(socket,data+bytes_read,bytestoread-bytes_read);
		if (-1==ret) {
			return false;
		}
		bytes_read +=ret;
                if (!ret) micro_sleep(50000);
	}
	return true;
}


int fNET_Write (int socket,int bytestowrite,const char *data) {
	return write(socket,data,bytestowrite);
}
char fNET_fgetc (int socket) {
	char myc='\0';
	fNET_Recv(socket,&myc,sizeof(char));
	return myc;
}
void fNET_cleanup(){

}
void fNET_startup() {

}
bool fNET_getHostByName (const char * hostname, unsigned short port, sockaddr_in & connexto) {
  return false;
}

int fNET_listen (unsigned short port, const char * hostname) {
  return -1;    
}
int fNET_ConnectTo (const char * hostname, unsigned short port) {
  return -1;
}
int fNET_Accept (int hServerSocket) {
  return -1;
}
int fNET_AcceptFrom ( unsigned short port,const char * hostname) {
return -1;
}
