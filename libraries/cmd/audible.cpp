/*
 * audible.cpp
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


#include "cmd/audible.h"

#include "gfx_generic/vec.h"
#include "root_generic/vs_globals.h"
#include "src/universe.h"
#include "src/audiolib.h"
#include "root_generic/options.h"
#include "root_generic/configxml.h"

static const SoundType typesArray[] = {SoundType::engine, SoundType::shield, SoundType::armor,
        SoundType::hull, SoundType::explosion, SoundType::cloaking,
        SoundType::jump};

Audible::Audible() {
    for (const SoundType &soundType : typesArray) {
        sounds[soundType] = -1;
    }

    defaultSoundNames[SoundType::engine] = configuration().audio.unit_audio.afterburner;
    defaultSoundNames[SoundType::shield] = configuration().audio.unit_audio.shield;
    defaultSoundNames[SoundType::armor] = configuration().audio.unit_audio.armor;
    defaultSoundNames[SoundType::hull] = configuration().audio.unit_audio.armor;
    defaultSoundNames[SoundType::explosion] = configuration().audio.unit_audio.explode;
    defaultSoundNames[SoundType::cloaking] = configuration().audio.unit_audio.cloak;
}

Audible::~Audible() = default;

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
    const int shield_sound = sounds[SoundType::shield];
    static boost::optional<int> player_shield_sound{};
    if (player_shield_sound == boost::none) {
        player_shield_sound = AUDCreateSoundWAV(configuration().audio.unit_audio.player_shield_hit);
    }
    int current_player_sound = (player_shield_sound != -1 ? player_shield_sound : shield_sound).get();
    playSound(pnt, shield_sound, current_player_sound);
}

void Audible::playArmorDamageSound(const Vector &pnt) {
    const int armor_sound = sounds[SoundType::armor];
    static boost::optional<int> player_armor_sound{};
    if (player_armor_sound == boost::none) {
        player_armor_sound = AUDCreateSoundWAV(configuration().audio.unit_audio.player_armor_hit);
    }
    int current_player_sound = (player_armor_sound != -1 ? player_armor_sound : armor_sound).get();
    playSound(pnt, armor_sound, current_player_sound);
}

void Audible::playHullDamageSound(const Vector &pnt) {
    const int hull_sound = sounds[SoundType::hull];
    static boost::optional<int> player_hull_sound{};
    if (player_hull_sound == boost::none) {
        player_hull_sound = AUDCreateSoundWAV(configuration().audio.unit_audio.player_hull_hit);
    }
    int current_player_sound = (player_hull_sound != -1 ? player_hull_sound : hull_sound).get();
    playSound(pnt, hull_sound, current_player_sound);
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

    if (!unit->IsPlayerShip()) {
        if (configuration().audio.ai_sound) {
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
