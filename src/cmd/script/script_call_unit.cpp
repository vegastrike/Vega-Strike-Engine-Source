#include <string>
#include "gfx/aux_texture.h"
#include "gfx/cockpit.h"
#include "gfx/animation.h"
#include "cmd/planet.h"

void	AddAnimation( Cockpit * cp, std::string anim)
{
	  Hashtable <std::string, Animation, char [63]> AniHashTable;
	  Animation * ani= AniHashTable.Get(anim);
	  if (NULL==ani) {
	    ani = new Animation (anim.c_str());
	    AniHashTable.Put(anim,ani);
	  }
	  cp->SetCommAnimation (ani);
}

bool	PlanetHasLights( Unit * un)
{
	return (((GamePlanet *)un)->hasLights());
}

