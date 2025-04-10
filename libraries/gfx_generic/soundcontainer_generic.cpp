/**
 * soundcontainer_generic.cpp
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
 * Copyright (C) 2022 Stephen G. Tuggy
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
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
