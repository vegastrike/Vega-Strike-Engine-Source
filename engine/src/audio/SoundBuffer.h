/*
 * SoundBuffer.h
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
#ifndef VEGA_STRIKE_ENGINE_AUDIO_SOUND_BUFFER_H
#define VEGA_STRIKE_ENGINE_AUDIO_SOUND_BUFFER_H

//
// C++ Interface: Audio::SoundBuffer
//

#include "Exceptions.h"
#include "Format.h"

namespace Audio {

/**
 * Sound Buffer class
 *
 * @remarks
 *      This class represents a buffer for sound data.
 *      Though codecs usually treat samples as raw bytes in some uninteresting
 *      (for the api) format, renderers don't have that luxury.
 *      @par This class encapsulates buffers and associates a format to them,
 *      allowing for the implementation of format conversion (should it be
 *      needed).
 *      @par At this point no conversion is supported, and requiring such
 *      a conversion would raise a NotImplementedException.
 */
class SoundBuffer {
private:
    void *buffer;
    unsigned int byteCapacity;
    unsigned int bytesUsed;

    Format format;

public:
    /** Create an empty buffer (zero capacity, default format) */
    SoundBuffer();

    /** Create a buffer of specified sample capacity and format */
    SoundBuffer(unsigned int capacity, const Format &format);

    /** Create a copy of the other buffer
     * @remarks Only used bytes will be copied.
     */
    SoundBuffer(const SoundBuffer &other);

    /** Set a buffer's capacity.
     * @param capacity The buffer's capacity in bytes
     * @remarks Destroys the current data in the buffer.
     */
    void reserve(unsigned int capacity);

    /** Set a buffer's capacity and format.
     * @param capacity The buffer's capacity in samples (or frames) for 'format'
     * @param format The new format associated to the buffer
     * @remarks Destroys the current data in the buffer.
     */
    void reserve(unsigned int capacity, const Format &format);

    /** Get a buffer's byte capacity */
    unsigned int getByteCapacity() const {
        return byteCapacity;
    }

    /** Get a buffer's sample capacity
     * @remarks Frame capacity actually, which is not the same for multichannel formats.
     */
    unsigned int getSampleCapacity() const {
        return byteCapacity / format.frameSize();
    }

    /** Get the portion of the buffer actually used for holding useful data */
    unsigned int getUsedBytes() const {
        return bytesUsed;
    }

    /** Get write access to the buffer */
    void *getBuffer() {
        return buffer;
    }

    /** Get read access to the buffer */
    const void *getBuffer() const {
        return buffer;
    }

    /** Get the buffer's format */
    const Format &getFormat() const {
        return format;
    }

    /** Set the format of the stream mantaining the capacity yet destroying all current data */
    void setFormat(const Format &newFormat) {
        format = newFormat;
        bytesUsed = 0;
    }

    /** Set the portion of the buffer actually used for holding useful data */
    void setUsedBytes(unsigned int used) {
        bytesUsed = used;
    }

    /** Get a buffer's sample capacity for a certain format */
    unsigned int getSampleCount() const {
        return bytesUsed / format.frameSize();
    }

    /** Reformat the samples in the buffer without reallocating if possible (inplace)
     * @remarks If the new format requires more bytes than the buffer's byte capacity,
     *      reallocation will be unavoidable. However, if the same buffer is used
     *      for conversion of several packets, subsequent operations on same-sized
     *      packets will not require such a reallocation, since if the new format
     *      requires less bytes only the used bytes count will be modified leaving
     *      the same byte capacity.
     */
    void reformat(const Format &newFormat);

    /** Copy the given buffer as if SoundBuffer(buffer) was called */
    SoundBuffer &operator=(const SoundBuffer &other);

    /** Swap buffer contents and format
     * It's an inherently quick operation, since it only swaps pointers and descriptors.
     */
    void swap(SoundBuffer &other);

    /** Free extra memory allocated */
    void optimize();
    void clear();
};

};

#endif //VEGA_STRIKE_ENGINE_AUDIO_SOUND_BUFFER_H
