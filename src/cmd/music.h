#ifndef _MUSIC_H_
#define _MUSIC_H_
#include "container.h"
#include <vector>
#include <string>
class Music {
  std::vector <std::string> playlist;
  UnitContainer p;
  int song;//currently playing song
  int SelectTracks();
 public:
  Music (const char * playlist, Unit * parent);
  void Listen();
  ~Music();

};


#endif
