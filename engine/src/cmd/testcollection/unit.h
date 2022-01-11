/**
* unit.h
*
* Copyright (C) 2001-2002 Daniel Horn
* Copyright (C) 2002-2019 pyramid3d and other Vega Strike Contributors
* Copyright (C) 2019-2022 Stephen G. Tuggy and other Vega Strike Contributors
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

#ifndef __UNIT_TEST_H_
#define __UNIT_TEST_H_
#include <stdio.h>

class Unit
{
public:
    bool killed;
    bool zapped;
    int  ucref;
    Unit( bool kill ) : killed( kill )
    {
        ucref  = 0;
        zapped = false;
    }
    void Kill()
    {
        if (zapped == true)
            printf( "segfault" );
        killed = true;
    }
    bool Killed()
    {
        if (zapped == true)
            printf( "segfault" );
        return killed;
    }

    void Ref()
    {
        if (zapped == true)
            printf( "segfault" );
        ucref += 1;
    }
    void UnRef()
    {
        if (zapped == true)
            printf( "segfault" );
        ucref -= 1;
        if (ucref == 0 && killed)
            zapped = true;
    }
};
#endif

