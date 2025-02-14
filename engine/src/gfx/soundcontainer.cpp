/*
 * soundcontainer.cpp
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

#include "soundcontainer.h"

GameSoundContainer::GameSoundContainer(const SoundContainer &other)
        : SoundContainer(other), triggered(false), loaded(false) {
}

GameSoundContainer::~GameSoundContainer() {
    unload();
}

void GameSoundContainer::load() {
    if (!loaded) {
        loadImpl();
        loaded = true;
    }
}

void GameSoundContainer::unload() {
    if (loaded) {
        unloadImpl();
        loaded = false;
    }
}

void GameSoundContainer::play() {
    if (!loaded) {
        load();
    }

    if (!isLooping() || !triggered) {
        playImpl();
        triggered = true;
    }
}

void GameSoundContainer::stop() {
    if (loaded && triggered) {
        stopImpl();
        triggered = false;
    }
}

bool GameSoundContainer::isPlaying() const {
    if (!loaded || !triggered) {
        return false;
    } else {
        return isPlayingImpl();
    }
}
