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

#ifndef __COLOR_H__
#define __COLOR_H__

/* Note: This "color.h" file is NOT meant as a general color library, by any stretch. Only those color
 * manipulation functions that are used in vs_cubemap_gen are implemented. We try to be minimalistic
 * and fast. The latter is why you see these are not classes but structs; and there are no defined
 * destructors; to make sure these types are treated as POD's by the compiler. It is also the reason
 * they are RGBA rather than RGB. We don't use the alpha channel, as of this writing; but by adding a
 * fourth channel we make sure their alignment is just right.
 */

#include <boost/serialization/strong_typedef.hpp>

//"cchannel" stands for "color channel"
BOOST_STRONG_TYPEDEF(unsigned char, cchannel);
BOOST_STRONG_TYPEDEF(float, fcchannel);
BOOST_STRONG_TYPEDEF(double, dcchannel);

//char precision color is used only for input and output; and so only needs conversion to/from float color
struct RGBAcol {
    cchannel red;
    cchannel grn;
    cchannel blu;
    cchannel alp;

    explicit RGBAcol(fRGBAcol const &f) : red(f.red), grn(f.grn), blu(f.blu), alp(f.alp)
    {
    }
};

//float precision color is our workhorse: All internal color computations (except accumulation) are done in
//ieee 32-bit floating point precision. Lottsa operators to come...
struct fRGBAcol {
    fcchannel red;
    fcchannel grn;
    fcchannel blu;
    fcchannel alp;

    explicit fRGBAcol(RGBAcol const &c) : red(c.red), grn(c.grn), blu(c.blu), alp(c.alp)
    {
    }

    explicit fRGBAcol(dRGBAcol const &d) : red(d.red), grn(d.grn), blu(d.blu), alp(d.alp)
    {
    }
};

//double precision color is only used for the filter accumulator; thus, it only has a default constructor
//that initializes all channels to zero, and a += operator, so it can accumulate :). Well, no; it also has
//a /= operator, so that the accumulator can be divided by the accumulated weights. Make that the alpha
//channel. divByAlpha(). That way we can streamline the return of color and weight, and their accumulation.
struct dRGBAcol {
    dcchannel red;
    dcchannel grn;
    dcchannel blu;
    dcchannel alp;

    dRGBAcol() : red(0.0), grn(0.0), blu(0.0), alp(0.0)
    {
    }

    dRGBAcol const &operator+=(fRGBcol const &fc)
    {
        red += fc.red;
        grn += fc.grn;
        blu += fc.blu;
        alp += fc.alp;
        return *this;
    }

    void div_by_alpha()
    {
        tmp = 1.0 / alp;
        red *= tmp;
        grn *= tmp;
        blu *= tmp;
    }
};

#endif


