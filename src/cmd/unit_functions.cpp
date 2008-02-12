#include "unit.h"
#include "gfx/vec.h"
#include "gfx/animation.h"
#include "gfx/cockpit_generic.h"
#include "faction_generic.h"
#include "savegame.h"
#include "xml_support.h"
#include "unit_factory.h"
#include "gfx/halo.h"
#include "gfx/mesh.h"
#include "gfx/sphere.h"
#include "gfx/bsp.h"
#include "gfx/sprite.h"
#include "audiolib.h"
#ifndef OPCODE_COLLIDER
#include "collide/rapcol.h"
#else
#include "collide2/CSopcodecollider.h"
#endif
#include "unit_collide.h"
#include <string>
#include <set>

// Various functions that were used in .cpp files that are now included because of
// the temple GameUnit class
// If not separated from those files functions would be defined in multiple places
// Those functions are client specific

// Wrappers used in unit_xml.cpp
VSSprite * createVSSprite(const char *file)
{
  return new VSSprite (file);
}
bool isVSSpriteLoaded(const VSSprite* sprite) {
  return sprite->LoadSuccess();
}
void deleteVSSprite(VSSprite * sprite) {
  delete sprite;
}
// From communication_xml.cpp
int createSound( string file, bool val)
{
	return AUDCreateSoundWAV(file,val);
}

// From unit_xml.cpp
using stdext::hash_map;
static stdext::hash_map<std::string,Animation *> cached_ani;
std::set<std::string> tempcache;
void cache_ani (string s) {
  tempcache.insert(s);
}
void update_ani_cache () {
    for (std::set<std::string>::iterator it=tempcache.begin(); it!=tempcache.end(); it++)
        if (cached_ani.find(*it)==cached_ani.end())
			cached_ani.insert(std::pair <std::string,Animation *>(*it,new Animation ((*it).c_str(),false,.1,BILINEAR,false)));
    tempcache.clear();
}
std::string getRandomCachedAniString () {
  if (cached_ani.size()) {
    unsigned int rn = rand()%cached_ani.size();
	stdext::hash_map<std::string,Animation *>::iterator j=cached_ani.begin();
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
	stdext::hash_map<std::string,Animation *>::iterator j=cached_ani.begin();
    for (unsigned int i=0;i<rn;i++) {
      j++;
    }
    return (*j).second;
  }else{
    return NULL;
  }
}
