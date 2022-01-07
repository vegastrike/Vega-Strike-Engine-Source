/**
 * SimpleSound.cpp
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
// C++ Implementation: Audio::SimpleSound
//

#include "SimpleSound.h"

#include "CodecRegistry.h"
#include "Stream.h"

namespace Audio {

SimpleSound::SimpleSound(const std::string &name, VSFileSystem::VSFileType _type, bool streaming)
        : Sound(name, streaming),
          type(_type)
{
}

SimpleSound::~SimpleSound()
{
}

void SimpleSound::loadStream()
{
    if (isStreamLoaded()) {
        throw (ResourceAlreadyLoadedException());
    }

    // Open stream and initialize shared pointer
    stream.reset(
            CodecRegistry::getSingleton()->open(
                    getName(),
                    getType()
            )
    );

    // Copy format
    getFormat() = getStream()->getFormat();
}

void SimpleSound::closeStream()
{
    if (!isStreamLoaded()) {
        throw (ResourceNotLoadedException());
    }
    stream.reset();
}

SharedPtr<Stream> SimpleSound::getStream() const
{
    if (!isStreamLoaded()) {
        throw (ResourceNotLoadedException());
    }
    return stream;
}

void SimpleSound::readBuffer(SoundBuffer &buffer)
{
    if (buffer.getFormat() == getFormat()) {
        // Same formats, so all we have to do is read bytes ;)
        buffer.setUsedBytes(
                getStream()->read(buffer.getBuffer(), buffer.getByteCapacity())
        );
    } else {
        // Save the buffer format, we'll have to reformat to this format
        Format targetFormat = buffer.getFormat();

        // Set buffer format to stream format
        buffer.setFormat(getFormat());

        // Now read bytes from the stream
        buffer.setUsedBytes(
                getStream()->read(buffer.getBuffer(), buffer.getByteCapacity())
        );

        // Finally we have to reformat the buffer back to the original format
        buffer.reformat(targetFormat);
    }
}

void SimpleSound::abortLoad()
{
    // Intentionally blank
}

};
