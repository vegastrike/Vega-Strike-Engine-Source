/**
* filter_one_texel.h
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

#ifndef __FILTER_ONE_TEX_H__
#define __FILTER_ONE_TEX_H__

/*
* This class is a functor that is created for each texel of the target texture. It's operator()() takes a ray
* as input and returns a color. Computing this color is not trivial, however: It must iterate trough the source
* texels within a given angular radius of the ray, and accumulate light and "weights". The weight for each
* source texel is a product of its filter angle based weight --Phong: weight = pow( cos(angle), shininess )--,
* and the solid angle of that source texel, as perceived from the center of the cube.
*/

class cube;
#include "../../units/radians.h"
#include "../../units/steradians.h"


class filter_one_texel
{
    mem_tex<S> const & src_tex_;
    Shininess          shininess_;
    void               init_constants();
    Radians            max_radius_;
    float              cosa_;
    float              sina_;
    dRGBAcol           accumulator; //weights accumulated in alpha channel
    void plane_min_max( float len_sqr, float x, float y, float& minx, float& maxx,  float& miny,  float& maxy );
public:
    virtual ~filter_one_texel();
    filter_one_texel
    (
        mem_tex<S> const & source
      , Shininess const & shininess
    )
    : source_(source)
    , shininess_(shininess)
    {
        init_constants();
    }
    fRGBAcol operator()( fvector const * ray );
};


#endif


