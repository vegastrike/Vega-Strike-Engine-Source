
int GetMaxVolume();
void changehome (bool to, bool linuxhome=true);
#ifdef __APPLE__
#include "lin_time.h"
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/wait.h>
#include <vector>
#include <string>

#include "inet.h"

extern "C" {
#include "../fmod.h"
}
using std::string;
using std::vector;
void music_finished();
bool AllDone=false;
void Cleanup(int sockets_to_be_cleaned) {
    FSOUND_Close();
    INET_cleanup();
    if (sockets_to_be_cleaned !=-1)
        INET_close (sockets_to_be_cleaned);
}
signed char endcallback (FSOUND_STREAM * stream, void * buf, int len, int param) {
    if (!buf) {
        AllDone=true;
    }
    return 0;
}
struct Music {
    FSOUND_STREAM * m;
    int channel;
    Music() {
        m=NULL;channel=0;
    }
    void Stop() {
        if (m) FSOUND_Stream_Stop(m);
    }
    void Free () {
        if (m) FSOUND_Stream_Close(m);
        m=NULL;
    }
    bool SimpleLoad(const char * file) {
        m=FSOUND_Stream_OpenFile (file,FSOUND_NORMAL | FSOUND_MPEGACCURATE,0);
        return m!=NULL;
    }
    bool Load (const char * file);
    void Play(float fadeout){
        if (!m) return;
        assert (AllDone==false);
        FSOUND_Stream_SetEndCallback(m,endcallback,0);
        FSOUND_Stream_SetSynchCallback(m, endcallback, 0);
        channel = FSOUND_Stream_Play(FSOUND_FREE, m);
        SetVolume(0);
        if (fadeout*10>1) {
            for (unsigned int i=0;i<fadeout*10;i++) {
                SetVolume(i/((float)fadeout*10));
                micro_sleep (100000);
            }
        }
        SetVolume(1);
    }
    void FadeOut (float fadeout) {
        if (!m) return;
        if (fadeout*10>1) {
            for (unsigned int i=0;i<fadeout*10;i++) {
                SetVolume(1-(i/((float)fadeout*10)));
                micro_sleep (100000);
            }
        }
        SetVolume(0);
    }
    
    void SetVolume(float vol) {
        if (m) {
            F_API FSOUND_SetVolume(this->channel,(int)(vol*GetMaxVolume()));
        }
    }
};
#endif
int fadeout=0, fadein=0;
float volume=0;
int bits=0,done=0;



int sende=1;
std::string curmus;
bool watch_for_sende (pid_t p) {
    int status;
    if (waitpid(p,&status,WNOHANG)) {
        ++sende;
        if (sende>0) {
            music_finished();
        }
        return true;
    }
    return false;
}

void ForkedProcess (const char * file, float fade, int fd) {
    printf ("initing device drivers\n");
    if (!FSOUND_Init(44100, 64, FSOUND_INIT_GLOBALFOCUS))
    {
        printf("SOUND Error %d\n", FSOUND_GetError());
        exit(1);
    }
    Music mus;
    mus.Load(file);
    if (mus.m) {
        mus.Play(fade);

        printf ("f0rked playing %s\n",file);
        while (!INET_BytesToRead(fd)) {
            if (AllDone) {
                printf ("song done\n");
                mus.Free();
                return;
            }
        }
        mus.Stop();
        mus.Free();
    }
    return;
}
int ForkThenPlay (const char * file, float fade, int mysocket) {
    printf ("playing %s",file);
    int filedes[2]={-1,-1};
    //socketpair(AF_UNIX,SOCK_STREAM,0,filedes);
    pipe(filedes);
    pid_t fark = fork();
    if (fark) {
        close(mysocket);
        printf ("f0rking subprocess\n");
        ForkedProcess(file,fade,filedes[0]);
        Cleanup(filedes[0]);
        exit(0);//Dun!
        return -1;
    }
    printf ("forking main loop\n");
    curmus=file;
    return filedes[1];
}
int mysocket = -1;
void music_finished () {
    printf ("send e ratio %d\n",(int)sende);
    if (sende) {
        char data='e';
        INET_Write(mysocket,sizeof(char),&data);
        printf("\ne\n[SONG DONE]\n");
        curmus="";
    }    
}
void FinishOlde (int open_file) {
    if (open_file!=-1) {
        if (INET_Write (open_file,1,"k")>0) {
            sende--;//kill
        }
        INET_close(open_file);
        open_file=-1;
    }
}


