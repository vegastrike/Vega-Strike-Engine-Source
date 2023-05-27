/**
 * FFCodec.h
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
#ifndef VEGA_STRIKE_ENGINE_AUDIO_FFCODEC_H
#define VEGA_STRIKE_ENGINE_AUDIO_FFCODEC_H

//
// C++ Interface: Audio::OggCodec
//

#include "Codec.h"

namespace Audio {

/**
 * OggCodec factory class, for Ogg audio streams.
 * @see CodecRegistry to create OggCodec instances.
 */
class FFCodec : public Codec {
public:
    FFCodec();

    virtual ~FFCodec();

    /** @see Codec::canHandle */
    virtual bool canHandle(const std::string &path,
            bool canOpen,
            VSFileSystem::VSFileType type = VSFileSystem::UnknownFile);

    /** @see Codec::open */
    virtual Stream *open(const std::string &path, VSFileSystem::VSFileType type = VSFileSystem::UnknownFile);
};

};

#endif //VEGA_STRIKE_ENGINE_AUDIO_FFCODEC_H
