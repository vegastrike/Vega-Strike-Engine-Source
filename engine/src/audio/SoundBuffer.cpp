/**
 * C++ Implementation: Audio::SoundBuffer
 *
 * Copyright (C) Daniel Horn
 * Copyright (C) 2020 pyramid3d, Nachum Barcohen, Stephen G. Tuggy,
 * and other Vega Strike contributors.
 * Copyright (C) 2022 Stephen G. Tuggy
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


#include "SoundBuffer.h"

#include <cstdlib>
#include <memory.h>
#include <utility>

namespace Audio {

SoundBuffer::SoundBuffer()
        : buffer(0),
        byteCapacity(0),
        bytesUsed(0) {
}

SoundBuffer::SoundBuffer(unsigned int capacity, const Format &format)
        : buffer(0),
        byteCapacity(0),
        bytesUsed(0) {
    reserve(capacity, format);
}

SoundBuffer::SoundBuffer(const SoundBuffer &other) {
    bytesUsed = byteCapacity = other.bytesUsed;
    buffer = malloc(byteCapacity);
    if (buffer == 0) {
        throw OutOfMemoryException();
    }
    memcpy(buffer, other.buffer, bytesUsed);
    format = other.format;
}

SoundBuffer &SoundBuffer::operator=(const SoundBuffer &other) {
    bytesUsed = byteCapacity = other.bytesUsed;
    buffer = realloc(buffer, byteCapacity);
    if (buffer == 0) {
        throw OutOfMemoryException();
    }
    memcpy(buffer, other.buffer, bytesUsed);
    format = other.format;

    return *this;
}

void SoundBuffer::reserve(unsigned int capacity) {
    byteCapacity = capacity;
    bytesUsed = 0;

    buffer = realloc(buffer, byteCapacity);
    if (buffer == 0) {
        throw OutOfMemoryException();
    }
}

void SoundBuffer::reserve(unsigned int capacity, const Format &_format) {
    format = _format;
    reserve(capacity * _format.frameSize());
}

void SoundBuffer::reformat(const Format &newFormat) {
    if (newFormat != format) {
        throw (NotImplementedException("Format conversion"));
    }
}

void SoundBuffer::swap(SoundBuffer &other) {
    std::swap(buffer, other.buffer);
    std::swap(byteCapacity, other.byteCapacity);
    std::swap(bytesUsed, other.bytesUsed);
    std::swap(format, other.format);
}

void SoundBuffer::optimize() {
    if (bytesUsed == 0) {
        if (buffer) {
            free(buffer);
            buffer = nullptr;
        }
        bytesUsed = byteCapacity = 0;
    } else {
        if (bytesUsed != byteCapacity) {
            void *newBuffer = realloc(buffer, byteCapacity = bytesUsed);
            if (newBuffer) {
                buffer = newBuffer;
            } else {
                throw OutOfMemoryException();
            }
        }
    }
}

};
