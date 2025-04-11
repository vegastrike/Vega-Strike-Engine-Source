/*
 * soundcontainer_generic.cpp
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


#include <string>

#include "gfx_generic/soundcontainer_generic.h"

#include "src/SharedPool.h"

void SoundContainer::init(const StringPool::Reference &_soundfile, bool _looping, float _gain) {
    this->soundFile = _soundfile;
    this->looping = _looping;
    this->gain = _gain;
}

SoundContainer::SoundContainer(const SoundContainer &other) {
    init(other.soundFile, other.looping, other.gain);
}

SoundContainer::SoundContainer(const std::string &_soundfile, bool _looping, float _gain) {
    init(stringPool.get(_soundfile), _looping, _gain);
}

SoundContainer::SoundContainer(const StringPool::Reference &_soundfile, bool _looping, float _gain) {
    init(_soundfile, _looping, _gain);
}

SoundContainer::~SoundContainer() {
}
