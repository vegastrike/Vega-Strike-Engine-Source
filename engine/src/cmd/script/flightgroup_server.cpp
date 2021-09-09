/**
* flightgroup_server.cpp
*
* Copyright (c) 2001-2002 Daniel Horn
* Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
* Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
*
* https://github.com/vegastrike/Vega-Strike-Engine-Source
*
* This file is part of Vega Strike.
*
* Vega Strike is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* Vega Strike is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
*/

#include "mission.h"
#include "flightgroup.h"
#include "cmd/unit_generic.h"
#include <stdio.h>

Flightgroup* Flightgroup::newFlightgroup( const std::string &name,
                                          const std::string &type,
                                          const std::string &faction,
                                          const std::string &order,
                                          int num_ships,
                                          int num_waves,
                                          const std::string &logo_tex,
                                          const std::string &logo_alp,
                                          Mission *mis )
{
    Flightgroup *fg    = mis->findFlightgroup( name, faction );
    Flightgroup *fgtmp = fg;
    if (fg == NULL)
        fg = new Flightgroup;
    fg->Init( fgtmp, name, type, faction, order, num_ships, num_waves, mis );
    if ( !logo_tex.empty() ) {
        if ( logo_alp.empty() )
            fg->squadLogoStr = logo_tex;
        else
            fg->squadLogoStr = logo_alp;
    }
    return fg;
}

Flightgroup&Flightgroup::operator=( Flightgroup &other )
{
    printf( "warning: Flightgroup::operator= may *really* not work properly.\n"
            "In fact, this shouldn't be called at all anyway!\n" );
    return other;
}

Flightgroup::~Flightgroup() {}

