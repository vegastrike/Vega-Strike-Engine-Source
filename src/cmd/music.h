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
  int SelectTracks(int &i);
  void LoadMusic (const char *file);
  std::vector <std::vector <std::string> > playlist;
 public:
  enum Playlist {NOLIST=-1,BATTLELIST=0,PEACELIST,PANICLIST,VICTORYLIST,LOSSLIST,MAXLIST};
  Music (Unit * parent);
  void Skip();
  int Addlist (std::string listfile);
  void GotoSong (int whichlist,int whichsong,bool skip);
  void GotoSong (std::string mus);
  void SkipRandSong(int whichlist);
  void SkipRandList();
  void Listen();
  ~Music();
};


#endif
