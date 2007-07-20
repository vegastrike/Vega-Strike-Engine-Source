#include "vs_globals.h"
#include "configxml.h"
#include "star_system_generic.h"
#include "cmd/unit_generic.h"

VegaConfig * createVegaConfig( char * file)
{
	return new VegaConfig( file);
}
class Music;
class Unit;
class Animation;

void	UpdateAnimatedTexture() {}
void	TerrainCollide() {}
void	UpdateTerrain() {}
void	UpdateCameraSnds() {}
void	NebulaUpdate( StarSystem * ss) {}
void	SwitchUnits2( Unit * nw) {}
void	DoCockpitKeys() {}
void	bootstrap_draw (const std::string &message, Animation * SplashScreen) {}
void createObjects(std::vector <std::string> &playersaveunit, std::vector <StarSystem *> &ssys, std::vector <QVector>& savedloc, vector<vector<string> > &savefiles) {}
void	disableTerrainDraw( ContinuousTerrain *ct) {}
void    /*GFXDRVAPI*/ GFXLight::SetProperties(enum LIGHT_TARGET lighttarg, const GFXColor &color) {}

