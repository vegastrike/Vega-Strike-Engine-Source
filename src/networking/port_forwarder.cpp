#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string>

#include <vector>

#include <fcntl.h>
#include <unistd.h>
#include "inet.h"
#include "inet_file.h"
#include <sys/types.h>
#include <pwd.h>
using std::string;
//assumes null termination;
float fadein=0;
float fadeout=0;
float volume=1;
bool scanChar (string s, char c, float & val) {
  char cc[2]={c,0};
  int loc;
  bool anyvalid=false;
  while ((loc=s.find(cc))!=string::npos) {
    bool valid = (loc==0);
    if (!valid && loc>0) {
      valid=((s[loc-1]=='\n')||(s[loc-1]=='\r'));
    }
    s=s.substr(loc+1);
    if (valid){
      sscanf (s.c_str(),"%f",&val);
      anyvalid=true;
    }
  }
  return anyvalid;
} 
bool quit=false;
bool ForwardBytes (int fd,int mysocket_write,char buf[65536]){
  if (INET_BytesToRead(fd)) {
    int ammt = INET_Recv (fd,buf,65535);
    if (ammt==0) {
      return false;
    }
    if (ammt==-1)
      return true;
    buf[ammt]=0;
    while (-1==INET_Write(mysocket_write,ammt,buf)) {
      
    }
  }
  return true;
}

int main (int argc , char ** argv) {
  char buf[65536];
    INET_startup() ;    
    int fd=-1;
    int ofd = -1;
    int outport =6000;
    char * outaddress="192.168.1.6";
    if (argc>2) {
      outaddress = (char *)argv[2];
    }
    if (argc>3) {
      outport=atoi(argv[3]);
    }
    ofd = INET_ConnectTo(outaddress,outport);
    if (ofd!=-1) 
    for (int i=0;i<10&&fd==-1;i++) {
        int port =4364;
        if (argc>1) port=atoi(argv[1]);
        fd = INET_AcceptFrom(port,"localhost");
      
    }
    if (fd==-1)
      return 1;
    timeval t;
    t.tv_sec = 5;
    t.tv_usec=0;
    fd_set fdset;
    FD_ZERO(&fdset);
    FD_SET(fd,&fdset);
    FD_SET(ofd,&fdset);
    fcntl (fd,F_SETFL,O_NONBLOCK);
    fcntl (ofd,F_SETFL,O_NONBLOCK);
    while (1) {
      //      select(fd>ofd?(fd+1):(ofd+1),&fdset,NULL,NULL,&t);
      if (!ForwardBytes (fd,ofd,buf))
	break;
      if (!ForwardBytes(ofd,fd,buf)) {
	break;
      }
    }
    INET_cleanup();
    return 0;
}
