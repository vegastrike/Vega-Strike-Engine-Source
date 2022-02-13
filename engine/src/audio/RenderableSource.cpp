/*
 * RenderableSource.cpp
 *
 * Copyright (C) Daniel Horn
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike contributors
 * Copyright (C) 2021-2022 Stephen G. Tuggy
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
// C++ Implementation: Audio::RenderableSource
//

#include "RenderableSource.h"
#include "vs_logging.h"

namespace Audio {

RenderableSource::RenderableSource(Source *_source) :
        source(_source) {
}

RenderableSource::~RenderableSource() {
    // Just in case.
    source = 0;
}

void RenderableSource::startPlaying(Timestamp start) {
    try {
        startPlayingImpl(start);
    } catch (const EndOfStreamException &) {
        // Silently discard EOS, results in the more transparent
        // behavior of simply notifying listeners of source
        // termination ASAP, which is also accurate.
    };
}

void RenderableSource::stopPlaying() {
    // Cannot be playing if an exception rises,
    // as that implies a problem with the underlying API
    try {
        if (isPlaying()) {
            stopPlayingImpl();
        }
    } catch (const Exception &e) {
    }
}

bool RenderableSource::isPlaying() const {
    try {
        return isPlayingImpl();
    } catch (const Exception &e) {
        // Cannot be playing if an exception rises,
        // as that implies a problem with the underlying API
        return false;
    }
}

Timestamp RenderableSource::getPlayingTime() const {
    return getPlayingTimeImpl();
}

void RenderableSource::update(int flags, const Listener &sceneListener) {
    try {
        updateImpl(flags, sceneListener);
    } catch (const Exception &e) {
        VS_LOG(warning, (boost::format("Ignoring exception in renderable update: %1%") % e.what()));
    }
}

void RenderableSource::seek(Timestamp time) {
    seekImpl(time);
}

};
