#include <string>
#include "unit_generic.h"
using std::string;

// Wrappers used in unit_xml.cpp
void addShieldMesh(  Unit::XML * xml, const char *filename, const float scale,int faction,class Flightgroup * fg)
{
	xml->shieldmesh_str = string(filename);
	xml->shieldmesh = NULL;
}
void addRapidMesh( Unit::XML * xml, const char *filename, const float scale,int faction,class Flightgroup * fg)
{
	xml->rapidmesh_str = string(filename);
	xml->rapidmesh = NULL;
}
void addBSPMesh( Unit::XML * xml, const char *filename, const float scale,int faction,class Flightgroup * fg)
{
	xml->bspmesh_str = string(filename);
	xml->bspmesh = NULL;
}
void pushMesh( Unit::XML * xml, const char *filename, const float scale,int faction,class Flightgroup * fg)
{
	xml->meshes_str.push_back(string( filename));
	xml->meshes.push_back( NULL);
}

Mount * createMount( const std::string& name, short int ammo=-1, short int volume=-1)
{
	return new Mount (name.c_str(), ammo,volume);
}

Sprite * createSprite(const char *file)
{
	return NULL;
}

int AUDCreateSoundWAV (const std::string &, const bool LOOP=false) {return -1;}
int AUDCreateSoundMP3 (const std::string &, const bool LOOP=false) {return -1;}
int AUDCreateSound (int sound,const bool LOOP=false) {return -1;}
int AUDCreateSound (const std::string &,const bool LOOP=false) {return -1;}

void cache_ani (string s) {
}
void update_ani_cache () {
}
std::string getRandomCachedAniString () {
    return "";
}
Animation* getRandomCachedAni () {
    return NULL;
}

// From communication_xml.cpp
int createSound( string file, bool val)
{
	return -1;
}

