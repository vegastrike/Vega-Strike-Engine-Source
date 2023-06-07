/*
 * Copyright (C) 2001-2023 Daniel Horn, pyramid3d, Stephen G. Tuggy, Benjamen R. Meyer,
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
// NO HEADER GUARD

#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <algorithm>
#include <utility>

#ifdef _WIN32
    #include <hash_map>
#else //_WIN32
    #if __GNUC__ != 2
        #include <ext/hash_map>
    #endif //__GNUC__ != 2
#endif //_WIN32

#include <boost/smart_ptr.hpp>

#if !defined (_WIN32) && !defined (__CYGWIN__)
    #include <stdlib.h>
#else //!defined (_WIN32) && !defined (__CYGWIN__)
    #ifndef NOMINMAX
        //tells VCC not to generate min/max macros
        #define NOMINMAX
    #endif //NOMINMAX
    #include <windows.h>
#endif //!defined (_WIN32) && !defined (__CYGWIN__)

#define GL_GLEXT_PROTOTYPES 1

#if defined (__APPLE__) || defined (MACOSX)
    #include <OpenGL/gl.h>
    #include <OpenGL/glext.h>
#else //defined (__APPLE__) || defined (MACOSX)
    #include <GL/gl.h>
    #include <GL/glext.h>
#endif //defined (__APPLE__) || defined (MACOSX)

