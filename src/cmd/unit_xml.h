#ifndef _UNIT_XML_H
#define _UNIT_XML_H

#include "unit_generic.h"

void addShieldMesh(  Unit::XML * xml, const char *filename, const float scale,int faction,class Flightgroup * fg);
void addRapidMesh( Unit::XML * xml, const char *filename, const float scale,int faction,class Flightgroup * fg);
void addBSPMesh( Unit::XML * xml, const char *filename, const float scale,int faction,class Flightgroup * fg);
void pushMesh( Unit::XML * xml, const char *filename, const float scale,int faction,class Flightgroup * fg, int startframe/*-1 is random frame*/, double texturestarttime);

Mount * createMount( const std::string& name, int ammo, int volume, float xyscale=0, float zscale=0); //short fix
VSSprite * createVSSprite(const char *file);
bool isVSSpriteLoaded(const VSSprite *);
void deleteVSSprite(VSSprite *);

void cache_ani (string s);

/* ADDED FOR extensible use of unit pretty print and unit loading */ 

enum UNITLOADTYPE {DEFAULT,NO_MESH};

extern UNITLOADTYPE current_unit_load_mode;



#endif
