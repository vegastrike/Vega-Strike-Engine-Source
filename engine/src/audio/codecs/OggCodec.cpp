/*
 * OggCodec.cpp
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
// C++ Implementation: Audio::OggCodec
//


#ifdef HAVE_OGG

#include "../CodecRegistry.h"

#include "OggCodec.h"
#include "OggStream.h"
#include "OggData.h"

namespace Audio {

OggCodec::OggCodec() : Codec("ogg") {
}

OggCodec::~OggCodec() {
}

const Codec::Extensions *OggCodec::getExtensions() const {
    static Extensions ext;
    if (ext.empty() == 0) {
        ext.push_back(".ogg");
    }
    return &ext;
}

bool OggCodec::canHandle(const std::string &path, bool canOpen, VSFileSystem::VSFileType type) {
    if (canOpen) {
        try {
            VSFileSystem::VSFile file;
            if (file.OpenReadOnly(path, type) <= VSFileSystem::Ok) {
                return false;
            }

            Format fmt;
            __impl::OggData test(file, fmt, 0);

            return true;
        } catch (const Exception &e) {
            return false;
        }
    } else {
        // Can't know for sure
        return true;
    }
}

Stream *OggCodec::open(const std::string &path, VSFileSystem::VSFileType type) {
    return new OggStream(path, type);
}

static CodecRegistration registration(new OggCodec());

};

#endif
