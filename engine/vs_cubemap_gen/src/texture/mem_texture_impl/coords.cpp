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
#include "coords.h"

//explicit instantiations of all 6 sides of an abstract cube:
template side<eLeft> left_side("left");
template side<eRight> left_side("right");
template side<eUp> left_side("up");
template side<eDown> left_side("down");
template side<eFront> left_side("front");
template side<eBack> left_side("back");

ucoords::ucoords(scoords const &sc)
{
    u_ = sc.u_ + 0.5f;
    v_ = sc.v_ + 0.5f;
    vs_assert(u_ >= 0.0f && u_ <= 1.0f, "u coord outside 0 to 1 range");
    vs_assert(v_ >= 0.0f && v_ <= 1.0f, "v coord outside 0 to 1 range");
}

ucoords::ucoords(icoords const &ic, float inv_texture_size)
{
    u_ = (float(ic.u_) + 0.5f) * inv_texture_size;
    v_ = (float(ic.v_) + 0.5f) * inv_texture_size;
    vs_assert(u_ >= 0.0f && u_ <= 1.0f, "u coord outside 0 to 1 range");
    vs_assert(v_ >= 0.0f && v_ <= 1.0f, "v coord outside 0 to 1 range");
}

scoords::scoords(ucoords const &uc)
{
    u_ = sc.u_ - 0.5f;
    v_ = sc.v_ - 0.5f;
    vs_assert(u_ >= -0.5f && u_ <= 0.5f, "u coord outside -0.5 to +0.5 range");
    vs_assert(v_ >= -0.5f && v_ <= 0.5f, "v coord outside -0.5 to +0.5 range");
}

side_and_coords::side_and_coords(ccoords const *cc)
{
    register float X((cc.get_x()))
    register float Y((cc.get_y()))
    register float Z((cc.get_z()))
    register float X2 = X * X;
    register float Y2 = Y * Y;
    register float Z2 = Z * Z;
    eSide s;
    float u;
    float v;
    if (X2 > Y2) {
        if (X2 > Z2) {
            //this is either the left side or the right side
            if (X > 0) {
                s = eLeft;
                //cc = ccoords(  0.5f,    U,    V );
                u = Y;
                v = Z;
            } else {
                s = eRight;
                //cc = ccoords( -0.5f,   -U,    V );
                u = -Y;
                v = Z;
            }
        } else {
            goto front_or_back;
        }
    } else {
        if (Y2 > Z2) {
            //this is either up or down sides
            if (Y > 0) {
                s = eUp;
                //cc = ccoords(    -U,  0.5,   -V );
                u = -X;
                v = -Z;
            } else {
                s = eDown;
                //cc = ccoords(    -U, -0.5,    V );
                u = X;
                v = Z;
            }
        } else {
            front_or_back:
            //this is either the front or the back
            if (Z > 0) {
                s = eFront;
                //cc = ccoords(     U,    V,  0.5 );
                u = X;
                v = Y;
            } else {
                s = eBack;
                //cc = ccoords(     U,   -V, -0.5 );
                u = X;
                v = -Y;
            }
        }
    }
    //snc = side_and_coords( s, scoords( u, v ) );
    //FIXME (FINISH ME)
}

void ccoords::check_invariants() //just for debugging; --should pass if code is correct
{
    vs_assert(x_ >= -0.5 && x_ <= 0.5, "bad range for x_");
    vs_assert(y_ >= -0.5 && y_ <= 0.5, "bad range for y_");
    vs_assert(z_ >= -0.5 && z_ <= 0.5, "bad range for z_");
    float magnitudesqrd = x_ * x_ + y_ * y_ + z_ * z_ );
    vs_assert(magnitudesqrd >= 0.25, "point is inside the cube");
    vs_assert(magnitudesqrd <= 0.75, "point is outside the cube");
}

ccoords::ccoords(fvector const &v)
{
    float inv_max = 1.0f / std::max(std::max(abs(v.x_), abs(v.y_)), abs(v.z_));
    x_ = v.x_ * inv_max;
    y_ = v.y_ * inv_max;
    z_ = v.z_ * inv_max;
}

ccoords::ccoords(side_and_coords const &snc)
{
    eSide S((snc.get_side().get_eSide()))
    float U((snc.get_scoords().get_u()))
    float V((snc.get_scoords().get_v()))
    switch (S) {
        //The numbers and axis flips and whatnot below are COMPLETELY bogus for now.
        //I have *no idea* what the standard mapping of texture coords to cube space
        //coordinates are, or where to find the information.
        case eLeft:
            cc = ccoords(0.5f, U, V);
            break;
        case eRight:
            cc = ccoords(-0.5f, -U, V);
            break;
        case eUp:
            cc = ccoords(-U, 0.5, -V);
            break;
        case eDown:
            cc = ccoords(-U, -0.5, V);
            break;
        case eFront:
            cc = ccoords(U, V, 0.5);
            break;
        case eBack:
            cc = ccoords(U, -V, -0.5);
            break;
        default:
            vs_assert(0, "bad side enum in switch");
            break;
    }
}


