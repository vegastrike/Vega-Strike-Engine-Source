/**
 * soundcontainer_aldrv.cpp
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


#include "soundcontainer_aldrv.h"
#include "audiolib.h"

AldrvSoundContainer::AldrvSoundContainer(const SoundContainer &other)
        : GameSoundContainer(other), sound(-2) {
}

AldrvSoundContainer::~AldrvSoundContainer() {
    // unload already takes care
}

void AldrvSoundContainer::loadImpl() {
    sound = AUDCreateSoundWAV(getSoundFile(), isLooping());
    AUDSoundGain(sound, getGain());
}

void AldrvSoundContainer::unloadImpl() {
    if (sound >= 0) {
        stopImpl();
        AUDDeleteSound(sound, false);
        sound = -2;
    }
}

void AldrvSoundContainer::playImpl() {
    AUDStartPlaying(sound);
}

void AldrvSoundContainer::stopImpl() {
    AUDStopPlaying(sound);
}

bool AldrvSoundContainer::isPlayingImpl() const {
    return AUDIsPlaying(sound);
}

