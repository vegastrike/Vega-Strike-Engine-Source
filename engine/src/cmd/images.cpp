/**
* images.cpp
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

#include "cmd/images.h"
#include "gfx/cockpit_generic.h"

template < typename BOGUS >
UnitImages<BOGUS>::UnitImages()
{
    VSCONSTRUCT1('i');

    //No cockpit reference here
    if (!cockpit_damage) {
        unsigned int numg = (1+MAXVDUS+UnitImages< void >::NUMGAUGES)*2;
        cockpit_damage = (float*) malloc( (numg)*sizeof (float) );
        for (unsigned int damageiterator = 0; damageiterator < numg; ++damageiterator)
            cockpit_damage[damageiterator] = 1;
    }
}


template < typename BOGUS > //added by chuck starchaser, to try to break dependency to VSSprite in vegaserver
UnitImages<BOGUS>::~UnitImages()
{
    if (pExplosion)
        delete pExplosion;
    if (pHudImage)
        delete pHudImage;
    VSDESTRUCT1
}

//explicit instantiation needed
template struct UnitImages<void>;

static UnitImages<void> bleh;

