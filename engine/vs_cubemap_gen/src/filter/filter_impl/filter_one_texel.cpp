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

#include "../../first.h"
#include "../filter_one_texel.h"

void filter_one_texel::init_constants() {
    cosa_ = ::cosf(max_radius_);
    sina_ = ::sinf(max_radius_);
}

//very important for this function to be inlined; otherwise the branches are not predictable
inline void clip_to_square(float &x, float &y) {
    if (x * x > y * y) {
        //either left or right sides
        if (x > 0.0f) {
            //right side
            y = 0.5f * y / x;
            x = 0.5f;
        } else {
            //left side
            y = -0.5f * y / x;
            x = -0.5f;
        }
    } else {
        //either top or bottom side
        if (y > 0.0f) {
            //top side
            x = 0.5f * x / y;
            y = 0.5f;
        } else {
            //bottom side
            x = -0.5f * x / y;
            y = -0.5f;
        }
    }
}

#define UPD_MIN_X(x) \
    do {               \
    if( (x) < minx )   \
        minx = x;      \
    }                  \
    while(0)
#define UPD_MIN_Y(y) \
    do {               \
    if( (y) < miny )   \
        miny = y;      \
    }                  \
    while(0)
#define UPD_MAX_X(x) \
    do {               \
    if( maxx < (x) )   \
        maxx = x;      \
    }                  \
    while(0)
#define UPD_MAX_Y(y) \
    do {               \
    if( maxy < (y) )   \
        maxy = y;      \
    }                  \
    while(0)

//plane_min_max is given either an xy plane view, or an xz view, or a yz plane view of the ray in the cube; and
//it treats that view as generalized xy plane problem. The problem is to find what range of x or y needs to be
//iterated through for filtering. It creates two vectors, p1 and p2, which are at max_angle_ on either side of
//the x,y (ray) vector. The texture areas to be iterated lie between these two vectors. What follows after is
//a rather ugly mess of switches and conditionals, as cases in which they intersect the same side have to be
//treated differently from cases in which they intersect adjacent, or opposite sides...
//Very important for this function to be inlined; otherwise the branches are not predictable
inline void filter_one_texel::plane_min_max(float len_sqr,
        float x,
        float y,
        float &minx,
        float &maxx,
        float &miny,
        float &maxy) {
    float inv_len = 1.0f / ::sqrtf(len_sqr);
    x *= inv_len;
    y *= inv_len;
    float cosax = cosa_ * x;
    float cosay = cosa_ * y;
    float sinax = sina_ * x;
    float sinay = sina_ * y;
    float p1x = cosax - sinay;
    float p1y = cosay + sinax;
    float p2x = cosax + sinay;
    float p2y = cosay - sinax;
    //now the ugliness begins...
    clip_to_square(p1x, p1y);
    clip_to_square(x, y);
    clip_to_square(p2x, p2y);
    if (x == 0.5f) //ray intersects right side
    {
        if (p1x == 0.5f) //p1 intersects same side
        {
            if (p1y > y)
                UPD_MAX_Y(p1y);
            else
                UPD_MIN_Y(p1y);
        } else //p1 intersects adjacent side
        {
            UPD_MAX_X(0.5f);
            UPD_MIN_X(p1x);
        }
        if (p2x == 0.5f) //p2 intersects same side
        {
            if (p2y > y)
                UPD_MAX_Y(p2y);
            else
                UPD_MIN_Y(p2y);
        } else //p2 intersects adjacent side
        {
            UPD_MAX_X(0.5f);
            UPD_MIN_X(p2x);
        }
    }
    if (y == 0.5f) //ray intersects top side
    {
        if (p1y == 0.5f) //p1 intersects same side
        {
            if (p1x > x)
                UPD_MAX_X(p1x);
            else
                UPD_MIN_X(p1x);
        } else //p1 intersects adjacent side
        {
            UPD_MAX_Y(0.5f);
            UPD_MIN_Y(p1y);
        }
        if (p2y == 0.5f) //p2 intersects same side
        {
            if (p2x > x)
                UPD_MAX_X(p2x);
            else
                UPD_MIN_X(p2x);
        } else //p2 intersects adjacent side
        {
            UPD_MAX_Y(0.5f);
            UPD_MIN_Y(p2y);
        }
    }
    if (x == -0.5f) //ray intersects left side
    {
        if (p1x == -0.5f) //p1 intersects same side
        {
            if (p1y > y)
                UPD_MAX_Y(p1y);
            else
                UPD_MIN_Y(p1y);
        } else //p1 intersects adjacent side
        {
            UPD_MIN_X(-0.5f);
            UPD_MAX_X(p1x);
        }
        if (p2x == -0.5f) //p2 intersects same side
        {
            if (p2y > y)
                UPD_MAX_Y(p2y);
            else
                UPD_MIN_Y(p2y);
        } else //p2 intersects adjacent side
        {
            UPD_MIN_X(-0.5f);
            UPD_MAX_X(p2x);
        }
    }
    if (y == -0.5f) //ray intersects bottom side
    {
        if (p1y == -0.5f) //p1 intersects same side
        {
            if (p1x > x)
                UPD_MAX_X(p1x);
            else
                UPD_MIN_X(p1x);
        } else //p1 intersects adjacent side
        {
            UPD_MIN_Y(-0.5f);
            UPD_MAX_Y(p1y);
        }
        if (p2y == -0.5f) //p2 intersects same side
        {
            if (p2x > x)
                UPD_MAX_X(p2x);
            else
                UPD_MIN_X(p2x);
        } else //p2 intersects adjacent side
        {
            UPD_MIN_Y(-0.5f);
            UPD_MAX_Y(p2y);
        }
    }
}

