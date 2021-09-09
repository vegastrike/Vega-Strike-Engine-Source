/**
* masks.cpp
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

#include "masks.h"

bool checkbit( int &number, int bit_number )
{
    if ( (bit_number > 31) || (bit_number < 0) )
        return 0;
    if ( ( number&(1<<bit_number) ) != 0 )
        return 1;

    else
        return 0;
}

void dosetbit( int &number, int bit_number )
{
    if ( (bit_number > 31) || (bit_number < 0) )
        return;
    number = ( number|(1<<bit_number) );
}

void unsetbit( int &number, int bit_number )
{
    if ( (bit_number > 31) || (bit_number < 0) )
        return;
    int temp = 0;
    if (bit_number != 0)
        temp = ( ( 0xffffffff<<(bit_number+1) )|( 0xffffffff>>(32-bit_number) ) );
    else
        temp = ( 0xffffffff<<(bit_number+1) );
    number = number&temp;
}

void flipbit( int &number, int bit_number )
{
    if ( (bit_number > 31) || (bit_number < 0) )
        return;
    if (checkbit( number, bit_number ) == 1)
        unsetbit( number, bit_number );
    else
        dosetbit( number, bit_number );
}

float Delta( float a, float b )
{
    float ans = a-b;
    if (ans < 0)
        return -1.0*ans;
    else
        return ans;
}

