/*
 * Copyright (C) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike contributors.
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
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

#ifndef __FILTER_H__
#define __FILTER_H__

class mem_cubemap;
#include "../units/radians.h"
#include "../units/steradians.h"

class filter_mip {
    mem_tex<S> const &source_;
    mem_tex<S> &target_;
    Radians radius_;
    void init_constants();
    Steradians solid_angle_;
    float shininess_;
    //etceteras...
public:
    virtual ~filter_mip();
    filter
    (
            mem_tex<S>
    const & source
    ,
    mem_tex<S> const &target
    , Radians
    const & radius
    )
    :

    source_ (source)
    , target_(target)
    , radius_(radius)
    {
        init_constants();
    }

    void do_it();
};

#endif


