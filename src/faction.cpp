#include "faction.h"
#include "cmd/unit.h"
#include "vs_globals.h"
#include "gfx/aux_texture.h"
#include "gfx/animation.h"
#include "cmd/music.h"

using namespace FactionUtil;

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

GameFaction::~GameFaction() {
  delete logo;
}
