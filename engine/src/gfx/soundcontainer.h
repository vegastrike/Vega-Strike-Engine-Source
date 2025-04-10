/**
 * soundcontainer.h
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
 * Copyright (C) 2022-2023 Stephen G. Tuggy, Benjamen R. Meyer
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
#ifndef VEGA_STRIKE_ENGINE_GFX_SOUND_CONTAINER_H
#define VEGA_STRIKE_ENGINE_GFX_SOUND_CONTAINER_H

#include <string>

#include "gfx_generic/soundcontainer_generic.h"

/**
 * A reference to a soundfile that can be lazily loaded.
 *
 * This abstract base class specifies the common interface
 * for all implementations of sound containers, see
 * concrete implementations for more details.
 */
class GameSoundContainer : public SoundContainer {
    bool triggered;
    bool loaded;

public:
    /**
     * Create a refernece to an optionally-looping sound file.
     */
    GameSoundContainer(const SoundContainer &other);

    virtual ~GameSoundContainer();

    /**
     * Forced loading of the resource. It's not necessary to call it,
     * but useful if you know you'll need it later and want to avoid
     * the latency of loading it on-demand.
     */
    void load();

    /**
     * Forced unloading of the resource. It's not necessary to call it,
     * but useful if you know you won't be needing the resource for a while.
     */
    void unload();

    /**
     * Start playing. For a looping source, it starts the loop if not already
     * playing. For a non-looping source, it will start playing from time 0
     * even if it's already playing.
     */
    void play();

    /**
     * Stop playing.
     */
    void stop();

    /**
     * Return whether or not the sound is really playing now.
     */
    bool isPlaying() const;

    /**
     * Return whether or not the sound was triggered with play().
     * Will return true after play(), false after stop() and right
     * after creation.
     */
    bool isTriggered() const {
        return triggered;
    }

    // Concrete classes implement the following:
protected:
    virtual void loadImpl() = 0;
    virtual void unloadImpl() = 0;

    /**
     * Start playing. Needs not account for loopgin vs non-looping,
     * just start playing the sound.
     */
    virtual void playImpl() = 0;

    virtual void stopImpl() = 0;

    /**
     * Return whether or not the sound is really playing now.
     */
    virtual bool isPlayingImpl() const = 0;

};

#endif //VEGA_STRIKE_ENGINE_GFX_SOUND_CONTAINER_H
