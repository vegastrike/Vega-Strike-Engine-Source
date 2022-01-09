/**
* macosx_math.h
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

#ifndef MACOSX_MATH
#define MACOSX_MATH
/// works on *any* float... INF is 0x7ffffff and -INF is -(1<<31) NaN is 0
extern int float_to_int( float );
extern int double_to_int( float ); //works on *any* float
#if defined (__APPLE__) || defined (MACOSX)
    #include <math.h>
extern float sqrtf( float v );
extern float cosf( float v );
extern float sinf( float v );
extern float tanf( float v );
extern float powf( float v, float p );
#endif

#endif

