/**
 * OggData.cpp
 *
 * Copyright (C) Daniel Horn
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike
 * contributors
 * Copyright (C) 2022 Stephen G. Tuggy
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


//
// C++ implementation: Audio::__impl::OggData
//


#ifdef HAVE_OGG
#include <sys/types.h>
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#include "OggData.h"
#include "../Exceptions.h"

#include <limits>

#include <vorbis/vorbisfile.h>
#include "vsfilesystem.h"

namespace Audio {

namespace __impl {

using std::numeric_limits;

size_t OggData::read_func(void *ptr, size_t size, size_t nmemb, void *datasource) {
    return ((VSFileSystem::VSFile *) datasource)->Read(ptr, size * nmemb);
}

int OggData::seek_func(void *datasource, ogg_int64_t offset, int whence) {
    if (offset > numeric_limits<long>::max()) {
        return -1;
    }

    VSFileSystem::VSFile *file = (VSFileSystem::VSFile *) datasource;
    switch (whence) {
        case SEEK_SET:
            file->GoTo((long) offset);
            break;
        case SEEK_END:
            file->GoTo(file->Size() + (long) offset);
            break;
        case SEEK_CUR:
            file->GoTo(file->GetPosition() + (long) offset);
            break;
        default:
            return -1;
    }

    return 0;
}

int OggData::close_func(void *datasource) {
    ((VSFileSystem::VSFile *) datasource)->Close();
    return 0;
}

long OggData::tell_func(void *datasource) {
    return ((VSFileSystem::VSFile *) datasource)->GetPosition();
}

int OggData::nativeIsLsb() {
    union {
        short s;
        char c[sizeof(short)];
    };
    s = 1;
    return c[0] ? 1 : 0;
}

OggData::OggData(VSFileSystem::VSFile &file, Format &fmt, int streamIdx, bool test) {
    callbacks.read_func = &read_func;
    callbacks.seek_func = &seek_func;
    callbacks.close_func = &close_func;
    callbacks.tell_func = &tell_func;

    streamIndex = streamIdx;

    if (test) {
        if (ov_test_callbacks(&file, &vorbisFile, NULL, 0, callbacks)) {
            throw FileFormatException("File \"" + file.GetFilename() + "\"is not ogg vorbis");
        }
    } else {
        if (ov_open_callbacks(&file, &vorbisFile, NULL, 0, callbacks)) {
            throw FileFormatException("File \"" + file.GetFilename() + "\"is not ogg vorbis");
        }

        vorbis_info *info = ov_info(&vorbisFile, streamIndex);
        fmt.sampleFrequency = info->rate;
        fmt.channels = info->channels;
        fmt.bitsPerSample = 16;
        fmt.nativeOrder = nativeIsLsb();
        fmt.signedSamples = 1;
    }
}

OggData::~OggData() {
    ov_clear(&vorbisFile);
}
}

};

#endif // HAVE_OGG

