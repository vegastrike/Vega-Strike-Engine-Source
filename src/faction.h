#ifndef __FACTION_H
#define __FACTION_H

#include "faction_generic.h"
#include "gfx/aux_texture.h"

class GameFaction: public Faction {

  public:
    static void LoadXML(const char * factionfile, char * xmlbuffer=NULL, int buflength=0);
    static void beginElement(void *userData, const XML_Char *name, const XML_Char **atts);
	static void endElement(void *userData, const XML_Char *name);
	GameFaction(): Faction() {}
    ~GameFaction(); //destructor
 };

namespace FactionUtil {
		Animation * createAnimation( const char * anim);
		Texture * createTexture( const char * tex, const char * tmp, bool force=false);
		Texture * createTexture( const char * tex, bool force=false);
		std::vector <class Animation *> * GetAnimation(int faction, int n, unsigned char &sex);
		Animation * getRandAnimation (int whichfaction,std::string&which);
		void LoadFactionPlaylists();
		/** Still in faction_xml.cpp because createUnit **/
		void LoadContrabandLists();

};

#endif
