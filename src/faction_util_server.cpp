#include "cmd/unit_generic.h"
#include "faction_generic.h"
using namespace FactionUtil;
Faction::~Faction() {
  delete [] factionname;
  if (contraband) {
    contraband->Kill();
  }
}

std::vector <Animation *>* FactionUtil::GetRandCommAnimation(int faction, Unit *, unsigned char &sex) { return NULL;}

std::vector <Animation *>* FactionUtil::GetAnimation (int faction, int n, unsigned char &sex) { return NULL;}

Animation * FactionUtil::createAnimation( const char * anim) { return NULL;}
Texture * FactionUtil::createTexture( const char * tex, const char * tmp, bool force) { return NULL;}
Texture * FactionUtil::createTexture( const char * tex, bool force) { return NULL;}
void FactionUtil::LoadFactionPlaylists() {}

Texture * FactionUtil::getForceLogo (int faction) { return NULL;}
Texture *FactionUtil::getSquadLogo (int faction) { return NULL;}
int FactionUtil::GetNumAnimation (int faction) { return 0;}

