#ifndef __SAVENET_UTIL_H
#define __SAVENET_UTIL_H

#include <string>
#include "networking/client.h"
#include "gfx/cockpit_generic.h"

using std::string;

namespace SaveNetUtil
{
	//void	SaveFiles( Cockpit * cp, string savestr, string xmlstr, string path);
	void	GetSaveBuffer( string savestr, string xmlstr, char * buffer);
	void	GetSaveStrings( int numplayer, string & savestr, string & xmlstr);
	void	GetSaveStrings( Client * clt, string & savestr, string & xmlstr);
}

#endif

