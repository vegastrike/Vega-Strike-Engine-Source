#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include "inet.h"
#include "inet_file.h"
#include "lin_time.h"
bool ForwardBytes (int fd,int mysocket_write,char buf[65536]){
  if (INET_BytesToRead(fd)) {
    int ammt = INET_Recv (fd,buf,65536);
    if (ammt==0||ammt==-1) {
      return false;
    }
    fNET_Write(mysocket_write,ammt,buf);
  }
  return true;
}
bool  ForwardFileBytes (int mysocket_read,int fd) {
  char c;
  if (1==read (mysocket_read,&c,1)) {
    printf ("recved bytes %c %d",c,(int)c);
    int famt = INET_Write (fd,1,&c);
    if (famt!=1) {
      return false;
    }
  }
  return true;
}


bool current_client_dead (pid_t p) {
  int status=0;
  if (!waitpid (p,&status,WNOHANG)) {
    return false;
  }
  return true;
}
pid_t F0rkProcess (int &write, int &read) {
  char reader[]="myreadsocketXXXXXX";
  char writer[]="mywritesocketXXXXXX";
  write = mkstemp (writer);
  int tmpread = mkstemp (reader);
  read = open (reader,O_RDONLY|O_SHLOCK|O_TRUNC,0xffffffff);
  close (tmpread);
  pid_t retval = fork();
  if (retval) {
    return retval;
  }
  execlp("./soundserver.child","./soundserver.child",writer,reader,NULL);
  //trashed!
}
int main (int argc , char ** argv) {
  char buf[65536];
    INET_startup() ;    
    int fd=-1;
    int mysocket_read = -1;
    int mysocket_write=-1;
    for (int i=0;i<10&&fd==-1;i++) {
        int port =4364;
        if (argc>1) port=atoi(argv[1]);
        fd = INET_AcceptFrom(port,"localhost");
	
    }
    if (fd==-1)
      return 1;
    bool done=false;
    while (!done) {
      pid_t mypid = F0rkProcess (mysocket_write,mysocket_read);
      while ((!done) && (!current_client_dead(mypid))) {
	done = !ForwardBytes (fd,mysocket_write,buf);
	if (!done)
	  done = !ForwardFileBytes (mysocket_read,fd);
	micro_sleep (50000);
      }
      char c='e';
      INET_Write(fd,1,&c);
    }
    INET_cleanup();
}
