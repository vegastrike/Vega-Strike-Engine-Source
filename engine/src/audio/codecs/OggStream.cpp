/*
 * OggStream.cpp
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


//
// C++ Implementation: Audio::OggStream
//


#ifdef HAVE_OGG

#include "OggStream.h"
#include "OggData.h"

#include <utility>
#include <limits>
#include <stdlib.h>

#include <vorbis/vorbisfile.h>
#include "root_generic/vsfilesystem.h"

#ifndef OGG_BUFFER_SIZE
#define OGG_BUFFER_SIZE 4096*2*2
#endif

namespace Audio {

OggStream::OggStream(const std::string &path, VSFileSystem::VSFileType type)
        : Stream(path) {
    if (file.OpenReadOnly(path, type) <= VSFileSystem::Ok) {
        throw FileOpenException("Error opening file \"" + path + "\"");
    }
    oggData = new __impl::OggData(file, getFormatInternal(), 0);

    // Cache duration in case ov_time_total gets expensive
    duration = ov_time_total(&oggData->vorbisFile, oggData->streamIndex);

    // Allocate read buffer
    readBufferSize = OGG_BUFFER_SIZE;
    readBufferAvail = 0;
    readBuffer = malloc(readBufferSize);
}

OggStream::~OggStream() {
    // destructor closes the file already
    delete oggData;
}

double OggStream::getLengthImpl() const {
    return duration;
}

double OggStream::getPositionImpl() const {
    return ov_time_tell(&oggData->vorbisFile);
}

void OggStream::seekImpl(double position) {
    if (position >= duration) {
        throw EndOfStreamException();
    }

    readBufferAvail = 0;

    switch (ov_time_seek(&oggData->vorbisFile, position)) {
        case 0:
            break;
        case OV_ENOSEEK:
            throw Exception("Stream not seekable");
        case OV_EINVAL:
            throw Exception("Invalid argument or state");
        case OV_EREAD:
            throw Exception("Read error");
        case OV_EFAULT:
            throw Exception("Internal logic fault, bug or heap/stack corruption");
        case OV_EBADLINK:
            throw CorruptStreamException(false);
        default:
            throw Exception("Unidentified error code");
    }
}

void OggStream::getBufferImpl(void *&buffer, unsigned int &bufferSize) {
    if (readBufferAvail == 0) {
        throw NoBufferException();
    }

    buffer = readBuffer;
    bufferSize = readBufferAvail;
}

void OggStream::nextBufferImpl() {
    int curStream = oggData->streamIndex;
    long ovr;
    switch (ovr = ov_read(&oggData->vorbisFile,
            (char *) readBuffer, readBufferSize,
            0, 2, 1, &curStream)) {
        case OV_HOLE:
            throw CorruptStreamException(false);
        case OV_EBADLINK:
            throw CorruptStreamException(false);
        case 0:
            throw EndOfStreamException();
        default:
            readBufferSize = ovr;
    }
}

};

#endif // HAVE_OGG

