#ifndef _MUSIC_H_
#define _MUSIC_H_
#include "container.h"
#include <vector>
#include <string>

class Music {
  bool random;
  float maxhull;
  int socket;
  UnitContainer p;
  int song;//currently playing song
  int lastlist;
  float vol;
  int SelectTracks();
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
  Music (Unit * parent);
  void Skip();
  int loopsleft;  
  int Addlist (std::string listfile);
  void ChangeVolume(float inc=0);
  void GotoSong (int whichlist,int whichsong,bool skip);
  void GotoSong (std::string mus);
  void SkipRandSong(int whichlist);
  void SkipRandList();
  void Listen();
  ~Music();
};
extern Music *muzak;


#endif
