/**
 * Sound.cpp
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
// C++ Implementation: Audio::Sound
//

#include "Sound.h"

#include "utils.h"

namespace Audio {

Sound::Sound(const std::string &_name, bool streaming) :
        name(_name) {
    flags.loaded = false;
    flags.loading = false;
    flags.streaming = streaming;
}

Sound::~Sound() {
    unload();
}

void Sound::load(bool wait) {
    if (!isLoaded()) {
        if (!isLoading()) {
            loadImpl(wait);
        }
        if (wait && !isLoaded()) {
            waitLoad();
        }
    }
}

void Sound::waitLoad() {
    while (isLoading()) {
        Audio::sleep(10);
    }
}

void Sound::unload() {
    if (isLoading()) {
        abortLoad();
        if (isLoading()) {
            waitLoad();
        }
    }
    if (isLoaded()) {
        unloadImpl();
        flags.loaded = false;
    }
}

void Sound::onLoaded(bool success) {
    flags.loaded = success;
    flags.loading = false;
}

void Sound::abortLoad() {
    // Do nothing, there's no background load
}

};
