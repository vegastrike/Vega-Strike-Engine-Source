/**
 * audible.h
 *
 * Copyright (C) Daniel Horn
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike
 * contributors
 * Copyright (C) 2022-2023 Stephen G. Tuggy, Benjamen R. Meyer
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
#ifndef VEGA_STRIKE_ENGINE_CMD_AUDIBLE_H
#define VEGA_STRIKE_ENGINE_CMD_AUDIBLE_H

#include "gfx_generic/vec.h" // TODO: replace with class Vector;
#include <map>
#include <string>

enum class SoundType {
    engine, shield, armor, hull, explosion, cloaking, jump
};

class Audible {
private:
    std::map<SoundType, int> sounds;
    std::map<SoundType, std::string> defaultSoundNames;

public:
    Audible();
    void addDefaultSounds();
    void addSounds(std::string (*nextElement)(std::string &), std::string soundsString);
    void addSound(std::string soundString, SoundType type);
    void adjustSound(SoundType type);
    void adjustSound(SoundType type, const QVector &position, const Vector &velocity);
    void playSound(SoundType type);

    void playShieldDamageSound(const Vector &pnt);
    void playArmorDamageSound(const Vector &pnt);
    void playHullDamageSound(const Vector &pnt);
    void playEngineSound();
protected:

    void playExplosionDamageSound();


    // TODO: make into a proper destructor
    void killSounds();

private:
    void playSound(const Vector &pnt, int sound, int playerSound);
    void playDopplerSound(const Vector &pnt, int sound);
};

#endif //VEGA_STRIKE_ENGINE_CMD_AUDIBLE_H
