/*
 * SimpleSource.cpp
 *
 * Copyright (C) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike contributors
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
// C++ Implementation: Audio::SimpleSource
//

#include "SimpleSource.h"

namespace Audio {

SimpleSource::~SimpleSource() {
}

SimpleSource::SimpleSource(vega_types::SharedPtr<Sound> sound, bool looping) :
        Source(sound, looping),
        playing(false),
        scene(0) {
}

void SimpleSource::notifySceneAttached(SimpleScene *scn) {
    scene = scn;
}

SimpleScene *SimpleSource::getScene() const {
    return scene;
}

void SimpleSource::startPlayingImpl(Timestamp start) {
    // If it's playing, must stop and restart - cannot simply play again.
    if (isPlaying()) {
        stopPlaying();
    }

    setLastKnownPlayingTime(start);
    playing = true;

    if (getScene()) {
        getScene()->notifySourcePlaying(shared_from_this(), true);
    }
}

void SimpleSource::stopPlayingImpl() {
    if (isPlaying()) {
        playing = false;

        if (getScene()) {
            getScene()->notifySourcePlaying(shared_from_this(), false);
        }
    }
}

bool SimpleSource::isPlayingImpl() const {
    return playing;
}

};
