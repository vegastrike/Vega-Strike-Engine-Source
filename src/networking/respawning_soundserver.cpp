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
#include "lin_time.h"
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
void ScanOptions (char * buf, int ammt) {
  string bleh(buf,ammt);
  scanChar (bleh,'i',fadein);
  scanChar (bleh,'I',fadein);
  scanChar (bleh,'O',fadeout);
  scanChar (bleh,'o',fadeout);
  scanChar (bleh,'v',volume);
  scanChar (bleh,'V',volume);
  float test;
  quit = scanChar (bleh,'t',test)|| scanChar (bleh,'T',test);
}
bool ForwardBytes (int fd,int mysocket_write,char buf[65536]){
  if (INET_BytesToRead(fd)) {
    int ammt = INET_Recv (fd,buf,65535);
    if (ammt==0||ammt==-1) {
      return false;
    }
    buf[ammt]=0;
    fNET_Write(mysocket_write,ammt,buf);
    ScanOptions (buf,ammt);
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
void changehome (bool,bool);
pid_t F0rkProcess (int &write, int &read) {
  char writer_str[65536]; writer_str[0]=0;
  char reader_str[65536]; reader_str[0]=0;
  {
    char pwd[32768];
    pwd[32766]=pwd[32767]=0;
    char reader[]="myreadsocketXXXXXX";
    char writer[]="mywritsocketXXXXXX";
    changehome(true,true);
    write = mkstemp (writer);
    int tmpread = mkstemp (reader);
    getcwd (pwd,32766);
    changehome(false,false);
    strcat (pwd,"/");
    strcpy (writer_str,pwd);
    strcpy (reader_str,pwd);
    strcat (writer_str,writer);
    strcat (reader_str,reader);
    read = open (reader_str,O_RDONLY|O_SHLOCK|O_TRUNC,0xffffffff);
    close (tmpread);
  }
  pid_t retval = fork();
  if (retval) {
    char mystr[8192];
    sprintf (mystr,"i%f\r\no%f\r\nv%f\r\n",fadein,fadeout,volume);
    fNET_Write (write,strlen(mystr),mystr);//don't write null
    return retval;
  }
  execlp("./soundserver.child","./soundserver.child",writer_str,reader_str,NULL);
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
    while (!done&&!quit) {
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

void changehome (bool to, bool linuxhome=true) {
  static std::vector <std::string> paths;
  if (to) {
    char mycurpath[8192];
    getcwd(mycurpath,8191);
    mycurpath[8191]='\0';
    paths.push_back (mycurpath);
#ifndef _WIN32
    if (linuxhome) {
      struct passwd *pwent;
      pwent = getpwuid (getuid());
      chdir (pwent->pw_dir);
    }
#endif
    chdir (".vegastrike");
  }else {
    if (!paths.empty()) {
      chdir (paths.back().c_str());
      paths.pop_back();
    }
  }
}
