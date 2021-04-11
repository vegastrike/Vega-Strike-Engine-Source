#ifndef _CMD_WEAPON_XML_H_
#define _CMD_WEAPON_XML_H_

#include "weapon_info.h"
#include "gfx/vec.h"

#include <string>


void beginElement( void *userData, const char *name, const char **atts );



void setWeaponInfoToBuffer( weapon_info wi, char *netbuf, int &bufsize );             //WARNING : ALLOCATES A CHAR * BUFFER SO IT MUST BE DELETED AFTER THAT CALL


void LoadWeapons( const char *filename );
weapon_info * getTemplate( const std::string &key );

#endif

