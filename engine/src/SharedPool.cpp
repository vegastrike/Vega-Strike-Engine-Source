/*
 * SharedPool.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Creator: Daniel Horn
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

#include <string>
#include "SharedPool.h"

template<typename T, typename RT>
SharedPool<T, RT> *SharedPool<T, RT>::ms_singleton = 0;

template<typename T, typename RT>
SharedPool<T, RT>::SharedPool()
#ifdef __GLIBC__
        : referenceCounter(
#if defined (_WIN32) || __GNUC__ != 2
        RT::min_buckets
#endif
)
#endif
{
    if (ms_singleton == 0) {
        ms_singleton = this;
    }
}

template<typename T, typename RT>
SharedPool<T, RT>::~SharedPool() {
    if (ms_singleton == this) {
        ms_singleton = 0;
    }
}

class SharedPoolInitializer {
    StringPool stringPool;
};

static SharedPoolInitializer sharedPoolInitializer;

