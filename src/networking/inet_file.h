#ifndef __INET_FILE_H__
#define __INET_FILE_H__

bool fNET_BytesToRead (int socket);

bool fNET_Read (int socket, char *data, int bytestoread);

int fNET_Recv (int socket, char *data, int bytestoread);

int fNET_Write (int socket,int bytestowrite,const char *data);

int fNET_listen (unsigned short port, const char *addr=NULL);
int fNET_Accept (int hServerSocket);

int fNET_AcceptFrom (unsigned short port, const char * addr=NULL);

int fNET_ConnectTo (const char * hostname, unsigned short port);

char fNET_fgetc (int socket);

void fNET_close (int socket);

void fNET_startup();

void fNET_cleanup();
#endif
