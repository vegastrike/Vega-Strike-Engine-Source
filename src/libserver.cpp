#include "vs_globals.h"
#include "configxml.h"

VegaConfig * createVegaConfig( char * file)
{
	return new VegaConfig( file);
}
