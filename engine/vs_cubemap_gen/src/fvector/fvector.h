/**
* fvector.h
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

#ifndef __FVECTOR_H__
#define __FVECTOR_H__

//fvector used to be a class, but it's a struct for speed reasons, to appear as a POD to the compiler
struct fvector
{
    float x_, y_, z_;
    float w_; //not used; just padding for alignment, for now.
    fvector(): x(0.0f), y(0.0f), z(0.0f) {}
    explicit fvector( ccoords const & cc );
    void normalize();
    void fast_renormalize();
};


#endif


