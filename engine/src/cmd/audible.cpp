/*
 * audible.cpp
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


#include "audible.h"

#include "gfx/vec.h"
#include "vs_globals.h"
#include "universe.h"
#include "audiolib.h"
#include "options.h"

static const SoundType typesArray[] = {SoundType::engine, SoundType::shield, SoundType::armor,
        SoundType::hull, SoundType::explosion, SoundType::cloaking,
        SoundType::jump};

Audible::Audible() {
    for (const SoundType &soundType : typesArray) {
        sounds[soundType] = -1;
    }

    defaultSoundNames[SoundType::engine] = vs_config->getVariable("unitaudio", "afterburner", "sfx10.wav");
    defaultSoundNames[SoundType::shield] = vs_config->getVariable("unitaudio", "shield", "sfx09.wav");
    defaultSoundNames[SoundType::armor] = vs_config->getVariable("unitaudio", "armor", "sfx08.wav");
    defaultSoundNames[SoundType::hull] = vs_config->getVariable("unitaudio", "armor", "sfx08.wav");
    defaultSoundNames[SoundType::explosion] = vs_config->getVariable("unitaudio", "explode", "explosion.wav");
    defaultSoundNames[SoundType::cloaking] = vs_config->getVariable("unitaudio", "cloak", "sfx43.wav");
}

void Audible::addDefaultSounds() {
    for (const SoundType &soundType : typesArray) {
        if (sounds[soundType] == -1) {
            sounds[soundType] = AUDCreateSound(defaultSoundNames[soundType], false);
        }
    }
}

void Audible::addSounds(std::string (*nextElement)(std::string &), std::string soundsString) {
    bool useDefaultSounds = false;

    for (SoundType type : typesArray) {
        if (!useDefaultSounds) {
            string soundString = nextElement(soundsString);
            if (soundString.length()) {
                sounds[type] = AUDCreateSoundWAV(soundString, false);
            } else {
                useDefaultSounds = true;
            }
        }

        if (useDefaultSounds) {
            sounds[type] = AUDCreateSoundWAV(defaultSoundNames[type], false);
        }
    }
}

void Audible::addSound(string soundString, SoundType type) {
    sounds[type] = AUDCreateSoundWAV(soundString, false);
}

void Audible::adjustSound(SoundType type) {
    Unit *unit = static_cast<Unit *>(this);
    AUDAdjustSound(sounds[type], unit->cumulative_transformation.position, unit->cumulative_velocity);
}

void Audible::adjustSound(SoundType type, const QVector &position, const Vector &velocity) {
    AUDAdjustSound(sounds[type], position, velocity);
}

void Audible::playSound(SoundType type) {
    AUDStartPlaying(sounds[type]);
}

void Audible::playShieldDamageSound(const Vector &pnt) {
    const int shieldSound = sounds[SoundType::shield];
    static int playerShieldSound = AUDCreateSoundWAV(vs_options::instance().player_shield_hit);
    int currentPlayerSound = playerShieldSound != -1 ? playerShieldSound : shieldSound;
    playSound(pnt, shieldSound, currentPlayerSound);
}

void Audible::playArmorDamageSound(const Vector &pnt) {
    const int armorSound = sounds[SoundType::armor];
    static int playerArmorSound = AUDCreateSoundWAV(vs_options::instance().player_armor_hit);
    int currentPlayerSound = playerArmorSound != -1 ? playerArmorSound : armorSound;
    playSound(pnt, armorSound, currentPlayerSound);
}

void Audible::playHullDamageSound(const Vector &pnt) {
    const int hullSound = sounds[SoundType::hull];
    static int playerHullSound = AUDCreateSoundWAV(vs_options::instance().player_hull_hit);
    int currentPlayerSound = playerHullSound != -1 ? playerHullSound : hullSound;
    playSound(pnt, hullSound, currentPlayerSound);
}

void Audible::playEngineSound() {
    Unit *unit = static_cast<Unit *>(this);
    AUDPlay(sounds[SoundType::engine], unit->Position(), unit->GetVelocity(), 1);
}

void Audible::playExplosionDamageSound() {
    playDopplerSound(Vector(), sounds[SoundType::hull]);
}

void Audible::killSounds() {
    for (auto const &pair : sounds) {
        int sound = pair.second;
        if (sound != -1) {
            AUDStopPlaying(sound);
            AUDDeleteSound(sound);
        }
    }
}

void Audible::playSound(const Vector &pnt, int sound, int playerSound) {
    Unit *unit = static_cast<Unit *>(this);

    if (AUDIsPlaying(sound)) {
        AUDStopPlaying(sound);
    }

    if (!_Universe->isPlayerStarship(unit)) {
        if (vs_options::instance().ai_sound) {
            playDopplerSound(pnt, sound);
        }
    } else {
        playDopplerSound(pnt, playerSound);
    }
}

// TODO: this is just a name I gave the method based on the fact it modifies
// the sound based on the velocity and location
void Audible::playDopplerSound(const Vector &pnt, int sound) {
    Unit *unit = static_cast<Unit *>(this);

    QVector position = unit->ToWorldCoordinates(pnt).Cast() +
            unit->cumulative_transformation.position;
    Vector velocity = unit->Velocity;
    const float gain = 1;
    AUDPlay(sound, position, velocity, gain);
}
