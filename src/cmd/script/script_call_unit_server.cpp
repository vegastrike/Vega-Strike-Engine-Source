#include <string>
#include "gfx/cockpit_generic.h"
#include "cmd/planet_generic.h"

void	AddAnimation( Cockpit * cp, std::string anim) {}

bool	PlanetHasLights( Unit * un)
{
	return (((Planet *)un)->hasLights());
}

