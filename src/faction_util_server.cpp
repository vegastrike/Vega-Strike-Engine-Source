#include "cmd/unit_generic.h"
#include "faction_generic.h"

Faction::~Faction() {
  delete [] factionname;
  if (contraband) {
    contraband->Kill();
  }
}

std::vector <Animation *>* FactionUtil::GetRandAnimation(int faction, unsigned char &sex) { return NULL;}
Animation * FactionUtil::getRandAnimation (int whichfaction, std::string &which) { return NULL;}
std::vector <Animation *>* FactionUtil::GetAnimation (int faction, int n, unsigned char &sex) { return NULL;}

Animation * FactionUtil::createAnimation( const char * anim) { return NULL;}
Texture * FactionUtil::createTexture( const char * tex, const char * tmp, bool force=false) { return NULL;}
Texture * FactionUtil::createTexture( const char * tex, bool force=false) { return NULL;}
void FactionUtil::LoadFactionPlaylists() {}

Texture * FactionUtil::getForceLogo (int faction) {}
Texture *FactionUtil::getSquadLogo (int faction) {}
int FactionUtil::GetNumAnimation (int faction) {}

