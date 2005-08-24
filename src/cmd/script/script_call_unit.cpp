#include <Python.h>
#include <string>

#include "gfx/aux_texture.h"
#include "gfx/cockpit.h"
#include "gfx/animation.h"
#include "cmd/planet.h"

bool	PlanetHasLights( Unit * un)
{
	return (((GamePlanet *)un)->hasLights());
}

