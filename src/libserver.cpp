#include "vs_globals.h"
#include "configxml.h"
#include "cmd/unit_generic.h"

void cleanup()
{
	exit(1);
}

VegaConfig * createVegaConfig( char * file)
{
	return new VegaConfig( file);
}
class Music;
class Unit;

void	UpdateAnimatedTexture() {}
void	TerrainCollide() {}
void	UpdateTerrain() {}
void	UpdateCameraSnds() {}
void	NebulaUpdate( StarSystem * ss) {}
void	TestMusic() {}
void SwitchUnits2( Unit * nw) {}
void DoCockpitKeys() {}

