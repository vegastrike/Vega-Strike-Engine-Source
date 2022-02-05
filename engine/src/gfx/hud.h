/*
 * hud.h
 *
 * Copyright (C) 2001-2002 Daniel Horn and Alan Shieh
 * Copyright (C) 2002-2019 pyramid3d, ace123, dan_w, jacks, klaussfreire,
 *  and other Vega Strike Contributors
 * Copyright (C) 2019-2021 Stephen G. Tuggy, Roy Falk,
 *  and other Vega Strike Contributors
 * Copyright (C) 2022 Stephen G. Tuggy
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


#ifndef __HUD_H
#define __HUD_H

#include <string>
#include "vec.h"
#include "gfxlib_struct.h"
class Texture;

class TextPlane
{
    std::string myText;

//Texture *myFont;
    Vector myFontMetrics;     //i = width, j = height
    Vector myDims;
    int    numlet;
/*
 *  struct GlyphPosition {
 *       float left, right, top, bottom;
 *  } myGlyphPos[256];
 */
public:
    GFXColor col, bgcol;
    TextPlane( const struct GFXColor &col = GFXColor( 1, 1, 1, 1 ), const struct GFXColor &bgcol = GFXColor( 0, 0, 0, 0 ) );
    ~TextPlane();
    void SetPos( float x, float y )
    {
        myFontMetrics.k = y;
        myDims.k = x;
    }
    void SetCharSize( float x, float y )
    {
        myFontMetrics.i = x;
        myFontMetrics.j = y;
    }
    void GetCharSize( float &x, float &y )
    {
        x = myFontMetrics.i;
        y = myFontMetrics.j;
    }
    void GetPos( float &y, float &x )
    {
        y = myFontMetrics.k;
        x = myDims.k;
    }
    void SetSize( float x, float y )
    {
        myDims.i = x;
        myDims.j = y;
    }
    void GetSize( float &x, float &y )
    {
        x = myDims.i;
        y = myDims.j;
    }
    int Draw( int offset = 0 ); //returns number of lines
    int Draw( const std::string &text,
              int offset = 0,
              bool start_one_line_lower = false,
              bool force_highquality = false,
              bool automatte = false );
    void SetText( const std::string &newText )
    {
        myText = newText;
    }
    std::string GetText() const
    {
        return myText;
    }
};

#endif

