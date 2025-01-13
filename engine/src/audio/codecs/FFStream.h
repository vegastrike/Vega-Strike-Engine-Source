/*
 * FFStream.h
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
#ifndef VEGA_STRIKE_ENGINE_AUDIO_FF_STREAM_H
#define VEGA_STRIKE_ENGINE_AUDIO_FF_STREAM_H

//
// C++ Interface: Audio::Codec
//

#ifdef HAVE_FFMPEG

#include "../Stream.h"

#include "vsfilesystem.h"

namespace Audio {

    namespace __impl {
        struct FFData;
    };

    /**
     * FFStream class, used by the FFCodec to decode various audio streams
     *
     * @remarks Container formats with multiple embedded streams are supported by
     *      using the special path form "[path]|[stream number]". By default, the
     *      first audio stream is opened.
     *
     * @see Stream, CodecRegistry.
     *
     */
    class FFStream : public Stream
    {
    private:
        __impl::FFData *ffData;

    public:
        /** Open the specified OGG file, or whine about it
         * @param path the file path
         * @param streamIndex the substream index (for multiple stream containing formats). By
         *      default, the first audio stream is opened.
         * @param type the file type, used by resource management APIs
         */
        FFStream(const std::string& path, int streamIndex = 0, VSFileSystem::VSFileType type = VSFileSystem::UnknownFile);

        virtual ~FFStream();

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

#endif //HAVE_FFMPEG

#endif //VEGA_STRIKE_ENGINE_AUDIO_FF_STREAM_H
