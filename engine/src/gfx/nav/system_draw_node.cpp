/*
 * system_draw_node.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file. Specifically:
 * hellcatv, ace123, surfdargent, klaussfreire, jacks, pyramid3d
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

#include "system_draw_node.h"

// Can't include header because drawgalaxy.cpp doesn't have one
extern void DrawNode(int type, float size, float x, float y,
        std::string source, navscreenoccupied *screenoccupation, bool moused,
        GFXColor race, bool mouseover = false, bool willclick = false, std::string insector = "");
extern char GetSystemColor(std::string source);
extern float SYSTEM_DEFAULT_SIZE;
extern GFXColor GrayColor;

SystemDrawNode::SystemDrawNode(int type,
        float size,
        float x,
        float y,
        std::string source,
        unsigned index,
        navscreenoccupied *so,
        bool moused,
        GFXColor race)
        : type(type),
        size(size),
        x(x),
        y(y),
        index(index),
        source(source),
        moused(moused),
        color(GetSystemColor(source)),
        race(race),
        screenoccupation(so) {
}

SystemDrawNode::SystemDrawNode()
        : size(SYSTEM_DEFAULT_SIZE),
        x(0),
        y(0),
        index(0),
        source(),
        moused(false),
        color('v'),
        race(GrayColor),
        screenoccupation(NULL) {
}

void SystemDrawNode::Draw(bool mouseover, bool willclick) {
    DrawNode(type, size, x, y, source, screenoccupation, moused, race, mouseover, willclick);
}

unsigned SystemDrawNode::GetIndex() const {
    return index;
}


bool operator<(const SystemDrawNode &lhs, const SystemDrawNode &rhs) {
    return lhs.source < rhs.source;
}

bool operator==(const SystemDrawNode &lhs, const SystemDrawNode &rhs) {
    return lhs.source == rhs.source;
}