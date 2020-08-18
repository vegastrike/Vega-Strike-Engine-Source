/*
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn
 *
 * http://vegastrike.sourceforge.net/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#ifndef _GENERICUNIVERSE_H
#define _GENERICUNIVERSE_H

#define NUM_LIGHT 8
#define NUM_FORCES 64

#include <memory>
#include "star_system_generic.h"
#include "faction_generic.h"
#include "stardate.h"
#include "cmd/collection.h"
#include "star_system.h"
#include "universe_globals.h"

class Cockpit;
class Camera;
class Unit;
class Texture;
class StarSystem;

/**
 * Class Universe Deals with universal constants. It is a global,
 * accessed from anywhere as _Universe-> Universe may be queried for
 * Relationships, the current star system rendering is taking place in
 * etc.  It acts as a wrapper to the active Star System.
 * Additionally it handles beginning and ending the main loop.
 * And starting and ending graphics. (incl the task of wiping temp lights)
 * Deprecated: loaded dynamic gldrv module
 */
/*namespace GalaxyXML
{
class Galaxy;
}
class Universe
{
protected:





private:


public:











    Universe();
    Universe( int argc, char **argv, const char *galaxy, bool server = false );
    void Init( const char *gal );










public:

};*/

#endif