#undef UPD_MAX_Y
#undef UPD_MAX_X
#undef UPD_MIN_Y
#undef UPD_MIN_X

fRGBAcol filter_one_texel::operator()(fvector const *ray /*normalized*/ ) {
    //FIRST WE NEED TO ESTABLISH ITERATION BOUNDARIES FOR THE SOURCE CUBE
    //If we establish the 2-coord min and max on two planes, that's enough; and the best two planes to
    //do this are those whose ray projections are largest
    float x2 = ray.x_ * ray.x_;
    float y2 = ray.y_ * ray.y_;
    float z2 = ray.z_ * ray.z_;
    float xy_len_sqr = x2 + y2;
    float xz_len_sqr = x2 + z2;
    float yz_len_sqr = y2 + z2;
    float minx, maxx, miny, maxy, minz, maxz;
    minx = miny = minz = 0.5f;
    maxx = maxy = maxz = -0.5f;
    float inv_len = 0.0f;
    //Note: in the conditions below, there's no need to consider equality, if instead of
    //mapping target size-1 to a 90 degree frustrum we map float(size)-0.98f, for example.
    //Maximum of two of if blocks below will execute; perhaps only one.
    if (xy_len_sqr > xz_len_sqr || xy_len_sqr > yz_len_sqr) {
        plane_min_max(xy_len_sqr, ray.x_ * inv_len, ray.y_ * inv_len, minx, maxx, miny, maxy);
    }
    if (xz_len_sqr > xy_len_sqr || xz_len_sqr > yz_len_sqr) {
        plane_min_max(xz_len_sqr, ray.x_ * inv_len, ray.z_ * inv_len, minx, maxx, minz, maxz);
    }
    if (yz_len_sqr > xy_len_sqr || yz_len_sqr > xz_len_sqr) {
        plane_min_max(yz_len_sqr, ray.y_ * inv_len, ray.z_ * inv_len, miny, maxy, minz, maxz);
    }
    //now that we have our three pairs of min and max, we can proceed to generate iteration
    //ranges for each of the six sides
    if (maxx == +0.5f) //--i.e., if positive x side is included in the filter input range
    {
        side_and_coords snc1 = side_and_coords(ccoords(0.5f, miny, minz));
        side_and_coords snc2 = side_and_coords(ccoords(0.5f, maxy, maxz));
        assert(snc1.get_side() == snc2.get_side() || !"weirdness...");
        RectIterRanges
        source_.get_iteration_ranges(snc1, snc2);
    }
    if (maxy == +0.5f) //--i.e., if positive y side is included in the filter input range
    {
    }
    if (maxz == +0.5f) //--i.e., if positive z side is included in the filter input range
    {
    }
    if (minx == -0.5f) //--i.e., if negative x side is included in the filter input range
    {
    }
    if (miny == -0.5f) //--i.e., if negative y side is included in the filter input range
    {
    }
    if (minz == -0.5f) //--i.e., if negative z side is included in the filter input range
    {
    }
}





