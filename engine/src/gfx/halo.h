/**
* halo.h
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

#ifndef __HALO_H
#define __HALO_H

#include "gfxlib.h"
#include "vec.h"
#include "quaternion.h"

#define TranslucentWhite ( GFXColor( 1, 1, 1, .5 ) )
#define ZeroQvector ( QVector( 0, 0, 0 ) )

class Halo
{
    QVector position;
    float   sizex;
    float   sizey;
    int     decal;
    int     quadnum;
public: ~Halo();
    Halo( const char *texture,
          const GFXColor &col = TranslucentWhite,
          const QVector &pos = ZeroQvector,
          float sizx = 1,
          float sizy = 1 );
    void Draw( const Transformation &quat = identity_transformation, const Matrix &m = identity_matrix, float alpha = -1 );
    static void ProcessDrawQueue();
    void SetDimensions( float wid, float hei )
    {
        sizex = wid;
        sizey = hei;
    }
    void SetPosition( const QVector &k )
    {
        position = k;
    }
    QVector& Position()
    {
        return position;
    }
    void SetColor( const GFXColor &col );
    void GetDimensions( float &wid, float &hei )
    {
        wid = sizex;
        hei = sizey;
    }
};

#endif

