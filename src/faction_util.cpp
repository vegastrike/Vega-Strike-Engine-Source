#include "cmd/unit_generic.h"
#include "faction_generic.h"
#include "gfx/aux_texture.h"
#include <assert.h>

using namespace FactionUtil;

Faction::~Faction() {
  delete [] factionname;
  if (contraband) {
    contraband->Kill();
  }
  delete logo;
}

Texture * FactionUtil::getForceLogo (int faction) {
  return factions[faction]->logo;
}
//fixme--add squads in here
Texture *FactionUtil::getSquadLogo (int faction) {
  if (!factions[faction]->secondaryLogo) {
    return getForceLogo (faction);
  }else {
    return factions[faction]->secondaryLogo;
  }
}

int FactionUtil::GetNumAnimation (int faction) {
  return factions[faction]->comm_faces.size();
}

// COMES FROM FACTION_XML.CPP

std::vector <Animation *>* FactionUtil::GetAnimation (int faction, int n, unsigned char &sex) {
  sex = factions[faction]->comm_face_sex[n];
  return &factions[faction]->comm_faces[n];
}
std::vector <Animation *>* FactionUtil::GetRandAnimation(int faction, unsigned char &sex) {
  if (factions[faction]->comm_faces.size()>0) {
    return GetAnimation ( faction,rand()%factions[faction]->comm_faces.size(),sex);
  }else {
    sex=0;
    return NULL;
  }
}
Animation * FactionUtil::getRandAnimation (int whichfaction, std::string &which) {
  if (whichfaction<(int)factions.size()) {
    if (factions[whichfaction]->explosion_name.size()) {
      int whichexp = rand()%factions[whichfaction]->explosion_name.size();
      which = factions[whichfaction]->explosion_name[whichexp];
      return factions[whichfaction]->explosion[whichexp];
    }
  }
  return NULL;
}

#include "vs_globals.h"
#include "cmd/unit_generic.h"
#include "gfx/aux_texture.h"
#include "gfx/animation.h"
#include "cmd/music.h"

void FactionUtil::LoadFactionPlaylists() {
  for (unsigned int i=0;i<factions.size();i++) {
	string fac=FactionUtil::GetFaction(i);
    fac+=".m3u";
    factions[i]->playlist= muzak->Addlist (fac.c_str());
  }
}

Animation * FactionUtil::createAnimation( const char * anim)
{
	return new Animation( anim);
}

Texture * FactionUtil::createTexture( const char * tex, bool force)
{
	if( force)
		return new Texture( tex, 0,MIPMAP,TEXTURE2D,TEXTURE_2D,GFXTRUE);
	else
		return new Texture( tex, 0,MIPMAP,TEXTURE2D,TEXTURE_2D,GFXFALSE);
}

Texture * FactionUtil::createTexture( const char * tex, const char * tmp, bool force)
{
	if( force)
		return new Texture( tex, tmp, 0,MIPMAP,TEXTURE2D,TEXTURE_2D,1,0,GFXTRUE);
	else
		return new Texture( tex, tmp, 0,MIPMAP,TEXTURE2D,TEXTURE_2D,1,0,GFXFALSE);
}

