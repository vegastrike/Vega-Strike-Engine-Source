/*
 * faction_util_server.cpp
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

#include "cmd/unit_generic.h"
#include "faction_generic.h"
using namespace FactionUtil;

Faction::~Faction() {
    delete[] factionname;
    if (contraband.get()) {
        contraband->Kill();
    }
}

std::vector<Animation *> *FactionUtil::GetRandCommAnimation(int faction, Unit *, unsigned char &sex) {
    return NULL;
}

std::vector<Animation *> *FactionUtil::GetAnimation(int faction, int n, unsigned char &sex) {
    return NULL;
}

Animation *FactionUtil::createAnimation(const char *anim) {
    return NULL;
}

Texture *FactionUtil::createTexture(const char *tex, const char *tmp, bool force) {
    return NULL;
}

Texture *FactionUtil::createTexture(const char *tex, bool force) {
    return NULL;
}

void FactionUtil::LoadFactionPlaylists() {
}

Texture *FactionUtil::getForceLogo(int faction) {
    return NULL;
}

Texture *FactionUtil::getSquadLogo(int faction) {
    return NULL;
}

int FactionUtil::GetNumAnimation(int faction) {
    return 0;
}

