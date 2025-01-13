/*
 * soundcontainer_generic.h
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
#ifndef VEGA_STRIKE_ENGINE_GFX_SOUND_CONTAINER_GENERIC_H
#define VEGA_STRIKE_ENGINE_GFX_SOUND_CONTAINER_GENERIC_H

#include <string>

#include "SharedPool.h"

/**
 * A reference to a soundfile that can be lazily loaded.
 * 
 * This base class implement no actions on it, see concrete subclasses for that,
 * but it does contain all common data points.
 */
class SoundContainer {
private:
    StringPool::Reference soundFile;
    float gain;
    bool looping;

protected:
    void init(const StringPool::Reference &soundfile, bool looping, float gain);

public:
    /**
     * Create a refernece to an optionally-looping sound file.
     */
    SoundContainer(const SoundContainer &other);
    explicit SoundContainer(const std::string &soundfile, bool looping = false, float gain = 1.0f);
    explicit SoundContainer(const StringPool::Reference &soundfile, bool looping = false, float gain = 1.0f);

    const std::string &getSoundFile() const {
        return soundFile.get();
    }

    const bool isLooping() const {
        return looping;
    }

    const bool getGain() const {
        return gain;
    }

    virtual ~SoundContainer();
};

#endif //VEGA_STRIKE_ENGINE_GFX_SOUND_CONTAINER_GENERIC_H
