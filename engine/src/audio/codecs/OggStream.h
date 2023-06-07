/**
 * OggStream.h
 *
 * Copyright (C) Daniel Horn
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike
 * contributors
 * Copyright (C) 2022-2023 Stephen G. Tuggy, Benjamen R. Meyer
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
#ifndef VEGA_STRIKE_ENGINE_AUDIO_OGG_STREAM_H
#define VEGA_STRIKE_ENGINE_AUDIO_OGG_STREAM_H

//
// C++ Interface: Audio::Codec
//

#ifdef HAVE_OGG

#include "../Stream.h"

#include "vsfilesystem.h"

namespace Audio {

namespace __impl {
struct OggData;
};

/**
 * OggStream class, used by the OggCodec to decode Ogg-vorbis audio streams
 *
 * @remarks Container formats with multiple embedded streams are supported by
 *      using the special path form "[path]|[stream number]". By default, the
 *      first audio stream is opened.
 *
 * @see Stream, CodecRegistry.
 *
 */
class OggStream : public Stream {
private:
    double duration;
    __impl::OggData *oggData;
    VSFileSystem::VSFile file;

    void *readBuffer;
    unsigned int readBufferAvail;
    unsigned int readBufferSize;

public:
    /** Open the specified OGG file, or whine about it
     * @remarks Container formats with multiple embedded streams are supported by
     *      using the special path form "[path]|[stream number]". By default, the
     *      first audio stream is opened.
     */
    OggStream(const std::string &path, VSFileSystem::VSFileType type = VSFileSystem::UnknownFile);

    virtual ~OggStream();

protected:

    /** @see Stream::getLengthImpl */
    virtual double getLengthImpl() const;

    /** @see Stream::getPositionImpl */
    virtual double getPositionImpl() const;

    /** @see Stream::seekImpl */
    virtual void seekImpl(double position);

    /** @see Stream::getBufferImpl */
    virtual void getBufferImpl(void *&buffer, unsigned int &bufferSize);

    /** @see Stream::nextBufferImpl */
    virtual void nextBufferImpl();
};

};

#endif //HAVE_OGG

#endif //VEGA_STRIKE_ENGINE_AUDIO_OGG_STREAM_H
