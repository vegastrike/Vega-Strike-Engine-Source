/*
 * OpenALSimpleSound.cpp
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


//
// C++ Implementation: Audio::OpenALSimpleSound
//

#include "OpenALSimpleSound.h"
#include "OpenALHelpers.h"

#include "../../CodecRegistry.h"
#include "../../Stream.h"
#include "al.h"

#ifdef max
#undef max
#endif

#include <utility>
#include <list>
#include <string>

using namespace Audio::__impl::OpenAL;

namespace Audio {

OpenALSimpleSound::OpenALSimpleSound(const std::string &name, VSFileSystem::VSFileType type) :
        SimpleSound(name, type, false),
        bufferHandle(AL_NULL_BUFFER) {
}

OpenALSimpleSound::~OpenALSimpleSound() {
}

void OpenALSimpleSound::loadImpl(bool wait) {
    // just in case
    unloadImpl();

    try {

        flags.loading = 1;

        // load the stream
        try {
            loadStream();
        } catch (const ResourceAlreadyLoadedException &e) {
            // Weird...
            getStream()->seek(0);
        }
        SharedPtr<Stream> stream = getStream();

        // setup formatted buffer
        // if the format does not match an OpenAL built-in format, we must convert it.
        Format targetFormat = stream->getFormat();
        targetFormat.signedSamples = (targetFormat.bitsPerSample > 8);
        targetFormat.nativeOrder = 1;
        if (targetFormat.bitsPerSample > 8) {
            targetFormat.bitsPerSample = 16;
        } else {
            targetFormat.bitsPerSample = 8;
        }

        // Set capacity to half a second or 16k samples, whatever's bigger
        size_t bufferCapacity =
                std::max(16384U, targetFormat.sampleFrequency / 2);

        // Prepare a list of buffers, we'll stack them here and later append them
        std::list<SoundBuffer> buffers;

        try {
            while (true) {
                // Prepare a new buffer
                buffers.push_back(SoundBuffer());
                SoundBuffer &buffer = buffers.back();
                buffer.reserve(bufferCapacity, targetFormat);

                // Fill it in
                readBuffer(buffer);

                // Make sure we're not wasting memory
                buffer.optimize();

                // Break if there's no more data
                if (buffer.getUsedBytes() == 0) {
                    buffers.pop_back();
                    break;
                }
            }
            closeStream();
        } catch (const EndOfStreamException &e) {
            closeStream();
        } catch (const Exception &e) {
            closeStream();
            throw e;
        }

        // Free the stream, asap
        stream.reset();

        // Collapse the chunks into a single buffer
        SoundBuffer buffer;

        if (buffers.size() > 1) {
            // Create a compound buffer with all buffers concatenated
            {
                unsigned int finalBytes = 0;
                for (std::list<SoundBuffer>::const_iterator it = buffers.begin(); it != buffers.end(); ++it) {
                    finalBytes += it->getUsedBytes();
                }
                buffer.reserve(finalBytes);
            }

            {
                char *buf = (char *) buffer.getBuffer();
                for (std::list<SoundBuffer>::const_iterator it = buffers.begin(); it != buffers.end(); ++it) {
                    memcpy(buf, it->getBuffer(), it->getUsedBytes());
                    buf += it->getUsedBytes();
                    buffer.setUsedBytes(buffer.getUsedBytes() + it->getUsedBytes());
                }
            }
        } else if (buffers.size() > 0) {
            buffer.swap(buffers.back());
        } else {
            throw CorruptStreamException(true);
        }

        // Free the buffers, asap
        // The AL will copy to their own buffers, freeing now kind of makes certain
        // the AL will have enough memory to do so
        // (kind of since if memory is allocated off the DSP card, it could still fail)
        buffers.clear();

        // Send the data to the AL
        clearAlError();

        alGenBuffers(1, &bufferHandle);
        checkAlError();

        alBufferData(bufferHandle,
                asALFormat(targetFormat),
                buffer.getBuffer(), buffer.getUsedBytes(),
                targetFormat.sampleFrequency);
        checkAlError();

        onLoaded(true);
    } catch (const Exception &e) {
        onLoaded(false);
        throw e;
    }
}

void OpenALSimpleSound::unloadImpl() {
    if (bufferHandle == AL_NULL_BUFFER) {
        return;
    }

    alDeleteBuffers(1, &bufferHandle);
    bufferHandle = AL_NULL_BUFFER;
}

};
