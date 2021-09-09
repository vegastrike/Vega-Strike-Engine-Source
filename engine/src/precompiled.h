/**
* precompiled.h
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

#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <algorithm>
#include <utility>

#ifdef _WIN32
        #include <hash_map>
#else
        #if __GNUC__ != 2
                #include <ext/hash_map>
        #endif
#endif

#include <boost/smart_ptr.hpp>

#if !defined (_WIN32) && !defined (__CYGWIN__)
        #include <stdlib.h>
#else
        #ifndef NOMINMAX
        #define NOMINMAX
        #endif //tells VCC not to generate min/max macros
        #include <windows.h>
#endif

#define GL_GLEXT_PROTOTYPES 1

#if defined (__APPLE__) || defined (MACOSX)
    #include <OpenGL/gl.h>
    #include <OpenGL/glext.h>
#else
    #include <GL/gl.h>
    #include <GL/glext.h>
#endif