int main(int argc, char **argv)
{
    int open_file=-1;

    INET_startup();
    GetMaxVolume();
    // load the song
    for (int i=0;i<10&&mysocket==-1;i++) {
        int port = 4364;
        if (argc>1)
            port=atoi(argv[1]);
        mysocket = INET_AcceptFrom(port,"localhost");
    }
    if (mysocket==-1)
        return 1;
    printf("\n[CONNECTED]\n");
    char ministr[2]={'\0','\0'};
    while (!done) {
        char arg;
        string str;
        if (INET_BytesToRead (mysocket)) {
            arg=INET_fgetc(mysocket);
            printf("%c",arg);
            switch(arg) {
                case 'p':
                case 'P':
                {
                    arg=INET_fgetc(mysocket);
                    while (arg!='\0'&&arg!='\n') {
                        if (arg!='\r') {
                            ministr[0]=arg;
                            str+=ministr;
                        }
                        arg=INET_fgetc(mysocket);
                    }
                    printf("%s",str.c_str());
                    if (str!=curmus) {
                        FinishOlde (open_file);
                        open_file=ForkThenPlay(str.c_str(),fadeout,mysocket);
                        curmus=str;
                    } else {
                        printf("\n[%s WITH %d FADEIN AND %d FADEOUT IS ALREADY PLAYING]\n",str.c_str(),fadein,fadeout);
                    }
                }
                    break;
                case 'i':
                case 'I':
                {
                    arg=INET_fgetc(mysocket);
                    while (arg!='\0'&&arg!='\n') {
                        if (arg!='\r') {
                            ministr[0]=arg;
                            str+=ministr;
                        }
                        arg=INET_fgetc(mysocket);
                    }
                    printf("%s",str.c_str());
                    fadein=atoi(str.c_str());
                    printf("\n[SETTING FADEIN TO %d]\n",fadein);
                }
                    break;
                case 'o':
                case 'O':
                {
                    arg=INET_fgetc(mysocket);
                    while (arg!='\0'&&arg!='\n') {
                        if (arg!='\r') {
                            ministr[0]=arg;
                            str+=ministr;
                        }
                        arg=INET_fgetc(mysocket);
                    }
                    printf("%s",str.c_str());
                    fadeout=atoi(str.c_str());
                    printf("\n[SETTING FADEOUT TO %d]\n",fadeout);
                }
                    break;
                case 'v':
                case 'V':
                {
                    arg=INET_fgetc(mysocket);
                    while (arg!='\0'&&arg!='\n') {
                        if (arg!='\r') {
                            ministr[0]=arg;
                            str+=ministr;
                        }
                        arg=INET_fgetc(mysocket);
                    }
                    printf("%s",str.c_str());
                    volume=atof(str.c_str());
                    printf("\n[SETTING VOLUME TO %f]\n",volume);
                    //music.SetVolume(volume);
                }
                    break;
                case 't':
                case 'T':
                case '\0':
                    INET_close (mysocket);
                    done=true;
                    printf("\n[TERMINATING MUSIC SERVER]\n");
                    break;
            }
        }
    }
    Cleanup(open_file);

    return(0);
}


bool Music::Load(const char * file) {
    if(!SimpleLoad(file)){
        changehome (true,false);
        SimpleLoad(file);
        changehome (false);
        if(m==NULL){
            changehome (true,true);
            SimpleLoad(file);
            changehome(false);
            if (m==NULL) {
                return false;
            }
        }
    }
    return true;
}


void changehome (bool to, bool linuxhome) {
    static std::vector <std::string> paths;
    if (to) {
        char mycurpath[8192];
        getcwd(mycurpath,8191);
        mycurpath[8191]='\0';
        paths.push_back (std::string(mycurpath));
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


int GetMaxVolume () {
#ifdef __APPLE__
    static int maxVol = FSOUND_GetSFXMasterVolume();
    return maxVol;
#else
    return 0;
#endif

}
