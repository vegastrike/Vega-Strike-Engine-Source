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

Mount * createMount( const std::string& name, short int ammo=-1, short int volume=-1, float xyscale=0, float zscale=0)
{
	return new Mount (name.c_str(), ammo,volume, xyscale,zscale);
}

Sprite * createSprite(const char *file)
{
	return NULL;
}

void cache_ani (string s) {}
void update_ani_cache () {}

std::string getRandomCachedAniString () {
    return "";
}
Animation* getRandomCachedAni () {
    return NULL;
}
