#include "vs_globals.h"
#include "configxml.h"

void cleanup()
{
	exit(1);
}

VegaConfig * createVegaConfig( char * file)
{
	return new VegaConfig( file);
}
