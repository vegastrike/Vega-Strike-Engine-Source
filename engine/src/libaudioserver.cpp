/*
 * Copyright (C) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike contributors.
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
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */

#include <string>
#include "audiolib.h"
#include "cockpit_generic.h"

void AUDAdjustSound(int i, QVector const &qv, Vector const &vv) {
}

bool AUDIsPlaying(int snd) {
    return false;
}

void AUDSoundGain(int snd, float howmuch, bool) {
}

void AUDRefreshSounds() {
}

int AUDCreateSoundWAV(const std::string &, const bool LOOP) {
    return -1;
}

int AUDCreateSoundMP3(const std::string &, const bool LOOP) {
    return -1;
}

int AUDCreateSound(int sound, const bool LOOP) {
    return -1;
}

int AUDCreateSound(const std::string &, const bool LOOP) {
    return -1;
}

void AUDStartPlaying(int i) {
}

void AUDStopPlaying(int i) {
}

void AUDDeleteSound(int i) {
}

void AUDDeleteSound(int i, bool b) {
}

void AUDPlay(const int sound, const QVector &pos, const Vector &vel, const float gain) {
}

QVector AUDListenerLocation() {
    return QVector(0, 0, 0);
}

//From communication_xml.cpp
int createSound(std::string file, bool val) {
    return -1;
}

//soundContainer::~soundContainer () {}

