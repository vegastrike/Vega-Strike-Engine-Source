#include "faction.h"
#include "cmd/unit_generic.h"

using namespace FactionUtil;

vector <Faction *> factions; //the factions

void FactionUtil::LoadFactionPlaylists() {}

Animation * FactionUtil::createAnimation( const char * anim) {return NULL;}
Texture * FactionUtil::createTexture( const char * tex, const char * tmp, bool force) {return NULL;}
Texture * FactionUtil::createTexture( const char * tex, bool force) {return NULL;}

Faction::~Faction() {
  delete [] factionname;
  if (contraband) {
    contraband->Kill();;
  }
}