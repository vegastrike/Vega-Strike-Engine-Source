#ifndef __INET_H__
#define __INET_H__

unsigned long INET_BytesToRead (int socket);

bool INET_Read (int socket, char *data, int bytestoread);

int INET_Write (int socket,int bytestowrite,const char *data);

int INET_listen (unsigned short port, const char *addr=NULL);
int INET_Accept (int hServerSocket);

int INET_AcceptFrom (unsigned short port, const char * addr=NULL);

int INET_ConnectTo (const char * hostname, unsigned short port);

char INET_fgetc (int socket);

void INET_close (int socket);

void INET_startup();

void INET_cleanup();
#endif
