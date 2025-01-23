/*
 * debug_vs.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
 *
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
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef VEGA_STRIKE_ENGINE_DEBUG_VS_H
#define VEGA_STRIKE_ENGINE_DEBUG_VS_H

//#define VS_DEBUG
//#define VS_DEBUG1
//#define VS_DEBUG2
//#define VS_DEBUG3

#ifndef VS_DEBUG

inline void VS_DEBUG_ERROR() {
}

#define VSCONSTRUCT1(a)
#define VSDESTRUCT1
#define VSCONSTRUCT2(a)
#define VSDESTRUCT2
#define VSCONSTRUCT3(a)
#define VSDESTRUCT3

#else //VS_DEBUG
void VS_DEBUG_ERROR();

#error
#include "hashtable.h"
extern Hashtable< long, char, 65535 >constructed;
extern Hashtable< long, char, 65535 >destructed;
#define VSCONST( a )                                                                  \
    do {                                                                              \
        if (constructed.Get( (long) this ) != NULL) VS_DEBUG_ERROR();                 \
        if (destructed.Get( (long) this ) != NULL) destructed.Delete( (long) this );  \
        if (destructed.Get( (long) this ) != NULL) VS_DEBUG_ERROR();                  \
        constructed.Put( (long) this, (char*) a );                                    \
    }                                                                                 \
    while (0)

#define VSDEST                                                         \
    do {                                                               \
        if (constructed.Get( (long) this ) == NULL) VS_DEBUG_ERROR();  \
        else constructed.Delete( (long) this );                        \
        if (constructed.Get( (long) this ) != NULL) VS_DEBUG_ERROR();  \
        if (destructed.Get( (long) this ) != NULL) VS_DEBUG_ERROR();   \
        destructed.Put( (long) this, (char*) this );                   \
    }                                                                  \
    while (0)

#define VSCONSTRUCT1( a ) VSCONST( a )
#define VSDESTRUCT1 VSDEST

#ifdef VS_DEBUG2
#define VSCONSTRUCT2( a ) VSCONST( a )
#define VSDESTRUCT2 VSDEST

#ifdef VS_DEBUG3
#define VSCONSTRUCT3( a ) VSCONST( a )
#define VSDESTRUCT3 VSDEST
#else //VS_DEBUG3
#define VSCONSTRUCT2( a )
#define VSDESTRUCT2
#define VSCONSTRUCT3( a )
#define VSDESTRUCT3
#endif //VS_DEBUG3

#else //VS_DEBUG2
#define VSCONSTRUCT1( a )
#define VSDESCRUCT1
#define VSCONSTRUCT2( a )
#define VSDESCRUCT2
#define VSCONSTRUCT3( a )
#define VSDESCRUCT3

#endif //VS_DEBUG2

#endif //VS_DEBUG

#endif //VEGA_STRIKE_ENGINE_DEBUG_VS_H
