#ifndef _MUSIC_H_
#define _MUSIC_H_
#include "container.h"
#include <vector>
#include <string>

class Music {
  bool random;
  float maxhull;
  std::vector <std::string> playlist [3];
  UnitContainer p;
  int song;//currently playing song
  int SelectTracks(int &i);
  void LoadMusic (int which, const char *file);
 public:
  Music (Unit * parent);
  void Skip();
  void Listen();
  ~Music();

};


#endif
