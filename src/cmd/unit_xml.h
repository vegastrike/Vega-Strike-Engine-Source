#ifndef _UNIT_XML_H
#define _UNIT_XML_H

#include "unit_generic.h"

void addShieldMesh(  Unit::XML * xml, const char *filename, const float scale,int faction,class Flightgroup * fg);
void addRapidMesh( Unit::XML * xml, const char *filename, const float scale,int faction,class Flightgroup * fg);
void addBSPMesh( Unit::XML * xml, const char *filename, const float scale,int faction,class Flightgroup * fg);
void pushMesh( Unit::XML * xml, const char *filename, const float scale,int faction,class Flightgroup * fg);

Mount * createMount( const std::string& name, short int ammo=-1, short int volume=-1);
Sprite * createSprite(const char *file);

void cache_ani (string s);

#endif
