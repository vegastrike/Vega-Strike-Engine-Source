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
#include "collide/rapcol.h"
#include "unit_collide.h"
#include "mount.h"
// Various functions that were used in .cpp files that are now included because of
// the temple GameUnit class
// If not separated from those files functions would be defined in multiple places
// Those functions are client specific

// Wrappers used in unit_xml.cpp
void addShieldMesh( Unit::XML * xml, const char *filename, const float scale,int faction,class Flightgroup * fg)
{
	xml->shieldmesh = new Mesh(filename, Vector(scale,scale,scale), faction,fg);
}
void addRapidMesh( Unit::XML * xml, const char *filename, const float scale,int faction,class Flightgroup * fg)
{
	xml->rapidmesh = new Mesh(filename, Vector(scale,scale,scale), faction,fg);
}
void addBSPMesh( Unit::XML * xml, const char *filename, const float scale,int faction,class Flightgroup * fg)
{
	xml->bspmesh = new Mesh(filename, Vector(scale,scale,scale), faction,fg);
}
void pushMesh( Unit::XML * xml, const char *filename, const float scale,int faction,class Flightgroup * fg)
{
	xml->meshes.push_back(new Mesh(filename, Vector(scale,scale,scale), faction,fg));
}

Mount * createMount(const std::string& name, short int ammo=-1, short int volume=-1, float xyscale=0, float zscale=0)
{
	return new Mount (name.c_str(), ammo,volume,xyscale, zscale);
}

Sprite * createSprite(const char *file)
{
	return new Sprite (file);
}

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
