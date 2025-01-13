/*
 * FFCodec.cpp
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
// C++ Implementation: Audio::FFCodec
//


#ifdef HAVE_FFMPEG

#include "../CodecRegistry.h"

#include "FFCodec.h"
#include "FFStream.h"

namespace Audio {

    FFCodec::FFCodec() : Codec("ffmpeg")
    {
    }

    FFCodec::~FFCodec()
    {
    }

    bool FFCodec::canHandle(const std::string& path, bool canOpen, VSFileSystem::VSFileType type)
    {
        if (canOpen) {
            // I don't really know a way to test files in ffmpeg
            // TODO: find out
            return true;
        } else {
            // Can't know for sure
            return true;
        }
    }

    Stream* FFCodec::open(const std::string& path, VSFileSystem::VSFileType type)
    {
        size_t sep = path.find_last_of('|');
        int streamIndex = (sep != std::string::npos) ? atoi(path.c_str() + sep + 1) : 0;
        std::string filepath = path.substr(0, sep);

        return new FFStream(filepath, streamIndex, type);
    }

    static CodecRegistration registration(new FFCodec());

};

#endif
