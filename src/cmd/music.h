#ifndef _MUSIC_H_
#define _MUSIC_H_
#include "container.h"
#include <vector>
#include <string>
#include <list>

namespace Muzak {
#ifndef _WIN32
  extern void * readerThread(void* input);
#else
  extern DWORD WINAPI readerThread(PVOID input);
#endif
};

class Music {
  bool random;
  float maxhull;
  int socketr;
  int socketw;
  UnitContainer p;
  int song;//currently playing song
  int lastlist;
  float vol;
  float soft_vol;
  float soft_vol_up_latency;
  float soft_vol_down_latency;
  bool LoadMusic (const char *file);
  struct PlayList {
    std::vector <std::string> songs;
    unsigned int counter;
    PlayList() {counter=0;}
    bool empty()const {return songs.empty();}
    size_t size()const {return songs.size();}
    std::string& operator [](size_t index) {return songs[index];} 
    const std::string& operator [](size_t index) const {return songs[index];} 
    void push_back (std::string s) {songs.push_back(s);}
  };
  std::vector <PlayList> playlist;
 public:
  enum Playlist {NOLIST=-1,BATTLELIST=0,PEACELIST,PANICLIST,VICTORYLIST,LOSSLIST,MAXLIST};
  Music (Unit * parent=NULL);
  int loopsleft;  
  void Listen();
  ~Music();
  void SetParent(Unit * parent);

  void GotoSong (int whichlist,int whichsong,bool skip,int layer=-1);

  static int  Addlist (std::string listfile);
  static void SetLoops(int numloops,int layer=-1);
  static void ChangeVolume(float inc=0,int layer=-1);

  static void Skip(int layer=-1);
  static void Stop(int layer=-1);
  static void SkipRandSong(int whichlist, int layer=-1);
  static void SkipRandList(int layer=-1);

  static void GotoSong (std::string mus, int layer=-1);

  static void InitMuzak();
  static void CleanupMuzak();
  static void MuzakCycle();

  static void SetVolume(float vol, int layer=-1,bool hardware=false,float latency_override=-1);
  static void Mute(bool mute=true, int layer=-1);

private:
  void _GotoSong (std::string mus);
  int  _Addlist (std::string listfile);
  void _SetVolume(float vol=0,bool hardware=false,float latency_override=-1);
  void _SkipRandSong(int whichlist,int layer=-1);
  void _SkipRandList(int layer=-1);
  void _Skip(int layer=-1);
  void _Stop();

  std::list<std::string> recent_songs;

  int SelectTracks(int layer=-1);

#ifndef _WIN32
  friend void * Muzak::readerThread(void* input);
#else
  friend DWORD WINAPI Muzak::readerThread(PVOID input);
#endif

#ifdef _WIN32
  void * a_thread;
#else
  pthread_t a_thread;
#endif

  int thread_initialized;
  volatile int killthread;
  volatile int threadalive;
  volatile int moredata;

  std::string cur_song_file;
};
extern Music *muzak;
extern int muzak_count;


#endif
