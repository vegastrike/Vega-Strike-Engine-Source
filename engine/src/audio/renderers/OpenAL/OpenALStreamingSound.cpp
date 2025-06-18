/*
 * OpenALStreamingSound.cpp
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
// C++ Implementation: Audio::OpenALSimpleSound
//

#include "OpenALStreamingSound.h"
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

// Handy macro
#define NUM_BUFFERS (sizeof(bufferHandles) / sizeof(bufferHandles[0]))

namespace Audio {

OpenALStreamingSound::OpenALStreamingSound(const std::string &name, VSFileSystem::VSFileType type,
        unsigned int _bufferSamples) :
        SimpleSound(name, type, true),
        bufferSamples(_bufferSamples) {
    for (size_t i = 0; i < NUM_BUFFERS; ++i) {
        bufferHandles[i] = AL_NULL_BUFFER;
    }
}

OpenALStreamingSound::~OpenALStreamingSound() {
}

void OpenALStreamingSound::loadImpl(bool wait) {
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
        targetFormat = stream->getFormat();
        targetFormat.signedSamples = (targetFormat.bitsPerSample > 8);
        targetFormat.nativeOrder = 1;
        if (targetFormat.bitsPerSample > 8) {
            targetFormat.bitsPerSample = 16;
        } else {
            targetFormat.bitsPerSample = 8;
        }

        // Set capacity to a quarter second or 16k samples, whatever's bigger
        // TODO: make it configurable. But first, implement a central configuration repository.
        bufferSamples = std::max(16384U, targetFormat.sampleFrequency / 4);

        // Prepare a buffer, so we avoid repeated allocation/deallocation
        buffer.reserve(bufferSamples, targetFormat);

        // Prepare AL buffers
        clearAlError();
        alGenBuffers(NUM_BUFFERS, bufferHandles);
        checkAlError();

        // Initialize the buffer queue
        flushBuffers();

        onLoaded(true);
    } catch (const Exception &e) {
        onLoaded(false);
        throw e;
    }
}

void OpenALStreamingSound::flushBuffers() {
    // Mark as detached, so that readAndFlip() knows to initialize the source
    // and streaming indices
    readBufferIndex = 0;

    // Mark the playBufferIndex as uninitialized, by setting it to NUM_BUFFERS
    // readAndFlip will initialize it later.
    playBufferIndex = NUM_BUFFERS;
}

void OpenALStreamingSound::unloadImpl() {
    if (isStreamLoaded()) {
        closeStream();
    }
    if (bufferHandles[0] != AL_NULL_BUFFER) {
        alDeleteBuffers(sizeof(bufferHandles) / sizeof(bufferHandles[0]), bufferHandles);
    }
}

ALBufferHandle OpenALStreamingSound::readAndFlip() {
    if (!isLoaded()) {
        throw ResourceNotLoadedException(getName());
    }

    // Check for a full queue
    if (playBufferIndex == readBufferIndex) {
        return AL_NULL_BUFFER;
    }

    bufferStarts[readBufferIndex] = getStream()->getPosition();

    readBuffer(buffer);

    // Break if there's no more data
    if (buffer.getUsedBytes() == 0) {
        throw EndOfStreamException();
    }

    ALBufferHandle bufferHandle = bufferHandles[readBufferIndex];

    clearAlError();
    alBufferData(bufferHandle,
            asALFormat(targetFormat),
            buffer.getBuffer(), buffer.getUsedBytes(),
            targetFormat.sampleFrequency);
    checkAlError();

    if (playBufferIndex == NUM_BUFFERS) {
        playBufferIndex = readBufferIndex;
    }

    readBufferIndex = (readBufferIndex + 1) % NUM_BUFFERS;
    return bufferHandle;
}

void OpenALStreamingSound::unqueueBuffer(ALBufferHandle buffer) {
    if (playBufferIndex < NUM_BUFFERS && buffer == bufferHandles[playBufferIndex]) {
        playBufferIndex = (playBufferIndex + 1) % NUM_BUFFERS;
    }
}

void OpenALStreamingSound::seek(double position) {
    if (!isLoaded()) {
        throw ResourceNotLoadedException(getName());
    }

    getStream()->seek(position);
}

Timestamp OpenALStreamingSound::getTimeBase() const {
    return bufferStarts[playBufferIndex];
}

};
