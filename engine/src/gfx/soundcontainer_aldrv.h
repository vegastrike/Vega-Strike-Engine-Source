/*
 * soundcontainer_aldrv.h
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
#ifndef VEGA_STRIKE_ENGINE_GFX_SOUND_CONTAINER_ALDRV_H
#define VEGA_STRIKE_ENGINE_GFX_SOUND_CONTAINER_ALDRV_H

#include <string>

#include "soundcontainer.h"

/**
 * A reference to a soundfile that can be lazily loaded.
 * 
 * This class implements the operations on sounds using aldrv
 */
class AldrvSoundContainer : public GameSoundContainer {
private:
    int sound;

public:
    /**
     * Create a refernece to an optionally-looping sound file.
     */
    AldrvSoundContainer(const SoundContainer &other);

    virtual ~AldrvSoundContainer();

protected:
    virtual void loadImpl();
    virtual void unloadImpl();
    virtual void playImpl();
    virtual void stopImpl();
    virtual bool isPlayingImpl() const;
};

#endif //VEGA_STRIKE_ENGINE_GFX_SOUND_CONTAINER_ALDRV_H
