/*
 * OggData.h
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
// NO HEADER GUARD

//
// C++ implementation: Audio::__impl::OggData
//

#ifdef HAVE_OGG

#include "../Format.h"
#include <vorbis/vorbisfile.h>

#include "vsfilesystem.h"

namespace Audio {

namespace __impl {
struct OggData {
    OggVorbis_File vorbisFile;
    ov_callbacks callbacks;
    int streamIndex;

    static size_t read_func(void *ptr, size_t size, size_t nmemb, void *datasource);
    static int seek_func(void *datasource, ogg_int64_t offset, int whence);
    static int close_func(void *datasource);
    static long tell_func(void *datasource);
    static int nativeIsLsb();

    OggData(VSFileSystem::VSFile &file, Format &fmt, int streamIdx, bool test = false);
    ~OggData();
};
}

};

#endif // HAVE_OGG

