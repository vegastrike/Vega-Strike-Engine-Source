#include "unit_generic.h"
#include "gfx/vec.h"
#include "gfx/animation.h"
#include "gfx/cockpit_generic.h"
#include "faction_generic.h"
#include "savegame.h"
#include "xml_support.h"
#include "unit_factory.h"
#include "audiolib.h"
// Various functions that were used in .cpp files that are now included because of
// the temple GameUnit class
// If not separated from those files functions would be defined in multiple places
// Those functions are client specific

// From communication_xml.cpp
int createSound( string file, bool val)
{
	return AUDCreateSoundWAV(file,val);
}

// From unit_damage.cpp
std::vector <Mesh *> MakeMesh(unsigned int mysize) {
  std::vector <Mesh *> temp;
  for (unsigned int i=0;i<mysize;i++) {
    temp.push_back(NULL);
  }
  return temp;
}


// From unit_xml.cpp
using std::map;
static std::map<std::string,Animation *> cached_ani;
vector <std::string> tempcache;
void cache_ani (string s) {
  tempcache.push_back (s);
}
void update_ani_cache () {
  while (tempcache.size()) {
    string explosion_type = tempcache.back();
    tempcache.pop_back();
    if (cached_ani.find (explosion_type)==cached_ani.end()) {
      cached_ani.insert (pair <std::string,Animation *>(explosion_type,new Animation (explosion_type.c_str(),false,.1,BILINEAR,false)));
    }
  }
}
std::string getRandomCachedAniString () {
  if (cached_ani.size()) {
    unsigned int rn = rand()%cached_ani.size();
    map<std::string,Animation *>::iterator j=cached_ani.begin();
    for (unsigned int i=0;i<rn;i++) {
      j++;
    }
    return (*j).first;
  }else{
    return "";
  }  

}
Animation* getRandomCachedAni () {
  if (cached_ani.size()) {
    unsigned int rn = rand()%cached_ani.size();
    map<std::string,Animation *>::iterator j=cached_ani.begin();
    for (unsigned int i=0;i<rn;i++) {
      j++;
    }
    return (*j).second;
  }else{
    return NULL;
  }
}
